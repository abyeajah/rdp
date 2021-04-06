#include "remmina_plugin_python_common.h"

PyObject *__last_result;


static const gchar *MISSING_ATTR_ERROR_FMT =
	"Error creating Remmina plugin. Python plugin instance is missing member: %s\n";

const gchar* ATTR_NAME = "name";
const gchar* ATTR_ICON_NAME = "icon_name";
const gchar* ATTR_DESCRIPTION = "description";
const gchar* ATTR_VERSION = "version";
const gchar* ATTR_ICON_NAME_SSH = "icon_name_ssh";
const gchar* ATTR_FEATURES = "features";
const gchar* ATTR_BASIC_SETTINGS = "basic_settings";
const gchar* ATTR_ADVANCED_SETTINGS = "advanced_settings";
const gchar* ATTR_SSH_SETTING = "ssh_setting";
const gchar* ATTR_EXPORT_HINTS = "export_hints";
const gchar* ATTR_PREF_LABEL = "pref_label";
const gchar* ATTR_INIT_ORDER = "init_order";


PyObject *remmina_plugin_python_last_result()
{
    return __last_result;
}

PyObject *remmina_plugin_python_last_result_set(PyObject *last_result)
{
    return __last_result = last_result;
}

gboolean remmina_plugin_python_check_error()
{
    if (PyErr_Occurred()) {
        PyErr_Print();
        return TRUE;
    }
    return FALSE;
}

void remmina_plugin_python_log_method_call(PyObject *instance, const gchar *method)
{
    g_print("Python@%ld: %s.%s() -> %s\n",
            PyObject_Hash(instance),
            instance->ob_type->tp_name,
            method,
            PyObject_Str(remmina_plugin_python_last_result())); \

}

long remmina_plugin_python_to_enum_or_default(PyObject *instance, gchar *constant_name, long def) {
    PyObject *attr = PyObject_GetAttrString(instance, constant_name);
    if (attr && PyLong_Check(attr)) {
        return PyLong_AsLong(attr);
    } else {
        return def;
    }
}

gboolean remmina_plugin_python_check_attribute(PyObject* instance, const gchar* attr_name)
{
	if (PyObject_HasAttrString(instance, attr_name))
	{
		g_printerr(MISSING_ATTR_ERROR_FMT, attr_name);
		return FALSE;
	}

	return TRUE;
}
