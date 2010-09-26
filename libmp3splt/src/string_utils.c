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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>
#include <stdarg.h>

#include <ctype.h>

#include "splt.h"

static int splt_su_append_one(char **str, const char *to_append,
    size_t to_append_size);
static void splt_su_clean_string_(splt_state *state, char *s, int *error,
    int ignore_dirchar);
static int splt_su_is_illegal_char(char c, int ignore_dirchar);

char *splt_su_replace_all(const char *str, char *to_replace,
    char *replacement, int *error)
{
  if (str == NULL)
  {
    return NULL;
  }

  char *new_str = NULL;
  int err = SPLT_OK;

  if (to_replace == NULL || replacement == NULL)
  {
    int err = splt_su_copy(str, &new_str);
    if (err < 0) { *error = err; }
    return new_str;
  }

  const char *ptr = str;
  const char *prev_ptr = ptr;
  while ((ptr = strstr(ptr, to_replace)) != NULL)
  {
    err = splt_su_append(&new_str, prev_ptr, ptr - prev_ptr,
        replacement, strlen(replacement), NULL);
    if (err != SPLT_OK) { goto error; }
    ptr += strlen(to_replace);
    prev_ptr = ptr;
  }

  if (prev_ptr != NULL)
  {
    err = splt_su_append(&new_str, prev_ptr, (str + strlen(str)) - prev_ptr, NULL);
    if (err != SPLT_OK) { goto error; }
  }
 
  return new_str;

error:
  if (new_str)
  {
    free(new_str);
  }
  *error = err;

  return NULL;
}

int splt_su_append_str(char **str, const char *to_append, ...)
{
  int err = SPLT_OK;
  va_list ap;

  va_start(ap, to_append);

  while (to_append)
  {
    size_t to_append_size = strlen(to_append);
    err = splt_su_append(str, to_append, to_append_size, NULL);
    if (err < 0) { break; }
    to_append = va_arg(ap, const char *);
  }

  va_end(ap);

  return err;
}

int splt_su_append(char **str, const char *to_append, ...)
{
  int err = SPLT_OK;
  va_list ap;

  va_start(ap, to_append);

  while (to_append)
  {
    size_t to_append_size = va_arg(ap, size_t);
    err = splt_su_append_one(str, to_append, to_append_size);
    if (err < 0) { break; }
    to_append = va_arg(ap, const char *);
  }

  va_end(ap);

  return err;
}

void splt_su_free_replace(char **str, char *replacement)
{
  if (!str)
  {
    return;
  }

  if (*str)
  {
    free(*str);
  }

  *str = replacement;
}

int splt_su_copy(const char *src, char **dest)
{
  if (!dest)
  {
    return SPLT_OK;
  }

  if (*dest)
  {
    free(*dest);
    *dest = NULL;
  }

  if (src == NULL)
  {
    *dest = NULL;
    return SPLT_OK;
  }

  int length = strlen(src) + 1;
  if ((*dest = malloc(sizeof(char) * length)) == NULL)
  {
    return SPLT_ERROR_CANNOT_ALLOCATE_MEMORY;
  }

  snprintf(*dest, length, "%s", src);

  return SPLT_OK;
}

void splt_su_clean_string(splt_state *state, char *s, int *error)
{
  splt_su_clean_string_(state, s, error, SPLT_FALSE);
}

char *splt_su_cut_spaces(char *c)
{
  while (isspace(*c))
  {
    c++;
  }

  return c;
}

void splt_su_cut_spaces_from_end(char *c)
{
  char *end = strchr(c, '\0');
  if (!end)
  {
    return;
  }

  end--;

  while (isspace(*end))
  {
    *end = '\0';
    end--;
  }
}

char *splt_su_trim_spaces(char *c)
{
  splt_su_cut_spaces_from_end(c);
  return splt_su_cut_spaces(c);
}

int splt_su_is_empty_line(char *line)
{
  if (!line)
  {
    return SPLT_TRUE;
  }

  size_t size = strlen(line);
  int i = 0;
  for (i = 0;i < size;i++)
  {
    if (!isspace(line[i]))
    {
      return SPLT_FALSE;
    }
  }

  return SPLT_TRUE;
}

void splt_su_line_to_unix(char *line)
{
  size_t line_size = strlen(line);
  if (line_size > 1)
  {
    if (line[line_size-2] == '\r')
    {
      line[line_size-2] = '\n';
      line[line_size-1] = '\0';
    }
  }
}

const char *splt_su_get_fname_without_path(const char *filename)
{
  char *c = NULL;
  while ((c = strchr(filename, SPLT_DIRCHAR)) != NULL)
  {
    filename = c + 1;
  }

  return filename;
}

char *splt_su_get_fname_with_path_and_extension(splt_state *state, int *error)
{
  int err = SPLT_OK;
  char *output_fname_with_path = NULL;

  int current_split = splt_t_get_current_split(state);
  const char *output_fname = splt_sp_get_splitpoint_name(state, current_split, error);

  if (!output_fname)
  {
    char *stdout_str = NULL;
    err = splt_su_copy("-", &stdout_str);
    if (err < 0) { *error = err; }
    return stdout_str;
  }

  if (strcmp(output_fname, "-") == 0)
  {
    char *result = NULL;
    err = splt_su_copy(output_fname, &result);
    if (err < 0) { goto error; }
    return result;
  }

  const char *extension = splt_p_get_extension(state, &err);
  if (err < 0) { goto error; }

  const char *new_filename_path = splt_t_get_new_filename_path(state);
  if (new_filename_path[0] == '\0')
  {
    err = splt_su_append_str(&output_fname_with_path, output_fname,
        extension, NULL);
    if (err < 0) { goto error; }
  }
  else
  {
    if (new_filename_path[strlen(new_filename_path)-1] == SPLT_DIRCHAR)
    {
      err = splt_su_append_str(&output_fname_with_path, new_filename_path,
          output_fname, extension, NULL);
    }
    else
    {
      err = splt_su_append_str(&output_fname_with_path, new_filename_path,
          SPLT_DIRSTR, output_fname, extension, NULL);
    }
    if (err < 0) { goto error; }
  }

  const char *filename = splt_t_get_filename_to_split(state);
  if (splt_io_check_if_file(state, output_fname_with_path))
  {
    if (splt_check_is_the_same_file(state, filename, output_fname_with_path, &err))
    {
      splt_e_set_error_data(state,filename);
      err = SPLT_ERROR_INPUT_OUTPUT_SAME_FILE;
      goto error;
    }
  }

  //TODO: warning if a file already exists

  return output_fname_with_path;

error:
  if (output_fname_with_path)
  {
    free(output_fname_with_path);
    output_fname_with_path = NULL;
  }

  *error = err;

  return NULL;
}

void splt_su_cut_extension(char *str)
{
  char *point = strrchr(str , '.');
  if (point)
  {
    *point = '\0';
  }
}

char *splt_su_str_to_upper(const char *str, int *error)
{
  int err = SPLT_OK;

  if (!str)
  {
    return NULL;
  }

  char *result = NULL;
  err = splt_su_copy(str, &result);
  if (err < 0)
  {
    *error = err;
    return NULL;
  }

  int i = 0;
  for (i = 0;i < strlen(str);i++)
  {
    result[i] = toupper(str[i]);
  }

  return result;
}

void splt_su_str_cut_last_char(char *str)
{
  if (!str)
  {
    return;
  }

  str[strlen(str)-1] = '\0';
}

double splt_su_str_line_to_double(const char *str)
{
  if (!str)
  {
    return 0.0;
  }

  while ((*str != '\0') && (isdigit(*str) == 0))
  {
    str++;
  }

  return atof(str);
}

char *splt_su_get_file_with_output_path(splt_state *state, char *filename, int *error)
{
  int err = SPLT_OK;
  char *new_fname = NULL;

  if (filename == NULL)
  {
    return NULL;
  }

  splt_su_clean_string(state, filename, error);
  if (error < 0) { return NULL; }

  const char *path_of_split = splt_t_get_path_of_split(state);
  if (path_of_split)
  {
    if (path_of_split[strlen(path_of_split)] == SPLT_DIRCHAR)
    {
      splt_su_append_str(&new_fname, path_of_split, filename, NULL);
      if (err < 0) { *error = err; }
      return new_fname;
    }

    splt_su_append_str(&new_fname, path_of_split, SPLT_DIRSTR, filename, NULL);
    if (err < 0) { *error = err; }
    return new_fname;
  }

  err = splt_su_copy(filename, &new_fname);
  if (err < 0) { *error = err; }
  return new_fname;
}

int splt_su_str_ends_with(const char *str1, const char *str2)
{
  if (!str1 || !str2)
  {
    return SPLT_FALSE;
  }

  int str1_end_index = strlen(str1) - 1;
  int str2_end_index = strlen(str2) - 1;

  while (str1_end_index >= 0 && str2_end_index >= 0)
  {
    if (str1[str1_end_index] != str2[str2_end_index])
    {
      return SPLT_FALSE;
    }

    str1_end_index--;
    str2_end_index--;
  }

  return SPLT_TRUE;
}

char *splt_su_format_messagev(splt_state *state, const char *message, va_list ap)
{
  int counter = 0;

  int written_chars = 0;

  int size = 255;
  char *mess = malloc(sizeof(char) * size);
  if (mess == NULL)
  {
    splt_d_send_memory_error_message(state);
    splt_e_error(SPLT_IERROR_CHAR, __func__, 0, _("not enough memory"));
    return NULL;
  }

  while (counter < LONG_MAX)
  {
    written_chars = vsnprintf(mess, size, message, ap);

    if ((written_chars > -1) &&
        (written_chars+1 < size))
    {
      break;
    }
    else {
      size += 255;
    }

    if ((mess = realloc(mess, size)) == NULL)
    {
      free(mess);
      splt_d_send_memory_error_message(state);
      splt_e_error(SPLT_IERROR_CHAR, __func__, 0, _("not enough memory"));
      return NULL;
    }

    counter++;
  }

  return mess;
}

char *splt_su_get_formatted_message(splt_state *state, char *message, ...)
{
  char *mess = NULL;

  va_list ap;
  va_start(ap, message);
  mess = splt_su_format_messagev(state, message, ap);
  va_end(ap);

  return mess;
}

static int splt_su_append_one(char **str, const char *to_append, size_t to_append_size)
{
  if (str == NULL || to_append == NULL || to_append[0] == '\0' || to_append_size == 0)
  {
    return SPLT_OK;
  }

  size_t new_size = 0;

  if (*str == NULL)
  {
    new_size = to_append_size + 1;
    *str = malloc(new_size);
    if (*str == NULL)
    {
      return SPLT_ERROR_CANNOT_ALLOCATE_MEMORY;
    }

    *str[0] = '\0';
  }
  else
  {
    new_size = to_append_size + strlen(*str) + 1;
    *str = realloc(*str, new_size);
    if (*str == NULL)
    {
      return SPLT_ERROR_CANNOT_ALLOCATE_MEMORY;
    }
  }

  strncat(*str, to_append, to_append_size);

  return SPLT_OK;
}

static int splt_su_is_illegal_char(char c, int ignore_dirchar)
{
  if ((ignore_dirchar) && (c == SPLT_DIRCHAR))
  {
    return SPLT_FALSE;
  }

  //for the sake of filename portability, we take the the windows illegal
  //characters (will be changed upon feature request)
  if ((c == '\\') || (c == '/') || (c == ':') || (c == '*') ||
      (c == '?') || (c == '"') || (c == '<') ||
      (c == '>') || (c == '|') || (c == '\r'))
  {
    return SPLT_TRUE;
  }

  return SPLT_FALSE;
}

static void splt_su_clean_string_(splt_state *state, char *s, int *error, int ignore_dirchar)
{
  int i = 0, j=0;
  char *copy = NULL;
  if (s)
  {
    copy = strdup(s);
    if (copy)
    {
      for (i=0; i<=strlen(copy); i++)
      {
        if (! splt_su_is_illegal_char(copy[i], ignore_dirchar))
        {
          s[j++] = copy[i];
        }
        else
        {
          s[j++] = '_';
        }
      }
      free(copy);
      copy = NULL;

      // Trim string. I will never stop to be surprised about cddb strings dirtiness! ;-)
      for (i=strlen(s)-1; i >= 0; i--) 
      {
        if (s[i]==' ')
        {
          s[i] = '\0';
        }
        else 
        {
          break;
        }
      }
    }
    else
    {
      *error = SPLT_ERROR_CANNOT_ALLOCATE_MEMORY;
    }
  }
}
