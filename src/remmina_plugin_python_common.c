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
 * @file remmina_plugin_python_common.c
 * @brief Contains the implementation of common used functions regarding Python and Remmina.
 * @author Mathias Winterhalter
 * @date 07.04.2021
 */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// I N C L U D E S
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "remmina_plugin_python_common.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// D E C L A R A T I O N S
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PyObject* __last_result;

static const gchar* MISSING_ATTR_ERROR_FMT =
	"Error creating Remmina plugin. Python plugin instance is missing member: %s\n";

const gchar* ATTR_NAME = "name";
const gchar* ATTR_ICON_NAME = "icon_name";
const gchar* ATTR_DESCRIPTION = "description";
const gchar* ATTR_VERSION = "version";
const gchar* ATTR_ICON_NAME_SSH = "icon_name_ssh";
const gchar* ATTR_FEATURES = "features";
const gchar* ATTR_BASIC_SETTINGS = "basic_settings";
const gchar* ATTR_ADVANCED_SETTINGS = "advanced_settings";
const gchar* ATTR_SSH_SETTING = "ssh_setting";
const gchar* ATTR_EXPORT_HINTS = "export_hints";
const gchar* ATTR_PREF_LABEL = "pref_label";
const gchar* ATTR_INIT_ORDER = "init_order";

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// A P I
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PyObject* remmina_plugin_python_last_result()
{
	return __last_result;
}

PyObject* remmina_plugin_python_last_result_set(PyObject* last_result)
{
	return __last_result = last_result;
}

gboolean remmina_plugin_python_check_error()
{
	if (PyErr_Occurred())
	{
		PyErr_Print();
		return TRUE;
	}
	return FALSE;
}

void remmina_plugin_python_log_method_call(PyObject* instance, const gchar* method)
{
	g_print("Python@%ld: %s.%s(...) -> %s\n",
		PyObject_Hash(instance),
		instance->ob_type->tp_name,
		method,
		PyObject_Str(remmina_plugin_python_last_result()));

}

long remmina_plugin_python_get_attribute_long(PyObject* instance, gchar* attr_name, long def)
{
	PyObject* attr = PyObject_GetAttrString(instance, attr_name);
	if (attr && PyLong_Check(attr))
	{
		return PyLong_AsLong(attr);
	}
	else
	{
		return def;
	}
}

gboolean remmina_plugin_python_check_attribute(PyObject* instance, const gchar* attr_name)
{
	if (PyObject_HasAttrString(instance, attr_name))
	{
		return TRUE;
	}

	g_printerr(MISSING_ATTR_ERROR_FMT, attr_name);
	return FALSE;
}
