//
// Created by toolsdevler on 07.04.21.
//

#include "remmina_plugin_python_common.h"
#include "remmina_plugin_python_protocol.h"
#include "remmina_plugin_python_remmina.h"

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
