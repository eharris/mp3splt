/**********************************************************
 *
 * libmp3splt -- library based on mp3splt,
 *               for mp3/ogg splitting without decoding
 *
 * Copyright (c) 2002-2005 M. Trotta - <mtrotta@users.sourceforge.net>
 * Copyright (c) 2005-2006 Munteanu Alexandru - io_alex_2002@yahoo.fr
 *
 * http://mp3splt.sourceforge.net
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307,
 * USA.
 *
 *********************************************************/

/****************************/
/* utils for conversion */

long splt_u_convert_hundreths (char *s);
float splt_u_convert2dB(double input);
double splt_u_convertfromdB(float input);

/****************************/
/* utils for file infos */

int splt_u_getword (FILE *in, off_t offset, int mode,
                    unsigned long *headw);
off_t splt_u_flength (FILE *in);

/*****************************************************/
/* utils manipulating strings (including filenames) */

char *splt_u_cleanstring (char *s);
char *splt_u_cut_spaces_at_the_end(char *c);
char *splt_u_cut_spaces_from_begin(char *c);
char *splt_u_get_real_name(char *filename);
void splt_u_set_complete_new_filename(splt_state *state, int *error);
char *splt_u_get_fname_with_path_and_extension(splt_state *state,
                                               int *error);

/****************************/
/* utils for splitpoints */

int splt_u_cut_splitpoint_extension(splt_state *state, int index);
void splt_u_order_splitpoints(splt_state *state, int len);

/****************************/
/* utils for the tags       */
int splt_u_put_tags_from_string(splt_state *state, char *tags);

/*******************************/
/* utils for the output format */

int splt_u_parse_outformat(char *s, splt_state *state);
int splt_u_put_output_filename(splt_state *state);

/******************************/
/* utils for debug and errors */

void splt_u_error(int error_type, const char *function,
                  int arg_int, char *arg_char);

/****************************/
/* utils miscellaneous */

float splt_u_silence_position(struct splt_ssplit *temp, float off);
void splt_u_print_debug(char *message,double optional,char *optional2);
double splt_u_get_double_pos(long split);
int splt_u_create_directory(char *dir);
