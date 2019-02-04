/*
 * Remmina - The GTK+ Remote Desktop Client
 *
 * Copyright (C) 2010 Jay Sorg
 * Copyright (C) 2010-2011 Vic Lee
 * Copyright (C) 2011 Marc-Andre Moreau <marcandre.moreau@gmail.com>
 * Copyright (C) 2014-2015 Antenore Gatta, Fabio Castelli, Giovanni Panozzo
 * Copyright (C) 2016-2019 Antenore Gatta, Giovanni Panozzo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 *  In addition, as a special exception, the copyright holders give
 *  permission to link the code of portions of this program with the
 *  OpenSSL library under certain conditions as described in each
 *  individual source file, and distribute linked combinations
 *  including the two.
 *  You must obey the GNU General Public License in all respects
 *  for all of the code used other than OpenSSL. *  If you modify
 *  file(s) with this exception, you may extend this exception to your
 *  version of the file(s), but you are not obligated to do so. *  If you
 *  do not wish to do so, delete this exception statement from your
 *  version. *  If you delete this exception statement from all source
 *  files in the program, then also delete it here.
 *
 */

#include "rdp_plugin.h"
#include "rdp_cliprdr.h"
#include "rdp_event.h"

#include <freerdp/freerdp.h>
#include <freerdp/channels/channels.h>
#include <freerdp/client/cliprdr.h>
#include <winpr/shell.h>
#include <sys/time.h>
#include <pthread.h>
#include <string.h>
#include "rdp_plugin.h"
#include "rdp_cliprdr_download_files.h"


/* Clipboard transfer block size, must be a UINT32 value */
#define REMMINA_CLIP_TRANSFER_BLOCKSIZE_START 0x4000
#define REMMINA_CLIP_TRANSFER_BLOCKSIZE_MAX 0x2000000

enum {
	// https://docs.microsoft.com/it-it/windows/desktop/com/dropeffect-constants
	WIN_DROPEFFECT_NONE = 0,
	WIN_DROPEFFECT_COPY = 1,
	WIN_DROPEFFECT_MOVE = 2,
	WIN_DROPEFFECT_LINK = 4,
	WIN_DROPEFFECT_SCROLL = 0x80000000
};

static char *scolors[] = {
	"\e[31m",
	"\e[32m",
	"\e[33m",
	"\e[34m",
	"\e[35m",
	"\e[36m" };

static char* gettid() {
	static char r[100];	// Not reentrant, remove in production code or you will die! ;)
	pthread_t tid = pthread_self();
	
	sprintf(r, "[%s%u\e[m]", scolors[((unsigned)tid)%6], (unsigned int)tid);
	return r;
	
}

static rfFilePasteTracker* fpt_new()
{
	return g_malloc0(sizeof(rfFilePasteTracker));
}


static void fpt_free(rfFilePasteTracker *fpt)
{
	if (fpt->cancellable) {
		g_cancellable_cancel(fpt->cancellable);
		g_object_unref(fpt->cancellable);
		fpt->cancellable = NULL;
	}

	if (fpt->destdir) {
		g_free(fpt->destdir);
		fpt->destdir = NULL;
	}
}

static void sem_wait(rfFilePasteTracker *fpt, int wait_for_status)
{
	/* Consumer, wait the semaphore to be in wait_for_stauts
	 * wait for at most tmout ms */
	printf("GIO: %s %s wait_for_status=%d\n", gettid(), __func__, wait_for_status);
	pthread_mutex_lock(&fpt->sem_mutex);
	for(;;) {
		if (fpt->tokens[wait_for_status])
			break;
		printf("GIO: %s time=%llu inner loop blocking wait for cond=%d\n", gettid(),
			(long long unsigned int)time(NULL),
			wait_for_status);
		if (pthread_cond_wait(&(fpt->sem_cond[wait_for_status]), &fpt->sem_mutex)) {
			printf("GIO: %s error in pthread_cond_wait(): %s\n", gettid(), strerror(errno));
		}
	}
	fpt->tokens[wait_for_status]--;
	printf("GIO: %s %llu sem is in status %d so we can unlock and continue\n", gettid(),
		(long long unsigned int)time(NULL),
		wait_for_status);
	pthread_mutex_unlock(&fpt->sem_mutex);
}

static void sem_unblock(rfFilePasteTracker *fpt, int status)
{
	/* Producer */
	pthread_mutex_lock(&fpt->sem_mutex);
	printf("GIO: %s signaling cond %d to unlock\n", gettid(),  status);
	if(fpt->tokens[status] != 0) {
		printf("GIO: \e[31minternal error:\e[m token for %d is too high\n", status);
	}
	fpt->tokens[status]++;
	pthread_cond_signal(&fpt->sem_cond[status]);
	pthread_mutex_unlock(&fpt->sem_mutex);
}

static UINT64 elapsedms(struct timeval tstart)
{
	struct timeval now;
	gettimeofday(&now, NULL);
	return (now.tv_sec - tstart.tv_sec) * 1000 + ((now.tv_usec - tstart.tv_usec) / 1000);
}

static gboolean create_directory_if_needed(const char *dir)
{
	/* Create needed directories (this is sync, we should convert to async one day) */
	GFile *d;
	GFileInfo *finfo;
	GFileType t;
	GError *perr;
	d = g_file_new_for_path(dir);

	printf("GIO: %s %s %s\n", gettid(), __func__, dir);
	perr = NULL;
	finfo = g_file_query_info(d, G_FILE_ATTRIBUTE_STANDARD_TYPE,
		G_FILE_QUERY_INFO_NONE, NULL, &perr);
	if (!finfo) {
		if (perr->code == G_IO_ERROR_NOT_FOUND)
			goto createdir;
		printf("Unable to get file info for %s\n", dir);
		return FALSE;
	}
	t = g_file_info_get_file_type(finfo);
	g_object_unref(finfo);
	if (t != G_FILE_TYPE_DIRECTORY) {
		printf("Error: %s is not a directory\n", dir);
		return FALSE;
	}

	return TRUE;	// It's a directory

createdir:
	perr = NULL;
	if (!g_file_make_directory(d, NULL, &perr)) {
		printf("Unable to create direcotry %s\n", dir);
		g_object_unref(d);
		return FALSE;
	}
	g_object_unref(d);
	return TRUE;
}

static gboolean create_pasted_file_paths(char *destdir, WCHAR *winsubpathW, char **pasted_filename, char **pasted_filename_part)
{
	gchar *fn;
	char *s, *d, c;
	gsize fnsz;
	gchar *winsubpath;

	*pasted_filename = NULL;
	*pasted_filename_part = NULL;

	if (destdir == NULL || strlen(destdir) == 0)
		return FALSE;

	ConvertFromUnicode(CP_UTF8, 0, winsubpathW, _wcslen(winsubpathW) + 1, (CHAR**)&winsubpath, 0, NULL, NULL);

	if (strlen(winsubpath) == 0) {
		free(winsubpath);
		return FALSE;
	}

	/* Allocate the full space for topdir/winsubpath and terminating zero + ".part" */
	fnsz = strlen(destdir) + strlen(winsubpath) + 7;
	fn = g_malloc(fnsz);
	g_strlcpy(fn, destdir, fnsz);

	if (fn[strlen(fn)-1] != '/')
		g_strlcat(fn, "/", fnsz);

	/* Concatenate winsubpath, changing \ into / and stripping unallowed chars */
	s = winsubpath;
	d = fn + strlen(fn);
	if (!create_directory_if_needed(fn))
		goto err;
	while((c = *s++) != 0) {
		if (c == '\\') {
			*d++ = '/';
			*d = 0;
			if (!create_directory_if_needed(fn))
				goto err;
		} else
			*d++ = c;
	}
	*d++=0;

	*pasted_filename = g_strdup(fn);
	g_strlcat(fn,".part", fnsz);
	*pasted_filename_part = g_strdup(fn);

	free(winsubpath);
	return TRUE;

err:
	free(winsubpath);
	g_free(*pasted_filename);
	*pasted_filename = NULL;
	g_free(*pasted_filename_part);
	*pasted_filename_part = NULL;
	return FALSE;

}


UINT rdp_cliprdr_download_server_format_data_response(CliprdrClientContext* context, const CLIPRDR_FORMAT_DATA_RESPONSE* formatDataResponse)
{
	/* ServerFormatDataResponse() callback, it's called on the freerdp thread,
	 * not in our thread, when the server has an answer for ClientFormatDataRequest().
	 * Signal srv_data_request_wait() to unlock */

	rfClipboard* clipboard;
	clipboard = (rfClipboard*)context->custom;

	printf("GIO: %s %s\n", gettid(), __func__);

	/* Pass formatDataResponse to download thread */
	clipboard->fpt->formatDataResponse = formatDataResponse;
	printf("    GIO: len=%u\n", (unsigned)formatDataResponse->dataLen);
	/* Unlock download thread */
	printf("GIO: %s %s sem_unblock to SEM_FORMAT_DATA_RESPONSE_ARRIVED\n", gettid(), __func__);
	sem_unblock(clipboard->fpt, SEM_FORMAT_DATA_RESPONSE_ARRIVED);

	/* Lock freerdp thread until we have an answer. This
	 * is needed to keep formatDataResponse allocated */
	printf("GIO: %s %s blocked waiting SEM_FORMAT_DATA_RESPONSE_RC_READY\n",gettid(), __func__);
	sem_wait(clipboard->fpt, SEM_FORMAT_DATA_RESPONSE_RC_READY);
	printf("GIO: %s %s can continue due to sem=SEM_FORMAT_DATA_RESPONSE_RC_READY\n",gettid(), __func__);

	return clipboard->fpt->formatDataResponse_result;

}

static void srv_format_data_response_answer(rfClipboard* clipboard, UINT rc)
{
	clipboard->fpt->formatDataResponse_result = rc;
	printf("GIO: %s %s setting semaphore to SEM_FORMAT_DATA_RESPONSE_RC_READY\n", gettid(), __func__);
	sem_unblock(clipboard->fpt, SEM_FORMAT_DATA_RESPONSE_RC_READY);
	/* Allow libfreerdp thread to wake up and send response.
	 * No better ideas here */
	// usleep(1000);
}

static void srv_format_data_request_start(rfClipboard* clipboard, UINT32 format)
{
	CLIPRDR_FORMAT_DATA_REQUEST* pFormatDataRequest;
	RemminaPluginRdpEvent rdp_event = { 0 };
	RemminaProtocolWidget *gp;

	gp = clipboard->rfi->protocol_widget;
	clipboard->format = format;	// Not really useful

	/* Queue a call to ClientFormatDataRequest() on the next libfreerdp
	 * eventloop cycle */

	pFormatDataRequest = (CLIPRDR_FORMAT_DATA_REQUEST*)malloc(sizeof(CLIPRDR_FORMAT_DATA_REQUEST));
	ZeroMemory(pFormatDataRequest, sizeof(CLIPRDR_FORMAT_DATA_REQUEST));
	pFormatDataRequest->requestedFormatId = format;
	rdp_event.type = REMMINA_RDP_EVENT_TYPE_CLIPBOARD_SEND_CLIENT_FORMAT_DATA_REQUEST;
	rdp_event.clipboard_formatdatarequest.pFormatDataRequest = pFormatDataRequest;
	remmina_rdp_event_event_push(gp, &rdp_event);

	/* ClientFormatDataRequest() will be called by libfreerdp thread,
	 * now we wait for result in ServerFormatDataResponse callback:
	 * block until arrival */
	printf("GIO: %s %s waiting for SEM_FORMAT_DATA_RESPONSE_ARRIVED\n", gettid(), __func__);
	sem_wait(clipboard->fpt, SEM_FORMAT_DATA_RESPONSE_ARRIVED);
	printf("GIO: %s %s can return due to arrival of SEM_FORMAT_DATA_RESPONSE_ARRIVED\n", gettid(), __func__);

}

UINT rdp_cliprdr_download_server_file_contents_response(CliprdrClientContext* context, const CLIPRDR_FILE_CONTENTS_RESPONSE* fileContentsResponse)
{
	/* ServerFileContentsResponse() callback, it's called on the freerdp thread,
	 * not in our thread, when the server has an answer for ClientFileContentsRequest().
	 * Signal srv_data_request_wait() to unlock */

	rfClipboard* clipboard;
	clipboard = (rfClipboard*)context->custom;

	/* Pass fileContentsResponse to download thread */
	clipboard->fpt->fileContentsResponse = fileContentsResponse;
	/* Unlock download thread */
	sem_unblock(clipboard->fpt, SEM_FILE_CONTENTS_REQUEST_ARRIVED);

	/* Lock freerdp thread until we have an answer. This
	 * is needed to keep fileContentsResponse allocated */
	sem_wait(clipboard->fpt, SEM_FILE_CONTENTS_REQUEST_RC_READY);

	return clipboard->fpt->fileContentsResponse_result;
}

static void srv_file_contents_response_answer(rfClipboard* clipboard, UINT rc)
{
	printf("GIO: %s %s\n", gettid(), __func__);
	clipboard->fpt->fileContentsResponse_result = rc;
	sem_unblock(clipboard->fpt, SEM_FILE_CONTENTS_REQUEST_RC_READY);
	/* Allow libfreerdp thread to wake up and send response.
	 * No better ideas here */
	// usleep(1000);

}

static void srv_client_file_contents_request_start(rfClipboard* clipboard,
                                       UINT32 streamid,
                                       ULONG index, UINT32 flag, DWORD positionhigh,
                                       DWORD positionlow, ULONG nreq)
{
	CLIPRDR_FILE_CONTENTS_REQUEST *pFileContentsRequest;
	RemminaPluginRdpEvent rdp_event = { 0 };
	RemminaProtocolWidget *gp;

	gp = clipboard->rfi->protocol_widget;

	pFileContentsRequest = (CLIPRDR_FILE_CONTENTS_REQUEST*)malloc(sizeof(CLIPRDR_FILE_CONTENTS_REQUEST));
	ZeroMemory(pFileContentsRequest, sizeof(CLIPRDR_FILE_CONTENTS_REQUEST));
	pFileContentsRequest->streamId = streamid;
	pFileContentsRequest->listIndex = index;
	pFileContentsRequest->dwFlags = flag;
	pFileContentsRequest->nPositionLow = positionlow;
	pFileContentsRequest->nPositionHigh = positionhigh;
	pFileContentsRequest->cbRequested = nreq;
	pFileContentsRequest->haveClipDataId = FALSE;
	pFileContentsRequest->clipDataId = 0;
	pFileContentsRequest->msgFlags = 0;

	printf("GIO: %s invio ClientFileContentsRequest per file index=%u size=%u\n", gettid(), (unsigned)index, (unsigned)nreq);

	rdp_event.type = REMMINA_RDP_EVENT_TYPE_CLIPBOARD_SEND_CLIENT_FILE_CONTENTS_REQUEST;
	rdp_event.clipboard_filecontentsrequest.pFileContentsRequest = pFileContentsRequest;
	remmina_rdp_event_event_push(gp, &rdp_event);

	/* ClientFileContentsRequest() will be called by libfreerdp thread,
	 * now we wait for result in ServerFileContentsResponse() callback:
	 * block until arrival */
	printf("GIO: %s %s waiting for SEM_FILE_CONTENTS_REQUEST_ARRIVED\n", gettid(), __func__);
	sem_wait(clipboard->fpt, SEM_FILE_CONTENTS_REQUEST_ARRIVED);
	printf("GIO: %s %s can return due to arrival of SEM_FILE_CONTENTS_REQUEST_ARRIVED\n", gettid(), __func__);

}

static void *rdp_cliprdr_download_thread(void *ptr)
{
	rfClipboard* clipboard;
	FILEGROUPDESCRIPTOR *pFileGroupDescriptor;
	FILEDESCRIPTOR *pfd;
	const CLIPRDR_FILE_CONTENTS_RESPONSE* fileContentsResponse;
	int nFile;
	UINT32 preferred_dropeffect;
	UINT64 total_file_size, total_transferred_size, filesize, filepointer, blksz;
	UINT64 tms, trate;
	char *fn, *fnpart;
	struct timeval fblock_st;
	UINT32 adjusted_fblock_size;
	GFile *ofile, *fdest;
	GFileInfo *finfo;
	GFileOutputStream* os;
	GError *err;
	gsize written, sz;
	int percent, i;
	gboolean rcwr, rcc;
	char *utf8_fname;

	clipboard = (rfClipboard*)ptr;

	printf("GIO: %s this is the main download filethread\n", gettid());

	/* The clipboard is busy and we are using it, no other operations can occur */
	clipboard->srv_clip_data_wait = SCDW_DOWNLOADTHREAD;


	pthread_mutex_init(&clipboard->fpt->sem_mutex, NULL);
	for(i = 0; i < SEM_MAX; i ++) {
		pthread_cond_init(&clipboard->fpt->sem_cond[i], NULL);
		clipboard->fpt->tokens[i] = 0;
	}
	clipboard->fpt->cancellable = g_cancellable_new();


	/* Start requesting file data from the server. */

	/* The 1st request is for the preferred dropeffect via ClientFormatDataRequest() */
	printf("GIO #1 %s requesting to server the preferred drop effect, ID is %u\n",gettid(), (unsigned)clipboard->preferred_dropeffect_id);
	srv_format_data_request_start(clipboard, clipboard->preferred_dropeffect_id);
	printf("GIO #2 %s\n", gettid());
	if (clipboard->fpt->formatDataResponse->dataLen == 4) {
		preferred_dropeffect = *((UINT32*)clipboard->fpt->formatDataResponse->requestedFormatData);
	} else {
		printf("GIO: asked for drop effect, but something different arrived. Len is %llu\n",
			(long long unsigned int)clipboard->fpt->formatDataResponse->dataLen);
		preferred_dropeffect = WIN_DROPEFFECT_COPY;
	}
	srv_format_data_response_answer(clipboard, CHANNEL_RC_OK);

	printf("GIO: %s preferred_dropeffect = %u\n", gettid(), (unsigned)preferred_dropeffect);

	/* The 2nd request is for the file list (FILEGROUPDESCRIPTOR block)  via ClientFormatDataRequest() */
	srv_format_data_request_start(clipboard, clipboard->filegroupdescriptorw_id);
	pFileGroupDescriptor = malloc(clipboard->fpt->formatDataResponse->dataLen);
	memcpy(pFileGroupDescriptor, clipboard->fpt->formatDataResponse->requestedFormatData, clipboard->fpt->formatDataResponse->dataLen);
	srv_format_data_response_answer(clipboard, CHANNEL_RC_OK);

	printf("GIO: I have a filegroupdescriptorw from the server, with %u files\n", (unsigned)pFileGroupDescriptor->cItems);
	total_file_size = 0;
	for(nFile = 0; nFile < pFileGroupDescriptor->cItems; nFile++) {
		pfd = &pFileGroupDescriptor->fgd[nFile];
		// https://docs.microsoft.com/en-us/windows/desktop/api/shlobj_core/ns-shlobj_core-_filedescriptora
		gchar *utf8_fname;
		ConvertFromUnicode(CP_UTF8, 0, (WCHAR*)pfd->cFileName, _wcslen((WCHAR*)pfd->cFileName) + 1, (CHAR**)&utf8_fname, 0, NULL, NULL);
		printf("\t%s\n", utf8_fname);
		free(utf8_fname);
		total_file_size += (((UINT64)pfd->nFileSizeHigh) << 32) + pfd->nFileSizeLow;
	}

	adjusted_fblock_size = REMMINA_CLIP_TRANSFER_BLOCKSIZE_START;
	total_transferred_size = 0;
	for(nFile = 0; nFile < pFileGroupDescriptor->cItems; nFile++) {
		pfd = &pFileGroupDescriptor->fgd[nFile];
		/* Skip directory */
		if ((pfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
			continue;
		/* Print informations for this file */
		ConvertFromUnicode(CP_UTF8, 0, (WCHAR*)pfd->cFileName, _wcslen((WCHAR*)pfd->cFileName) + 1, (CHAR**)&utf8_fname, 0, NULL, NULL);
		printf("GIO: %s starting transfer of %llu bytes for %s. Requesting stream FILECONTENTS_SIZE...\n",
					gettid(),
					(((unsigned long long)pfd->nFileSizeHigh) << 32) + pfd->nFileSizeLow,
					utf8_fname);
		free(utf8_fname);
		/* Read file size again from server */
		srv_client_file_contents_request_start(clipboard, 0, nFile, FILECONTENTS_SIZE, 0, 0, 8);
		fileContentsResponse = clipboard->fpt->fileContentsResponse;
		printf("GIO: %s we have a file size from the server:\n", gettid());
		printf("  streamID=%u\n", (unsigned)fileContentsResponse->streamId);
		filesize = *((UINT64 *)fileContentsResponse->requestedData);
		printf("  size=%llu\n", (long long unsigned int)filesize);
		srv_file_contents_response_answer(clipboard, CHANNEL_RC_OK);

		/* Create needed directories */
		if (!create_pasted_file_paths(clipboard->fpt->destdir, pfd->cFileName, &fn, &fnpart)) {
			printf("GIO: unable to create destination directories.\n");
			continue;
		}

		/* Create new output file */
		ofile = g_file_new_for_path(fnpart);
		g_file_delete(ofile, clipboard->fpt->cancellable, NULL);
		err = NULL;
		os = g_file_create(ofile, 0,
			clipboard->fpt->cancellable, &err);
		if (!os) {
			printf("GIO: unable to create file %s: %s\n", fnpart, err->message);
			free(fn);
			free(fnpart);
			g_error_free (err);
			continue;
		}

		/* Start downloading file */
		filepointer = 0;
		rcwr = TRUE;
		while(filepointer < filesize) {

			if (clipboard->fpt->abort_requested) {
				goto abort;
			}

			gettimeofday(&fblock_st, NULL);
			blksz = filesize - filepointer;
			if (blksz >= adjusted_fblock_size)
				blksz = adjusted_fblock_size;
			srv_client_file_contents_request_start(clipboard, 0, nFile, FILECONTENTS_RANGE, filepointer >> 32, filepointer & 0xFFFFFFFF, (UINT32)blksz);
			/* We have a block !*/
			fileContentsResponse = clipboard->fpt->fileContentsResponse;
			printf("   %s we requested %llu bytes and we have a block of %llu bytes\n", gettid(), (long long unsigned int)blksz, (long long unsigned int)fileContentsResponse->cbRequested);
			if (fileContentsResponse->cbRequested == 0)
				goto berror;
			tms = elapsedms(fblock_st);
			trate = fileContentsResponse->cbRequested / tms;	// T is in ms, bytes/10e-3s = bytes *10e3/s = KB/s
			printf("   transfer speed is %llu KB/sec\n", (long long unsigned int)trate);
			percent = (int)((filepointer + fileContentsResponse->cbRequested) * 100 / filesize);
			printf("   progress %llu of %llu: %d%%\n", (long long unsigned int)filepointer + fileContentsResponse->cbRequested, (long long unsigned int)filesize ,percent);
			percent = (int)((total_transferred_size + fileContentsResponse->cbRequested) * 100 / total_file_size);
			printf("   progress on total transfer %llu of %llu: %d%%\n", (long long unsigned int)total_transferred_size + fileContentsResponse->cbRequested, (long long unsigned int)total_file_size ,percent);


			/* If tms is less than one second, the double the value of adjusted_fblock_size
			 * up to REMMINA_CLIP_TRANSFER_BLOCKSIZE_MAX */
			if (tms < 1000 && (2 * adjusted_fblock_size) < REMMINA_CLIP_TRANSFER_BLOCKSIZE_MAX) {
				adjusted_fblock_size *= 2;
				printf("GIO: increasing adjusted_fblock_size to %llu\n", (long long unsigned int)adjusted_fblock_size);
			}
			/* Save data */
			rcwr = g_output_stream_write_all(G_OUTPUT_STREAM(os), fileContentsResponse->requestedData,
				fileContentsResponse->cbRequested, &written, clipboard->fpt->cancellable,
				NULL);

			/* Advance to next block */
			filepointer += fileContentsResponse->cbRequested;
			total_transferred_size += fileContentsResponse->cbRequested;

			srv_file_contents_response_answer(clipboard, CHANNEL_RC_OK);	// This also invalidates fileCOntentsResponse
			fileContentsResponse = NULL;
			printf("GIO: lower\n");

			remmina_plugin_service->protocol_plugin_emit_signal_with_int_param(clipboard->rfi->protocol_widget, "pastefiles-status", percent);

			printf("GIO: lower2\n");

			if (!rcwr) {
					printf("GIO: error writing file\n");
					break;
			}
		}
		/* Close file */
		rcc = g_output_stream_close(G_OUTPUT_STREAM(os), clipboard->fpt->cancellable, NULL);
		if (!rcwr || !rcc) {
			if (!rcc)
				printf("GIO: error closing file\n");
			free(fn);
			free(fnpart);
			continue;
		}
		os = NULL;

		/* Check that the just closed file has the requested size */
		finfo = g_file_query_info(ofile, G_FILE_ATTRIBUTE_STANDARD_SIZE,
			G_FILE_QUERY_INFO_NONE, clipboard->fpt->cancellable, NULL);
		if (!finfo) {
			printf("Unable to get file info for downloaded file.\n");
			free(fn);
			free(fnpart);
			continue;
		}
		sz = g_file_info_get_size(finfo);
		g_object_unref(finfo);
		if (sz != filesize) {
			printf("Error downloading file: size is %llu but expected size is %llu\n",
				(long long unsigned int)sz, (long long unsigned int)filesize);
			free(fn);
			free(fnpart);
			continue;
		}

		/* Delete destination file name if exists */
		fdest = g_file_new_for_path(fn);
		g_file_delete(fdest, clipboard->fpt->cancellable, NULL);

		/* Rename .part file into fn */
		if (!g_file_move(ofile, fdest, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, NULL)) {
			printf("Unable to rename .part filename\n");
			g_file_delete(ofile, NULL, NULL);
			g_object_unref(ofile);
			g_object_unref(fdest);
			free(fn);
			free(fnpart);
			continue;
		}
		g_object_unref(fdest);
		fdest = NULL;
		g_object_unref(ofile);
		ofile = NULL;
		free(fn);
		free(fnpart);
	}

	printf("GIO: all file transfer are complete. Ending thread.\n");


bailout:
	free(pFileGroupDescriptor);

	/* Cleanup data allocated in rdp_cliprdr_download_start() */
	fpt_free(clipboard->fpt);
	clipboard->fpt = NULL;
	clipboard->srv_clip_data_wait = SCDW_NONE;
	printf("GIO: download cleanup completed\n");

	/* Signaling main thread that we are finished */
	remmina_plugin_service->protocol_plugin_emit_signal_with_int_param(clipboard->rfi->protocol_widget, "pastefiles-status", -1);

	return NULL;


berror:
	printf("GIO: %s aborting due to an error\n", gettid());
abort:
	if (os) {
		g_output_stream_close(G_OUTPUT_STREAM(os), clipboard->fpt->cancellable, NULL);
		os = NULL;
	}
	if (fn) free(fn);
	if (fnpart) free(fnpart);
	if (ofile)
		g_file_delete(ofile, clipboard->fpt->cancellable, NULL);

	printf("GIO: %s transfer aborted\n", gettid());
	goto bailout;


}


int rdp_cliprdr_download_start(rfClipboard* clipboard, const char *destdir)
{
	int rc;

	if (clipboard->fpt) {
		fpt_free(clipboard->fpt);
	}

	remmina_plugin_service->protocol_plugin_emit_signal_with_int_param(clipboard->rfi->protocol_widget, "pastefiles-status", 0);

	clipboard->fpt = fpt_new();
	clipboard->fpt->destdir = g_strdup(destdir);

	clipboard->fpt->abort_requested = FALSE;
	rc = pthread_create(&clipboard->fpt->paste_thread, NULL,
		rdp_cliprdr_download_thread, (void *)clipboard);

	if (rc) {
		printf("unable to create thread in function %s: %s\n", __func__,
			strerror(errno));
		return 1;
	}

	return 0;

}



void rdp_cliprdr_download_abort(rfClipboard* clipboard)
{
	/* Can be called from main GTK thread or libfreerdp
	 * thread */

	if (clipboard->srv_clip_data_wait != SCDW_DOWNLOADTHREAD)
		return; // Nothing to abort

	printf("GIO: %s download abort requested time = %llu\n", gettid(),
		(long long unsigned int)time(NULL));

	clipboard->fpt->abort_requested = TRUE;
	
}

