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

#include <glib.h>
#include <gtk/gtk.h>
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <glib.h>
#include <Python.h>
#include <structmember.h>

#include "remmina/plugin.h"

#ifdef WITH_TRACE_CALLS
#define CallPythonMethod(instance, name, params, ...)                             \
    remmina_plugin_python_last_result_set(PyObject_CallMethod(instance, name, params, ##__VA_ARGS__)); \
    remmina_plugin_python_log_method_call(instance, name);                                          \
    remmina_plugin_python_check_error()
#else
#define CallPythonMethod(instance, name, params, ...)                             \
    PyObject_CallMethod(instance, name, params, ##__VA_ARGS__); \
    remmina_plugin_python_check_error()
#endif // WITH_TRACE_CALLS

G_BEGIN_DECLS

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

PyObject *remmina_plugin_python_last_result();
PyObject *remmina_plugin_python_last_result_set(PyObject *result);

void remmina_plugin_python_log_method_call(PyObject* instance, const gchar *method);

/**
 * @brief Checks if an error has occurred and prints it.
 *
 * @return Returns TRUE if an error has occurred.
 */
gboolean remmina_plugin_python_check_error();

long remmina_plugin_python_to_enum_or_default(PyObject *instance, gchar *constant_name, long def);

gboolean remmina_plugin_python_check_attribute(PyObject* instance, const gchar* attr_name);

G_END_DECLS
