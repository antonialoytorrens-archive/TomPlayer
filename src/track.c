/**
 * \file track.c
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

#include <string.h>
#include <stdlib.h>
#include <tag_c.h>

#include "engine.h"
#include "track.h"

static TagLib_File *current_file;
static char *current_filename;
static struct track_tags current_tags;

const char * track_get_current_filename(void){
    const char * ret; 
    if (current_filename != NULL){
        ret = (const char *)strrchr(current_filename, '/');
        if (ret != NULL){
            return ret+1;
        }
    }
    return current_filename;
}

bool track_has_changed(const char * filename){   
   if (current_filename != NULL){
      /* Check whether the current file has changed */
      if (strcmp(current_filename, filename) == 0){
          return false;
      }
   }
   return true;
}

bool track_update(const char * filename){
  TagLib_Tag *tag;
  const TagLib_AudioProperties *properties;
  size_t cover_len;
  char * buffer;  
  
  /* Release current */
  track_release();
 
  
  /* Set new infos */    
  current_filename = strdup(filename);
  if (current_filename == NULL){    
      return false;
  }
  
  if (eng_get_mode() == MODE_AUDIO){
    taglib_set_strings_unicode(0);
    current_file = taglib_file_new(filename);
    if(current_file == NULL){
        return false;  
    }
    
    tag = taglib_file_tag(current_file);
    if (tag != NULL){ 
        current_tags.title = taglib_tag_title(tag);
        current_tags.artist = taglib_tag_artist(tag);
        current_tags.album =  taglib_tag_album(tag);
        if (taglib_tag_year(tag) > 0){
            snprintf(current_tags.year, sizeof(current_tags.year), "%d", taglib_tag_year(tag));    
            current_tags.year[sizeof(current_tags.year)-1] = 0;
        } else {
            current_tags.year[0] = 0;
        }
        current_tags.comment = taglib_tag_comment(tag);
        if (taglib_tag_track(tag) > 0){
            snprintf(current_tags.track, sizeof(current_tags.track), "%d", taglib_tag_track(tag));
            current_tags.track[sizeof(current_tags.track)-1] = 0;
        } else {
            current_tags.track[0] = 0;
        }
        current_tags.genre = taglib_tag_genre(tag);
        if (taglib_tag_track(tag) > 0) {
            snprintf(current_tags.nb, sizeof(current_tags.nb), "%02d", taglib_tag_track(tag));
            current_tags.nb[sizeof(current_tags.nb) -1] = 0;    
        } else {
            current_tags.nb[0] = 0;
        }
    }
    properties = taglib_file_audioproperties(current_file);
    if(properties != NULL) {
        current_tags.length = taglib_audioproperties_length(properties);
        current_tags.bitrate = taglib_audioproperties_bitrate(properties);
        current_tags.sample_rate = taglib_audioproperties_samplerate(properties);
        current_tags.channels =  taglib_audioproperties_channels(properties);
    }    
    cover_len = taglib_file_cover_size(current_file);
    if (cover_len > 0){
        buffer = (char *)malloc(cover_len);
        taglib_file_cover(current_file, buffer, cover_len);
        ilGenImages(1, &current_tags.coverart);
        ilBindImage(current_tags.coverart);
        if (!ilLoadL(IL_TYPE_UNKNOWN, buffer, cover_len)){
            ilDeleteImages( 1, &current_tags.coverart);
            current_tags.coverart = 0;
        }    
        free(buffer);
    }
  }
  return true;
}

const struct track_tags *track_get_tags(void){
    return &current_tags;
}

void track_release(void){
    taglib_tag_free_strings();
    if (current_file != NULL){
        taglib_file_free(current_file);
        current_file = NULL;
    }
    if (current_tags.coverart != 0){
         ilDeleteImages( 1, &current_tags.coverart);
    }
    free(current_filename);
    current_filename = NULL;
    memset(&current_tags, 0, sizeof(current_tags));
}
