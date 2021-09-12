//
// Created by toolsdevler on 07.04.21.
//
#include "remmina_plugin_python_common.h"
#include "remmina_plugin_python_tool.h"

GPtrArray* plugin_map;

/**
 *
 */
void remmina_plugin_python_tool_init(void)
{
	plugin_map = g_ptr_array_new();
}

void remmina_plugin_python_tool_exec_func_wrapper(RemminaToolPlugin* instance)
{
	PyPlugin* plugin = remmina_plugin_python_get_plugin(plugin_map, (RemminaPlugin*)instance);
	CallPythonMethod(plugin->instance, "exec_func", NULL);
}

RemminaPlugin* remmina_plugin_python_create_tool_plugin(PyPlugin* plugin)
{
	PyObject* instance = plugin->instance;

	if (!remmina_plugin_python_check_attribute(instance, ATTR_NAME))
	{
		return NULL;
	}

	RemminaToolPlugin* remmina_plugin = (RemminaToolPlugin*)malloc(sizeof(RemminaToolPlugin));

	remmina_plugin->type = REMMINA_PLUGIN_TYPE_TOOL;
	remmina_plugin->domain = GETTEXT_PACKAGE;
	remmina_plugin->name = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_NAME));
	remmina_plugin->version = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_VERSION));
	remmina_plugin->description = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_DESCRIPTION));
	remmina_plugin->exec_func = remmina_plugin_python_tool_exec_func_wrapper;

	plugin->tool_plugin = remmina_plugin;
	plugin->generic_plugin = (RemminaPlugin*)remmina_plugin;

	g_ptr_array_add(plugin_map, plugin);

	return (RemminaPlugin*)remmina_plugin;
}
