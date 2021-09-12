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
	TRACE_CALL(__func__);

	plugin_map = g_ptr_array_new();
}

/**
 * @brief
 */
GtkWidget* remmina_plugin_python_pref_get_pref_body_wrapper(RemminaPrefPlugin* instance)
{
	TRACE_CALL(__func__);

	PyPlugin* plugin = remmina_plugin_python_get_plugin(plugin_map, (RemminaPlugin*)instance);

	PyObject* result = CallPythonMethod(plugin->instance, "entry_func", "O", plugin->instance);
	if (result == Py_None)
	{
		return NULL;
	}

	return (GtkWidget*)pygobject_get(result);
}

RemminaPlugin* remmina_plugin_python_create_pref_plugin(PyPlugin* plugin)
{
	TRACE_CALL(__func__);

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
	plugin->generic_plugin = (RemminaPlugin*)remmina_plugin;

	g_ptr_array_add(plugin_map, plugin);

	return (RemminaPlugin*)remmina_plugin;
}
