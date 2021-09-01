//
// Created by toolsdevler on 07.04.21.
//

#include "remmina_plugin_python_common.h"
#include "remmina_plugin_python_file.h"
#include "remmina_plugin_python_remmina_file.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// D E C L A R A T I O N S
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GPtrArray* plugin_map;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// A P I
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
void remmina_plugin_python_file_init(void)
{
	plugin_map = g_ptr_array_new();
}

static PyPlugin* get_plugin(RemminaToolPlugin* instance)
{
	guint index = 0;
	for (int i = 0; i < plugin_map->len; ++i)
	{
		PyPlugin* plugin = (PyPlugin*)g_ptr_array_index(plugin_map, i);
		if (!plugin->generic_plugin || !plugin->generic_plugin->name)
			continue;
		if (g_str_equal(instance->name, plugin->generic_plugin->name))
		{
			return plugin;
		}
	}
	g_printerr("[%s:%d]: No plugin named %s!\n", __FILE__, __LINE__, instance->name);
	return NULL;
}

gboolean remmina_plugin_python_file_import_test_func_wrapper(RemminaFilePlugin* instance, const gchar* from_file)
{
	PyPlugin* plugin = get_plugin(instance);
	PyObject* result = CallPythonMethod(plugin->instance, "import_test_func", "s", from_file);
	return result == Py_None || result != Py_False;
}

RemminaFile* remmina_plugin_python_file_import_func_wrapper(RemminaFilePlugin* instance, const gchar* from_file)
{
	PyPlugin* plugin = get_plugin(instance);
	PyObject* result = CallPythonMethod(plugin->instance, "import_func", "s", from_file);
	return result == Py_None || result != Py_False;
}

gboolean remmina_plugin_python_file_export_test_func_wrapper(RemminaFilePlugin* instance, RemminaFile* file)
{
	PyPlugin* plugin = get_plugin(instance);
	PyObject* result = CallPythonMethod(plugin->instance, "export_test_func", "O", remmina_plugin_python_remmina_file_to_python(file));
	return result == Py_None || result != Py_False;
}

gboolean
remmina_plugin_python_file_export_func_wrapper(RemminaFilePlugin* instance, RemminaFile* file, const gchar* to_file)
{
	PyPlugin* plugin = get_plugin(instance);
	PyObject* result = CallPythonMethod(plugin->instance, "export_func", "s", to_file);
	return result == Py_None || result != Py_False;
}

RemminaPlugin* remmina_plugin_python_create_file_plugin(PyPlugin* plugin)
{
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
	plugin->generic_plugin = remmina_plugin;

	g_ptr_array_add(plugin_map, plugin);

	return remmina_plugin;
}


