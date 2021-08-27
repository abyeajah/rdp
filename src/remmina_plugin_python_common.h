/*
 * Remmina - The GTK+ Remote Desktop Client
 * Copyright (C) 2014-2021 Antenore Gatta, Giovanni Panozzo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 *  In addition, as a special exception, the copyright holders give
 *  permission to link the code of portions of this program with the
 *  OpenSSL library under certain conditions as described in each
 *  individual source file, and distribute linked combinations
 *  including the two.
 *  You must obey the GNU General Public License in all respects
 *  for all of the code used other than OpenSSL. *  If you modify
 *  file(s) with this exception, you may extend this exception to your
 *  version of the file(s), but you are not obligated to do so. *  If you
 *  do not wish to do so, delete this exception statement from your
 *  version. *  If you delete this exception statement from all source
 *  files in the program, then also delete it here.
 *
 */

#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// I N C L U D E S
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <glib.h>
#include <gtk/gtk.h>
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <glib.h>
#include <Python.h>
#include <structmember.h>
#include "pygobject.h"

#include "remmina/plugin.h"
#include "config.h"

G_BEGIN_DECLS

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// D E C L A R A T I O N S
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern const gchar* ATTR_NAME;
extern const gchar* ATTR_ICON_NAME;
extern const gchar* ATTR_DESCRIPTION;
extern const gchar* ATTR_VERSION;
extern const gchar* ATTR_ICON_NAME_SSH;
extern const gchar* ATTR_FEATURES;
extern const gchar* ATTR_BASIC_SETTINGS;
extern const gchar* ATTR_ADVANCED_SETTINGS;
extern const gchar* ATTR_SSH_SETTING;
extern const gchar* ATTR_EXPORT_HINTS;
extern const gchar* ATTR_PREF_LABEL;
extern const gchar* ATTR_INIT_ORDER;

/**
 * If WITH_PYTHON_TRACE_CALLS is defined, it logs the calls to the Python code and logs errors in case.
 */
#ifdef WITH_PYTHON_TRACE_CALLS
#define CallPythonMethod(instance, name, params, ...)                             \
		remmina_plugin_python_last_result_set(PyObject_CallMethod(instance, name, params, ##__VA_ARGS__)); \
		remmina_plugin_python_log_method_call(instance, name);                                          \
		remmina_plugin_python_check_error()
#else
/**
 * If WITH_TRACE_CALL is not defined, it still logs errors but doesn't print the call anymore.
 */
#define CallPythonMethod(instance, name, params, ...)           \
    PyObject_CallMethod(instance, name, params, ##__VA_ARGS__); \
    remmina_plugin_python_check_error()
#endif // WITH_PYTHON_TRACE_CALLS


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// T Y P E S
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * The Python abstraction of the protocol widget struct.
 *
 * @details This struct is responsible to provide the same accessibility to the protocol widget for Python as for
 * native plugins.
 */
typedef struct
{
	PyObject_HEAD
	RemminaProtocolWidget* gp;
} PyRemminaProtocolWidget;

/**
 * Maps an instance of a Python plugin to a Remmina one.
 *
 * @details This is used to map a Python plugin instance to the Remmina plugin one. Also instance specific data as the
 * protocol widget are stored in this struct.
 */
typedef struct
{
	RemminaProtocolPlugin* protocol_plugin;
	RemminaFilePlugin* file_plugin;
	RemminaSecretPlugin* secret_plugin;
	RemminaToolPlugin* tool_plugin;
	RemminaEntryPlugin* entry_plugin;
	RemminaPrefPlugin* pref_plugin;
	RemminaPlugin* generic_plugin;
	PyRemminaProtocolWidget* gp;
	PyObject* instance;
} PyPlugin;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// A P I
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Gets the result of the last python method call.
 */
PyObject* remmina_plugin_python_last_result();

/**
 * Sets the result of the last python method call.
 * @return Returns the passed result (it's done to be compatible with the CallPythonMethod macro).
 */
PyObject* remmina_plugin_python_last_result_set(PyObject* result);

/**
 * Prints a log message to inform the user a python message has been called.
 *
 * @detail This method is called from the CALL_PYTHON macro if WITH_PYTHON_TRACE_CALLS is defined.
 *
 * @param instance The instance that contains the called method.
 * @param method The name of the method called.
 */
void remmina_plugin_python_log_method_call(PyObject* instance, const gchar* method);

/**
 * Checks if an error has occurred and prints it.
 *
 * @return Returns TRUE if an error has occurred.
 */
gboolean remmina_plugin_python_check_error();

/**
 * Gets the attribute as long value.
 *
 * @param instance The instance of the object to get the attribute.
 * @param constant_name The name of the attribute to get.
 * @param def The value to return if the attribute doesn't exist or is not set.
 *
 * @return The value attribute as long.
 */
long remmina_plugin_python_get_attribute_long(PyObject* instance, gchar* attr_name, long def);

/**
 * Checks if a given attribute exists.
 *
 * @param instance The object to check for the attribute.
 * @param attr_name The name of the attribute to check.
 *
 * @return Returns TRUE if the attribute exists.
 */
gboolean remmina_plugin_python_check_attribute(PyObject* instance, const gchar* attr_name);

G_END_DECLS
