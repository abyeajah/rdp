//
// Created by toolsdevler on 07.04.21.
//

#include "remmina_plugin_python_common.h"
#include "remmina_plugin_python_file.h"

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

gboolean remmina_plugin_python_file_import_test_func_wrapper(RemminaFilePlugin* instance, const gchar* from_file)
{
	return FALSE;
}

RemminaFile* remmina_plugin_python_file_import_func_wrapper(RemminaFilePlugin* instance, const gchar* from_file)
{
	return NULL;
}

gboolean remmina_plugin_python_file_export_test_func_wrapper(RemminaFilePlugin* instance, RemminaFile* file)
{
	return FALSE;
}

gboolean
remmina_plugin_python_file_export_func_wrapper(RemminaFilePlugin* instance, RemminaFile* file, const gchar* to_file)
{
	return FALSE;
}

RemminaPlugin* remmina_plugin_python_create_file_plugin(PyPlugin* plugin)
{
	PyObject* instance = plugin->instance;

	if (!remmina_plugin_python_check_attribute(instance, ATTR_NAME))
	{
		return NULL;
	}

	RemminaFilePlugin* remmina_plugin = (RemminaFilePlugin*)malloc(sizeof(RemminaFilePlugin));

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
