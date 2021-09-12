//
// Created by toolsdevler on 07.04.21.
//

#include "remmina_plugin_python_common.h"
#include "remmina_plugin_python_file.h"
#include "remmina_plugin_python_remmina_file.h"
#include "remmina_file.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// D E C L A R A T I O N S
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GPtrArray* plugin_map;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// A P I
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void remmina_plugin_python_file_init(void)
{
	TRACE_CALL(__func__);

	plugin_map = g_ptr_array_new();
}

gboolean remmina_plugin_python_file_import_test_func_wrapper(RemminaFilePlugin* instance, const gchar* from_file)
{
	TRACE_CALL(__func__);

	PyObject* result = NULL;

	PyPlugin* plugin = remmina_plugin_python_get_plugin(plugin_map, (RemminaPlugin*)instance);

	if (plugin)
	{
		result = CallPythonMethod(plugin->instance, "import_test_func", "s", from_file);
	}

	return result == Py_None || result != Py_False;
}

RemminaFile* remmina_plugin_python_file_import_func_wrapper(RemminaFilePlugin* instance, const gchar* from_file)
{
	TRACE_CALL(__func__);

	PyObject* result = NULL;

	PyPlugin* plugin = remmina_plugin_python_get_plugin(plugin_map, (RemminaPlugin*)instance);
	if (plugin)
	{
		return NULL;
	}

	result = CallPythonMethod(plugin->instance, "import_func", "s", from_file);

	if (result == Py_None || result == Py_False)
	{
		return NULL;
	}

	return (RemminaFile*)result;
}

gboolean remmina_plugin_python_file_export_test_func_wrapper(RemminaFilePlugin* instance, RemminaFile* file)
{
	TRACE_CALL(__func__);

	PyObject* result = NULL;

	PyPlugin* plugin = remmina_plugin_python_get_plugin(plugin_map, (RemminaPlugin*)instance);
	if (plugin)
	{
		result = CallPythonMethod(plugin->instance,
			"export_test_func",
			"O",
			remmina_plugin_python_remmina_file_to_python(file));
	}

	return result == Py_None || result != Py_False;
}

gboolean
remmina_plugin_python_file_export_func_wrapper(RemminaFilePlugin* instance, RemminaFile* file, const gchar* to_file)
{
	TRACE_CALL(__func__);

	PyObject* result = NULL;

	PyPlugin* plugin = remmina_plugin_python_get_plugin(plugin_map, (RemminaPlugin*)instance);
	if (plugin)
	{
		result = CallPythonMethod(plugin->instance, "export_func", "s", to_file);
	}

	return result == Py_None || result != Py_False;
}

RemminaPlugin* remmina_plugin_python_create_file_plugin(PyPlugin* plugin)
{
	TRACE_CALL(__func__);

	PyObject* instance = plugin->instance;

	if (!remmina_plugin_python_check_attribute(instance, ATTR_NAME))
	{
		return NULL;
	}

	RemminaFilePlugin* remmina_plugin = (RemminaFilePlugin*)remmina_plugin_python_malloc(sizeof(RemminaFilePlugin));

	remmina_plugin->type = REMMINA_PLUGIN_TYPE_FILE;
	remmina_plugin->domain = GETTEXT_PACKAGE;
	remmina_plugin->name = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_NAME));
	remmina_plugin->version = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_VERSION));
	remmina_plugin->description = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_DESCRIPTION));
	remmina_plugin->export_hints = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_EXPORT_HINTS));

	remmina_plugin->import_test_func = remmina_plugin_python_file_import_test_func_wrapper;
	remmina_plugin->import_func = remmina_plugin_python_file_import_func_wrapper;
	remmina_plugin->export_test_func = remmina_plugin_python_file_export_test_func_wrapper;
	remmina_plugin->export_func = remmina_plugin_python_file_export_func_wrapper;

	plugin->file_plugin = remmina_plugin;
	plugin->generic_plugin = (RemminaPlugin*)remmina_plugin;

	g_ptr_array_add(plugin_map, plugin);

	return (RemminaPlugin*)remmina_plugin;
}
