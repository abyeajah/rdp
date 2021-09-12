//
// Created by toolsdevler on 07.04.21.
//

#include "remmina_plugin_python_common.h"
#include "remmina_plugin_python_secret.h"
#include "remmina_plugin_python_remmina_file.h"

GPtrArray* plugin_map;

/**
 *
 */
void remmina_plugin_python_secret_init(void)
{
	TRACE_CALL(__func__);
	plugin_map = g_ptr_array_new();
}

gboolean remmina_plugin_python_secret_init_wrapper(RemminaSecretPlugin* instance)
{
	TRACE_CALL(__func__);
	PyPlugin* plugin = remmina_plugin_python_get_plugin(plugin_map, (RemminaPlugin*)instance);
	PyObject* result = CallPythonMethod(plugin->instance, "init", NULL);
	return result == Py_None || result != Py_False;
}

gboolean remmina_plugin_python_secret_is_service_available_wrapper(RemminaSecretPlugin* instance)
{
	TRACE_CALL(__func__);
	PyPlugin* plugin = remmina_plugin_python_get_plugin(plugin_map, (RemminaPlugin*)instance);
	PyObject* result = CallPythonMethod(plugin->instance, "is_service_available", NULL);
	return result == Py_None || result != Py_False;
}

void
remmina_plugin_python_secret_store_password_wrapper(RemminaSecretPlugin* instance, RemminaFile* file, const gchar* key, const gchar* password)
{
	TRACE_CALL(__func__);
	PyPlugin* plugin = remmina_plugin_python_get_plugin(plugin_map, (RemminaPlugin*)instance);
	CallPythonMethod(plugin->instance, "store_password", "Oss", (PyObject*)remmina_plugin_python_remmina_file_to_python(file), key, password);
}

gchar*
remmina_plugin_python_secret_get_password_wrapper(RemminaSecretPlugin* instance, RemminaFile* file, const gchar* key)
{
	TRACE_CALL(__func__);
	PyPlugin* plugin = remmina_plugin_python_get_plugin(plugin_map, (RemminaPlugin*)instance);
	PyObject* result = CallPythonMethod(plugin->instance, "get_password", "Os", (PyObject*)remmina_plugin_python_remmina_file_to_python(file), key);
	Py_ssize_t len = PyUnicode_GetLength(result);
	if (len == 0)
	{
		return NULL;
	}
	else
	{
		return remmina_plugin_python_copy_string_from_python(result, len);
	}
}

void
remmina_plugin_python_secret_delete_password_wrapper(RemminaSecretPlugin* instance, RemminaFile* file, const gchar* key)
{
	TRACE_CALL(__func__);
	PyPlugin* plugin = remmina_plugin_python_get_plugin(plugin_map, (RemminaPlugin*)instance);
	CallPythonMethod(plugin->instance, "delete_password", "Os", (PyObject*)remmina_plugin_python_remmina_file_to_python(file), key);
}

RemminaPlugin* remmina_plugin_python_create_secret_plugin(PyPlugin* plugin)
{
	TRACE_CALL(__func__);
	PyObject* instance = plugin->instance;

	if (!remmina_plugin_python_check_attribute(instance, ATTR_NAME))
	{
		return NULL;
	}

	RemminaSecretPlugin* remmina_plugin = (RemminaSecretPlugin*)malloc(sizeof(RemminaSecretPlugin));

	remmina_plugin->type = REMMINA_PLUGIN_TYPE_SECRET;
	remmina_plugin->domain = GETTEXT_PACKAGE;
	remmina_plugin->name = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_NAME));
	remmina_plugin->version = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_VERSION));
	remmina_plugin->description = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_DESCRIPTION));
	remmina_plugin->init_order = PyLong_AsLong(PyObject_GetAttrString(instance, ATTR_INIT_ORDER));

	remmina_plugin->init = remmina_plugin_python_secret_init_wrapper;
	remmina_plugin->is_service_available = remmina_plugin_python_secret_is_service_available_wrapper;
	remmina_plugin->store_password = remmina_plugin_python_secret_store_password_wrapper;
	remmina_plugin->get_password = remmina_plugin_python_secret_get_password_wrapper;
	remmina_plugin->delete_password = remmina_plugin_python_secret_delete_password_wrapper;

	plugin->secret_plugin = remmina_plugin;
	plugin->generic_plugin = (RemminaPlugin*)remmina_plugin;

	g_ptr_array_add(plugin_map, plugin);

	return (RemminaPlugin*)remmina_plugin;
}