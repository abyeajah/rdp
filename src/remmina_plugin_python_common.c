/*
 * Remmina - The GTK+ Remote Desktop Client
 * Copyright (C) 2014-2021 Antenore Gatta, Giovanni Panozzo, Mathias Winterhalter (ToolsDevler)
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
 */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// I N C L U D E S
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "remmina_plugin_python_common.h"

#include <assert.h>
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// D E C L A R A T I O N S
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * A cache to store the last result that has been returned by the Python code using CallPythonMethod
 * (@see remmina_plugin_python_common.h)
 */
PyObject* __last_result;

static const gchar* MISSING_ATTR_ERROR_FMT = "Python plugin instance is missing member: %s\n";
static const gchar* MALLOC_RETURNED_NULL_ERROR_FMT = "Unable to allocate %d in memory!\n";
static const gchar* LOG_METHOD_CALL_FMT = "Python@%ld: %s.%s(...) -> %s\n";
static const gchar* PLUGIN_NOT_FOUND_FMT = "[%s:%d]: No plugin named %s!\n";

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

/**
 * To prevent any memory related attack or accidental allocation of an excessive amount of byes, this limit should
 * always be used to check for a sane amount of bytes to allocate.
 */
static const int REASONABLE_LIMIT_FOR_MALLOC = 1024 * 1024;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// A P I
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PyObject* remmina_plugin_python_last_result(void)
{
	TRACE_CALL(__func__);

	return __last_result;
}

PyObject* remmina_plugin_python_last_result_set(PyObject* last_result)
{
	TRACE_CALL(__func__);

	return __last_result = last_result;
}

gboolean remmina_plugin_python_check_error(void)
{
	TRACE_CALL(__func__);

	if (PyErr_Occurred())
	{
		PyErr_Print();
		return TRUE;
	}
	return FALSE;
}

void remmina_plugin_python_log_method_call(PyObject* instance, const gchar* method)
{
	TRACE_CALL(__func__);

	assert(instance);
	assert(method);
	g_print(LOG_METHOD_CALL_FMT, PyObject_Hash(instance), instance->ob_type->tp_name, method,
		PyObject_Str(remmina_plugin_python_last_result()));
}

long remmina_plugin_python_get_attribute_long(PyObject* instance, const gchar* attr_name, long def)
{
	TRACE_CALL(__func__);

	assert(instance);
	assert(attr_name);
	PyObject* attr = PyObject_GetAttrString(instance, attr_name);
	if (attr && PyLong_Check(attr))
	{
		return PyLong_AsLong(attr);
	}

	return def;
}

gboolean remmina_plugin_python_check_attribute(PyObject* instance, const gchar* attr_name)
{
	TRACE_CALL(__func__);

	assert(instance);
	assert(attr_name);
	if (PyObject_HasAttrString(instance, attr_name))
	{
		return TRUE;
	}

	g_printerr(MISSING_ATTR_ERROR_FMT, attr_name);
	return FALSE;
}

void* remmina_plugin_python_malloc(int bytes)
{
	TRACE_CALL(__func__);

	assert(bytes > 0);
	assert(bytes <= REASONABLE_LIMIT_FOR_MALLOC);
	void* result = malloc(bytes);

	if (!result)
	{
		g_printerr(MALLOC_RETURNED_NULL_ERROR_FMT, bytes);
		perror("malloc");
	}

	return result;
}

gchar* remmina_plugin_python_copy_string_from_python(PyObject* string, Py_ssize_t len)
{
	TRACE_CALL(__func__);

	gchar* result = NULL;
	if (len <= 0 || string == NULL)
	{
		return NULL;
	}

	const gchar* py_str = PyUnicode_AsUTF8(string);
	if (py_str)
	{
		const int label_size = sizeof(gchar) * (len + 1);
		result = (gchar*)remmina_plugin_python_malloc(label_size);
		result[len] = '\0';
		memcpy(result, py_str, len);
	}

	return result;
}

PyPlugin* remmina_plugin_python_get_plugin(GPtrArray* plugin_map, RemminaPlugin* instance)
{
	TRACE_CALL(__func__);

	assert(plugin_map);
	assert(instance);

	for (gint i = 0; i < plugin_map->len; ++i)
	{
		PyPlugin* plugin = (PyPlugin*)g_ptr_array_index(plugin_map, i);
		if (plugin->generic_plugin && plugin->generic_plugin->name
			&& g_str_equal(instance->name, plugin->generic_plugin->name))
		{
			return plugin;
		}
	}

	g_printerr(PLUGIN_NOT_FOUND_FMT, __FILE__, __LINE__, instance->name);

	return NULL;
}