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
 * @file remmina_plugin_python_module.c
 * @brief Implementation of the Python module 'remmina'.
 * @author Mathias Winterhalter
 * @date 14.10.2020
 *
 * This file acts as a broker between Remmina and the Python plugins. It abstracts the communication flow
 * over the RemminaPluginService and redirects calls to the correct Python plugin. The PyRemminaProtocolWidget
 * takes care of providing the API inside the Python script.
 *
 * @see http://www.remmina.org/wp for more information.
 */

#include "config.h"
#include "remmina_plugin_python_common.h"
#include "remmina_plugin_manager.h"
#include "remmina/plugin.h"
#include "remmina_protocol_widget.h"
#include "remmina_plugin_python_remmina.h"

#include "remmina_plugin_python_protocol.h"
#include "remmina_plugin_python_entry.h"
#include "remmina_plugin_python_tool.h"
#include "remmina_plugin_python_file.h"
#include "remmina_plugin_python_secret.h"
#include "remmina_plugin_python_pref.h"

RemminaPlugin* remmina_plugin_python_create_protocol_plugin(PyObject* instance)
{
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
		remmina_plugin->basic_settings = (RemminaProtocolSetting*)malloc(sizeof(RemminaProtocolSetting) * len);
		memset(&remmina_plugin->basic_settings[len], 0, sizeof(RemminaProtocolSetting));

		for (Py_ssize_t i = 0; i < len; ++i)
		{
			RemminaProtocolSetting* dest = remmina_plugin->basic_settings + i;
			remmina_plugin_python_to_protocol_setting(dest, PyList_GetItem(list, i));
		}
	}

	list = PyObject_GetAttrString(instance, "advanced_settings");
	len = PyList_Size(list);
	if (len)
	{
		remmina_plugin->advanced_settings =
			(RemminaProtocolSetting*)malloc(sizeof(RemminaProtocolSetting) * (len + 1));
		memset(&remmina_plugin->advanced_settings[len], 0, sizeof(RemminaProtocolSetting));

		for (Py_ssize_t i = 0; i < len; ++i)
		{
			RemminaProtocolSetting* dest = remmina_plugin->advanced_settings + i;
			remmina_plugin_python_to_protocol_setting(dest, PyList_GetItem(list, i));
		}
	}

	list = PyObject_GetAttrString(instance, "features");
	len = PyList_Size(list);
	if (len)
	{
		remmina_plugin->features = (RemminaProtocolFeature*)malloc(sizeof(RemminaProtocolFeature) * (len + 1));
		memset(&remmina_plugin->features[len], 0, sizeof(RemminaProtocolFeature));

		for (Py_ssize_t i = 0; i < len; ++i)
		{
			RemminaProtocolFeature* dest = remmina_plugin->features + i;
			remmina_plugin_python_to_protocol_feature(dest, PyList_GetItem(list, i));
		}
	}

	remmina_plugin->ssh_setting = (RemminaProtocolSSHSetting)remmina_plugin_python_to_enum_or_default(instance,
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

	return remmina_plugin;
}

RemminaPlugin* remmina_plugin_python_create_entry_plugin(PyObject* instance)
{
	if (!remmina_plugin_python_check_attribute(instance, ATTR_NAME))
	{
		return NULL;
	}

	RemminaEntryPlugin* remmina_plugin = (RemminaEntryPlugin*)malloc(sizeof(RemminaEntryPlugin));

	remmina_plugin->type = REMMINA_PLUGIN_TYPE_ENTRY;
	remmina_plugin->domain = GETTEXT_PACKAGE;
	remmina_plugin->name = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_NAME));
	remmina_plugin->version = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_VERSION));
	remmina_plugin->description = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_DESCRIPTION));
	remmina_plugin->entry_func = remmina_plugin_python_entry_entry_func_wrapper;

	return remmina_plugin;
}
RemminaPlugin* remmina_plugin_python_create_file_plugin(PyObject* instance)
{
	typedef struct _RemminaFilePlugin
	{
		RemminaPluginType type;
		const gchar* name;
		const gchar* description;
		const gchar* domain;
		const gchar* version;

		gboolean (* import_test_func)(const gchar* from_file);
		RemminaFile* (* import_func)(const gchar* from_file);
		gboolean (* export_test_func)(RemminaFile* file);
		gboolean (* export_func)(RemminaFile* file, const gchar* to_file);
		const gchar* export_hints;
	} RemminaFilePlugin;
	if (!remmina_plugin_python_check_attribute(instance, ATTR_NAME))
	{
		return NULL;
	}

	RemminaFilePlugin* remmina_plugin = (RemminaFilePlugin*)malloc(sizeof(RemminaFilePlugin));

	remmina_plugin->type = REMMINA_PLUGIN_TYPE_ENTRY;
	remmina_plugin->domain = GETTEXT_PACKAGE;
	remmina_plugin->name = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_NAME));
	remmina_plugin->version = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_VERSION));
	remmina_plugin->description = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_DESCRIPTION));
	remmina_plugin->export_hints = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_EXPORT_HINTS));

	remmina_plugin->import_test_func = remmina_plugin_python_file_import_test_func_wrapper;
	remmina_plugin->import_func = remmina_plugin_python_file_import_func_wrapper;
	remmina_plugin->export_test_func = remmina_plugin_python_file_export_test_func_wrapper;
	remmina_plugin->export_func = remmina_plugin_python_file_export_func_wrapper;

	return remmina_plugin;
}
RemminaPlugin* remmina_plugin_python_create_tool_plugin(PyObject* instance)
{
	if (!remmina_plugin_python_check_attribute(instance, ATTR_NAME))
	{
		return NULL;
	}

	RemminaToolPlugin* remmina_plugin = (RemminaToolPlugin*)malloc(sizeof(RemminaToolPlugin));

	remmina_plugin->type = REMMINA_PLUGIN_TYPE_ENTRY;
	remmina_plugin->domain = GETTEXT_PACKAGE;
	remmina_plugin->name = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_NAME));
	remmina_plugin->version = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_VERSION));
	remmina_plugin->description = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_DESCRIPTION));
	remmina_plugin->exec_func = remmina_plugin_python_tool_exec_func_wrapper;

	return remmina_plugin;
}
RemminaPlugin* remmina_plugin_python_create_pref_plugin(PyObject* instance)
{
	if (!remmina_plugin_python_check_attribute(instance, ATTR_NAME))
	{
		return NULL;
	}

	RemminaPrefPlugin* remmina_plugin = (RemminaPrefPlugin*)malloc(sizeof(RemminaPrefPlugin));

	remmina_plugin->type = REMMINA_PLUGIN_TYPE_ENTRY;
	remmina_plugin->domain = GETTEXT_PACKAGE;
	remmina_plugin->name = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_NAME));
	remmina_plugin->version = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_VERSION));
	remmina_plugin->description = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_DESCRIPTION));
	remmina_plugin->pref_label = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_PREF_LABEL));
	remmina_plugin->get_pref_body = remmina_plugin_python_pref_get_pref_body_wrapper;

	return remmina_plugin;
}

RemminaPlugin* remmina_plugin_python_create_secret_plugin(PyObject* instance)
{
	if (!remmina_plugin_python_check_attribute(instance, ATTR_NAME))
	{
		return NULL;
	}

	RemminaSecretPlugin* remmina_plugin = (RemminaSecretPlugin*)malloc(sizeof(RemminaSecretPlugin));

	remmina_plugin->type = REMMINA_PLUGIN_TYPE_ENTRY;
	remmina_plugin->domain = GETTEXT_PACKAGE;
	remmina_plugin->name = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_NAME));
	remmina_plugin->version = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_VERSION));
	remmina_plugin->description = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_DESCRIPTION));
	remmina_plugin->init_order = PyLong_AsLong(PyObject_GetAttr(instance, ATTR_INIT_ORDER));

	remmina_plugin->init = remmina_plugin_python_secret_init_wrapper;
	remmina_plugin->is_service_available = remmina_plugin_python_secret_is_service_available_wrapper;
	remmina_plugin->store_password = remmina_plugin_python_secret_store_password_wrapper;
	remmina_plugin->get_password = remmina_plugin_python_secret_get_password_wrapper;
	remmina_plugin->delete_password = remmina_plugin_python_secret_delete_password_wrapper;

	return remmina_plugin;
}
