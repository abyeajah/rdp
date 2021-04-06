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

#include "remmina/plugin.h"


G_BEGIN_DECLS


/**
 * @brief Handles the initialization of the Python plugin.
 * @details This function prepares the plugin structure and calls the init method of the
 * plugin Python class.
 *
 * @param   gp  The protocol widget used by the plugin.
 */
void remmina_protocol_init_wrapper(RemminaProtocolWidget* gp);

/**
 * @brief
 * @details
 *
 * @param   gp  The protocol widget used by the plugin.
 */
gboolean remmina_protocol_open_connection_wrapper(RemminaProtocolWidget* gp);

/**
 * @brief
 * @details
 *
 * @param   gp  The protocol widget used by the plugin.
 */
gboolean remmina_protocol_close_connection_wrapper(RemminaProtocolWidget* gp);

/**
 * @brief
 * @details
 *
 * @param   gp  The protocol widget used by the plugin.
 */
gboolean remmina_protocol_query_feature_wrapper(RemminaProtocolWidget* gp, const RemminaProtocolFeature* feature);

/**
 * @brief
 * @details
 *
 * @param   gp  The protocol widget used by the plugin.
 */
void remmina_protocol_call_feature_wrapper(RemminaProtocolWidget* gp, const RemminaProtocolFeature* feature);

/**
 * @brief
 * @details
 *
 * @param   gp  The protocol widget used by the plugin.
 */
void remmina_protocol_send_keytrokes_wrapper(RemminaProtocolWidget* gp, const guint keystrokes[], const gint keylen);

/**
 * @brief
 * @details
 *
 * @param   gp  The protocol widget used by the plugin.
 */
gboolean remmina_protocol_get_plugin_screenshot_wrapper(RemminaProtocolWidget* gp, RemminaPluginScreenshotData* rpsd);

G_END_DECLS
