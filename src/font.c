/**
 * \file font.c
 * \author nullpointer & Wolfgar 
 * \brief This module enables to generate RGBA bitmap buffer with some text in it
 * 
 * $URL$
 * $Rev$
 * $Author$
 * $Date$
 *
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



#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_CACHE_H
#include FT_CACHE_MANAGER_H

#include "log.h"
#include "font.h"

/* FIXME hardcoded font */
#define FONT_FILENAME "res/font/decker.ttf" 

/* state module variables */
static struct{
    /* FT objects instanciated on init */
    FT_Library    library;
    FTC_Manager   cache_manager;
    FTC_SBitCache sbits_cache;
    FT_Face       face;
    
    /* height and width of currently drawn string */
    int height, width;
    /* RGBA image buffer*/
    unsigned char * image;
    /* Default and current Font sizes */
    int default_size;
    int size;
} state;

static bool draw_bitmap(const struct font_color * color,
                        FTC_SBit sbit,
                        int x, int y)
{

  FT_Int  i, j, p, q;
  FT_Int  x_max = x + sbit->width;
  FT_Int  y_max = y + sbit->height;

//  log_write(LOG_DEBUG, "draw_bitmap  %i %i %i %i",x, x_max, y, y_max);

  for (i = x, p = 0; i < x_max; i++, p++) {
    for (j = y, q = 0; j < y_max; j++, q++) {
          state.image[(4*((j*state.width) + i)) + 0] = color->r;
          state.image[(4*((j*state.width) + i)) + 1] = color->g;
          state.image[(4*((j*state.width) + i)) + 2] = color->b;
          state.image[(4*((j*state.width) + i)) + 3] = sbit->buffer[ q * sbit->width + p] ; 
    }
  }
  return true;
}


/* FIXME Only one face is used for now... */
static FT_Error face_requester( FTC_FaceID  face_id,
                     FT_Library  lib,
                     FT_Pointer  request_data,
                     FT_Face*    aface )
{ 
  *aface = state.face;
  return 0;
}
  
  
bool  font_get_size(const char * text, int * width, int * height, int * orig)
{
  int n;
  int num_chars;
  FT_Vector pen;
  FT_Error  error;
  int up, down, max_up, max_down;
  FTC_SBit sbit;
  FTC_ImageTypeRec im_type;
  FT_UInt index;
  
  im_type.flags = FT_LOAD_TARGET_NORMAL;  
  im_type.face_id = &state;  
  im_type.width =  state.size;
  im_type.height = state.size;
  
  max_up = max_down = *orig = 0;
  num_chars = strlen(text);
  pen.x = 0;
  pen.y = 0;
  *height = 0;

  for (n = 0; n < num_chars; n++) {
    index = FT_Get_Char_Index(state.face, text[n]);
    error = FTC_SBitCache_Lookup(state.sbits_cache,
                                 &im_type,                                       
                                 index,
                                 &sbit,
                                 NULL);
     //log_write(LOG_DEBUG, " xadv :%i - yadv : %i - h :%i top :%i", sbit->xadvance, sbit->yadvance, sbit->height, sbit->top);

    /* increment pen position */
    pen.x += sbit->xadvance;
    pen.y += sbit->yadvance;
    up = sbit->top;
    down =  sbit->height - sbit->top;    
    if (up > max_up)
      max_up = up;
    if (down > max_down){ 
      max_down = down;
      *orig = down;
    }
    if (*height < (max_up+max_down)){
        *height = (max_up+max_down) ;
    }
  }

  *width  = pen.x;
  //log_write(LOG_DEBUG, "height : %i - width : %i - orig %i", *height, *width, *orig);
  return true;
}



void font_release(void) {
  if (state.face != NULL) {
    FT_Done_Face(state.face);
    state.face = NULL;
  }
  if (state.library != NULL) {
    FT_Done_FreeType(state.library);
    state.library = NULL;
  }
}


/**
 * \warning The caller will have to free image_buffer
 */
bool font_draw(const struct font_color *color,  const char *text, unsigned char **image_buffer, int *w, int *h)
{
  FTC_SBit sbit;
  FTC_ImageTypeRec im_type;
  FT_UInt index;
  FT_Vector pen;
  FT_Error  error;
  int num_chars;
  int orig;
  int n;

  if (font_get_size(text, &state.width, &state.height, &orig) == false){
    return false;
  }

  *w = state.width;
  *h = state.height;
  state.image = malloc(state.width * state.height * 4);
  if (state.image == NULL){
    return false;
  }
  memset (state.image, 0, state.width * state.height * 4);
  *image_buffer = state.image;

  num_chars = strlen(text);
  pen.x = 0;
  pen.y = 0;

  im_type.face_id = &state;
  im_type.width = state.size; 
  im_type.height = state.size;
  im_type.flags = FT_LOAD_TARGET_NORMAL;
  
  for (n = 0; n < num_chars; n++) {
    index = FT_Get_Char_Index(state.face, text[n]);         
    error = FTC_SBitCache_Lookup(state.sbits_cache,
                                 &im_type,                                       
                                 index,
                                 &sbit,
                                 NULL);
    //log_write(LOG_DEBUG, "err : %i size : %i for index %i - sbit :%x - xadvance : %i left %i top %i", error, state.size, index, sbit, sbit->xadvance, sbit->left, sbit->top );
    /* now, draw to our target surface (convert position) */    
    draw_bitmap( color,
                 sbit,
                 pen.x + sbit->left,
                 state.height - sbit->top - orig);
    /* increment pen position */
    pen.x += sbit->xadvance; //slot->advance.x;
    pen.y += sbit->yadvance; //slot->advance.y;
  }
  state.image = NULL;
  return true;
}


bool font_init(int size)
{
  FT_Error  error;

  /* Prevent multiple init without release */
  if (state.library != NULL)
    return false;

  /* FIXME error handling omitted */
  
  /* Initialize library */
  error = FT_Init_FreeType(&state.library);              
  /* Load font */
  error |= FT_New_Face(state.library, FONT_FILENAME, 0, &state.face); 
  /* Initialize cache */
  error |= FTC_Manager_New(state.library, 0, 0, 0,
                          face_requester, 0, &state.cache_manager);
  error |= FTC_SBitCache_New(state.cache_manager, &state.sbits_cache);  
  log_write(LOG_DEBUG, __FILE__ ":Font module initialized : %i", error);
  
  state.default_size = size;
  state.size = size;
  return true;
}


int font_change_size(int size)
{    
    state.size = size;    
    return 0;
}

int font_restore_default_size(void)
{
    state.size = state.default_size;
    return 0;
}
