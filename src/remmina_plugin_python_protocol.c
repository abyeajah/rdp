#include "remmina_plugin_python_common.h"
#include "remmina_plugin_python_protocol.h"
#include "remmina_plugin_python_remmina.h"

#include "remmina_protocol_widget.h"

GPtrArray* plugin_map;

/**
 *
 */
void remmina_plugin_python_protocol_init(void)
{
	plugin_map = g_ptr_array_new();
}

void remmina_protocol_init_wrapper(RemminaProtocolWidget* gp)
{
	TRACE_CALL(__func__);
	PyPlugin* py_plugin = remmina_plugin_python_module_get_plugin(gp);
	py_plugin->gp->gp = gp;
	CallPythonMethod(py_plugin->instance, "init", "O", py_plugin->gp);
}

gboolean remmina_protocol_open_connection_wrapper(RemminaProtocolWidget* gp)
{
	TRACE_CALL(__func__);
	PyPlugin* py_plugin = remmina_plugin_python_module_get_plugin(gp);
	if (py_plugin)
	{
		PyObject* result = CallPythonMethod(py_plugin->instance, "open_connection", "O", py_plugin->gp);
		return result == Py_True;
	}
	else
	{
		return gtk_false();
	}
}

gboolean remmina_protocol_close_connection_wrapper(RemminaProtocolWidget* gp)
{
	TRACE_CALL(__func__);
	PyPlugin* py_plugin = remmina_plugin_python_module_get_plugin(gp);
	PyObject* result = CallPythonMethod(py_plugin->instance, "close_connection", "O", py_plugin->gp);
	return result == Py_True;
}

gboolean remmina_protocol_query_feature_wrapper(RemminaProtocolWidget* gp,
	const RemminaProtocolFeature* feature)
{
	TRACE_CALL(__func__);
	PyPlugin* py_plugin = remmina_plugin_python_module_get_plugin(gp);
	PyObject* result = CallPythonMethod(py_plugin->instance, "query_feature", "O", py_plugin->gp);
	return result == Py_True;
}

void remmina_protocol_call_feature_wrapper(RemminaProtocolWidget* gp, const RemminaProtocolFeature* feature)
{
	TRACE_CALL(__func__);
	PyPlugin* py_plugin = remmina_plugin_python_module_get_plugin(gp);
	PyObject* result = CallPythonMethod(py_plugin->instance, "call_feature", "O", py_plugin->gp);
}

void remmina_protocol_send_keytrokes_wrapper(RemminaProtocolWidget* gp,
	const guint keystrokes[],
	const gint keylen)
{
	TRACE_CALL(__func__);
	PyPlugin* py_plugin = remmina_plugin_python_module_get_plugin(gp);
	PyObject* result = CallPythonMethod(py_plugin->instance, "send_keystrokes", "O", py_plugin->gp);
}

gboolean remmina_protocol_get_plugin_screenshot_wrapper(RemminaProtocolWidget* gp,
	RemminaPluginScreenshotData* rpsd)
{
	TRACE_CALL(__func__);
	PyPlugin* py_plugin = remmina_plugin_python_module_get_plugin(gp);
	PyObject* result = CallPythonMethod(py_plugin->instance, "get_plugin_screenshot", "O", py_plugin->gp);
	return result == Py_True;
}

RemminaPlugin* remmina_plugin_python_create_protocol_plugin(PyPlugin* plugin)
{
	PyObject* instance = plugin->instance;

	if (!remmina_plugin_python_check_attribute(instance, ATTR_ICON_NAME_SSH)
		|| !remmina_plugin_python_check_attribute(instance, ATTR_ICON_NAME)
		|| !remmina_plugin_python_check_attribute(instance, ATTR_FEATURES)
		|| !remmina_plugin_python_check_attribute(instance, ATTR_BASIC_SETTINGS)
		|| !remmina_plugin_python_check_attribute(instance, ATTR_ADVANCED_SETTINGS)
		|| !remmina_plugin_python_check_attribute(instance, ATTR_SSH_SETTING))
	{
		return NULL;
	}

	RemminaProtocolPlugin* remmina_plugin = (RemminaProtocolPlugin*)malloc(sizeof(RemminaProtocolPlugin));

	remmina_plugin->type = REMMINA_PLUGIN_TYPE_PROTOCOL;
	remmina_plugin->domain = GETTEXT_PACKAGE;
	remmina_plugin->basic_settings = NULL;
	remmina_plugin->advanced_settings = NULL;
	remmina_plugin->features = NULL;

	remmina_plugin->name = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_NAME));
	remmina_plugin->description = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_DESCRIPTION));
	remmina_plugin->version = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_VERSION));
	remmina_plugin->icon_name = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_ICON_NAME));
	remmina_plugin->icon_name_ssh = PyUnicode_AsUTF8(PyObject_GetAttrString(instance, ATTR_ICON_NAME_SSH));

	PyObject* list = PyObject_GetAttrString(instance, "basic_settings");
	Py_ssize_t len = PyList_Size(list);
	if (len)
	{
		remmina_plugin->basic_settings = (RemminaProtocolSetting*)malloc(sizeof(RemminaProtocolSetting) * len);
		memset(&remmina_plugin->basic_settings[len], 0, sizeof(RemminaProtocolSetting));

		for (Py_ssize_t i = 0; i < len; ++i)
		{
			RemminaProtocolSetting* dest = remmina_plugin->basic_settings + i;
			remmina_plugin_python_to_protocol_setting(dest, PyList_GetItem(list, i));
		}
	}

	list = PyObject_GetAttrString(instance, "advanced_settings");
	len = PyList_Size(list);
	if (len)
	{
		remmina_plugin->advanced_settings =
			(RemminaProtocolSetting*)malloc(sizeof(RemminaProtocolSetting) * (len + 1));
		memset(&remmina_plugin->advanced_settings[len], 0, sizeof(RemminaProtocolSetting));

		for (Py_ssize_t i = 0; i < len; ++i)
		{
			RemminaProtocolSetting* dest = remmina_plugin->advanced_settings + i;
			remmina_plugin_python_to_protocol_setting(dest, PyList_GetItem(list, i));
		}
	}

	list = PyObject_GetAttrString(instance, "features");
	len = PyList_Size(list);
	if (len)
	{
		remmina_plugin->features = (RemminaProtocolFeature*)malloc(sizeof(RemminaProtocolFeature) * (len + 1));
		memset(&remmina_plugin->features[len], 0, sizeof(RemminaProtocolFeature));

		for (Py_ssize_t i = 0; i < len; ++i)
		{
			RemminaProtocolFeature* dest = remmina_plugin->features + i;
			remmina_plugin_python_to_protocol_feature(dest, PyList_GetItem(list, i));
		}
	}

	remmina_plugin->ssh_setting = (RemminaProtocolSSHSetting)remmina_plugin_python_get_attribute_long(instance,
		ATTR_SSH_SETTING,
		REMMINA_PROTOCOL_SSH_SETTING_NONE);

	remmina_plugin->init = remmina_protocol_init_wrapper;                             // Plugin initialization
	remmina_plugin->open_connection = remmina_protocol_open_connection_wrapper;       // Plugin open connection
	remmina_plugin->close_connection = remmina_protocol_close_connection_wrapper;     // Plugin close connection
	remmina_plugin->query_feature = remmina_protocol_query_feature_wrapper;           // Query for available features
	remmina_plugin->call_feature = remmina_protocol_call_feature_wrapper;             // Call a feature
	remmina_plugin->send_keystrokes =
		remmina_protocol_send_keytrokes_wrapper;                                      // Send a keystroke
	remmina_plugin->get_plugin_screenshot =
		remmina_protocol_get_plugin_screenshot_wrapper;                               // Screenshot support unavailable


	plugin->protocol_plugin = remmina_plugin;
	plugin->generic_plugin = remmina_plugin;

	g_ptr_array_add(plugin_map, plugin);

	return remmina_plugin;
}

PyPlugin* remmina_plugin_python_module_get_plugin(RemminaProtocolWidget* gp)
{
	static PyPlugin* cached_plugin = NULL;
	static RemminaProtocolWidget* cached_widget = NULL;
	if (gp == cached_widget)
	{
		return cached_plugin;
	}
	guint index = 0;
	for (int i = 0; i < plugin_map->len; ++i)
	{
		PyPlugin* plugin = (PyPlugin*)g_ptr_array_index(plugin_map, i);
		if (!plugin->generic_plugin || !plugin->generic_plugin->name)
			continue;
		if (g_str_equal(gp->plugin->name, plugin->generic_plugin->name))
		{
			cached_plugin = plugin;
			cached_widget = gp;
			return plugin;
		}
	}
	g_printerr("[%s:%d]: No plugin named %s!\n", __FILE__, __LINE__, gp->plugin->name);
	return NULL;
}
