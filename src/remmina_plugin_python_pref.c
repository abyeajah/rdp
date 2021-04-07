//
// Created by toolsdevler on 07.04.21.
//

#include "remmina_plugin_python_common.h"
#include "remmina_plugin_python_pref.h"

GPtrArray* plugin_map;

/**
 *
 */
void remmina_plugin_python_pref_init(void)
{
	plugin_map = g_ptr_array_new();
}

/**
 * @brief
 */
GtkWidget* remmina_plugin_python_pref_get_pref_body_wrapper(RemminaPrefPlugin* instance)
{
	PyPlugin* plugin = NULL;
	guint index = 0;
	for (int i = 0; i < plugin_map->len; ++i)
	{
		PyPlugin* tmp = (PyPlugin*)g_ptr_array_index(plugin_map, i);
		if (!tmp->generic_plugin || !tmp->generic_plugin->name)
			continue;
		if (g_str_equal(instance->name, tmp->generic_plugin->name))
		{
			plugin = tmp;
		}
	}

	if (!plugin)
	{
		g_printerr("[%s:%d]: No plugin named %s!\n", __FILE__, __LINE__, instance->name);
		return NULL;
	}

	PyObject* result = CallPythonMethod(plugin->instance, "entry_func", plugin->instance);
	if (result == Py_None)
	{
		return NULL;
	}

	return pygobject_get(result);
}

RemminaPlugin* remmina_plugin_python_create_pref_plugin(PyPlugin* plugin)
{
	PyObject* instance = plugin->instance;

	if (!remmina_plugin_python_check_attribute(instance, ATTR_NAME))
	{
		return NULL;
	}

	RemminaPrefPlugin* remmina_plugin = (RemminaPrefPlugin*)malloc(sizeof(RemminaPrefPlugin));

	remmina_plugin->type = REMMINA_PLUGIN_TYPE_PREF;
	remmina_plugin->domain = GETTEXT_PACKAGE;
	remmina_plugin->name = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_NAME));
	remmina_plugin->version = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_VERSION));
	remmina_plugin->description = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_DESCRIPTION));
	remmina_plugin->pref_label = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_PREF_LABEL));
	remmina_plugin->get_pref_body = remmina_plugin_python_pref_get_pref_body_wrapper;

	plugin->pref_plugin = remmina_plugin;
	plugin->generic_plugin = remmina_plugin;

	g_ptr_array_add(plugin_map, plugin);

	return remmina_plugin;
}
