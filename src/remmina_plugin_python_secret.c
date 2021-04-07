//
// Created by toolsdevler on 07.04.21.
//

#include "remmina_plugin_python_common.h"
#include "remmina_plugin_python_secret.h"

GPtrArray* plugin_map;

/**
 *
 */
void remmina_plugin_python_secret_init(void)
{
	plugin_map = g_ptr_array_new();
}

gboolean remmina_plugin_python_secret_init_wrapper(RemminaSecretPlugin* instance)
{

	return FALSE;
}

gboolean remmina_plugin_python_secret_is_service_available_wrapper(RemminaSecretPlugin* instance)
{
	return FALSE;
}

void
remmina_plugin_python_secret_store_password_wrapper(RemminaSecretPlugin* instance, RemminaFile* file, const gchar* key, const gchar* password)
{

}

gchar*
remmina_plugin_python_secret_get_password_wrapper(RemminaSecretPlugin* instance, RemminaFile* file, const gchar* key)
{
	return NULL;
}

void
remmina_plugin_python_secret_delete_password_wrapper(RemminaSecretPlugin* instance, RemminaFile* file, const gchar* key)
{

}

RemminaPlugin* remmina_plugin_python_create_secret_plugin(PyPlugin* plugin)
{
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
	remmina_plugin->init_order = PyLong_AsLong(PyObject_GetAttr(instance, ATTR_INIT_ORDER));

	remmina_plugin->init = remmina_plugin_python_secret_init_wrapper;
	remmina_plugin->is_service_available = remmina_plugin_python_secret_is_service_available_wrapper;
	remmina_plugin->store_password = remmina_plugin_python_secret_store_password_wrapper;
	remmina_plugin->get_password = remmina_plugin_python_secret_get_password_wrapper;
	remmina_plugin->delete_password = remmina_plugin_python_secret_delete_password_wrapper;

	plugin->secret_plugin = remmina_plugin;
	plugin->generic_plugin = remmina_plugin;

	g_ptr_array_add(plugin_map, plugin);

	return remmina_plugin;
}
