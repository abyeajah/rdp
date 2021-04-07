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

/**
 *
 */
gboolean remmina_plugin_python_check_mandatory_member(PyObject* instance, const gchar* member);

static PyObject* remmina_plugin_python_log_printf_wrapper(PyObject* self, PyObject* msg);
static PyObject* remmina_register_plugin_wrapper(PyObject* self, PyObject* plugin);
static PyObject* remmina_plugin_python_get_viewport(PyObject* self, PyObject* handle);
static PyObject* remmina_file_get_datadir_wrapper(PyObject* self, PyObject* plugin);
static PyObject* remmina_file_new_wrapper(PyObject* self, PyObject* plugin);
static PyObject* remmina_pref_set_value_wrapper(PyObject* self, PyObject* plugin);
static PyObject* remmina_pref_get_value_wrapper(PyObject* self, PyObject* plugin);
static PyObject* remmina_pref_get_scale_quality_wrapper(PyObject* self, PyObject* plugin);
static PyObject* remmina_pref_get_sshtunnel_port_wrapper(PyObject* self, PyObject* plugin);
static PyObject* remmina_pref_get_ssh_loglevel_wrapper(PyObject* self, PyObject* plugin);
static PyObject* remmina_pref_get_ssh_parseconfig_wrapper(PyObject* self, PyObject* plugin);
static PyObject* remmina_pref_keymap_get_keyval_wrapper(PyObject* self, PyObject* plugin);
static PyObject* remmina_log_print_wrapper(PyObject* self, PyObject* plugin);
static PyObject* remmina_log_printf_wrapper(PyObject* self, PyObject* plugin);
static PyObject* remmina_widget_pool_register_wrapper(PyObject* self, PyObject* plugin);
static PyObject* rcw_open_from_file_full_wrapper(PyObject* self, PyObject* plugin);
static PyObject* remmina_public_get_server_port_wrapper(PyObject* self, PyObject* plugin);
static PyObject* remmina_masterthread_exec_is_main_thread_wrapper(PyObject* self, PyObject* plugin);
static PyObject* remmina_gtksocket_available_wrapper(PyObject* self, PyObject* plugin);
static PyObject* remmina_protocol_widget_get_profile_remote_heigh_wrapper(PyObject* self, PyObject* plugin);
static PyObject* remmina_protocol_widget_get_profile_remote_width_wrapper(PyObject* self, PyObject* plugin);
static PyObject* remmina_plugin_python_show_dialog_wrapper(PyObject* self, PyObject* kwds, PyObject* args);
static PyObject* remmina_plugin_python_get_mainwindow_wrapper(PyObject* self, PyObject* args);

static PyMethodDef remmina_python_module_type_methods[] = {
	{ "register_plugin", remmina_register_plugin_wrapper, METH_O, NULL },
	{ "log_print", remmina_plugin_python_log_printf_wrapper, METH_VARARGS, NULL },
	{ "show_dialog", (PyCFunction)remmina_plugin_python_show_dialog_wrapper, METH_VARARGS | METH_KEYWORDS,
	  NULL },
	{ "get_main_window", remmina_plugin_python_get_mainwindow_wrapper, METH_NOARGS, NULL },
	{ "get_datadir", remmina_file_get_datadir_wrapper, METH_VARARGS, NULL },
	{ "file_new", remmina_file_new_wrapper, METH_VARARGS, NULL },

	{ "pref_set_value", remmina_pref_set_value_wrapper, METH_VARARGS, NULL },
	{ "pref_get_value", remmina_pref_get_value_wrapper, METH_VARARGS, NULL },
	{ "pref_get_scale_quality", remmina_pref_get_scale_quality_wrapper, METH_VARARGS, NULL },
	{ "pref_get_sshtunnel_port", remmina_pref_get_sshtunnel_port_wrapper, METH_VARARGS, NULL },
	{ "pref_get_ssh_loglevel", remmina_pref_get_ssh_loglevel_wrapper, METH_VARARGS, NULL },
	{ "pref_get_ssh_parseconfig", remmina_pref_get_ssh_parseconfig_wrapper, METH_VARARGS, NULL },
	{ "pref_keymap_get_keyval", remmina_pref_keymap_get_keyval_wrapper, METH_VARARGS, NULL },

	{ "widget_pool_register", remmina_widget_pool_register_wrapper, METH_VARARGS, NULL },

	{ "rcw_open_from_file_full", rcw_open_from_file_full_wrapper, METH_VARARGS, NULL },
	{ "public_get_server_port", remmina_public_get_server_port_wrapper, METH_VARARGS, NULL },
	{ "masterthread_exec_is_main_thread", remmina_masterthread_exec_is_main_thread_wrapper, METH_VARARGS, NULL },
	{ "gtksocket_available", remmina_gtksocket_available_wrapper, METH_VARARGS, NULL },
	{ "protocol_widget_get_profile_remote_width",
	  remmina_protocol_widget_get_profile_remote_width_wrapper, METH_VARARGS, NULL },
	{ "protocol_widget_get_profile_remote_heigh",
	  remmina_protocol_widget_get_profile_remote_heigh_wrapper, METH_VARARGS, NULL },
	{ NULL }  /* Sentinel */
};

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
 * @brief The definition of the Python module 'remmina'.
 */
static PyModuleDef remmina_python_module_type = {
	PyModuleDef_HEAD_INIT,
	.m_name = "remmina",
	.m_doc = "Remmina API.",
	.m_size = -1,
	.m_methods = remmina_python_module_type_methods
};

// -- Python Object -> Setting

static PyObject* python_protocol_setting_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
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

static int python_protocol_setting_init(PyRemminaProtocolSetting* self, PyObject* args, PyObject* kwds)
{
	static char* kwlist[] = { "type", "name", "label", "compact", "opt1", "opt2", NULL };
	PyObject* name, * label;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "|lOOpOO", kwlist, &self->settingType, &name, &label, &self
		->compact, &self->opt1, &self->opt2))
		return -1;

	Py_ssize_t len = PyUnicode_GetLength(label);
	if (len == 0)
	{
		self->label = "";
	}
	else
	{
		self->label = g_malloc(sizeof(char) * (len + 1));
		self->label[len] = 0;
		memcpy(self->label, PyUnicode_AsUTF8(label), len);
	}

	len = PyUnicode_GetLength(name);
	if (len == 0)
	{
		self->name = "";
	}
	else
	{
		self->name = g_malloc(sizeof(char) * (len + 1));
		self->name[len] = 0;
		memcpy(self->name, PyUnicode_AsUTF8(name), len);
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

static int python_protocol_feature_init(PyRemminaProtocolFeature* self, PyObject* args, PyObject* kwds)
{
	static char* kwlist[] = { "type", "id", "opt1", "opt2", "opt3", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "|llOOO", kwlist, &self->type, &self->id, &self->opt1, &self
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
 * @brief Is called from the Python engine when it initializes the 'remmina' module.
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
 * @brief Initializes all globals and registers the 'remmina' module in the Python engine.
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
	return Py_None;
}

static PyObject* remmina_file_new_wrapper(PyObject* self, PyObject* plugin)
{
	return Py_None;
}

static PyObject* remmina_pref_set_value_wrapper(PyObject* self, PyObject* plugin)
{
	return Py_None;
}

static PyObject* remmina_pref_get_value_wrapper(PyObject* self, PyObject* plugin)
{
	return Py_None;
}

static PyObject* remmina_pref_get_scale_quality_wrapper(PyObject* self, PyObject* plugin)
{
	return Py_None;
}

static PyObject* remmina_pref_get_sshtunnel_port_wrapper(PyObject* self, PyObject* plugin)
{
	return Py_None;
}

static PyObject* remmina_pref_get_ssh_loglevel_wrapper(PyObject* self, PyObject* plugin)
{
	return Py_None;
}

static PyObject* remmina_pref_get_ssh_parseconfig_wrapper(PyObject* self, PyObject* plugin)
{
	return Py_None;
}

static PyObject* remmina_pref_keymap_get_keyval_wrapper(PyObject* self, PyObject* plugin)
{
	return Py_None;
}

static PyObject* remmina_log_print_wrapper(PyObject* self, PyObject* plugin)
{
	return Py_None;
}

static PyObject* remmina_log_printf_wrapper(PyObject* self, PyObject* plugin)
{
	return Py_None;
}

static PyObject* remmina_widget_pool_register_wrapper(PyObject* self, PyObject* plugin)
{
	return Py_None;
}

static PyObject* rcw_open_from_file_full_wrapper(PyObject* self, PyObject* plugin)
{
	return Py_None;
}

static PyObject* remmina_public_get_server_port_wrapper(PyObject* self, PyObject* plugin)
{
	return Py_None;
}

static PyObject* remmina_masterthread_exec_is_main_thread_wrapper(PyObject* self, PyObject* plugin)
{
	return Py_None;
}

static PyObject* remmina_gtksocket_available_wrapper(PyObject* self, PyObject* plugin)
{
	return Py_None;
}

static PyObject* remmina_protocol_widget_get_profile_remote_heigh_wrapper(PyObject* self, PyObject* plugin)
{
	return Py_None;
}

static PyObject* remmina_protocol_widget_get_profile_remote_width_wrapper(PyObject* self, PyObject* plugin)
{
	return Py_None;
}

static gboolean remmina_plugin_equal(gconstpointer lhs, gconstpointer rhs)
{
	if (lhs && ((PyPlugin*)lhs)->generic_plugin && rhs)
		return g_str_equal(((PyPlugin*)lhs)->generic_plugin->name, ((gchar*)rhs));
	else
		return lhs == rhs;
}

static void to_generic(PyObject* field, gpointer* target)
{
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
			gchar* tmp = g_malloc(sizeof(char) * (len + 1));
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
	PyRemminaProtocolFeature* src = (PyRemminaProtocolFeature*)feature;
	Py_INCREF(feature);
	dest->id = src->id;
	dest->type = src->type;
	to_generic(src->opt1, &dest->opt1);
	to_generic(src->opt2, &dest->opt2);
	to_generic(src->opt3, &dest->opt3);
}

PyObject* remmina_plugin_python_show_dialog_wrapper(PyObject* self, PyObject* args, PyObject* kwds)
{
	static char* kwlist[] = { "type", "buttons", "message", NULL };
	GtkMessageType msgType;
	GtkButtonsType btnType;
	gchar* message;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "lls", kwlist, &msgType, &btnType, &message))
		return PyLong_FromLong(-1);

	remmina_main_show_dialog(msgType, btnType, message);

	return Py_None;
}

PyObject* remmina_plugin_python_get_mainwindow_wrapper(PyObject* self, PyObject* args)
{
	GtkWindow* result = remmina_main_get_window();

	if (!result)
		return Py_None;

	return pygobject_new((GObject*)result);
}
