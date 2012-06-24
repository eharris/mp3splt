/**********************************************************
 * mp3splt-gtk -- utility based on mp3splt,
 *                for mp3/ogg splitting without decoding
 *
 *
 * Copyright: (C) 2005-2012 Alexandru Munteanu
 * Contact: io_fx@yahoo.fr
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

#ifndef UI_TYPES_H

#define UI_TYPES_H

#include <libmp3splt/mp3splt.h>

#include "preferences_manager.h"

typedef struct
{
  gboolean checked;
  gint mins;
  gint secs;
  gint hundr_secs;
} Split_point;

typedef struct {
  gint root_x_pos;
  gint root_y_pos;
  gint width;
  gint height;
} ui_main_window;

typedef struct {
  long time;
  float level;
} silence_wave;

typedef struct {
  gchar *browser_directory;
  ui_main_window *main_win;
  GList *text_options_list;

  silence_wave *silence_points;
  gint number_of_silence_points;
  gint malloced_num_of_silence_points;
} ui_infos;

typedef struct {
  GtkWidget *window;
} gui_state;

typedef struct {
  gint splitting;
  gint quit_main_program;
} gui_status;

typedef struct {
  gint return_code;
  ui_infos *infos;
  preferences_state *preferences;
  splt_state *mp3splt_state;
  GArray *splitpoints;
  gui_state *gui;
  gui_status *status;
} ui_state;

#endif

