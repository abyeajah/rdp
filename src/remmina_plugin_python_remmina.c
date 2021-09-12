/*
 * Remmina - The GTK+ Remote Desktop Client
 * Copyright (C) 2016-2021 Antenore Gatta, Giovanni Panozzo
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
#include "remmina_plugin_python_common.h"
#include "remmina_main.h"
#include "remmina_plugin_manager.h"
#include "remmina/plugin.h"
#include "remmina_protocol_widget.h"
#include "remmina_log.h"

#include "remmina_plugin_python_remmina.h"
#include "remmina_plugin_python_protocol_widget.h"

#include "remmina_plugin_python_entry.h"
#include "remmina_plugin_python_file.h"
#include "remmina_plugin_python_protocol.h"
#include "remmina_plugin_python_tool.h"
#include "remmina_plugin_python_secret.h"
#include "remmina_plugin_python_pref.h"

#include "remmina_pref.h"
#include "remmina_ssh.h"
#include "remmina_file_manager.h"
#include "remmina_widget_pool.h"
#include "remmina_public.h"
#include "remmina_masterthread_exec.h"

#include <string.h>

/**
 * @file 	remmina_plugin_python_remmina.c
 * @author 	Mathias Winterhalter
 *
 * @brief 	Contains the API used by the Python modules.
 *
 * @detail 	In contrast to the wrapper functions that exist in the plugin specialisation files (e.g.
 * 			remmina_plugin_python_protocol.c or remmina_plugin_python_entry.c), this file contains the API for the
 * 			communication in the direction, from Python to Remmina. This means, if in the Python plugin a function
 * 			is called, that is defined in Remmina, C code, at least in this file, is executed.
 */

/**
 * Util function to check if a specific member is define in a Python object.
 */
gboolean remmina_plugin_python_check_mandatory_member(PyObject* instance, const gchar* member);

static PyObject* remmina_plugin_python_log_printf_wrapper(PyObject* self, PyObject* msg);
static PyObject* remmina_register_plugin_wrapper(PyObject* self, PyObject* plugin);
static PyObject* remmina_plugin_python_get_viewport(PyObject* self, PyObject* handle);
static PyObject* remmina_file_get_datadir_wrapper(PyObject* self, PyObject* plugin);
static PyObject* remmina_file_new_wrapper(PyObject* self, PyObject *args, PyObject *kwargs);
static PyObject* remmina_pref_set_value_wrapper(PyObject* self, PyObject *args, PyObject *kwargs);
static PyObject* remmina_pref_get_value_wrapper(PyObject* self, PyObject *args, PyObject *kwargs);
static PyObject* remmina_pref_get_scale_quality_wrapper(PyObject* self, PyObject* plugin);
static PyObject* remmina_pref_get_sshtunnel_port_wrapper(PyObject* self, PyObject* plugin);
static PyObject* remmina_pref_get_ssh_loglevel_wrapper(PyObject* self, PyObject* plugin);
static PyObject* remmina_pref_get_ssh_parseconfig_wrapper(PyObject* self, PyObject* plugin);
static PyObject* remmina_pref_keymap_get_keyval_wrapper(PyObject* self, PyObject *args, PyObject *kwargs);
static PyObject* remmina_log_print_wrapper(PyObject* self, PyObject *args, PyObject *kwargs);
static PyObject* remmina_log_printf_wrapper(PyObject* self, PyObject *args, PyObject *kwargs);
static PyObject* remmina_widget_pool_register_wrapper(PyObject* self, PyObject *args, PyObject *kwargs);
static PyObject* rcw_open_from_file_full_wrapper(PyObject* self, PyObject *args, PyObject *kwargs);
static PyObject* remmina_public_get_server_port_wrapper(PyObject* self, PyObject *args, PyObject *kwargs);
static PyObject* remmina_masterthread_exec_is_main_thread_wrapper(PyObject* self, PyObject* plugin);
static PyObject* remmina_gtksocket_available_wrapper(PyObject* self, PyObject* plugin);
static PyObject* remmina_protocol_widget_get_profile_remote_height_wrapper(PyObject* self, PyObject *args, PyObject *kwargs);
static PyObject* remmina_protocol_widget_get_profile_remote_width_wrapper(PyObject* self, PyObject *args, PyObject *kwargs);
static PyObject* remmina_plugin_python_show_dialog_wrapper(PyObject* self, PyObject *args, PyObject *kwargs);
static PyObject* remmina_plugin_python_get_mainwindow_wrapper(PyObject* self, PyObject* args);

/**
 * Declares functions for the Remmina module. These functions can be called from Python and are wired to one of the
 * functions here in this file.
 */
static PyMethodDef remmina_python_module_type_methods[] = {
	/**
	 * The first function that need to be called from the plugin code, since it registers the Python class acting as
	 * a Remmina plugin. Without this call the plugin will not be recognized.
	 */
	{ "register_plugin", remmina_register_plugin_wrapper, METH_O, NULL },

	/**
	 * Prints a string into the Remmina log infrastructure.
	 */
	{ "log_print", remmina_plugin_python_log_printf_wrapper, METH_VARARGS, NULL },

	/**
	 * Shows a GTK+ dialog.
	 */
	{ "show_dialog", (PyCFunction)remmina_plugin_python_show_dialog_wrapper, METH_VARARGS | METH_KEYWORDS,
	  NULL },

	/**
	 * Returns the GTK+ object of the main window of Remmina.
	 */
	{ "get_main_window", remmina_plugin_python_get_mainwindow_wrapper, METH_NOARGS, NULL },

	/**
	 * Calls remmina_file_get_datadir and returns its result.
	 */
	{ "get_datadir", remmina_file_get_datadir_wrapper, METH_VARARGS, NULL },

	/**
	 * Calls remmina_file_new and returns its result.
	 */
	{ "file_new", remmina_file_new_wrapper, METH_VARARGS, NULL },

	/**
	 * Calls remmina_pref_set_value and returns its result.
	 */
	{ "pref_set_value", remmina_pref_set_value_wrapper, METH_VARARGS, NULL },

	/**
	 * Calls remmina_pref_get_value and returns its result.
	 */
	{ "pref_get_value", remmina_pref_get_value_wrapper, METH_VARARGS, NULL },

	/**
	 * Calls remmina_pref_get_scale_quality and returns its result.
	 */
	{ "pref_get_scale_quality", remmina_pref_get_scale_quality_wrapper, METH_VARARGS, NULL },

	/**
	 * Calls remmina_pref_get_sshtunnel_port and returns its result.
	 */
	{ "pref_get_sshtunnel_port", remmina_pref_get_sshtunnel_port_wrapper, METH_VARARGS, NULL },

	/**
	 * Calls remmina_pref_get_ssh_loglevel and returns its result.
	 */
	{ "pref_get_ssh_loglevel", remmina_pref_get_ssh_loglevel_wrapper, METH_VARARGS, NULL },

	/**
	 * Calls remmina_pref_get_ssh_parseconfig and returns its result.
	 */
	{ "pref_get_ssh_parseconfig", remmina_pref_get_ssh_parseconfig_wrapper, METH_VARARGS, NULL },

	/**
	 * Calls remmina_pref_keymap_get_keyval and returns its result.
	 */
	{ "pref_keymap_get_keyval", remmina_pref_keymap_get_keyval_wrapper, METH_VARARGS, NULL },

	/**
	 * Calls remmina_widget_pool_register and returns its result.
	 */
	{ "widget_pool_register", remmina_widget_pool_register_wrapper, METH_VARARGS, NULL },

	/**
	 * Calls rcw_open_from_file_full and returns its result.
	 */
	{ "rcw_open_from_file_full", rcw_open_from_file_full_wrapper, METH_VARARGS, NULL },

	/**
	 * Calls remmina_public_get_server_port and returns its result.
	 */
	{ "public_get_server_port", remmina_public_get_server_port_wrapper, METH_VARARGS, NULL },

	/**
	 * Calls remmina_masterthread_exec_is_main_thread and returns its result.
	 */
	{ "masterthread_exec_is_main_thread", remmina_masterthread_exec_is_main_thread_wrapper, METH_VARARGS, NULL },

	/**
	 * Calls remmina_gtksocket_available and returns its result.
	 */
	{ "gtksocket_available", remmina_gtksocket_available_wrapper, METH_VARARGS, NULL },

	/**
	 * Calls remmina_protocol_widget_get_profile_remote_width and returns its result.
	 */
	{ "protocol_widget_get_profile_remote_width",
	  remmina_protocol_widget_get_profile_remote_width_wrapper, METH_VARARGS, NULL },

	/**
	 * Calls remmina_protocol_widget_get_profile_remote_height and returns its result.
	 */
	{ "remmina_protocol_widget_get_profile_remote_height",
	  remmina_protocol_widget_get_profile_remote_height_wrapper, METH_VARARGS, NULL },

	/* Sentinel */
	{ NULL }
};

/**
 * Adapter struct to handle Remmina protocol settings.
 */
typedef struct
{
	PyObject_HEAD
	RemminaProtocolSettingType settingType;
	gchar* name;
	gchar* label;
	gboolean compact;
	PyObject* opt1;
	PyObject* opt2;
} PyRemminaProtocolSetting;

/**
 * The definition of the Python module 'remmina'.
 */
static PyModuleDef remmina_python_module_type = {
	PyModuleDef_HEAD_INIT,
	.m_name = "remmina",
	.m_doc = "Remmina API.",
	.m_size = -1,
	.m_methods = remmina_python_module_type_methods
};

// -- Python Object -> Setting

/**
 * Initializes the memory and the fields of the remmina.Setting Python type.
 * @details This function is callback for the Python engine.
 */
static PyObject* python_protocol_setting_new(PyTypeObject* type, PyObject* args, PyObject* kwargs)
{
	TRACE_CALL(__func__);
	PyRemminaProtocolSetting* self = (PyRemminaProtocolSetting*)type->tp_alloc(type, 0);

	if (!self)
	{
		return NULL;
	}

	self->name = "";
	self->label = "";
	self->compact = FALSE;
	self->opt1 = NULL;
	self->opt2 = NULL;
	self->settingType = 0;

	return (PyObject*)self;
}

/**
 * Constructor of the remmina.Setting Python type.
 * @details This function is callback for the Python engine.
 */
static int python_protocol_setting_init(PyRemminaProtocolSetting* self, PyObject* args, PyObject* kwargs)
{
	TRACE_CALL(__func__);
	static gchar* kwlist[] = { "type", "name", "label", "compact", "opt1", "opt2", NULL };
	PyObject* name;
	PyObject* label;
	
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|lOOpOO", kwlist,
		&self->settingType, &name, &label, &self->compact, &self->opt1, &self->opt2))
	{
		return -1;
	}

	Py_ssize_t len = PyUnicode_GetLength(label);
	if (len == 0)
	{
		self->label = "";
	}
	else
	{
		self->label = remmina_plugin_python_copy_string_from_python(label, len);
		if (!self->label)
		{
			g_printerr("Unable to extract label during initialization of Python settings module!\n");
			remmina_plugin_python_check_error();
		}
	}

	len = PyUnicode_GetLength(name);
	if (len == 0)
	{
		self->name = "";
	}
	else
	{
		self->name = remmina_plugin_python_copy_string_from_python(label, len);
		if (!self->name)
		{
			g_printerr("Unable to extract name during initialization of Python settings module!\n");
			remmina_plugin_python_check_error();
		}
	}

	return 0;
}

static PyMemberDef python_protocol_setting_type_members[] = {
	{ "settingType", offsetof(PyRemminaProtocolSetting, settingType), T_INT, 0, NULL },
	{ "name", offsetof(PyRemminaProtocolSetting, name), T_STRING, 0, NULL },
	{ "label", offsetof(PyRemminaProtocolSetting, label), T_STRING, 0, NULL },
	{ "compact", offsetof(PyRemminaProtocolSetting, compact), T_BOOL, 0, NULL },
	{ "opt1", offsetof(PyRemminaProtocolSetting, opt1), T_OBJECT, 0, NULL },
	{ "opt2", offsetof(PyRemminaProtocolSetting, opt2), T_OBJECT, 0, NULL },
	{ NULL }
};

static PyTypeObject python_protocol_setting_type = {
	PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name = "remmina.Setting",
	.tp_doc = "Remmina Setting information",
	.tp_basicsize = sizeof(PyRemminaProtocolSetting),
	.tp_itemsize = 0,
	.tp_flags = Py_TPFLAGS_DEFAULT,
	.tp_new = python_protocol_setting_new,
	.tp_init = (initproc)python_protocol_setting_init,
	.tp_members = python_protocol_setting_type_members
};

// -- Python Type -> Feature

typedef struct
{
	PyObject_HEAD
	RemminaProtocolFeatureType type;
	gint id;
	PyObject* opt1;
	PyObject* opt2;
	PyObject* opt3;
} PyRemminaProtocolFeature;

static PyMemberDef python_protocol_feature_members[] = {
	{ "type", offsetof(PyRemminaProtocolFeature, type), T_INT, 0, NULL },
	{ "id", offsetof(PyRemminaProtocolFeature, id), T_STRING, 0, NULL },
	{ "opt1", offsetof(PyRemminaProtocolFeature, opt1), T_OBJECT, 0, NULL },
	{ "opt2", offsetof(PyRemminaProtocolFeature, opt2), T_OBJECT, 0, NULL },
	{ "opt3", offsetof(PyRemminaProtocolFeature, opt3), T_OBJECT, 0, NULL },
	{ NULL }
};

static PyObject* python_protocol_feature_new(PyTypeObject* type, PyObject* kws, PyObject* args)
{
	TRACE_CALL(__func__);
	PyRemminaProtocolFeature* self;
	self = (PyRemminaProtocolFeature*)type->tp_alloc(type, 0);
	if (!self)
		return NULL;

	self->id = 0;
	self->type = 0;
	self->opt1 = NULL;
	self->opt2 = NULL;
	self->opt3 = NULL;

	return (PyObject*)self;
}

static int python_protocol_feature_init(PyRemminaProtocolFeature* self, PyObject* args, PyObject* kwargs)
{
	TRACE_CALL(__func__);
	static char* kwlist[] = { "type", "id", "opt1", "opt2", "opt3", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|llOOO", kwlist, &self->type, &self->id, &self->opt1, &self
		->opt2, &self->opt3))
		return -1;

	return 0;
}

static PyTypeObject python_protocol_feature_type = {
	PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name = "remmina.Feature",
	.tp_doc = "Remmina Setting information",
	.tp_basicsize = sizeof(PyRemminaProtocolFeature),
	.tp_itemsize = 0,
	.tp_flags = Py_TPFLAGS_DEFAULT,
	.tp_new = python_protocol_feature_new,
	.tp_init = (initproc)python_protocol_feature_init,
	.tp_members = python_protocol_feature_members
};

/**
 * Is called from the Python engine when it initializes the 'remmina' module.
 * @details This function is only called by the Python engine!
 */
static PyMODINIT_FUNC remmina_plugin_python_module_initialize(void)
{
	TRACE_CALL(__func__);

	if (PyType_Ready(&python_protocol_setting_type) < 0)
	{
		g_printerr("Error initializing remmina.Setting!\n");
		PyErr_Print();
		return NULL;
	}

	if (PyType_Ready(&python_protocol_feature_type) < 0)
	{
		g_printerr("Error initializing remmina.Feature!\n");
		PyErr_Print();
		return NULL;
	}

	remmina_plugin_python_protocol_widget_type_ready();

	PyObject* module = PyModule_Create(&remmina_python_module_type);
	if (!module)
	{
		g_printerr("Error creating module 'remmina'!\n");
		PyErr_Print();
		return NULL;
	}

	PyModule_AddIntConstant(module, "BUTTONS_CLOSE", (long)GTK_BUTTONS_CLOSE);
	PyModule_AddIntConstant(module, "BUTTONS_NONE", (long)GTK_BUTTONS_NONE);
	PyModule_AddIntConstant(module, "BUTTONS_OK", (long)GTK_BUTTONS_OK);
	PyModule_AddIntConstant(module, "BUTTONS_CLOSE", (long)GTK_BUTTONS_CLOSE);
	PyModule_AddIntConstant(module, "BUTTONS_CANCEL", (long)GTK_BUTTONS_CANCEL);
	PyModule_AddIntConstant(module, "BUTTONS_YES_NO", (long)GTK_BUTTONS_YES_NO);
	PyModule_AddIntConstant(module, "BUTTONS_OK_CANCEL", (long)GTK_BUTTONS_OK_CANCEL);

	PyModule_AddIntConstant(module, "MESSAGE_INFO", (long)GTK_MESSAGE_INFO);
	PyModule_AddIntConstant(module, "MESSAGE_WARNING", (long)GTK_MESSAGE_WARNING);
	PyModule_AddIntConstant(module, "MESSAGE_QUESTION", (long)GTK_MESSAGE_QUESTION);
	PyModule_AddIntConstant(module, "MESSAGE_ERROR", (long)GTK_MESSAGE_ERROR);
	PyModule_AddIntConstant(module, "MESSAGE_OTHER", (long)GTK_MESSAGE_OTHER);

	PyModule_AddIntConstant(module, "PROTOCOL_SETTING_TYPE_SERVER", (long)REMMINA_PROTOCOL_SETTING_TYPE_SERVER);
	PyModule_AddIntConstant(module, "PROTOCOL_SETTING_TYPE_PASSWORD", (long)REMMINA_PROTOCOL_SETTING_TYPE_PASSWORD);
	PyModule_AddIntConstant(module, "PROTOCOL_SETTING_TYPE_RESOLUTION", (long)REMMINA_PROTOCOL_SETTING_TYPE_RESOLUTION);
	PyModule_AddIntConstant(module, "PROTOCOL_SETTING_TYPE_KEYMAP", (long)REMMINA_PROTOCOL_SETTING_TYPE_KEYMAP);
	PyModule_AddIntConstant(module, "PROTOCOL_SETTING_TYPE_TEXT", (long)REMMINA_PROTOCOL_SETTING_TYPE_TEXT);
	PyModule_AddIntConstant(module, "PROTOCOL_SETTING_TYPE_SELECT", (long)REMMINA_PROTOCOL_SETTING_TYPE_SELECT);
	PyModule_AddIntConstant(module, "PROTOCOL_SETTING_TYPE_COMBO", (long)REMMINA_PROTOCOL_SETTING_TYPE_COMBO);
	PyModule_AddIntConstant(module, "PROTOCOL_SETTING_TYPE_CHECK", (long)REMMINA_PROTOCOL_SETTING_TYPE_CHECK);
	PyModule_AddIntConstant(module, "PROTOCOL_SETTING_TYPE_FILE", (long)REMMINA_PROTOCOL_SETTING_TYPE_FILE);
	PyModule_AddIntConstant(module, "PROTOCOL_SETTING_TYPE_FOLDER", (long)REMMINA_PROTOCOL_SETTING_TYPE_FOLDER);

	PyModule_AddIntConstant(module, "PROTOCOL_FEATURE_TYPE_PREF", (long)REMMINA_PROTOCOL_FEATURE_TYPE_PREF);
	PyModule_AddIntConstant(module, "PROTOCOL_FEATURE_TYPE_TOOL", (long)REMMINA_PROTOCOL_FEATURE_TYPE_TOOL);
	PyModule_AddIntConstant(module, "PROTOCOL_FEATURE_TYPE_UNFOCUS", (long)REMMINA_PROTOCOL_FEATURE_TYPE_UNFOCUS);
	PyModule_AddIntConstant(module, "PROTOCOL_FEATURE_TYPE_SCALE", (long)REMMINA_PROTOCOL_FEATURE_TYPE_SCALE);
	PyModule_AddIntConstant(module, "PROTOCOL_FEATURE_TYPE_DYNRESUPDATE", (long)REMMINA_PROTOCOL_FEATURE_TYPE_DYNRESUPDATE);
	PyModule_AddIntConstant(module, "PROTOCOL_FEATURE_TYPE_GTKSOCKET", (long)REMMINA_PROTOCOL_FEATURE_TYPE_GTKSOCKET);

	PyModule_AddIntConstant(module, "PROTOCOL_SSH_SETTING_NONE", (long)REMMINA_PROTOCOL_SSH_SETTING_NONE);
	PyModule_AddIntConstant(module, "PROTOCOL_SSH_SETTING_TUNNEL", (long)REMMINA_PROTOCOL_SSH_SETTING_TUNNEL);
	PyModule_AddIntConstant(module, "PROTOCOL_SSH_SETTING_SSH", (long)REMMINA_PROTOCOL_SSH_SETTING_SSH);
	PyModule_AddIntConstant(module, "PROTOCOL_SSH_SETTING_REVERSE_TUNNEL", (long)REMMINA_PROTOCOL_SSH_SETTING_REVERSE_TUNNEL);
	PyModule_AddIntConstant(module, "PROTOCOL_SSH_SETTING_SFTP", (long)REMMINA_PROTOCOL_SSH_SETTING_SFTP);

	PyModule_AddIntConstant(module, "PROTOCOL_FEATURE_PREF_RADIO", (long)REMMINA_PROTOCOL_FEATURE_PREF_RADIO);
	PyModule_AddIntConstant(module, "PROTOCOL_FEATURE_PREF_CHECK", (long)REMMINA_PROTOCOL_FEATURE_PREF_CHECK);

	if (PyModule_AddObject(module, "Setting", (PyObject*)&python_protocol_setting_type) < 0)
	{
		Py_DECREF(&python_protocol_setting_type);
		Py_DECREF(module);
		PyErr_Print();
		return NULL;
	}

	Py_INCREF(&python_protocol_feature_type);
	if (PyModule_AddObject(module, "Feature", (PyObject*)&python_protocol_feature_type) < 0)
	{
		Py_DECREF(&python_protocol_setting_type);
		Py_DECREF(&python_protocol_feature_type);
		Py_DECREF(module);
		PyErr_Print();
		return NULL;
	}

	return module;
}

/**
 * Initializes all globals and registers the 'remmina' module in the Python engine.
 * @details This
 */
void remmina_plugin_python_module_init(void)
{
	TRACE_CALL(__func__);
	if (PyImport_AppendInittab("remmina", remmina_plugin_python_module_initialize))
	{
		g_print("Error initializing remmina module for python!\n");
		PyErr_Print();
	}

	remmina_plugin_python_entry_init();
	remmina_plugin_python_protocol_init();
	remmina_plugin_python_tool_init();
	remmina_plugin_python_pref_init();
	remmina_plugin_python_secret_init();
	remmina_plugin_python_file_init();
}

gboolean remmina_plugin_python_check_mandatory_member(PyObject* instance, const gchar* member)
{
	TRACE_CALL(__func__);
	if (PyObject_HasAttrString(instance, member))
		return TRUE;

	g_printerr("Missing mandatory member in Python plugin instance: %s\n", member);
	return FALSE;
}

static PyObject* remmina_plugin_python_log_printf_wrapper(PyObject* self, PyObject* msg)
{
	TRACE_CALL(__func__);
	char* fmt = NULL;

	if (PyArg_ParseTuple(msg, "s", &fmt))
	{
		remmina_log_printf(fmt);
	}
	else
	{
		g_print("Failed to load.\n");
		PyErr_Print();
		return Py_None;
	}

	return Py_None;
}

static PyObject* remmina_register_plugin_wrapper(PyObject* self, PyObject* plugin_instance)
{
	TRACE_CALL(__func__);

	if (plugin_instance)
	{
		if (!remmina_plugin_python_check_mandatory_member(plugin_instance, "name")
			|| !remmina_plugin_python_check_mandatory_member(plugin_instance, "version"))
		{
			return NULL;
		}

		/* Protocol plugin definition and features */
		const gchar* pluginType = PyUnicode_AsUTF8(PyObject_GetAttrString(plugin_instance, "type"));

		RemminaPlugin* remmina_plugin = NULL;

		PyPlugin* plugin = (PyPlugin*)malloc(sizeof(PyPlugin));
		plugin->instance = plugin_instance;
		Py_INCREF(plugin_instance);
		plugin->protocol_plugin = NULL;
		plugin->entry_plugin = NULL;
		plugin->file_plugin = NULL;
		plugin->pref_plugin = NULL;
		plugin->secret_plugin = NULL;
		plugin->tool_plugin = NULL;
		g_print("New Python plugin registered: %ld\n", PyObject_Hash(plugin_instance));

		if (g_str_equal(pluginType, "protocol"))
		{
			remmina_plugin = remmina_plugin_python_create_protocol_plugin(plugin);
		}
		else if (g_str_equal(pluginType, "entry"))
		{
			remmina_plugin = remmina_plugin_python_create_entry_plugin(plugin);
		}
		else if (g_str_equal(pluginType, "file"))
		{
			remmina_plugin = remmina_plugin_python_create_file_plugin(plugin);
		}
		else if (g_str_equal(pluginType, "tool"))
		{
			remmina_plugin = remmina_plugin_python_create_tool_plugin(plugin);
		}
		else if (g_str_equal(pluginType, "pref"))
		{
			remmina_plugin = remmina_plugin_python_create_pref_plugin(plugin);
		}
		else if (g_str_equal(pluginType, "secret"))
		{
			remmina_plugin = remmina_plugin_python_create_secret_plugin(plugin);
		}
		else
		{
			g_printerr("Unknown plugin type: %s\n", pluginType);
		}

		if (remmina_plugin)
		{
			if (remmina_plugin->type == REMMINA_PLUGIN_TYPE_PROTOCOL)
			{
				plugin->gp = remmina_plugin_python_protocol_widget_create();
			}

			remmina_plugin_manager_service.register_plugin((RemminaPlugin*)remmina_plugin);
		}
	}

	return Py_None;
}

static PyObject* remmina_file_get_datadir_wrapper(PyObject* self, PyObject* plugin)
{
	TRACE_CALL(__func__);
	PyObject* result = Py_None;
	const gchar* datadir = remmina_file_get_datadir();
	
	if (datadir)
	{
		result = PyUnicode_FromFormat("%s", datadir);
	}

	remmina_plugin_python_check_error();
	return Py_None;
}

static PyObject* remmina_file_new_wrapper(PyObject* self, PyObject *args, PyObject *kwargs)
{
	PyObject* result = Py_None;
	RemminaFile* file = remmina_file_new();
	if (file)
	{
		result = (PyObject*)file;
	}
	
	remmina_plugin_python_check_error();
	return result;
}

static PyObject* remmina_pref_set_value_wrapper(PyObject* self, PyObject *args, PyObject *kwargs)
{
	TRACE_CALL(__func__);
	static char* kwlist[] = { "key", "value", NULL };
	gchar* key, value;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "ss", kwlist, &key, &value))
	{
		return Py_None;
	}

	if (key)
	{
		remmina_pref_set_value(key, value);
	}	

	remmina_plugin_python_check_error();
	return Py_None;
}

static PyObject* remmina_pref_get_value_wrapper(PyObject* self, PyObject *args, PyObject *kwargs)
{
	TRACE_CALL(__func__);
	static char* kwlist[] = { "key", NULL };
	gchar* key;
	PyObject* result = Py_None;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s", kwlist, &key))
	{
		return Py_None;
	}

	if (key)
	{
		const gchar* value = remmina_pref_get_value(key);
		if (value) {
			result = PyUnicode_FromFormat("%s", result);
		}
	}	

	remmina_plugin_python_check_error();
	return result;
}

static PyObject* remmina_pref_get_scale_quality_wrapper(PyObject* self, PyObject* plugin)
{
	TRACE_CALL(__func__);
	PyObject* result = PyLong_FromLong(remmina_pref_get_scale_quality());
	remmina_plugin_python_check_error();
	return result;
}

static PyObject* remmina_pref_get_sshtunnel_port_wrapper(PyObject* self, PyObject* plugin)
{
	TRACE_CALL(__func__);
	PyObject* result = PyLong_FromLong(remmina_pref_get_sshtunnel_port());
	remmina_plugin_python_check_error();
	return result;
}

static PyObject* remmina_pref_get_ssh_loglevel_wrapper(PyObject* self, PyObject* plugin)
{
	TRACE_CALL(__func__);
	PyObject* result = PyLong_FromLong(remmina_pref_get_ssh_loglevel());
	remmina_plugin_python_check_error();
	return result;
}

static PyObject* remmina_pref_get_ssh_parseconfig_wrapper(PyObject* self, PyObject* plugin)
{
	TRACE_CALL(__func__);
	PyObject* result = PyLong_FromLong(remmina_pref_get_ssh_parseconfig());
	remmina_plugin_python_check_error();
	return result;
}

static PyObject* remmina_pref_keymap_get_keyval_wrapper(PyObject* self, PyObject *args, PyObject *kwargs)
{
	TRACE_CALL(__func__);
	static char* kwlist[] = { "keymap", "keyval", NULL };
	gchar* keymap;
	guint keyval;
	PyObject* result = Py_None;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "sl", kwlist, &keymap, &keyval))
	{
		return PyLong_FromLong(-1);
	}

	if (keymap)
	{
		const guint value = remmina_pref_keymap_get_keyval(keymap, keyval);
		result = PyLong_FromUnsignedLong(value);
	}

	remmina_plugin_python_check_error();
	return result;
}

static PyObject* remmina_log_print_wrapper(PyObject* self, PyObject *args, PyObject *kwargs)
{
	TRACE_CALL(__func__);
	static char kwlist[] = { "text", NULL };
	gchar* text;

	if (PyArg_ParseTupleAndKeywords(args, kwargs, "s", &text) && text)
	{
		remmina_log_print(text);
	}

	return Py_None;
}

static PyObject* remmina_widget_pool_register_wrapper(PyObject* self, PyObject *args, PyObject *kwargs)
{
	TRACE_CALL(__func__);
	static char* kwlist[] = { "widget", NULL };
	PyObject* widget;

	if (PyArg_ParseTupleAndKeywords(args, kwargs, "O", &widget) && widget)
	{
		remmina_widget_pool_register((GtkWidget*)pygobject_get(widget));
	}

	return Py_None;
}

static PyObject* rcw_open_from_file_full_wrapper(PyObject* self, PyObject *args, PyObject *kwargs)
{
	TRACE_CALL(__func__);
	static char* kwlist[] = { "remminafile", "data", "handler", NULL };
	PyPlugin* remminafile;
	PyObject* data;
	guint handler;


	if (PyArg_ParseTupleAndKeywords(args, kwargs, "OOOl", &remminafile, &data, &handler) &&
		remminafile && remminafile->file_plugin && data)
	{
		rcw_open_from_file_full(remminafile->file_plugin, NULL, (void*)data, handler);
	}

	return Py_None;
}

static PyObject* remmina_public_get_server_port_wrapper(PyObject* self, PyObject *args, PyObject *kwargs)
{
	TRACE_CALL(__func__);
	static char* kwlist[] = { "server", "defaultport", "host", "port", NULL };
	gchar* server;
	gint defaultport;

	if (PyArg_ParseTupleAndKeywords(args, kwargs, "slsl", &server, &defaultport) && server)
	{
		gchar* host;
		gint port;
		remmina_public_get_server_port(server, defaultport, &host, &port);

		PyObject* result = PyList_New(2);
		PyList_Append(result, PyUnicode_FromString(host));
		PyList_Append(result, PyLong_FromLong(port));
		return PyList_AsTuple(result);
	}

	return Py_None;
}

static PyObject* remmina_masterthread_exec_is_main_thread_wrapper(PyObject* self, PyObject* plugin)
{
	TRACE_CALL(__func__);
	return PyBool_FromLong(remmina_masterthread_exec_is_main_thread());
}

static PyObject* remmina_gtksocket_available_wrapper(PyObject* self, PyObject* plugin)
{
	TRACE_CALL(__func__);
	return PyBool_FromLong(remmina_gtksocket_available());
}

static PyObject* remmina_protocol_widget_get_profile_remote_height_wrapper(PyObject* self, PyObject *args, PyObject *kwargs)
{
	TRACE_CALL(__func__);
	static char* kwlist[] = { "widget", NULL };
	PyPlugin* plugin;

	if (PyArg_ParseTupleAndKeywords(args, kwargs, "O", &plugin) && plugin && plugin->gp)
	{
		remmina_protocol_widget_get_profile_remote_height(plugin->gp);
	}

	return Py_None;
}

static PyObject* remmina_protocol_widget_get_profile_remote_width_wrapper(PyObject* self, PyObject *args, PyObject *kwargs)
{
	TRACE_CALL(__func__);
	static char* kwlist[] = { "widget", NULL };
	PyPlugin* plugin;

	if (PyArg_ParseTupleAndKeywords(args, kwargs, "O", &plugin) && plugin && plugin->gp)
	{
		remmina_protocol_widget_get_profile_remote_width(plugin->gp);
	}

	return Py_None;
}

static gboolean remmina_plugin_equal(gconstpointer lhs, gconstpointer rhs)
{
	TRACE_CALL(__func__);
	if (lhs && ((PyPlugin*)lhs)->generic_plugin && rhs)
	{
		return g_str_equal(((PyPlugin*)lhs)->generic_plugin->name, ((gchar*)rhs));
	}
	else
	{
		return lhs == rhs;
	}
}

static void to_generic(PyObject* field, gpointer* target)
{
	TRACE_CALL(__func__);
	if (!field || field == Py_None)
	{
		*target = NULL;
		return;
	}
	Py_INCREF(field);
	if (PyUnicode_Check(field))
	{
		Py_ssize_t len = PyUnicode_GetLength(field);

		if (len == 0)
		{
			*target = "";
		}
		else
		{
			gchar* tmp = remmina_plugin_python_malloc(sizeof(char) * (len + 1));
			*(tmp + len) = 0;
			memcpy(tmp, PyUnicode_AsUTF8(field), len);
			*target = tmp;
		}

	}
	else if (PyLong_Check(field))
	{
		*target = malloc(sizeof(long));
		long* long_target = (long*)target;
		*long_target = PyLong_AsLong(field);
	}
	else if (PyTuple_Check(field))
	{
		Py_ssize_t len = PyTuple_Size(field);
		if (len)
		{
			gpointer* dest = (gpointer*)malloc(sizeof(gpointer) * (len + 1));
			memset(dest, 0, sizeof(gpointer) * (len + 1));

			for (Py_ssize_t i = 0; i < len; ++i)
			{
				PyObject* item = PyTuple_GetItem(field, i);
				to_generic(item, dest + i);
			}

			*target = dest;
		}
	}
	Py_DECREF(field);
}

void remmina_plugin_python_to_protocol_setting(RemminaProtocolSetting* dest, PyObject* setting)
{
	TRACE_CALL(__func__);
	PyRemminaProtocolSetting* src = (PyRemminaProtocolSetting*)setting;
	Py_INCREF(setting);
	dest->name = src->name;
	dest->label = src->label;
	dest->compact = src->compact;
	dest->type = src->settingType;
	to_generic(src->opt1, &dest->opt1);
	to_generic(src->opt2, &dest->opt2);
}

void remmina_plugin_python_to_protocol_feature(RemminaProtocolFeature* dest, PyObject* feature)
{
	TRACE_CALL(__func__);
	PyRemminaProtocolFeature* src = (PyRemminaProtocolFeature*)feature;
	Py_INCREF(feature);
	dest->id = src->id;
	dest->type = src->type;
	to_generic(src->opt1, &dest->opt1);
	to_generic(src->opt2, &dest->opt2);
	to_generic(src->opt3, &dest->opt3);
}

PyObject* remmina_plugin_python_show_dialog_wrapper(PyObject* self, PyObject* args, PyObject* kwargs)
{
	TRACE_CALL(__func__);
	static char* kwlist[] = { "type", "buttons", "message", NULL };
	GtkMessageType msgType;
	GtkButtonsType btnType;
	gchar* message;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "lls", kwlist, &msgType, &btnType, &message))
	{
		return PyLong_FromLong(-1);
	}

	remmina_main_show_dialog(msgType, btnType, message);

	return Py_None;
}

PyObject* remmina_plugin_python_get_mainwindow_wrapper(PyObject* self, PyObject* args)
{
	TRACE_CALL(__func__);
	GtkWindow* result = remmina_main_get_window();

	if (!result)
	{
		return Py_None;
	}

	return pygobject_new((GObject*)result);
}
