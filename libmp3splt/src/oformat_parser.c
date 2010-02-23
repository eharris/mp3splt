/**********************************************************
 *
 * libmp3splt -- library based on mp3splt,
 *               for mp3/ogg splitting without decoding
 *
 * Copyright (c) 2002-2005 M. Trotta - <mtrotta@users.sourceforge.net>
 * Copyright (c) 2005-2010 Alexandru Munteanu - io_fx@yahoo.fr
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

#include <string.h>
#include <ctype.h>

#include "splt.h"

static short splt_u_output_variable_is_valid(char v, int *amb);
static const char *splt_u_get_format_ptr(const char *format, char *temp);
static int splt_u_get_requested_num_of_digits(splt_state *state, const char *format,
    int *requested_num_of_digits, int is_alpha);
static void splt_u_alpha_track(splt_state *state, int nfield,
    char *fm, int fm_length, int number_of_digits, int tracknumber);

int splt_of_parse_outformat(char *s, splt_state *state)
{
  char *ptrs = NULL, *ptre = NULL;
  int i=0, amb = SPLT_OUTPUT_FORMAT_AMBIGUOUS, len=0;

  for (i=0; i<strlen(s); i++)
  {
    if (s[i]=='+') 
    {
      s[i]=' ';
    }
    else 
    {
      if (s[i] == SPLT_VARCHAR) 
      {
        s[i]='%';
      }
    }
  }

  ptrs = s;
  i = 0;
  ptre = strchr(ptrs+1, '%');
  if (s[0] != '%')
  {
    if (ptre==NULL)
    {
      len = strlen(ptrs);
    }
    else
    {
      len = ptre-ptrs;
    }
    if (len > SPLT_MAXOLEN)
    {
      len = SPLT_MAXOLEN;
    }
    strncpy(state->oformat.format[i++], ptrs, len);
  }
  else
  {
    ptre = s;
  }

  //if stdout, NOT ambiguous
  if (splt_io_input_is_stdout(state))
  {
    return SPLT_OUTPUT_FORMAT_OK;
  }

  char err[2] = { '\0' };

  if (ptre == NULL)
  {
    splt_e_set_error_data(state, err);
    return SPLT_OUTPUT_FORMAT_AMBIGUOUS;
  }
  ptrs = ptre;

  char *last_ptre = NULL;
  while (((ptre = strchr(ptrs+1, '%')) != NULL) && (i < SPLT_OUTNUM))
  {
    char cf = *(ptrs+1);

    len = ptre-ptrs;
    if (len > SPLT_MAXOLEN)
    {
      len = SPLT_MAXOLEN;
    }

    if (!splt_u_output_variable_is_valid(cf, &amb))
    {
      err[0] = cf;
      splt_e_set_error_data(state, err);
      return SPLT_OUTPUT_FORMAT_ERROR;
    }

    strncpy(state->oformat.format[i++], ptrs, len);
    ptrs = ptre;
    last_ptre = ptre;
  }

  if (last_ptre && *last_ptre != '\0')
  {
    char v = *(last_ptre+1);
    if (!splt_u_output_variable_is_valid(v, &amb))
    {
      err[0] = v;
      splt_e_set_error_data(state, err);
      return SPLT_OUTPUT_FORMAT_ERROR;
    }
  }

  strncpy(state->oformat.format[i], ptrs, strlen(ptrs));

  if (ptrs[1]=='t')
  {
    amb = SPLT_OUTPUT_FORMAT_OK;
  }

  if (ptrs[1]=='n')
  {
    amb = SPLT_OUTPUT_FORMAT_OK;
  }

  return amb;
}

int splt_of_put_output_format_filename(splt_state *state, int current_split)
{
  int error = SPLT_OK;

  int output_filenames = splt_o_get_int_option(state, SPLT_OPT_OUTPUT_FILENAMES);
  if (output_filenames == SPLT_OUTPUT_CUSTOM)
  {
    return error;
  }

  char *temp = NULL;
  char *fm = NULL;
  int i = 0;
  char *output_filename = NULL;
  int output_filename_size = 0;

  char *title = NULL;
  char *artist = NULL;
  char *album = NULL;
  char *performer = NULL;
  char *artist_or_performer = NULL;
  char *original_filename = NULL;

  int split_file_number = splt_t_get_current_split_file_number(state);
  int tags_index = split_file_number - 1;

  if (current_split == -1)
  {
    current_split = splt_t_get_current_split_file_number(state) - 1;
  }

  long mins = -1;
  long secs = -1;
  long hundr = -1;
  long point_value = splt_sp_get_splitpoint_value(state, current_split, &error);
  splt_co_get_mins_secs_hundr(point_value, &mins, &secs, &hundr);
  long next_mins = -1;
  long next_secs = -1;
  long next_hundr = -1;
  long next_point_value = -1;
  if (splt_sp_splitpoint_exists(state, current_split + 1))
  {
    next_point_value = splt_sp_get_splitpoint_value(state, current_split + 1, &error);
    long total_time = splt_t_get_total_time(state);
    if (total_time > 0 && next_point_value > total_time)
    {
      next_point_value = total_time;
    }
    splt_co_get_mins_secs_hundr(next_point_value, &next_mins, &next_secs, &next_hundr);
  }

  int fm_length = 0;

  //if we get the tags from the first file
  int remaining_tags_like_x = 
    splt_o_get_int_option(state,SPLT_OPT_ALL_REMAINING_TAGS_LIKE_X);
  if ((tags_index >= state->split.real_tagsnumber) &&
      (remaining_tags_like_x != -1))
  {
    tags_index = remaining_tags_like_x;
  }

  const char *output_format = splt_of_get_oformat(state);
  short write_eof = SPLT_FALSE;
  if ((next_point_value == LONG_MAX) &&
      (strcmp(output_format, SPLT_DEFAULT_OUTPUT) == 0))
  {
    write_eof = SPLT_TRUE;
  }

  splt_d_print_debug(state,"The output format is ", 0, output_format);

  long mMsShH_value = -1;
  short eof_written = SPLT_FALSE;

  for (i = 0; i < SPLT_OUTNUM; i++)
  {
    if (strlen(state->oformat.format[i]) == 0)
    {
      break;
    }
    //if we have some % in the format (@ has been converted to %)
    if (state->oformat.format[i][0] == '%')
    {
      //we allocate memory for the temp variable
      if (temp)
      {
        free(temp);
        temp = NULL;
      }

      int temp_len = strlen(state->oformat.format[i])+10;
      if ((temp = malloc(temp_len * sizeof(char))) == NULL)
      {
        error = SPLT_ERROR_CANNOT_ALLOCATE_MEMORY;
        goto end;
      }
      memset(temp, 0x0, temp_len);

      temp[0] = '%';
      temp[1] = 's';
      char char_variable = state->oformat.format[i][1];
      switch (char_variable)
      {
        case 's':
          mMsShH_value = secs;
          goto put_value;
        case 'S':
          mMsShH_value = next_secs;
          goto put_value;
        case 'm':
          mMsShH_value = mins;
          goto put_value;
        case 'M':
          mMsShH_value = next_mins;
          goto put_value;
        case 'h':
          mMsShH_value = hundr;
          goto put_value;
        case 'H':
          mMsShH_value = next_hundr;
put_value:
          if (!eof_written)
          {
            if (write_eof &&
                (char_variable == 'S' ||
                 char_variable == 'M' ||
                 char_variable == 'H'))
            {
              write_eof = SPLT_FALSE;
              eof_written = SPLT_TRUE;

              fm_length = strlen(temp) + 4;
              if ((fm = malloc(fm_length * sizeof(char))) == NULL)
              {
                error = SPLT_ERROR_CANNOT_ALLOCATE_MEMORY;
                goto end;
              }
              snprintf(fm, fm_length, temp, "EOF");
            }
            else if (mMsShH_value != -1)
            {
              temp[1] = '0';
              temp[2] = '2';
              temp[3] = 'l';
              temp[4] = 'd';

              const char *format = NULL;
              int offset = 5;

              //don't print out @h or @H if 0 for default output
              if ((strcmp(state->oformat.format_string, SPLT_DEFAULT_OUTPUT) == 0) &&
                  (mMsShH_value == 0) &&
                  (char_variable == 'h' || char_variable == 'H'))
              {
                if (char_variable == 'h')
                {
                  format = state->oformat.format[i]+2;
                  offset = 0;
                }
                else
                {
                  output_filename[strlen(output_filename)-1] = '\0';
                  break;
                }
              }
              else
              {
                format = splt_u_get_format_ptr(state->oformat.format[i], temp);
              }

              int requested_num_of_digits = 0;
              int max_number_of_digits = splt_u_get_requested_num_of_digits(state,
                  state->oformat.format[i], &requested_num_of_digits, SPLT_FALSE);

              snprintf(temp + offset, temp_len, format + 2);

              fm_length = strlen(temp) + 1 + max_number_of_digits;
              if ((fm = malloc(fm_length * sizeof(char))) == NULL)
              {
                error = SPLT_ERROR_CANNOT_ALLOCATE_MEMORY;
                goto end;
              }

              snprintf(fm, fm_length, temp, mMsShH_value);
            }
          }
          break;
        case 'A':
          if (splt_tu_tags_exists(state,tags_index))
          {
            artist_or_performer =
              (char *)splt_tu_get_tags_field(state,tags_index, SPLT_TAGS_PERFORMER);
            splt_su_clean_string(state, artist_or_performer, &error);
            if (error < 0) { goto end; };

            if (artist_or_performer == NULL || artist_or_performer[0] == '\0')
            {
              artist_or_performer = 
                (char *)splt_tu_get_tags_field(state,tags_index, SPLT_TAGS_ARTIST);
              splt_su_clean_string(state, artist_or_performer, &error);
              if (error < 0) { goto end; };
            }
          }
          else
          {
            artist_or_performer = NULL;
          }

          //
          if (artist_or_performer != NULL)
          {
            snprintf(temp+2,temp_len, state->oformat.format[i]+2);

            int artist_length = 0;
            artist_length = strlen(artist_or_performer);
            fm_length = strlen(temp) + artist_length + 1;
          }
          else
          {
            snprintf(temp,temp_len, state->oformat.format[i]+2);
            fm_length = strlen(temp) + 1;
          }

          if ((fm = malloc(fm_length * sizeof(char))) == NULL)
          {
            error = SPLT_ERROR_CANNOT_ALLOCATE_MEMORY;
            goto end;
          }

          //
          if (artist_or_performer != NULL)
          {
            snprintf(fm, fm_length, temp, artist_or_performer);
          }
          else
          {
            snprintf(fm, fm_length, temp);
          }

          break;
        case 'a':
          if (splt_tu_tags_exists(state,tags_index))
          {
            //we get the artist
            artist =
              (char *)splt_tu_get_tags_field(state,tags_index, SPLT_TAGS_ARTIST);
            splt_su_clean_string(state, artist, &error);
            if (error < 0) { goto end; };
          }
          else
          {
            artist = NULL;
          }

          //
          if (artist != NULL)
          {
            snprintf(temp+2,temp_len, state->oformat.format[i]+2);

            int artist_length = 0;
            artist_length = strlen(artist);
            fm_length = strlen(temp) + artist_length + 1;
          }
          else
          {
            snprintf(temp,temp_len, state->oformat.format[i]+2);
            fm_length = strlen(temp) + 1;
          }

          if ((fm = malloc(fm_length * sizeof(char))) == NULL)
          {
            error = SPLT_ERROR_CANNOT_ALLOCATE_MEMORY;
            goto end;
          }

          //
          if (artist != NULL)
          {
            snprintf(fm, fm_length, temp, artist);
          }
          else
          {
            snprintf(fm, fm_length, temp);
          }
          break;
        case 'b':
          if (splt_tu_tags_exists(state,tags_index))
          {
            //we get the album
            album =
              (char *)splt_tu_get_tags_field(state,tags_index, SPLT_TAGS_ALBUM);
            splt_su_clean_string(state, album, &error);
            if (error < 0) { goto end; };
          }
          else
          {
            album = NULL;
          }

          //
          if (album != NULL)
          {
            int album_length = 0;
            album_length = strlen(album);
            snprintf(temp+2, temp_len, state->oformat.format[i]+2);

            fm_length = strlen(temp) + album_length + 1;
          }
          else
          {
            snprintf(temp,temp_len, state->oformat.format[i]+2);
            fm_length = strlen(temp) + 1;
          }

          if ((fm = malloc(fm_length * sizeof(char))) == NULL)
          {
            error = SPLT_ERROR_CANNOT_ALLOCATE_MEMORY;
            goto end;
          }

          //
          if (album != NULL)
          {
            snprintf(fm, fm_length, temp, album);
          }
          else
          {
            snprintf(fm, fm_length, "%s", temp);
          }
          break;
        case 't':
          if (splt_tu_tags_exists(state,tags_index))
          {
            title = (char *)splt_tu_get_tags_field(state,tags_index, SPLT_TAGS_TITLE);
            splt_su_clean_string(state, title, &error);
            if (error < 0) { goto end; };
          }
          else
          {
            title = NULL;
          }

          //
          if (title != NULL)
          {
            int title_length = 0;
            title_length = strlen(title);
            snprintf(temp+2, temp_len, state->oformat.format[i]+2);

            fm_length = strlen(temp) + title_length + 1;
          }
          else
          {
            snprintf(temp,temp_len, state->oformat.format[i]+2);
            fm_length = strlen(temp) + 1;
          }

          if ((fm = malloc(fm_length * sizeof(char))) == NULL)
          {
            error = SPLT_ERROR_CANNOT_ALLOCATE_MEMORY;
            goto end;
          }

          //
          if (title != NULL)
          {
            snprintf(fm, fm_length, temp, title);
          }
          else
          {
            snprintf(fm, fm_length, temp);
          }
          break;
        case 'p':
          if (splt_tu_tags_exists(state,tags_index))
          {
            performer = (char *)splt_tu_get_tags_field(state,tags_index, SPLT_TAGS_PERFORMER);
            splt_su_clean_string(state, performer, &error);
            if (error < 0) { goto end; };
          }
          else
          {
            performer = NULL;
          }

          //
          if (performer != NULL)
          {
            int performer_length = 0;
            performer_length = strlen(performer);
            snprintf(temp+2, temp_len, state->oformat.format[i]+2);

            fm_length = strlen(temp) + performer_length + 1;
          }
          else
          {
            snprintf(temp,temp_len, state->oformat.format[i]+2);
            fm_length = strlen(temp) + 1;
          }

          if ((fm = malloc(fm_length * sizeof(char))) == NULL)
          {
            error = SPLT_ERROR_CANNOT_ALLOCATE_MEMORY;
            goto end;
          }

          if (performer != NULL)
          {
            snprintf(fm, fm_length, temp, performer);
          }
          else
          {
            snprintf(fm, fm_length, temp);
          }
          break;
        case 'l':
        case 'L':
        case 'u':
        case 'U':
        case 'n':
        case 'N':
          temp[1] = '0';
          temp[2] = splt_of_get_oformat_number_of_digits_as_char(state);
          temp[3] = 'd';

          int tracknumber = split_file_number;

          //if not time split, or normal split, or silence split or error,
          //we put the track number from the tags
          int split_mode = splt_o_get_int_option(state,SPLT_OPT_SPLIT_MODE);
          if ((isupper(state->oformat.format[i][1])) ||
              ((split_mode != SPLT_OPTION_TIME_MODE) &&
               (split_mode != SPLT_OPTION_NORMAL_MODE) &&
               (split_mode != SPLT_OPTION_SILENCE_MODE) &&
               (split_mode != SPLT_OPTION_ERROR_MODE)))
          {
            if (splt_tu_tags_exists(state, tags_index))
            {
              int *tags_track = (int *)splt_tu_get_tags_field(state, tags_index, SPLT_TAGS_TRACK);
              if (tags_track && *tags_track > 0)
              {
                tracknumber = *tags_track;
              }
            }
          }

          int requested_num_of_digits = 0;
          int max_num_of_digits = splt_u_get_requested_num_of_digits(state,
              state->oformat.format[i], &requested_num_of_digits, SPLT_FALSE);

          int alpha_requested_num_of_digits = 0;
          int alpha_max_num_of_digits = splt_u_get_requested_num_of_digits(state,
              state->oformat.format[i], &alpha_requested_num_of_digits, SPLT_TRUE);

          int is_numeric = toupper(state->oformat.format[i][1]) == 'N';
          if (is_numeric)
          {
            const char *format = splt_u_get_format_ptr(state->oformat.format[i], temp);

            snprintf(temp + 4, temp_len, format + 2);
            fm_length = strlen(temp) + 1 + max_num_of_digits;
          }
          else
          {
            fm_length = strlen(state->oformat.format[i]) + 1 + alpha_max_num_of_digits;
          }

          if ((fm = malloc(fm_length * sizeof(char))) == NULL)
          {
            error = SPLT_ERROR_CANNOT_ALLOCATE_MEMORY;
            goto end;
          }
          memset(fm, '\0', fm_length);

          if (is_numeric)
          {
            snprintf(fm, fm_length, temp, tracknumber);
          }
          else
          {
            splt_u_alpha_track(state, i, fm, fm_length,
                alpha_requested_num_of_digits, tracknumber);
          }
          break;
        case 'f':
          if (splt_t_get_filename_to_split(state) != NULL)
          {
            original_filename = strdup(splt_su_get_fname_without_path(splt_t_get_filename_to_split(state)));
            if (original_filename)
            {
              snprintf(temp+2,temp_len, state->oformat.format[i]+2);

              splt_su_cut_extension(original_filename);

              int filename_length = strlen(original_filename);

              fm_length = strlen(temp) + filename_length;
              if ((fm = malloc(fm_length * sizeof(char))) == NULL)
              {
                error = SPLT_ERROR_CANNOT_ALLOCATE_MEMORY;
                goto end;
              }

              snprintf(fm, fm_length, temp, original_filename);
              free(original_filename);
              original_filename = NULL;
            }
            else
            {
              error = SPLT_ERROR_CANNOT_ALLOCATE_MEMORY;
              goto end;
            }
          }
          break;
      }
    }
    else
    {
      fm_length = SPLT_MAXOLEN;
      if ((fm = malloc(fm_length * sizeof(char))) == NULL)
      {
        error = SPLT_ERROR_CANNOT_ALLOCATE_MEMORY;
        goto end;
      }

      strncpy(fm, state->oformat.format[i], SPLT_MAXOLEN);
    }

    int fm_size = 7;
    if (fm != NULL)
    {
      fm_size = strlen(fm);
    }

    //allocate memory for the output filename
    if (!output_filename)
    {
      if ((output_filename = malloc((1+fm_size)*sizeof(char))) == NULL)
      {
        error = SPLT_ERROR_CANNOT_ALLOCATE_MEMORY;
        goto end;
      }
      output_filename_size = fm_size;
      output_filename[0] = '\0';
    }
    else
    {
      output_filename_size += fm_size+1;
      if ((output_filename = realloc(output_filename, output_filename_size
              * sizeof(char))) == NULL)
      {
        error = SPLT_ERROR_CANNOT_ALLOCATE_MEMORY;
        goto end;
      }
    }

    if (fm != NULL)
    {
      strcat(output_filename, fm);
    }

    //we free fm
    if (fm)
    {
      free(fm);
      fm = NULL;
    }
  }

  splt_d_print_debug(state,"The new output filename is ",0,output_filename);
  int cur_splt = splt_t_get_current_split(state);
  int name_error = splt_sp_set_splitpoint_name(state, cur_splt, output_filename);
  if (name_error != SPLT_OK) { error = name_error; }

end:
  if (output_filename)
  {
    free(output_filename);
    output_filename = NULL;
  }
  if (fm)
  {
    free(fm);
    fm = NULL;
  }
  if (temp)
  {
    free(temp);
    temp = NULL;
  }

  return error;
}

static short splt_u_output_variable_is_valid(char v, int *amb)
{
  switch (v)
  {
    case 's':
    case 'S':
    case 'm':
    case 'M':
    case 'h':
    case 'H':
    case 'a':
    case 'A':
    case 'b':
    case 'f':
    case 'p':
      break;
    case 't':
    case 'l':
    case 'L':
    case 'u':
    case 'U':
    case 'n':
    case 'N':
      *amb = SPLT_OUTPUT_FORMAT_OK;
      break;
    default:
      return SPLT_FALSE;
  }

  return SPLT_TRUE;
}

static const char *splt_u_get_format_ptr(const char *format, char *temp)
{
  int format_length = strlen(format);
  const char *format_ptr = format;

  if ((format_length > 2) && isdigit(format[2]))
  {
    temp[2] = format[2];
    format_ptr = format + 1;
  }

  return format_ptr;
}

static int splt_u_get_requested_num_of_digits(splt_state *state, const char *format,
    int *requested_num_of_digits, int is_alpha)
{
  int format_length = strlen(format);
  int number_of_digits = 0;
  if (is_alpha)
  {
    number_of_digits = state->oformat.output_alpha_format_digits;
  }
  else
  {
    number_of_digits = splt_of_get_oformat_number_of_digits_as_int(state);
  }
  int max_number_of_digits = number_of_digits;
  *requested_num_of_digits = number_of_digits;

  if ((format_length > 2) && isdigit(format[2]))
  {
    *requested_num_of_digits = format[2] - '0';
  }

  if (*requested_num_of_digits > number_of_digits)
  {
    max_number_of_digits = *requested_num_of_digits;
  }

  return max_number_of_digits;
}

/*
 * Encode track number as 'A', 'B', ... 'Z', 'AA, 'AB', ...
 *
 * This is not simply "base 26"; note that the lowest 'digit' is from 'A' to
 * 'Z' (base 26), but all higher digits are 'A' to 'Z' plus 'nothing', i.e.,
 * base 27. In other words, since after 'Z' comes 'AA', we cannot use 'AA'
 * as the padded version of track number 1 ('A').
 *
 * This means that there are two distinct work modes:
 * - The normal encoding is as described above.
 * - When the user has specified the number of digits (padding), we encode
 *   the track number as simple base-26: 'AAA', 'AAB', ... 'AAZ', 'ABA',
 *   'ABB', ...
 */
static void splt_u_alpha_track(splt_state *state, int nfield,
    char *fm, int fm_length, int number_of_digits, int tracknumber)
{
  char *format = state->oformat.format[nfield];
  int lowercase = (toupper(format[1]) == 'L');
  char a = lowercase ? 'a' : 'A';
  int zerobased = tracknumber - 1;
  int i = 1, min_digits = state->oformat.output_alpha_format_digits;

  if (number_of_digits > 1)
  {
    /* Padding required => simple base-26 encoding */
    if (number_of_digits < min_digits)
      number_of_digits = min_digits;
    for (i = 1; i <= number_of_digits; ++ i, zerobased /= 26)
    {
      int digit = (zerobased % 26);
      fm[number_of_digits - i] = a + digit;
    }
  }
  else
  {
    /* No padding: First letter base-26, others base-27 */
    number_of_digits = min_digits;

    /* Start with the first, base-26 'digit' */
    fm[number_of_digits - 1] = a + (zerobased % 26);

    /* Now handle all other digits */
    zerobased /= 26;
    for (i = 2; i <= number_of_digits; ++ i, zerobased /= 27)
    {
      int digit = (zerobased % 27);
      fm[number_of_digits - i] = a + digit - 1;
    }
  }

  int offset = 0;
  if ((strlen(format) > 2) && isdigit(format[2]))
  {
    offset = 1;
  }
  snprintf(fm + number_of_digits, fm_length - number_of_digits,
      "%s", format + 2 + offset);
}


