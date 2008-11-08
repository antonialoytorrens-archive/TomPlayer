/**
 * \file font.c
 * \author nullpointer & Wolfgar 
 * \brief This module enables to generate RGBA bitmap buffer with some text in it
 * 
 * $URL$
 * $Rev:$
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

/* FT objects instanciated on init */
static FT_Library    library;
static FT_Face       face;

/* height and width of curently draw string */
static int height, width;
/* RGBA image buffer*/
static unsigned char * image;


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
          image[(4*((j*width) + i)) + 0] = color->r;
          image[(4*((j*width) + i)) + 1] = color->g;
          image[(4*((j*width) + i)) + 2] = color->b;
          image[(4*((j*width) + i)) + 3] = bitmap->buffer[ q * bitmap->width + p] ; 
    }
  }

/* We also set the color as transparent in the empty area to bypass a devil bug thaht occurs later otherwise...*/
  for ( i = x, p = 0; i < x_max; i++, p++ )
  {
    for ( j = 0, q = 0; j < y; j++, q++ )
    {
          image[(4*((j*width) + i)) + 0] = color->r;
          image[(4*((j*width) + i)) + 1] = color->g;
          image[(4*((j*width) + i)) + 2] = color->b;
          image[(4*((j*width) + i)) + 3] = 0;
    }
  }

  return true;
}

static bool  font_get_size(const char * text, int * width, int * height){
  int n;
  int num_chars;
  FT_Vector pen;
  FT_Error  error;
  FT_GlyphSlot  slot;

  num_chars = strlen(text);
  pen.x = 0 * 64;
  pen.y = 0 * 64;
  slot = face->glyph;
  *height = 0;

  for ( n = 0; n < num_chars; n++ ){
    /* load glyph image into the slot (erase previous one) */
    error = FT_Load_Char( face, text[n], FT_LOAD_RENDER );
    if ( error ){
      return false;
    }
    /* increment pen position */
    pen.x += slot->advance.x;
    pen.y += slot->advance.y;
    if (*height < (slot->metrics.height/64))
        *height = slot->metrics.height/64;
  }

  *width  = pen.x / 64;
  return true;
}



void font_release(void){
  if (face != NULL){
    FT_Done_Face( face );
    face = NULL;
  }
  if (library != NULL) {
    FT_Done_FreeType( library );
    library = NULL;
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
  int n;

  if (font_get_size(text, &width, &height) == false){
    return false;
  }

  *w = width;
  *h = height;
  image = malloc(width*height*4);
  if (image == NULL){
    return false;
  }
  memset (image,0,width*height*4);
  *image_buffer = image;

  num_chars = strlen(text);
  slot = face->glyph;

  /* the pen position in 26.6 cartesian space coordinates
     relative to the upper left corner  */
  pen.x = 0 * 64;
  pen.y = 0 * 64;

  for ( n = 0; n < num_chars; n++ )
  {

    /* load glyph image into the slot (erase previous one) */
    error = FT_Load_Char( face, text[n], FT_LOAD_RENDER );
    if ( error )
      continue;                 /* ignore errors */

    /* now, draw to our target surface (convert position) */
    draw_bitmap( color,
                 &slot->bitmap,
                 pen.x / 64 /*slot->bitmap_left*/,
                 height-slot->bitmap.rows);

    /* increment pen position */
    pen.x += slot->advance.x;
    pen.y += slot->advance.y;
  }
  image = NULL;
  return true;
}


bool font_init(int size){
  FT_Error  error;

  /* Prevent multiple init without release */
  if (library != NULL)
    return false;

  error = FT_Init_FreeType( &library );              /* initialize library */
  /* error handling omitted */

  error = FT_New_Face( library, FONT_FILENAME, 0, &face ); /* create face object */
  /* error handling omitted */

  /* set character size : use size ptt at 100dpi */
  error = FT_Set_Char_Size( face,size * 64, 0, 100, 0 );

  return true;
}
