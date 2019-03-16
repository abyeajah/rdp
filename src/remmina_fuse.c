/*
 * Remmina - The GTK+ Remote Desktop Client
 * Copyright (C) 2009-2011 Vic Lee
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

#include "config.h"

#include <glib.h>
#include <errno.h>
#include <unistd.h>
#include "remmina.h"
#include "remmina_fuse.h"

#define FUSE_USE_VERSION 26
#include <fuse_lowlevel.h>

static gchar *fuse_mountpoint = NULL;
static struct fuse_chan *fuse_ch;
static gboolean fuse_initialized = FALSE;
static struct fuse_args args = {
	.argc = 2,
	.argv = (char *[]){".","-oauto_unmount"},
	.allocated = 0
};

static const gchar fuse_subdir[] = "/remmina";
static pthread_t ft;

/* DEMO */
static const char *hello_str = "Hello World!\n";
static const char *hello_name = "hello";


static int rfuse_stat(fuse_ino_t ino, struct stat *stbuf)
{
	stbuf->st_ino = ino;
	switch (ino) {
	case 1:
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		break;

	case 2:
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(hello_str);
		break;

	default:
		return -1;
	}
	return 0;
}


static void rfuse_lookup(fuse_req_t req, fuse_ino_t parent, const char *name)
{
	struct fuse_entry_param e;

	if (parent != 1 || strcmp(name, hello_name) != 0)
		fuse_reply_err(req, ENOENT);
	else {
		memset(&e, 0, sizeof(e));
		e.ino = 2;
		e.attr_timeout = 1.0;
		e.entry_timeout = 1.0;
		rfuse_stat(e.ino, &e.attr);

		fuse_reply_entry(req, &e);
	}
}


static void rfuse_getattr(fuse_req_t req, fuse_ino_t ino,
			     struct fuse_file_info *fi)
{
	struct stat stbuf;

	(void) fi;

	memset(&stbuf, 0, sizeof(stbuf));
	if (rfuse_stat(ino, &stbuf) == -1)
		fuse_reply_err(req, ENOENT);
	else
		fuse_reply_attr(req, &stbuf, 1.0);
}


struct dirbuf {
	char *p;
	size_t size;
};

static void dirbuf_add(fuse_req_t req, struct dirbuf *b, const char *name,
		       fuse_ino_t ino)
{
	struct stat stbuf;
	size_t oldsize = b->size;
	b->size += fuse_add_direntry(req, NULL, 0, name, NULL, 0);
	b->p = (char *) realloc(b->p, b->size);
	memset(&stbuf, 0, sizeof(stbuf));
	stbuf.st_ino = ino;
	fuse_add_direntry(req, b->p + oldsize, b->size - oldsize, name, &stbuf,
			  b->size);
}

#define min(x, y) ((x) < (y) ? (x) : (y))

static int reply_buf_limited(fuse_req_t req, const char *buf, size_t bufsize,
			     off_t off, size_t maxsize)
{
	if (off < bufsize)
		return fuse_reply_buf(req, buf + off,
				      min(bufsize - off, maxsize));
	else
		return fuse_reply_buf(req, NULL, 0);
}


static void rfuse_readdir(fuse_req_t req, fuse_ino_t ino, size_t size,
			     off_t off, struct fuse_file_info *fi)
{
	(void) fi;

	if (ino != 1)
		fuse_reply_err(req, ENOTDIR);
	else {
		struct dirbuf b;

		memset(&b, 0, sizeof(b));
		dirbuf_add(req, &b, ".", 1);
		dirbuf_add(req, &b, "..", 1);
		dirbuf_add(req, &b, hello_name, 2);
		reply_buf_limited(req, b.p, b.size, off, size);
		free(b.p);
	}
}

static void rfuse_open(fuse_req_t req, fuse_ino_t ino,
			  struct fuse_file_info *fi)
{
	if (ino != 2)
		fuse_reply_err(req, EISDIR);
	else if ((fi->flags & 3) != O_RDONLY)
		fuse_reply_err(req, EACCES);
	else
		fuse_reply_open(req, fi);
}

static void rfuse_read(fuse_req_t req, fuse_ino_t ino, size_t size,
			  off_t off, struct fuse_file_info *fi)
{
	(void) fi;

	if(ino == 2)
		reply_buf_limited(req, hello_str, strlen(hello_str), off, size);
}




static struct fuse_lowlevel_ops hello_ll_oper = {
	.lookup		= rfuse_lookup,
	.getattr	= rfuse_getattr,
	.readdir	= rfuse_readdir,
	.open		= rfuse_open,
	.read		= rfuse_read,
};

static void *fuse_thread_start(void *arg)
{
	struct fuse_session *se;
	int err = -1;

	g_print("GIO: fuse thread!\n");

	se = fuse_lowlevel_new(&args, &hello_ll_oper, sizeof(hello_ll_oper), NULL);
	if (se != NULL) {
		fuse_session_add_chan(se, fuse_ch);
		err = fuse_session_loop(se);
		fuse_session_remove_chan(fuse_ch);
	}
	fuse_session_destroy(se);
	fuse_unmount(fuse_mountpoint, fuse_ch);
	g_free(fuse_mountpoint);
	fuse_initialized = FALSE;

	g_print("GIO: fuse thread ENDED, err=%d.\n",err);

	return NULL;
}

void remmina_fuse_init()
{
	const gchar *rtdir;
    gchar *fuse_remmina_topdir, *fpath;
	int err;
    DIR *d;
    struct dirent *de;

	if (fuse_initialized)
		return;

	rtdir = g_get_user_runtime_dir();
    fuse_remmina_topdir = g_strdup_printf("%s/%s", rtdir, fuse_subdir);
    /* Try to delete all undeleted/old mountdirs */
    d = opendir(fuse_remmina_topdir);
    if (d) {
        while((de = readdir(d)) != NULL) {
            if (strncmp(de->d_name, "pid_", 4) == 0) {
                fpath = g_strdup_printf("%s/%s", fuse_remmina_topdir, de->d_name);
                rmdir(fpath);
                g_free(fpath);
            }
        }
        closedir(d);
    }

    fuse_mountpoint = g_strdup_printf("%s/pid_%u", fuse_remmina_topdir, (unsigned)getpid());
    g_free(fuse_remmina_topdir);

	/* Try to create fuse_mountpoint */
	if (!g_file_test(fuse_mountpoint, G_FILE_TEST_EXISTS)) {
		mkdir(fuse_mountpoint, 0700);
	}

	if (!g_file_test(fuse_mountpoint, G_FILE_TEST_IS_DIR)) {
		g_print("REMMINA WARNING: %s should be a directory, but it's not.\n", fuse_mountpoint);
		g_free(fuse_mountpoint);
	}

	if (!(fuse_ch = fuse_mount(fuse_mountpoint, &args))) {
		g_print("REMMINA WARNING: Unable to mount fuse directory on %s\n", fuse_mountpoint);
		g_free(fuse_mountpoint);
	}
	fuse_initialized = TRUE;

	if ((err = pthread_create(&ft, NULL, fuse_thread_start, NULL)) != 0) {
		g_print("REMMINA WARNING: Unable to start fuse thread, error %s\n", strerror(err));
		fuse_unmount(fuse_mountpoint, fuse_ch);
		g_free(fuse_mountpoint);
		fuse_initialized = FALSE;
	}

}

void remmina_fuse_cleanup()
{

}

