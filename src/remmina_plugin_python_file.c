//
// Created by toolsdevler on 07.04.21.
//

#include "remmina_plugin_python_common.h"
#include "remmina_plugin_python_file.h"

gboolean remmina_plugin_python_file_import_test_func_wrapper(RemminaFilePlugin* instance, const gchar *from_file)
{
	return FALSE;
}

RemminaFile * remmina_plugin_python_file_import_func_wrapper(RemminaFilePlugin* instance, const gchar * from_file)
{
	return NULL;
}

gboolean remmina_plugin_python_file_export_test_func_wrapper(RemminaFilePlugin* instance, RemminaFile *file)
{
	return FALSE;
}

gboolean remmina_plugin_python_file_export_func_wrapper(RemminaFilePlugin* instance, RemminaFile *file, const gchar *to_file)
{
	return FALSE;
}

