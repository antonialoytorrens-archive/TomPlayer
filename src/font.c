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

#include "font.h"

/* FIXME hardcoded font */
#define FONT_FILENAME "res/font/decker.ttf" 

/* state module variables */
static struct{
    /* FT objects instanciated on init */
    FT_Library    library;
    FT_Face       face;
    /* height and width of currently drawn string */
    int height, width;
    /* RGBA image buffer*/
    unsigned char * image;
    int default_size;
}state;

static bool draw_bitmap( const struct font_color * color,
                  FT_Bitmap*  bitmap,
                  int     x,
                  int     y){

  FT_Int  i, j, p, q;
  FT_Int  x_max = x + bitmap->width;
  FT_Int  y_max = y + bitmap->rows ;
 
  for ( i = x, p = 0; i < x_max; i++, p++ )
  {
    for ( j = y, q = 0; j < y_max; j++, q++ )
    {
          state.image[(4*((j*state.width) + i)) + 0] = color->r;
          state.image[(4*((j*state.width) + i)) + 1] = color->g;
          state.image[(4*((j*state.width) + i)) + 2] = color->b;
          state.image[(4*((j*state.width) + i)) + 3] = bitmap->buffer[ q * bitmap->width + p] ; 
    }
  }
  return true;
}

bool  font_get_size(const char * text, int * width, int * height, int * orig){
  int n;
  int num_chars;
  FT_Vector pen;
  FT_Error  error;
  FT_GlyphSlot  slot;
  int up, down, max_up, max_down;

  max_up = max_down = *orig = 0;
  num_chars = strlen(text);
  pen.x = 0 * 64;
  pen.y = 0 * 64;
  slot = state.face->glyph;
  *height = 0;

  for ( n = 0; n < num_chars; n++ ){
    /* load glyph image into the slot (erase previous one) */
    error = FT_Load_Char( state.face, text[n], FT_LOAD_RENDER );
    if ( error ){
      return false;
    }
    /* increment pen position */
    pen.x += slot->advance.x;
    pen.y += slot->advance.y;
    up = slot->metrics.horiBearingY/64;
    down = slot->metrics.height/64 - slot->metrics.horiBearingY/64;
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

  /* FIXME size is dalse Find why - by the time add a constant */
  *width  = pen.x / 64 + 5;
  
  return true;
}



void font_release(void){
  if (state.face != NULL){
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
bool font_draw(const struct font_color * color,  const char * text, unsigned char ** image_buffer, int * w, int *h){
  FT_Vector pen;
  FT_Error  error;
  FT_GlyphSlot  slot;
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
  slot = state.face->glyph;

  /* the pen position in 26.6 cartesian space coordinates
     relative to the upper left corner  */
  pen.x = 0 * 64;
  pen.y = 0 * 64;

  for ( n = 0; n < num_chars; n++ )
  {

    /* load glyph image into the slot (erase previous one) */
    error = FT_Load_Char(state.face, text[n], FT_LOAD_RENDER);
    if ( error )
      continue;                 /* ignore errors */

    /* now, draw to our target surface (convert position) */
    draw_bitmap( color,
                 &slot->bitmap,
                 pen.x / 64 /*slot->bitmap_left*/,
                 state.height - slot->bitmap_top - orig);

    /* increment pen position */
    pen.x += slot->advance.x;
    pen.y += slot->advance.y;
  }
  state.image = NULL;
  return true;
}


bool font_init(int size){
  FT_Error  error;

  /* Prevent multiple init without release */
  if (state.library != NULL)
    return false;

  error = FT_Init_FreeType( &state.library );              /* initialize library */
  /* error handling omitted */
  error = FT_New_Face( state.library, FONT_FILENAME, 0, &state.face ); /* create face object */
  /* error handling omitted */
  /* set character size : use size ptt at 100dpi */
  error = FT_Set_Char_Size(state.face, size * 64, 0, 100, 0 );  
  state.default_size = size;
  
  return true;
}


int font_change_size(int size){
    return FT_Set_Char_Size(state.face, size * 64, 0, 100, 0 );  
}

int font_restore_default_size(void){
    return FT_Set_Char_Size(state.face, state.default_size * 64, 0, 100, 0 );  
}
