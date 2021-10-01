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

/**
 * @file remmina_plugin_python_common.c
 * @brief
 * @author Mathias Winterhalter
 * @date 07.04.2021
 */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// I N C L U D E S
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "remmina_plugin_python_common.h"
#include "remmina_plugin_python_protocol.h"
#include "remmina_plugin_python_remmina.h"

#include "remmina_protocol_widget.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// D E C L A R A T I O N S
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GPtrArray* plugin_map;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// A P I
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void remmina_plugin_python_protocol_init(void)
{
	plugin_map = g_ptr_array_new();
}

void remmina_protocol_init_wrapper(RemminaProtocolWidget* gp)
{
	TRACE_CALL(__func__);
	PyPlugin* py_plugin = remmina_plugin_python_module_get_plugin(gp);
	py_plugin->gp->gp = gp;
	CallPythonMethod(py_plugin->instance, "init", "O", py_plugin->gp);
}

gboolean remmina_protocol_open_connection_wrapper(RemminaProtocolWidget* gp)
{
	TRACE_CALL(__func__);
	PyPlugin* py_plugin = remmina_plugin_python_module_get_plugin(gp);
	if (py_plugin)
	{
		PyObject* result = CallPythonMethod(py_plugin->instance, "open_connection", "O", py_plugin->gp);
		return result == Py_True;
	}
	else
	{
		return gtk_false();
	}
}

gboolean remmina_protocol_close_connection_wrapper(RemminaProtocolWidget* gp)
{
	TRACE_CALL(__func__);
	PyPlugin* py_plugin = remmina_plugin_python_module_get_plugin(gp);
	PyObject* result = CallPythonMethod(py_plugin->instance, "close_connection", "O", py_plugin->gp);
	return result == Py_True;
}

gboolean remmina_protocol_query_feature_wrapper(RemminaProtocolWidget* gp,
	const RemminaProtocolFeature* feature)
{
	TRACE_CALL(__func__);
	PyPlugin* py_plugin = remmina_plugin_python_module_get_plugin(gp);
	PyObject* result = CallPythonMethod(py_plugin->instance, "query_feature", "O", py_plugin->gp);
	return result == Py_True;
}

void remmina_protocol_call_feature_wrapper(RemminaProtocolWidget* gp, const RemminaProtocolFeature* feature)
{
	TRACE_CALL(__func__);
	PyPlugin* py_plugin = remmina_plugin_python_module_get_plugin(gp);
	PyObject* result = CallPythonMethod(py_plugin->instance, "call_feature", "O", py_plugin->gp);
}

void remmina_protocol_send_keytrokes_wrapper(RemminaProtocolWidget* gp,
	const guint keystrokes[],
	const gint keylen)
{
	TRACE_CALL(__func__);
	PyPlugin* py_plugin = remmina_plugin_python_module_get_plugin(gp);
	PyObject* result = CallPythonMethod(py_plugin->instance, "send_keystrokes", "O", py_plugin->gp);
}

gboolean remmina_protocol_get_plugin_screenshot_wrapper(RemminaProtocolWidget* gp,
	RemminaPluginScreenshotData* rpsd)
{
	TRACE_CALL(__func__);
	PyPlugin* py_plugin = remmina_plugin_python_module_get_plugin(gp);
	PyObject* result = CallPythonMethod(py_plugin->instance, "get_plugin_screenshot", "O", py_plugin->gp);
	return result == Py_True;
}

gboolean remmina_protocol_map_event_wrapper(RemminaProtocolWidget* gp)
{
	PyPlugin* plugin = remmina_plugin_python_module_get_plugin(gp);
	PyObject* result = CallPythonMethod(plugin->instance, "map_event", "O", plugin->gp);
	return PyBool_Check(result) && result == Py_True;
}

gboolean remmina_protocol_unmap_event_wrapper(RemminaProtocolWidget* gp)
{
	PyPlugin* plugin = remmina_plugin_python_module_get_plugin(gp);
	PyObject* result = CallPythonMethod(plugin->instance, "unmap_event", "O", plugin->gp);
	return PyBool_Check(result) && result == Py_True;
}

RemminaPlugin* remmina_plugin_python_create_protocol_plugin(PyPlugin* plugin)
{
	PyObject* instance = plugin->instance;

	if (!remmina_plugin_python_check_attribute(instance, ATTR_ICON_NAME_SSH)
		|| !remmina_plugin_python_check_attribute(instance, ATTR_ICON_NAME)
		|| !remmina_plugin_python_check_attribute(instance, ATTR_FEATURES)
		|| !remmina_plugin_python_check_attribute(instance, ATTR_BASIC_SETTINGS)
		|| !remmina_plugin_python_check_attribute(instance, ATTR_ADVANCED_SETTINGS)
		|| !remmina_plugin_python_check_attribute(instance, ATTR_SSH_SETTING))
	{
		return NULL;
	}

	RemminaProtocolPlugin* remmina_plugin = (RemminaProtocolPlugin*)malloc(sizeof(RemminaProtocolPlugin));

	remmina_plugin->type = REMMINA_PLUGIN_TYPE_PROTOCOL;
	remmina_plugin->domain = GETTEXT_PACKAGE;
	remmina_plugin->basic_settings = NULL;
	remmina_plugin->advanced_settings = NULL;
	remmina_plugin->features = NULL;

	remmina_plugin->name = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_NAME));
	remmina_plugin->description = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_DESCRIPTION));
	remmina_plugin->version = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_VERSION));
	remmina_plugin->icon_name = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_ICON_NAME));
	remmina_plugin->icon_name_ssh = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_ICON_NAME_SSH));

	PyObject* list = PyObject_GetAttrString(instance, "basic_settings");
	Py_ssize_t len = PyList_Size(list);
	if (len)
	{
		RemminaProtocolSetting* basic_settings = (RemminaProtocolSetting*)malloc(sizeof(RemminaProtocolSetting) * len);
		memset(&basic_settings[len], 0, sizeof(RemminaProtocolSetting));

		for (Py_ssize_t i = 0; i < len; ++i)
		{
			RemminaProtocolSetting* dest = basic_settings + i;
			remmina_plugin_python_to_protocol_setting(dest, PyList_GetItem(list, i));
		}

		remmina_plugin->basic_settings = basic_settings;
	}

	list = PyObject_GetAttrString(instance, "advanced_settings");
	len = PyList_Size(list);
	if (len)
	{
		RemminaProtocolSetting* advanced_settings = (RemminaProtocolSetting*)malloc(
			sizeof(RemminaProtocolSetting) * (len + 1));
		memset(&advanced_settings[len], 0, sizeof(RemminaProtocolSetting));

		for (Py_ssize_t i = 0; i < len; ++i)
		{
			RemminaProtocolSetting* dest = advanced_settings + i;
			remmina_plugin_python_to_protocol_setting(dest, PyList_GetItem(list, i));
		}

		remmina_plugin->advanced_settings = advanced_settings;
	}

	list = PyObject_GetAttrString(instance, "features");
	len = PyList_Size(list);
	if (len)
	{
		RemminaProtocolFeature* features = (RemminaProtocolFeature*)malloc(sizeof(RemminaProtocolFeature) * (len + 1));
		memset(&features[len], 0, sizeof(RemminaProtocolFeature));

		for (Py_ssize_t i = 0; i < len; ++i)
		{
			RemminaProtocolFeature* dest = features + i;
			remmina_plugin_python_to_protocol_feature(dest, PyList_GetItem(list, i));
		}

		remmina_plugin->features = features;
	}

	remmina_plugin->ssh_setting = (RemminaProtocolSSHSetting)remmina_plugin_python_get_attribute_long(instance,
		ATTR_SSH_SETTING,
		REMMINA_PROTOCOL_SSH_SETTING_NONE);

	remmina_plugin->init = remmina_protocol_init_wrapper;                             // Plugin initialization
	remmina_plugin->open_connection = remmina_protocol_open_connection_wrapper;       // Plugin open connection
	remmina_plugin->close_connection = remmina_protocol_close_connection_wrapper;     // Plugin close connection
	remmina_plugin->query_feature = remmina_protocol_query_feature_wrapper;           // Query for available features
	remmina_plugin->call_feature = remmina_protocol_call_feature_wrapper;             // Call a feature
	remmina_plugin->send_keystrokes =
		remmina_protocol_send_keytrokes_wrapper;                                      // Send a keystroke
	remmina_plugin->get_plugin_screenshot =
		remmina_protocol_get_plugin_screenshot_wrapper;                               // Screenshot support unavailable

	remmina_plugin->map_event = remmina_protocol_map_event_wrapper;
	remmina_plugin->unmap_event = remmina_protocol_unmap_event_wrapper;

	plugin->protocol_plugin = remmina_plugin;
	plugin->generic_plugin = (RemminaPlugin*)remmina_plugin;

	g_ptr_array_add(plugin_map, plugin);

	return (RemminaPlugin*)remmina_plugin;
}

PyPlugin* remmina_plugin_python_module_get_plugin(RemminaProtocolWidget* gp)
{
	static PyPlugin* cached_plugin = NULL;
	static RemminaProtocolWidget* cached_widget = NULL;

	if (gp == cached_widget)
	{
		return cached_plugin;
	}

	for (gint i = 0; i < plugin_map->len; ++i)
	{
		PyPlugin* plugin = (PyPlugin*)g_ptr_array_index(plugin_map, i);
		if (!plugin->generic_plugin || !plugin->generic_plugin->name)
			continue;
		if (g_str_equal(gp->plugin->name, plugin->generic_plugin->name))
		{
			cached_plugin = plugin;
			cached_widget = gp;
			return plugin;
		}
	}

	g_printerr("[%s:%d]: No plugin named %s!\n", __FILE__, __LINE__, gp->plugin->name);

	return NULL;
}
