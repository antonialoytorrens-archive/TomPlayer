/**
 * \file track.h
 * \brief Handle current track infos
 *
 * $URL$
 * $Rev$
 * $Author$
 * $Date$
 */


/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __TRACK_H__
#define __TRACK_H__

#include <stdbool.h>
#include <IL/ilu.h>

struct track_tags{
      char nb[4];
      const char * title;
      const char * artist;
      const char * album;
      char year[8];
      const char * comment;
      char track[8];
      const char * genre;
      int length;
      int bitrate;
      int sample_rate;
      int channels;
      ILuint coverart;
};

bool track_update(const char * filename);
bool track_has_changed(const char * filename);
const struct track_tags *track_get_tags(void);
const char * track_get_current_filename(void);
void track_release(void);

    
#endif
