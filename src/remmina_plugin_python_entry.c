#include "remmina_plugin_python_common.h"
#include "remmina_plugin_python_entry.h"

GPtrArray* plugin_map;

/**
 *
 */
void remmina_plugin_python_entry_init(void)
{
	plugin_map = g_ptr_array_new();
}

void remmina_plugin_python_entry_entry_func_wrapper(RemminaEntryPlugin* instance)
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

	if (!plugin) {
		g_printerr("[%s:%d]: No plugin named %s!\n", __FILE__, __LINE__, instance->name);
		return;
	}

	CallPythonMethod(plugin->instance, "entry_func", plugin->instance);
}

RemminaPlugin* remmina_plugin_python_create_entry_plugin(PyPlugin* plugin)
{
	PyObject* instance = plugin->instance;

	if (!remmina_plugin_python_check_attribute(instance, ATTR_NAME))
	{
		return NULL;
	}

	RemminaEntryPlugin* remmina_plugin = (RemminaEntryPlugin*)malloc(sizeof(RemminaEntryPlugin));

	remmina_plugin->type = REMMINA_PLUGIN_TYPE_ENTRY;
	remmina_plugin->domain = GETTEXT_PACKAGE;
	remmina_plugin->name = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_NAME));
	remmina_plugin->version = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_VERSION));
	remmina_plugin->description = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_DESCRIPTION));
	remmina_plugin->entry_func = remmina_plugin_python_entry_entry_func_wrapper;

	plugin->entry_plugin = remmina_plugin;
	plugin->generic_plugin = remmina_plugin;

	g_ptr_array_add(plugin_map, plugin);

	return remmina_plugin;
}
