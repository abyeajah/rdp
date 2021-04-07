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

void remmina_plugin_python_tool_exec_func_wrapper(RemminaToolPlugin* instance)
{
	PyPlugin* plugin = get_plugin(instance);
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
	plugin->generic_plugin = remmina_plugin;

	g_ptr_array_add(plugin_map, plugin);

	return remmina_plugin;
}
