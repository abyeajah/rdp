//
// Created by toolsdevler on 07.04.21.
//

#include "remmina_plugin_python_common.h"
#include "remmina_plugin_python_secret.h"

gboolean remmina_plugin_python_secret_init_wrapper(RemminaSecretPlugin* instance)
{
	return FALSE;
}

gboolean remmina_plugin_python_secret_is_service_available_wrapper(RemminaSecretPlugin* instance)
{
	return FALSE;
}

void remmina_plugin_python_secret_store_password_wrapper(RemminaSecretPlugin* instance, RemminaFile* file, const gchar* key, const gchar* password)
{

}

gchar* remmina_plugin_python_secret_get_password_wrapper(RemminaSecretPlugin* instance, RemminaFile* file, const gchar* key)
{
	return NULL;
}

void remmina_plugin_python_secret_delete_password_wrapper(RemminaSecretPlugin* instance, RemminaFile* file, const gchar* key)
{

}
