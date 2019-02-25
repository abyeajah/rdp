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
#include "remmina.h"
#include "remmina_fuse.h"

#define FUSE_USE_VERSION 26
#include <fuse_lowlevel.h>

static gchar *fuse_mountpoint = NULL;
static gboolean fuse_initialized = FALSE;
static struct fuse_args args = {
	.argc = 2,
	.argv = (char *[]){".","-oauto_unmount"},
	.allocated = 0
};

const gchar fuse_subdir[] = "/remmina";

void remmina_fuse_init()
{
	const gchar *rtdir;

	if (fuse_initialized)
		return;

	rtdir = g_get_user_runtime_dir();
	fuse_mountpoint = g_malloc0(strlen(rtdir) + strlen(fuse_subdir) + 1);
	strcpy(fuse_mountpoint, rtdir);
	strcat(fuse_mountpoint, fuse_subdir);

	/* Try to create fuse_mountpoint */
	if (!g_file_test(fuse_mountpoint, G_FILE_TEST_EXISTS)) {
		mkdir(fuse_mountpoint, 0700);
	}

	if (!g_file_test(fuse_mountpoint, G_FILE_TEST_IS_DIR)) {
		g_print("REMMINA WARNING: %s should be a directory, but it's not.\n", fuse_mountpoint);
		g_free(fuse_mountpoint);
	}

	if (!fuse_mount(fuse_mountpoint, &args)) {
		g_print("REMMINA WARNING: Unable to mount fuse remmina directory on %s\n", fuse_mountpoint);
		g_free(fuse_mountpoint);
	}
	fuse_initialized = TRUE;

}

void remmina_fuse_cleanup()
{

}




