/*
 * Remmina - The GTK+ Remote Desktop Client
 * Copyright (C) 2012-2012 Jean-Louis Dupond
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

#include <freerdp/freerdp.h>
#include <freerdp/channels/channels.h>
#include <freerdp/client/cliprdr.h>
#include <winpr/shell.h>
#include <sys/time.h>

#include "rdp_plugin.h"
#include "rdp_cliprdr.h"
#include "rdp_event.h"
#include "rdp_cliprdr_download_files.h"

#define CLIPBOARD_TRANSFER_WAIT_TIME 2


enum {
	REMMINA_RDP_CLIPRDR_FORMAT_COPIED_FILES = 0x34670001	// use an ID that can't conflict with RDP clipboard format like CF_TEXT.

};



UINT32 remmina_rdp_cliprdr_get_format_from_gdkatom(GdkAtom atom)
{
	TRACE_CALL(__func__);
	UINT32 rc;
	gchar* name = gdk_atom_name(atom);
	printf("GIO: %s %s\n", __func__, name);
	rc = 0;
	if (g_strcmp0("UTF8_STRING", name) == 0 || g_strcmp0("text/plain;charset=utf-8", name) == 0) {
		rc = CF_UNICODETEXT;
	}
	if (g_strcmp0("TEXT", name) == 0 || g_strcmp0("text/plain", name) == 0) {
		rc =  CF_TEXT;
	}
	if (g_strcmp0("text/html", name) == 0) {
		rc =  CB_FORMAT_HTML;
	}
	if (g_strcmp0("image/png", name) == 0) {
		rc =  CB_FORMAT_PNG;
	}
	if (g_strcmp0("image/jpeg", name) == 0) {
		rc =  CB_FORMAT_JPEG;
	}
	if (g_strcmp0("image/bmp", name) == 0) {
		rc =  CF_DIB;
	}
	if (g_strcmp0("text/uri-list", name) == 0) {
		rc =  CB_FORMAT_TEXTURILIST;
	}
	g_free(name);
	return rc;
}

void remmina_rdp_cliprdr_get_target_types(UINT32** formats, UINT16* size, GdkAtom* types, int count)
{
	TRACE_CALL(__func__);
	int i;
	*size = 1;
	*formats = (UINT32*)malloc(sizeof(UINT32) * (count + 1));

	*formats[0] = 0;
	for (i = 0; i < count; i++) {
		UINT32 format = remmina_rdp_cliprdr_get_format_from_gdkatom(types[i]);
		if (format != 0) {
			(*formats)[*size] = format;
			(*size)++;
		}
	}

	*formats = realloc(*formats, sizeof(UINT32) * (*size));
}

static UINT8* lf2crlf(UINT8* data, int* size)
{
	TRACE_CALL(__func__);
	UINT8 c;
	UINT8* outbuf;
	UINT8* out;
	UINT8* in_end;
	UINT8* in;
	int out_size;

	out_size = (*size) * 2 + 1;
	outbuf = (UINT8*)malloc(out_size);
	out = outbuf;
	in = data;
	in_end = data + (*size);

	while (in < in_end) {
		c = *in++;
		if (c == '\n') {
			*out++ = '\r';
			*out++ = '\n';
		}else  {
			*out++ = c;
		}
	}

	*out++ = 0;
	*size = out - outbuf;

	return outbuf;
}

static void crlf2lf(UINT8* data, size_t* size)
{
	TRACE_CALL(__func__);
	UINT8 c;
	UINT8* out;
	UINT8* in;
	UINT8* in_end;

	out = data;
	in = data;
	in_end = data + (*size);

	while (in < in_end) {
		c = *in++;
		if (c != '\r')
			*out++ = c;
	}

	*size = out - data;
}

static UINT remmina_rdp_cliprdr_send_file_contents_failure(CliprdrClientContext* context,
	const CLIPRDR_FILE_CONTENTS_REQUEST* fileContentsRequest)
{
	CLIPRDR_FILE_CONTENTS_RESPONSE response = { 0 };
	response.msgFlags = CB_RESPONSE_FAIL;
	response.streamId = fileContentsRequest->streamId;
	response.msgFlags = fileContentsRequest->msgFlags;
	return context->ClientFileContentsResponse(context, &response);
}

static UINT remmina_rdp_cliprdr_server_file_range_request(rfClipboard* clipboard,
        const CLIPRDR_FILE_CONTENTS_REQUEST* fileContentsRequest)
{
	wClipboardFileRangeRequest request = { 0 };
	request.streamId = fileContentsRequest->streamId;
	request.listIndex = fileContentsRequest->listIndex;
	request.nPositionLow = fileContentsRequest->nPositionLow;
	request.nPositionHigh = fileContentsRequest->nPositionHigh;
	request.cbRequested = fileContentsRequest->cbRequested;
	return clipboard->delegate->ClientRequestFileRange(clipboard->delegate, &request);
}

static UINT remmina_rdp_cliprdr_server_file_size_request(rfClipboard* clipboard,
        const CLIPRDR_FILE_CONTENTS_REQUEST* fileContentsRequest)
{
	wClipboardFileSizeRequest request = { 0 };
	request.streamId = fileContentsRequest->streamId;
	request.listIndex = fileContentsRequest->listIndex;

	if (fileContentsRequest->cbRequested != sizeof(UINT64))
	{
		remmina_plugin_service->log_printf("unexpected FILECONTENTS_SIZE request: %"PRIu32" bytes",
				  fileContentsRequest->cbRequested);
	}

	return clipboard->delegate->ClientRequestFileSize(clipboard->delegate, &request);
}

static UINT remmina_rdp_cliprdr_clipboard_file_size_success(wClipboardDelegate* delegate,
        const wClipboardFileSizeRequest* request, UINT64 fileSize)
{
	CLIPRDR_FILE_CONTENTS_RESPONSE response = { 0 };
	rfClipboard* clipboard = delegate->custom;
	response.msgFlags = CB_RESPONSE_OK;
	response.streamId = request->streamId;
	response.msgFlags = FILECONTENTS_SIZE;
	response.cbRequested = sizeof(UINT64);
	response.requestedData = (BYTE*) &fileSize;
	return clipboard->context->ClientFileContentsResponse(clipboard->context, &response);
}

static UINT remmina_rdp_cliprdr_clipboard_file_size_failure(wClipboardDelegate* delegate,
        const wClipboardFileSizeRequest* request, UINT errorCode)
{
	CLIPRDR_FILE_CONTENTS_RESPONSE response = { 0 };
	rfClipboard* clipboard = delegate->custom;
	response.msgFlags = CB_RESPONSE_FAIL;
	response.streamId = request->streamId;
	response.msgFlags = FILECONTENTS_SIZE;
	return clipboard->context->ClientFileContentsResponse(clipboard->context, &response);
}

static UINT remmina_rdp_cliprdr_clipboard_file_range_success(wClipboardDelegate* delegate,
        const wClipboardFileRangeRequest* request, const BYTE* data, UINT32 size)
{
	CLIPRDR_FILE_CONTENTS_RESPONSE response = { 0 };
	rfClipboard* clipboard = delegate->custom;
	response.msgFlags = CB_RESPONSE_OK;
	response.streamId = request->streamId;
	response.msgFlags = FILECONTENTS_RANGE;
	response.cbRequested = size;
	response.requestedData = (BYTE*) data;
	return clipboard->context->ClientFileContentsResponse(clipboard->context, &response);
}

static UINT remmina_rdp_cliprdr_clipboard_file_range_failure(wClipboardDelegate* delegate,
        const wClipboardFileRangeRequest* request, UINT errorCode)
{
	CLIPRDR_FILE_CONTENTS_RESPONSE response = { 0 };
	rfClipboard* clipboard = delegate->custom;
	response.msgFlags = CB_RESPONSE_FAIL;
	response.streamId = request->streamId;
	response.msgFlags = FILECONTENTS_RANGE;
	return clipboard->context->ClientFileContentsResponse(clipboard->context, &response);
}


static UINT remmina_rdp_cliprdr_server_file_contents_request(CliprdrClientContext* context, const CLIPRDR_FILE_CONTENTS_REQUEST* fileContentsRequest)
{
	UINT error = NO_ERROR;
	rfClipboard* clipboard = context->custom;

	TRACE_CALL(__func__);

	printf("GIO: %s\n", __func__);

	/* See comments on xf_cliprdr_server_file_contents_request() of FreeRDP project */

	if ((fileContentsRequest->dwFlags & (FILECONTENTS_SIZE | FILECONTENTS_RANGE)) ==
		(FILECONTENTS_SIZE | FILECONTENTS_RANGE))
	{
		remmina_plugin_service->log_printf("invalid CLIPRDR_FILECONTENTS_REQUEST.dwFlags");
		return remmina_rdp_cliprdr_send_file_contents_failure(context, fileContentsRequest);
	}

	if (fileContentsRequest->dwFlags & FILECONTENTS_SIZE)
		error = remmina_rdp_cliprdr_server_file_size_request(clipboard, fileContentsRequest);

	if (fileContentsRequest->dwFlags & FILECONTENTS_RANGE)
		error = remmina_rdp_cliprdr_server_file_range_request(clipboard, fileContentsRequest);

	if (error)
	{
		remmina_plugin_service->log_printf("failed to handle CLIPRDR_FILECONTENTS_REQUEST: 0x%08X", error);
		return remmina_rdp_cliprdr_send_file_contents_failure(context, fileContentsRequest);
	}

	return CHANNEL_RC_OK;
}


void remmina_rdp_cliprdr_send_client_format_list(RemminaProtocolWidget *gp)
{
	TRACE_CALL(__func__);
	RemminaPluginRdpUiObject* ui;
	rfContext* rfi = GET_PLUGIN_DATA(gp);
	rfClipboard* clipboard;
	CLIPRDR_FORMAT_LIST *pFormatList;
	RemminaPluginRdpEvent rdp_event = { 0 };

	if (!rfi || !rfi->connected || rfi->is_reconnecting)
		return;

	printf("GIO: %s\n", __func__);

	clipboard = &(rfi->clipboard);

	ui = g_new0(RemminaPluginRdpUiObject, 1);
	ui->type = REMMINA_RDP_UI_CLIPBOARD;
	ui->clipboard.clipboard = clipboard;
	ui->clipboard.type = REMMINA_RDP_UI_CLIPBOARD_FORMATLIST;
	pFormatList = remmina_rdp_event_queue_ui_sync_retptr(gp, ui);

	rdp_event.type = REMMINA_RDP_EVENT_TYPE_CLIPBOARD_SEND_CLIENT_FORMAT_LIST;
	rdp_event.clipboard_formatlist.pFormatList = pFormatList;
	remmina_rdp_event_event_push(gp, &rdp_event);

}

static void remmina_rdp_cliprdr_send_client_capabilities(rfClipboard* clipboard)
{
	TRACE_CALL(__func__);
	CLIPRDR_CAPABILITIES capabilities;
	CLIPRDR_GENERAL_CAPABILITY_SET generalCapabilitySet;

	capabilities.cCapabilitiesSets = 1;
	capabilities.capabilitySets = (CLIPRDR_CAPABILITY_SET*)&(generalCapabilitySet);

	generalCapabilitySet.capabilitySetType = CB_CAPSTYPE_GENERAL;
	generalCapabilitySet.capabilitySetLength = 12;

	generalCapabilitySet.version = CB_CAPS_VERSION_2;
	generalCapabilitySet.generalFlags = CB_USE_LONG_FORMAT_NAMES |
		CB_STREAM_FILECLIP_ENABLED | CB_FILECLIP_NO_FILE_PATHS;

	clipboard->context->ClientCapabilities(clipboard->context, &capabilities);
}


static UINT remmina_rdp_cliprdr_monitor_ready(CliprdrClientContext* context, const CLIPRDR_MONITOR_READY* monitorReady)
{
	TRACE_CALL(__func__);
	rfClipboard* clipboard = (rfClipboard*)context->custom;
	RemminaProtocolWidget* gp;

	remmina_rdp_cliprdr_send_client_capabilities(clipboard);
	gp = clipboard->rfi->protocol_widget;
	remmina_rdp_cliprdr_send_client_format_list(gp);

	return CHANNEL_RC_OK;
}

static UINT remmina_rdp_cliprdr_server_capabilities(CliprdrClientContext* context, const CLIPRDR_CAPABILITIES* capabilities)
{
	TRACE_CALL(__func__);

	printf("GIO: %s\n", __func__);

	UINT32 i;
	const CLIPRDR_CAPABILITY_SET* caps;
	const CLIPRDR_GENERAL_CAPABILITY_SET* generalCaps;
	const BYTE* capsPtr = (const BYTE*) capabilities->capabilitySets;
	rfClipboard* clipboard = (rfClipboard*) context->custom;

	for (i = 0; i < capabilities->cCapabilitiesSets; i++) {
		caps = (const CLIPRDR_CAPABILITY_SET*) capsPtr;
		if (caps->capabilitySetType == CB_CAPSTYPE_GENERAL) {
			generalCaps = (const CLIPRDR_GENERAL_CAPABILITY_SET*) caps;
			if (generalCaps->generalFlags & CB_STREAM_FILECLIP_ENABLED) {
				printf("GIO: server supports CB_STREAM_FILECLIP_ENABLED\n");
				clipboard->streams_supported = TRUE;
			}
			capsPtr += caps->capabilitySetLength;
		}
	}

	return CHANNEL_RC_OK;
}


static UINT remmina_rdp_cliprdr_server_format_list(CliprdrClientContext* context, const CLIPRDR_FORMAT_LIST* formatList)
{
	TRACE_CALL(__func__);

	/* Called when a user do a "Copy" on the server: we collect all formats
	 * the server send us and then setup the local clipboard with the appropiate
	 * functions to request server data */

	RemminaPluginRdpUiObject* ui;
	RemminaProtocolWidget* gp;
	rfClipboard* clipboard;
	CLIPRDR_FORMAT* format;
	CLIPRDR_FORMAT_LIST_RESPONSE formatListResponse;
	gboolean have_files;

	int i;

	clipboard = (rfClipboard*)context->custom;

	rdp_cliprdr_download_abort(clipboard);

	gp = clipboard->rfi->protocol_widget;
	GtkTargetList* list = gtk_target_list_new(NULL, 0);

	have_files = FALSE;
	for (i = 0; i < formatList->numFormats; i++) {
		format = &formatList->formats[i];
		printf("GIO: format %d of %d received from server. ID=%u, NAME=%s\n", i, formatList->numFormats,(unsigned)format->formatId, format->formatName);
		if (format->formatId == CF_UNICODETEXT) {
			GdkAtom atom = gdk_atom_intern("UTF8_STRING", TRUE);
			gtk_target_list_add(list, atom, 0, CF_UNICODETEXT);
		}else if (format->formatId == CF_TEXT) {
			GdkAtom atom = gdk_atom_intern("TEXT", TRUE);
			gtk_target_list_add(list, atom, 0, CF_TEXT);
		}else if (format->formatId == CF_DIB) {
			GdkAtom atom = gdk_atom_intern("image/bmp", TRUE);
			gtk_target_list_add(list, atom, 0, CF_DIB);
		}else if (format->formatId == CF_DIBV5) {
			GdkAtom atom = gdk_atom_intern("image/bmp", TRUE);
			gtk_target_list_add(list, atom, 0, CF_DIBV5);
		}else if (format->formatId == CB_FORMAT_JPEG) {
			GdkAtom atom = gdk_atom_intern("image/jpeg", TRUE);
			gtk_target_list_add(list, atom, 0, CB_FORMAT_JPEG);
		}else if (format->formatId == CB_FORMAT_PNG) {
			GdkAtom atom = gdk_atom_intern("image/png", TRUE);
			gtk_target_list_add(list, atom, 0, CB_FORMAT_PNG);
		}else if (format->formatId == CB_FORMAT_HTML) {
			GdkAtom atom = gdk_atom_intern("text/html", TRUE);
			gtk_target_list_add(list, atom, 0, CB_FORMAT_HTML);
		}else if (format->formatName && strcmp(format->formatName, "FileGroupDescriptorW") == 0) {
			/* Files (1 of 3). Put a special reference inside the clipboard and take note of formatId*/
			GdkAtom atom;
			have_files = TRUE;
			atom = gdk_atom_intern("x-special/remmina-copied-files", TRUE);
			gtk_target_list_add(list, atom, 0, REMMINA_RDP_CLIPRDR_FORMAT_COPIED_FILES);
			clipboard->filegroupdescriptorw_id = format->formatId;
		}else if (format->formatName && strcmp(format->formatName, "FileContents") == 0) {
			/* Files (2 of 3). take note of formatId for FileContents*/
			clipboard->filecontents_id = format->formatId;
		}else if (format->formatName && strcmp(format->formatName, "Preferred DropEffect") == 0) {
			/* Files (3 of 3). take note of formatId for Preferred DropEffect */
			clipboard->preferred_dropeffect_id = format->formatId;
		}else {
			printf("GIO:    unknown format\n");
		}
	}

	/* Now we tell GTK to change the local keyboard calling gtk_clipboard_set_with_owner
	 * via REMMINA_RDP_UI_CLIPBOARD_SET_DATA
	 * GTK will immediately fire an "owner-change" event, that we should ignore */

	ui = g_new0(RemminaPluginRdpUiObject, 1);
	ui->type = REMMINA_RDP_UI_CLIPBOARD;
	ui->clipboard.clipboard = clipboard;
	ui->clipboard.type = REMMINA_RDP_UI_CLIPBOARD_SET_DATA;
	ui->clipboard.targetlist = list;
	remmina_rdp_event_queue_ui_sync_retint(gp, ui);

	/* And also signal to protocol widget that we have files or we do not have */
	remmina_plugin_service->protocol_plugin_emit_signal_with_int_param(gp, "pastefiles-status", have_files ? -1 : -2);

	/* Send FormatListResponse to server */

	formatListResponse.msgType = CB_FORMAT_LIST_RESPONSE;
	formatListResponse.msgFlags = CB_RESPONSE_OK; // Can be CB_RESPONSE_FAIL in case of error
	formatListResponse.dataLen = 0;

	return clipboard->context->ClientFormatListResponse(clipboard->context, &formatListResponse);

}

static UINT remmina_rdp_cliprdr_server_format_list_response(CliprdrClientContext* context, const CLIPRDR_FORMAT_LIST_RESPONSE* formatListResponse)
{
	TRACE_CALL(__func__);
	return CHANNEL_RC_OK;
}


static UINT remmina_rdp_cliprdr_server_format_data_request(CliprdrClientContext* context, const CLIPRDR_FORMAT_DATA_REQUEST* formatDataRequest)
{
	TRACE_CALL(__func__);

	RemminaPluginRdpUiObject* ui;
	RemminaProtocolWidget* gp;
	rfClipboard* clipboard;

	clipboard = (rfClipboard*)context->custom;
	gp = clipboard->rfi->protocol_widget;

	ui = g_new0(RemminaPluginRdpUiObject, 1);
	ui->type = REMMINA_RDP_UI_CLIPBOARD;
	ui->clipboard.clipboard = clipboard;
	ui->clipboard.type = REMMINA_RDP_UI_CLIPBOARD_GET_DATA;
	ui->clipboard.format = formatDataRequest->requestedFormatId;
	remmina_rdp_event_queue_ui_sync_retint(gp, ui);

	return CHANNEL_RC_OK;
}

static void remmina_rdp_cliprdr_start_async_format_data_request(RemminaProtocolWidget *gp, UINT32 format)
{
	TRACE_CALL(__func__);
	rfClipboard* clipboard;
	CLIPRDR_FORMAT_DATA_REQUEST* pFormatDataRequest;
	RemminaPluginRdpEvent rdp_event = { 0 };

	rfContext* rfi = GET_PLUGIN_DATA(gp);

	clipboard = &(rfi->clipboard);
	clipboard->format = format;

	pthread_mutex_lock(&clipboard->transfer_clip_mutex);
	pFormatDataRequest = (CLIPRDR_FORMAT_DATA_REQUEST*)malloc(sizeof(CLIPRDR_FORMAT_DATA_REQUEST));
	ZeroMemory(pFormatDataRequest, sizeof(CLIPRDR_FORMAT_DATA_REQUEST));
	pFormatDataRequest->requestedFormatId = clipboard->format;
	/* We never busywait here for a server file */
	clipboard->srv_clip_data_wait = SCDW_ASYNCWAIT;

	rdp_event.type = REMMINA_RDP_EVENT_TYPE_CLIPBOARD_SEND_CLIENT_FORMAT_DATA_REQUEST;
	rdp_event.clipboard_formatdatarequest.pFormatDataRequest = pFormatDataRequest;
	remmina_rdp_event_event_push(gp, &rdp_event);

	pthread_mutex_unlock(&clipboard->transfer_clip_mutex);

}

static UINT remmina_rdp_cliprdr_server_format_data_response(CliprdrClientContext* context, const CLIPRDR_FORMAT_DATA_RESPONSE* formatDataResponse)
{
	TRACE_CALL(__func__);

	const UINT8* data;
	size_t size;
	rfContext* rfi;
	RemminaProtocolWidget* gp;
	rfClipboard* clipboard;
	GdkPixbufLoader *pixbuf;
	gpointer output = NULL;
	RemminaPluginRdpUiObject *ui;

	clipboard = (rfClipboard*)context->custom;
	if (clipboard->srv_clip_data_wait == SCDW_FILEDOWNLOAD) {
		/* When the clipboard is downloading files, route this call to
		 * the appropriate function */
		return rdp_cliprdr_download_server_format_data_response(context, formatDataResponse);
	}

	gp = clipboard->rfi->protocol_widget;
	rfi = GET_PLUGIN_DATA(gp);

	data = formatDataResponse->requestedFormatData;
	size = formatDataResponse->dataLen;

	printf("GIO: %s size=%u format=%u\n", __func__, (unsigned)size, rfi->clipboard.format);

	// formatDataResponse->requestedFormatData is allocated
	//  by freerdp and freed after returning from this callback function.
	//  So we must make a copy if we need to preserve it

	if (size > 0) {
		switch (rfi->clipboard.format) {
		case CF_UNICODETEXT:
		{
			size = ConvertFromUnicode(CP_UTF8, 0, (WCHAR*)data, size / 2, (CHAR**)&output, 0, NULL, NULL);
			crlf2lf(output, &size);
			break;
		}

		case CF_TEXT:
		case CB_FORMAT_HTML:
		{
			output = (gpointer)calloc(1, size + 1);
			if (output) {
				memcpy(output, data, size);
				crlf2lf(output, &size);
			}
			break;
		}

		case CF_DIBV5:
		case CF_DIB:
		{
			wStream* s;
			UINT32 offset;
			GError *perr;
			BITMAPINFOHEADER* pbi;
			BITMAPV5HEADER* pbi5;

			pbi = (BITMAPINFOHEADER*)data;

			// offset calculation inspired by http://downloads.poolelan.com/MSDN/MSDNLibrary6/Disk1/Samples/VC/OS/WindowsXP/GetImage/BitmapUtil.cpp
			offset = 14 + pbi->biSize;
			if (pbi->biClrUsed != 0)
				offset += sizeof(RGBQUAD) * pbi->biClrUsed;
			else if (pbi->biBitCount <= 8)
				offset += sizeof(RGBQUAD) * (1 << pbi->biBitCount);
			if (pbi->biSize == sizeof(BITMAPINFOHEADER)) {
				if (pbi->biCompression == 3)         // BI_BITFIELDS is 3
					offset += 12;
			} else if (pbi->biSize >= sizeof(BITMAPV5HEADER)) {
				pbi5 = (BITMAPV5HEADER*)pbi;
				if (pbi5->bV5ProfileData <= offset)
					offset += pbi5->bV5ProfileSize;
			}
			s = Stream_New(NULL, 14 + size);
			Stream_Write_UINT8(s, 'B');
			Stream_Write_UINT8(s, 'M');
			Stream_Write_UINT32(s, 14 + size);
			Stream_Write_UINT32(s, 0);
			Stream_Write_UINT32(s, offset);
			Stream_Write(s, data, size);

			data = Stream_Buffer(s);
			size = Stream_Length(s);

			pixbuf = gdk_pixbuf_loader_new();
			perr = NULL;
			if ( !gdk_pixbuf_loader_write(pixbuf, data, size, &perr) ) {
				remmina_plugin_service->log_printf("[RDP] rdp_cliprdr: gdk_pixbuf_loader_write() returned error %s\n", perr->message);
			}else  {
				if ( !gdk_pixbuf_loader_close(pixbuf, &perr) ) {
					remmina_plugin_service->log_printf("[RDP] rdp_cliprdr: gdk_pixbuf_loader_close() returned error %s\n", perr->message);
					perr = NULL;
				}
				Stream_Free(s, TRUE);
				output = g_object_ref(gdk_pixbuf_loader_get_pixbuf(pixbuf));
			}
			g_object_unref(pixbuf);
			break;
		}

		case CB_FORMAT_PNG:
		case CB_FORMAT_JPEG:
		{
			pixbuf = gdk_pixbuf_loader_new();
			gdk_pixbuf_loader_write(pixbuf, data, size, NULL);
			output = g_object_ref(gdk_pixbuf_loader_get_pixbuf(pixbuf));
			gdk_pixbuf_loader_close(pixbuf, NULL);
			g_object_unref(pixbuf);
			break;
		}
		}


		pthread_mutex_lock(&clipboard->transfer_clip_mutex);
		pthread_cond_signal(&clipboard->transfer_clip_cond);
		if ( clipboard->srv_clip_data_wait == SCDW_BUSY_WAIT ) {
			clipboard->srv_data = output;
		}else  {
			// Clipboard data arrived from server when we are not busywaiting.
			// Just put it on the local clipboard

			ui = g_new0(RemminaPluginRdpUiObject, 1);
			ui->type = REMMINA_RDP_UI_CLIPBOARD;
			ui->clipboard.clipboard = clipboard;
			ui->clipboard.type = REMMINA_RDP_UI_CLIPBOARD_SET_CONTENT;
			ui->clipboard.data = output;
			ui->clipboard.format = clipboard->format;
			remmina_rdp_event_queue_ui_sync_retint(gp, ui);

			clipboard->srv_clip_data_wait = SCDW_NONE;

		}
		pthread_mutex_unlock(&clipboard->transfer_clip_mutex);
	}

	return CHANNEL_RC_OK;
}

void remmina_rdp_cliprdr_request_data(GtkClipboard *gtkClipboard, GtkSelectionData *selection_data, guint info, RemminaProtocolWidget* gp )
{
	TRACE_CALL(__func__);

	/* callback for gtk_clipboard_set_with_owner(),
	 * is called by GTK when someone press "Paste" on the client side.
	 * We ask to the server the data we need */

	CLIPRDR_FORMAT_DATA_REQUEST* pFormatDataRequest;
	rfClipboard* clipboard;
	rfContext* rfi = GET_PLUGIN_DATA(gp);
	RemminaPluginRdpEvent rdp_event = { 0 };
	struct timespec to;
	struct timeval tv;
	int rc;

	clipboard = &(rfi->clipboard);
	if ( clipboard->srv_clip_data_wait != SCDW_NONE ) {
		remmina_plugin_service->log_printf("[RDP] Cannot paste now, I’m transferring clipboard data from server. Try again later\n");
		return;
	}

	clipboard->format = info;

	printf("GIO: %s is gtk_clipboard_set_with_owner() callback, format = %d\n", __func__, clipboard->format);

	/* Request Clipboard content from the server, the request is async */
	pthread_mutex_lock(&clipboard->transfer_clip_mutex);

	pFormatDataRequest = (CLIPRDR_FORMAT_DATA_REQUEST*)malloc(sizeof(CLIPRDR_FORMAT_DATA_REQUEST));
	ZeroMemory(pFormatDataRequest, sizeof(CLIPRDR_FORMAT_DATA_REQUEST));
	pFormatDataRequest->requestedFormatId = clipboard->format;
	clipboard->srv_clip_data_wait = SCDW_BUSY_WAIT;

	rdp_event.type = REMMINA_RDP_EVENT_TYPE_CLIPBOARD_SEND_CLIENT_FORMAT_DATA_REQUEST;
	rdp_event.clipboard_formatdatarequest.pFormatDataRequest = pFormatDataRequest;
	remmina_rdp_event_event_push(gp, &rdp_event);

	/* Busy wait clibpoard data for CLIPBOARD_TRANSFER_WAIT_TIME seconds */
	gettimeofday(&tv, NULL);
	to.tv_sec = tv.tv_sec + CLIPBOARD_TRANSFER_WAIT_TIME;
	to.tv_nsec = tv.tv_usec * 1000;
	rc = pthread_cond_timedwait(&clipboard->transfer_clip_cond, &clipboard->transfer_clip_mutex, &to);

	if ( rc == 0 ) {
		/* Data has arrived without timeout */
		if (clipboard->srv_data != NULL) {
			if (info == CB_FORMAT_PNG || info == CF_DIB || info == CF_DIBV5 || info == CB_FORMAT_JPEG) {
				gtk_selection_data_set_pixbuf(selection_data, clipboard->srv_data);
				g_object_unref(clipboard->srv_data);
			}else  {
				gtk_selection_data_set_text(selection_data, clipboard->srv_data, -1);
				free(clipboard->srv_data);
			}
		}
		clipboard->srv_clip_data_wait = SCDW_NONE;
	} else {
		clipboard->srv_clip_data_wait = SCDW_ASYNCWAIT;
		if ( rc == ETIMEDOUT ) {
			remmina_plugin_service->log_printf("[RDP] Clipboard data has not been transferred from the server in %d seconds. Try to paste later.\n",
				CLIPBOARD_TRANSFER_WAIT_TIME);
		}else  {
			remmina_plugin_service->log_printf("[RDP] internal error: pthread_cond_timedwait() returned %d\n", rc);
			clipboard->srv_clip_data_wait = SCDW_NONE;
		}
	}
	pthread_mutex_unlock(&clipboard->transfer_clip_mutex);

}

void remmina_rdp_cliprdr_empty_clipboard(GtkClipboard *gtkClipboard, RemminaProtocolWidget* gp)
{
	TRACE_CALL(__func__);
	rfContext* rfi = GET_PLUGIN_DATA(gp);
	/* Called by GTK when clipboard is emptied */
	rdp_cliprdr_download_abort(&rfi->clipboard);
	remmina_plugin_service->protocol_plugin_emit_signal_with_int_param(gp, "pastefiles-status", -2);
}

CLIPRDR_FORMAT_LIST *remmina_rdp_cliprdr_get_client_format_list(RemminaProtocolWidget* gp)
{
	TRACE_CALL(__func__);

	GtkClipboard* gtkClipboard;
	rfContext* rfi = GET_PLUGIN_DATA(gp);
	GdkAtom* targets;
	gboolean result = 0;
	gint loccount, srvcount;
	gint formatId, i;
	CLIPRDR_FORMAT *formats;
	struct retp_t {
		CLIPRDR_FORMAT_LIST pFormatList;
		CLIPRDR_FORMAT formats[];
	} *retp;

	formats = NULL;

	retp = NULL;

	gtkClipboard = gtk_widget_get_clipboard(rfi->drawing_area, GDK_SELECTION_CLIPBOARD);
	if (gtkClipboard) {
		result = gtk_clipboard_wait_for_targets(gtkClipboard, &targets, &loccount);
	}

	if (result && loccount > 0) {
		formats = (CLIPRDR_FORMAT*)malloc(loccount * sizeof(CLIPRDR_FORMAT));
		srvcount = 0;
		for (i = 0; i < loccount; i++) {
			formatId = remmina_rdp_cliprdr_get_format_from_gdkatom(targets[i]);
			if ( formatId != 0 ) {
				formats[srvcount].formatName = NULL;
				if (formatId == CB_FORMAT_TEXTURILIST) {
					if (!ClipboardGetFormatId(rfi->clipboard.system, "text/uri-list"))
						continue;
					formats[srvcount].formatName = _strdup("FileGroupDescriptorW");
					printf("GIO: text/uri-list is accepted\n");
				}
				formats[srvcount].formatId = formatId;
				srvcount++;
			}
		}
		if (srvcount > 0) {
			retp = (struct retp_t *)malloc(sizeof(struct retp_t) + sizeof(CLIPRDR_FORMAT) * srvcount);
			retp->pFormatList.formats = retp->formats;
			retp->pFormatList.numFormats = srvcount;
			memcpy(retp->formats, formats, sizeof(CLIPRDR_FORMAT) * srvcount);
		} else {
			retp = (struct retp_t *)malloc(sizeof(struct retp_t));
			retp->pFormatList.formats = NULL;
			retp->pFormatList.numFormats = 0;
		}
		free(formats);
	} else {
		retp = (struct retp_t *)malloc(sizeof(struct retp_t) + sizeof(CLIPRDR_FORMAT));
		retp->pFormatList.formats = NULL;
		retp->pFormatList.numFormats = 0;
	}

	if (result)
		g_free(targets);

	retp->pFormatList.msgFlags = CB_RESPONSE_OK;

	return (CLIPRDR_FORMAT_LIST*)retp;
}

static void remmina_rdp_cliprdr_mt_get_format_list(RemminaProtocolWidget* gp, RemminaPluginRdpUiObject* ui)
{
	TRACE_CALL(__func__);
	ui->retptr = (void*)remmina_rdp_cliprdr_get_client_format_list(gp);
}


void remmina_rdp_cliprdr_get_clipboard_data(RemminaProtocolWidget* gp, RemminaPluginRdpUiObject* ui)
{
	TRACE_CALL(__func__);
	GtkClipboard* gtkClipboard;
	UINT8* inbuf = NULL;
	UINT8* outbuf = NULL;
	GdkPixbuf *image = NULL;
	int size = 0;
	rfContext* rfi = GET_PLUGIN_DATA(gp);
	RemminaPluginRdpEvent rdp_event = { 0 };

	printf("GIO: %s\n", __func__);

	gtkClipboard = gtk_widget_get_clipboard(rfi->drawing_area, GDK_SELECTION_CLIPBOARD);
	if (gtkClipboard) {
		switch (ui->clipboard.format) {
		case CF_TEXT:
		case CF_UNICODETEXT:
		case CB_FORMAT_HTML:
		{
			inbuf = (UINT8*)gtk_clipboard_wait_for_text(gtkClipboard);
			break;
		}

		case CB_FORMAT_PNG:
		case CB_FORMAT_JPEG:
		case CF_DIB:
		case CF_DIBV5:
		{
			image = gtk_clipboard_wait_for_image(gtkClipboard);
			break;
		}
		}
	}

	/* No data received, send nothing */
	if (inbuf != NULL || image != NULL) {
		switch (ui->clipboard.format) {
		case CF_TEXT:
		case CB_FORMAT_HTML:
		{
			size = strlen((char*)inbuf);
			outbuf = lf2crlf(inbuf, &size);
			break;
		}
		case CF_UNICODETEXT:
		{
			size = strlen((char*)inbuf);
			inbuf = lf2crlf(inbuf, &size);
			size = (ConvertToUnicode(CP_UTF8, 0, (CHAR*)inbuf, -1, (WCHAR**)&outbuf, 0) ) * sizeof(WCHAR);
			g_free(inbuf);
			break;
		}
		case CB_FORMAT_PNG:
		{
			gchar* data;
			gsize buffersize;
			gdk_pixbuf_save_to_buffer(image, &data, &buffersize, "png", NULL, NULL);
			outbuf = (UINT8*)malloc(buffersize);
			memcpy(outbuf, data, buffersize);
			size = buffersize;
			g_object_unref(image);
			break;
		}
		case CB_FORMAT_JPEG:
		{
			gchar* data;
			gsize buffersize;
			gdk_pixbuf_save_to_buffer(image, &data, &buffersize, "jpeg", NULL, NULL);
			outbuf = (UINT8*)malloc(buffersize);
			memcpy(outbuf, data, buffersize);
			size = buffersize;
			g_object_unref(image);
			break;
		}
		case CF_DIB:
		case CF_DIBV5:
		{
			gchar* data;
			gsize buffersize;
			gdk_pixbuf_save_to_buffer(image, &data, &buffersize, "bmp", NULL, NULL);
			size = buffersize - 14;
			outbuf = (UINT8*)malloc(size);
			memcpy(outbuf, data + 14, size);
			g_object_unref(image);
			break;
		}
		}
	}

	rdp_event.type = REMMINA_RDP_EVENT_TYPE_CLIPBOARD_SEND_CLIENT_FORMAT_DATA_RESPONSE;
	rdp_event.clipboard_formatdataresponse.data = outbuf;
	rdp_event.clipboard_formatdataresponse.size = size;
	remmina_rdp_event_event_push(gp, &rdp_event);

}

void remmina_rdp_cliprdr_set_clipboard_content(RemminaProtocolWidget* gp, RemminaPluginRdpUiObject* ui)
{
	TRACE_CALL(__func__);
	GtkClipboard* gtkClipboard;
	rfContext* rfi = GET_PLUGIN_DATA(gp);

	gtkClipboard = gtk_widget_get_clipboard(rfi->drawing_area, GDK_SELECTION_CLIPBOARD);
	if (ui->clipboard.format == CB_FORMAT_PNG || ui->clipboard.format == CF_DIB || ui->clipboard.format == CF_DIBV5 || ui->clipboard.format == CB_FORMAT_JPEG) {
		gtk_clipboard_set_image( gtkClipboard, ui->clipboard.data );
		g_object_unref(ui->clipboard.data);
	}else  {
		gtk_clipboard_set_text( gtkClipboard, ui->clipboard.data, -1 );
		free(ui->clipboard.data);
	}

}

void remmina_rdp_cliprdr_set_clipboard_data(RemminaProtocolWidget* gp, RemminaPluginRdpUiObject* ui)
{
	TRACE_CALL(__func__);
	GtkClipboard* gtkClipboard;
	GtkTargetEntry* targets;
	gint n_targets;
	rfContext* rfi = GET_PLUGIN_DATA(gp);

	targets = gtk_target_table_new_from_list(ui->clipboard.targetlist, &n_targets);
	gtkClipboard = gtk_widget_get_clipboard(rfi->drawing_area, GDK_SELECTION_CLIPBOARD);
	if (gtkClipboard && targets) {
		gtk_clipboard_set_with_owner(gtkClipboard, targets, n_targets,
			(GtkClipboardGetFunc)remmina_rdp_cliprdr_request_data,
			(GtkClipboardClearFunc)remmina_rdp_cliprdr_empty_clipboard, G_OBJECT(gp));
		gtk_target_table_free(targets, n_targets);
	}
}

void remmina_rdp_cliprdr_detach_owner(RemminaProtocolWidget* gp)
{
	/* When closing a rdp connection, we should check if gp is a clipboard owner.
	 * If it’s an owner, detach it from the clipboard */
	TRACE_CALL(__func__);
	rfContext* rfi = GET_PLUGIN_DATA(gp);
	GtkClipboard* gtkClipboard;

	if (!rfi || !rfi->drawing_area) return;

	gtkClipboard = gtk_widget_get_clipboard(rfi->drawing_area, GDK_SELECTION_CLIPBOARD);
	if (gtkClipboard && gtk_clipboard_get_owner(gtkClipboard) == (GObject*)gp) {
		rdp_cliprdr_download_abort(&rfi->clipboard);
		gtk_clipboard_clear(gtkClipboard);
	}

}

void remmina_rdp_cliprdr_retrieve_remote_clipboard_files(RemminaProtocolWidget *gp, const char *destdir)
{
	TRACE_CALL(__func__);

	/* callback:
	 * is called by rcw.c when someone wants to paste files to destir.
	 * very similar remmina_rdp_cliprdr_request_data(), but for files
	 * We do not provide data to the GTK clipboard. We save
	 * files directy into destdir.
	 */
	rfClipboard* clipboard;
	rfContext* rfi = GET_PLUGIN_DATA(gp);

	printf("GIO: %s paste file requested to dir %s\n", __func__, destdir);

	clipboard = &(rfi->clipboard);
	if (!clipboard || !clipboard->context || !clipboard->context->ClientFormatDataRequest)
		return;
	if ( clipboard->srv_clip_data_wait != SCDW_NONE ) {
		remmina_plugin_service->log_printf("[RDP] Cannot paste now, I’m transferring clipboard data from server. Try again later\n");
		return;
	}

	/* Check that we have valid file data on the clipoard at the server side */
	if (clipboard->filegroupdescriptorw_id == 0 ||
		clipboard->preferred_dropeffect_id == 0 ||
		clipboard->filecontents_id == 0)
		return;

	printf("GIO: %s #2\n", __func__);


	/* Start a new pastefile thread() */

	rdp_cliprdr_download_start(clipboard, destdir);

}
void remmina_rdp_cliprdr_stop_clipboard_transfer(RemminaProtocolWidget *gp)
{
	TRACE_CALL(__func__);
	rfClipboard* clipboard;
	rfContext* rfi = GET_PLUGIN_DATA(gp);
	clipboard = &(rfi->clipboard);
	if (!clipboard || !clipboard->context || !clipboard->fpt || clipboard->srv_clip_data_wait != SCDW_DOWNLOADTHREAD)
		return;

	rdp_cliprdr_download_abort(clipboard);

}


void remmina_rdp_event_process_clipboard(RemminaProtocolWidget* gp, RemminaPluginRdpUiObject* ui)
{
	TRACE_CALL(__func__);

	switch (ui->clipboard.type) {

	case REMMINA_RDP_UI_CLIPBOARD_FORMATLIST:
		remmina_rdp_cliprdr_mt_get_format_list(gp, ui);
		break;

	case REMMINA_RDP_UI_CLIPBOARD_GET_DATA:
		remmina_rdp_cliprdr_get_clipboard_data(gp, ui);
		break;

	case REMMINA_RDP_UI_CLIPBOARD_SET_DATA:
		remmina_rdp_cliprdr_set_clipboard_data(gp, ui);
		break;

	case REMMINA_RDP_UI_CLIPBOARD_SET_CONTENT:
		remmina_rdp_cliprdr_set_clipboard_content(gp, ui);
		break;
	}
}

void remmina_rdp_clipboard_init(rfContext *rfi)
{
	TRACE_CALL(__func__);
	printf("GIO: %s\n", __func__);
	// initialize rfi->clipboard
	rfClipboard *clipboard = &(rfi->clipboard);

	pthread_mutex_init(&clipboard->transfer_clip_mutex, NULL);
	pthread_cond_init(&clipboard->transfer_clip_cond, NULL);
	clipboard->srv_clip_data_wait = SCDW_NONE;

	clipboard->system = ClipboardCreate();
	clipboard->delegate = ClipboardGetDelegate(clipboard->system);
	clipboard->delegate->custom = clipboard;
	clipboard->delegate->ClipboardFileSizeSuccess = remmina_rdp_cliprdr_clipboard_file_size_success;
	clipboard->delegate->ClipboardFileSizeFailure = remmina_rdp_cliprdr_clipboard_file_size_failure;
	clipboard->delegate->ClipboardFileRangeSuccess = remmina_rdp_cliprdr_clipboard_file_range_success;
	clipboard->delegate->ClipboardFileRangeFailure = remmina_rdp_cliprdr_clipboard_file_range_failure;
}


void remmina_rdp_clipboard_free(rfContext *rfi)
{
	TRACE_CALL(__func__);

	if (rfi->clipboard.fpt) {
		rdp_cliprdr_download_abort(&rfi->clipboard);
	}

}


void remmina_rdp_cliprdr_init(rfContext* rfi, CliprdrClientContext* cliprdr)
{
	TRACE_CALL(__func__);

	rfClipboard* clipboard;
	clipboard = &(rfi->clipboard);

	rfi->clipboard.rfi = rfi;
	cliprdr->custom = (void*)clipboard;

	clipboard->context = cliprdr;
	cliprdr->MonitorReady = remmina_rdp_cliprdr_monitor_ready;
	cliprdr->ServerCapabilities = remmina_rdp_cliprdr_server_capabilities;
	cliprdr->ServerFormatList = remmina_rdp_cliprdr_server_format_list;
	cliprdr->ServerFormatListResponse = remmina_rdp_cliprdr_server_format_list_response;
	cliprdr->ServerFormatDataRequest = remmina_rdp_cliprdr_server_format_data_request;
	cliprdr->ServerFormatDataResponse = remmina_rdp_cliprdr_server_format_data_response;
	cliprdr->ServerFileContentsRequest = remmina_rdp_cliprdr_server_file_contents_request;
	cliprdr->ServerFileContentsResponse = rdp_cliprdr_download_server_file_contents_response;
}



