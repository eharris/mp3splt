/**********************************************************
 *
 * libmp3splt -- library based on mp3splt,
 *               for mp3/ogg splitting without decoding
 *
 * Copyright (c) 2002-2005 M. Trotta - <mtrotta@users.sourceforge.net>
 * Copyright (c) 2005-2011 Alexandru Munteanu - io_fx@yahoo.fr
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

/*! \file

\todo What is a split pair?
*/
#include "splt.h"

splt_pair *splt_pair_new(void *first, void *second)
{
  splt_pair *pair = malloc(sizeof(splt_pair));
  if (pair == NULL)
  {
    return NULL;
  }

  pair->first = first;
  pair->second = second;

  return pair;
}

void splt_pair_free(splt_pair **pair)
{
  if (!pair)
  {
    return;
  }

  if (!*pair)
  {
    return;
  }

  free(*pair);
  *pair = NULL;
}

void *splt_pair_first(splt_pair *pair)
{
  return pair->first;
}

void *splt_pair_second(splt_pair *pair)
{
  return pair->second;
}

