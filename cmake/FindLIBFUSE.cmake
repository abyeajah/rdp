# Remmina - The GTK+ Remote Desktop Client
#
# Copyright (C) 2016-2019 Antenore Gatta, Giovanni Panozzo
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, 
# Boston, MA  02110-1301, USA.

include(FindPackageHandleStandardArgs)

pkg_check_modules(PC_LIBFUSE fuse)

find_path(LIBFUSE_INCLUDE_DIR NAMES fuse.h
	HINTS ${PC_LIBFUSE_INCLUDE_DIRS})

find_library(LIBFUSE_LIBRARY NAMES fuse)

if (LIBFUSE_INCLUDE_DIR AND LIBFUSE_LIBRARY)
	find_package_handle_standard_args(LIBFUSE DEFAULT_MSG LIBFUSE_LIBRARY LIBFUSE_INCLUDE_DIR)
endif()

if (LIBFUSE_FOUND)
	add_definitions(-DHAVE_LIBFUSE)
	add_definitions(-D_FILE_OFFSET_BITS=64)
	set(LIBFUSE_LIBRARIES ${LIBFUSE_LIBRARY})
endif()

mark_as_advanced(LIBFUSE_INCLUDE_DIR LIBFUSE_LIBRARY)

