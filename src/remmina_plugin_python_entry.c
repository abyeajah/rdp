/*
 * Remmina - The GTK+ Remote Desktop Client
 * Copyright (C) 2014-2021 Antenore Gatta, Giovanni Panozzo
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

/**
 * @file remmina_plugin_python_entry.c
 * @brief Contains the wiring of a Python pluing based on RemminaPluginProtocol.
 * @author Mathias Winterhalter
 * @date 07.04.2021
 */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// I N C L U D E S
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "remmina_plugin_python_common.h"
#include "remmina_plugin_python_entry.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// D E C L A R A T I O N S
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
GPtrArray* plugin_map;

static const gchar* CREATE_ENTRY_PLUGIN_ERROR_FMT = "Unable to create entry plugin. Aborting!\n";

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// A P I
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void remmina_plugin_python_entry_init(void)
{
	plugin_map = g_ptr_array_new();
}

void remmina_plugin_python_entry_entry_func_wrapper(RemminaEntryPlugin* instance)
{
	PyPlugin* plugin = NULL;
	guint index = 0;
	for (int i = 0; i < plugin_map->len; ++i)
	{
		PyPlugin* tmp = (PyPlugin*)g_ptr_array_index(plugin_map, i);
		if (!tmp->generic_plugin || !tmp->generic_plugin->name)
			continue;
		if (g_str_equal(instance->name, tmp->generic_plugin->name))
		{
			plugin = tmp;
		}
	}

	if (!plugin) {
		g_printerr("[%s:%d]: No plugin named %s!\n", __FILE__, __LINE__, instance->name);
		return;
	}

	CallPythonMethod(plugin->instance, "entry_func", NULL);
}

RemminaPlugin* remmina_plugin_python_create_entry_plugin(PyPlugin* plugin)
{
	PyObject* instance = plugin->instance;

	if (!remmina_plugin_python_check_attribute(instance, ATTR_NAME))
	{
		g_printerr(CREATE_ENTRY_PLUGIN_ERROR_FMT);
		return NULL;
	}

	RemminaEntryPlugin* remmina_plugin = (RemminaEntryPlugin*)remmina_plugin_python_malloc(sizeof(RemminaEntryPlugin));

	remmina_plugin->type = REMMINA_PLUGIN_TYPE_ENTRY;
	remmina_plugin->domain = GETTEXT_PACKAGE;
	remmina_plugin->name = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_NAME));
	remmina_plugin->version = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_VERSION));
	remmina_plugin->description = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_DESCRIPTION));
	remmina_plugin->entry_func = remmina_plugin_python_entry_entry_func_wrapper;

	plugin->entry_plugin = remmina_plugin;
	plugin->generic_plugin = remmina_plugin;

	g_ptr_array_add(plugin_map, plugin);

	return remmina_plugin;
}
