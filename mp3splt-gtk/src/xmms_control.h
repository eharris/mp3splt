/**********************************************************
 *
 * mp3splt-gtk -- utility based on mp3splt,
 *                for mp3/ogg splitting without decoding
 *
 * Copyright (c) 2005-2012 Alexandru Munteanu - io_fx@yahoo.fr
 *
 * http://mp3splt.sourceforge.net/
 *
 *********************************************************/

/**********************************************************
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 * USA.
 *
 *********************************************************/

/**********************************************************
 * Filename: xmms_control.h
 *
 * header to xmms_control.c
 *
 * xmms_control.c controls the xmms player
 *********************************************************/

#ifndef XMMS_CONTROL_H

#define XMMS_CONTROL_H

#include "external_includes.h"
#include "ui_types.h"

void myxmms_get_song_infos(gchar *total_infos, ui_state *ui);
gchar *myxmms_get_filename(ui_state *ui);
gint myxmms_get_playlist_number(ui_state *ui);
gchar *myxmms_get_title_song(ui_state *ui);
gint myxmms_get_time_elapsed(ui_state *ui);
void myxmms_start(ui_state *ui);
void myxmms_select_last_file(ui_state *ui);
void myxmms_play_last_file(ui_state *ui);
void myxmms_add_files(GList *list, ui_state *ui);
void myxmms_set_volume(gint volume, ui_state *ui);
gint myxmms_get_volume(ui_state *ui);
void myxmms_start_with_songs(GList *list, ui_state *ui);
gint myxmms_is_running(ui_state *ui);
gint myxmms_is_paused(ui_state *ui);
void myxmms_play(ui_state *ui);
void myxmms_stop(ui_state *ui);
void myxmms_pause(ui_state *ui);
void myxmms_next(ui_state *ui);
void myxmms_prev(ui_state *ui);
void myxmms_jump(gint position, ui_state *ui);
gint myxmms_get_total_time(ui_state *ui);
gint myxmms_is_playing(ui_state *ui);

#endif

