/**
 * \file draw.c 
 * \brief This module implements low level drawing functions.
 *
 * Contrary to the main menu which relies on directfb, 
 * Tomplayer engine implements its own drawing functions.
 *
 * When in video mode, the frame buffer is used directly 
 * When in audio mode, the display is sent to mplayer to be displayed as overlay
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

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <IL/il.h>
#include <IL/ilu.h>

#include "debug.h"
#include "widescreen.h"
#include "play_int.h"
#include "font.h"
#include "engine.h"
#include "draw.h"

static unsigned short * screen_buffer;
static bool refresh;

void draw_refresh(void){
    int fb;
    static unsigned short * fb_mmap;   
    int screen_width, screen_height;
    
    if (!refresh)
        return;    
    ws_get_size(&screen_width, &screen_height);    
    if (fb_mmap == NULL){        
        fb = open( getenv( "FRAMEBUFFER" ), O_RDWR);
            if (fb < 0){  
                perror("unable to open fb ");
                return;
            }            
            fb_mmap = mmap(NULL,  screen_width*screen_height*2 , PROT_READ|PROT_WRITE,MAP_SHARED, fb, 0);
            if (fb_mmap == MAP_FAILED){
                perror("unable to mmap fb ");
                fb_mmap = NULL;
                close(fb);
                return;
            }
            close(fb);
    }
    memcpy(fb_mmap, screen_buffer, screen_width*screen_height*2);
    refresh = false;    
}

/** Write directly a RGB or RGBA buffer to the frame buffer */
static void display_RGB_to_fb(unsigned char * buffer, int x, int y, int w, int h, bool transparency){        
    unsigned short * buffer16;
    int buffer_size;
    int i,j ;
    int screen_width, screen_height;
    ws_get_size(&screen_width, &screen_height);

    if (screen_buffer == NULL){
        screen_buffer = malloc(screen_width * screen_height * 2);
    }
    if (x < 0) {
        x = 0;
    }
    if (y < 0) {
        y = 0;
    }
  
    
    /* Alloc buffer for RBG conversion */
    buffer_size = w * h * 2 ;
    buffer16 = malloc( buffer_size );
    if (buffer16 == NULL){
      fprintf(stderr, "Allocation error\n");
      return;
    }

    if (transparency){
        if (ws_are_axes_inverted() == 0){
            for (i=y; i<y+h; i++){
                for(j=x; j<x+w; j++){
                    buffer16[((i-y)*w)+(j-x)] = screen_buffer[j+(i*screen_width)];
                }
            }
        }else{
            int tmp;
            tmp = y;
            y = x;
            x = screen_width - tmp;
            /* Magic combination for inverted coordinates */
            for (i=x; i>x-h; i--){
                for(j=y; j<y+w; j++){
                    buffer16[(-i+x)*w+(j-y)] = screen_buffer[j*screen_width+i];
                }
            }
        }
    }

    for( i = 0; i < (w * h); i++ ){
        if (transparency == false ){
            /* Initial buffer is RGB24 */
            buffer16[i] =  ( (buffer[3*i] & 0xF8)  <<  8 | /* R 5 bits*/
                            (buffer[3*i+1] & 0xFC) << 3 | /* G 6 bits */
                            (buffer[3*i+2]>>3));          /* B 5 bits*/
        } else {
            /* Initial buffer is RGBA*/   
            /* FIXME only binary transparency for now ... */
            if (buffer[4*i+3] != 0){
                buffer16[i] =  ( (buffer[4*i] & 0xF8) << 8 |   /* R 5 bits*/
                                 (buffer[4*i+1] & 0xFC) << 3 | /* G 6 bits */
                                 (buffer[4*i+2]>>3));          /* B 5 bits*/
            }                       
        }
    }

    if (ws_are_axes_inverted() == 0){
        for (i=y; i<y+h; i++){
            for(j=x; j<x+w; j++){
                screen_buffer[j+(i*screen_width)] = buffer16[((i-y)*w)+(j-x)];
            }
        }
    } else {
        int tmp;

        tmp = y;
        y = x;
        x = screen_width - tmp;
        /* Magic combination for inverted coordinates */
        for (i=x; i>x-h; i--){
            for(j=y; j<y+w; j++){
                screen_buffer[j*screen_width+i] = buffer16[(-i+x)*w+(j-y)];
            }
        }
    }
    free( buffer16 );
    refresh = true;   
}


/** Display a RGB or RGBA buffer on screen */ 
void draw_RGB_buffer(unsigned char * buffer, int x, int y, int w, int h, bool transparency){
  char str[100];
  int buffer_size;
  int i;
    
  if (eng_get_mode() == MODE_VIDEO) {
    if (transparency){
      const struct skin_config * conf = skin_get_config();
      buffer_size = w * h * 4;
      PRINTDF(" couleur transparence %d, %d, %d \n", conf->r,conf->g,conf->b);
      /* Apply manually transparency */
      for (i=0; i < buffer_size; i+=4){
        if ((buffer[i] == conf->r) &&
            (buffer[i+1] == conf->g) &&
            (buffer[i+2] == conf->b)){
            buffer[i+3] = 0;
        }
       }
      sprintf(str, "RGBA32 %d %d %d %d %d %d\n", w, h, x, y , 0, 0);
      
    }
    else{
      sprintf(str, "RGB24 %d %d %d %d %d %d\n", w, h, x, y , 255, 0);        
      buffer_size = w*h*3;
    }
    playint_menu_write((unsigned char *)str, strlen(str));
    playint_menu_write(buffer, buffer_size);
  } else {    
    display_RGB_to_fb(buffer, x, y, w, h, transparency);
  }
}


/** Daw text on the skin 
 * \param size font size, if 0 then default size is used
 */
void draw_text(const char * text, int x, int y, int w, int h, const struct font_color *color, int size){
    unsigned char * buffer_to_display;    
    unsigned char * text_buffer;
    ILuint  img_id; 
    ILuint text_id;
    int text_width, text_height;         
        
    buffer_to_display = malloc(4 * w * h);    
    if (buffer_to_display == NULL)
        return;
    ilGenImages(1, &img_id);
    /* Bind to backgound image and copy the appropriate portion */
    ilBindImage(skin_get_background()); 
    ilCopyPixels(x, y, 0, w, h, 1,
                 IL_RGBA, IL_UNSIGNED_BYTE, buffer_to_display);                             
    ilBindImage(img_id);
    ilTexImage(w, h, 1, 
               4, IL_RGBA, IL_UNSIGNED_BYTE, buffer_to_display);
    /* Flip image because an ilTexImage is always LOWER_LEFT */
    iluFlipImage();
    
    /* Generate the font rendering in a dedicated image */    
    if (size != 0){
        font_change_size(size);
    }    
    if (font_draw(color, text, &text_buffer, &text_width, &text_height) == false){
        goto out_release_buffer;
    }
    
    ilGenImages(1, &text_id);
    ilBindImage(text_id);
    ilTexImage(text_width, text_height, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, text_buffer);         
    iluFlipImage();
    /* Combinate font and background */
    ilBindImage(img_id);  
    ilOverlayImage(text_id, 0, 0, 0);     
    ilCopyPixels(0, 0, 0, w, h, 1,
                 IL_RGBA, IL_UNSIGNED_BYTE, buffer_to_display);                 
    /* Display the result on screen */
    draw_RGB_buffer(buffer_to_display, x, y, w, h, true);     
    /* Free resources */      
    free(text_buffer);
    ilDeleteImages( 1, &text_id);
out_release_buffer:    
    free(buffer_to_display);
    ilDeleteImages( 1, &img_id);
    if (size != 0)
        font_restore_default_size();    
    return ;        
}


/** Display a cursor over the skin
 *
 * \param[in] cursor_id Image ID (Devil) to display
 * \param[in] frame_id Imagenumber to display in case of animation (0 if not used)
 * \param[in] x x coordinate where the image has to be displayed
 * \param[in] y y coordinate where the image has to be displayed
 *
 */
void draw_cursor(ILuint cursor_id, ILuint frame_id, int x, int y ){
  ILuint tmp_skin_id = 0;
  ILuint tmp_cursor_id = 0;
  int height, width;
  int buffer_size;
  unsigned char * buffer;

  /* Get cusor infos */
  ilBindImage(cursor_id);
  width  = ilGetInteger(IL_IMAGE_WIDTH);
  height = ilGetInteger(IL_IMAGE_HEIGHT);
  /* Alloc buffer for RBGA conversion */
  buffer_size = width * height * 4;
    buffer = malloc( buffer_size );
    if (buffer == NULL){
      fprintf(stderr, "Allocation error\n");
      return;
    }

  if (frame_id != 0){
    /* If we select a particular frame in an animation,
     *  we have to copy that impage into a dedicated name id*/
    ilActiveImage(frame_id);
    PRINTDF("Frame id : %i active image : %i origin mode : %i\n",frame_id, ilGetInteger(IL_ACTIVE_IMAGE), ilGetInteger(IL_ORIGIN_MODE));
    ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
    tmp_cursor_id = ilCloneCurImage();
    cursor_id = tmp_cursor_id;
  }

    /* copy the relevant skin background zone */
  ilBindImage(skin_get_background());
    ilCopyPixels(x, y, 0, width, height, 1,
            IL_RGBA, IL_UNSIGNED_BYTE, buffer);
  ilGenImages(1, &tmp_skin_id);
  ilBindImage(tmp_skin_id);
  ilTexImage(width, height, 1, 4,IL_RGBA, IL_UNSIGNED_BYTE, buffer);
  /* Flip image because an ilTexImage is always LOWER_LEFT */
  iluFlipImage();

  /* Overlay cursor on background */
  ilOverlayImage(cursor_id,0,0,0);

  /* Update display zone */
  ilCopyPixels(0, 0, 0,
        width, height, 1,
        IL_RGBA, IL_UNSIGNED_BYTE, buffer);

  draw_RGB_buffer(buffer, x, y, width, height, true);

  /* Cleanup */
  ilDeleteImages( 1, &tmp_skin_id);
  if (tmp_cursor_id != 0){
    ilDeleteImages( 1, &tmp_cursor_id);
  }
  free(buffer);
}

void draw_img(ILuint img){
    int height, width;
    unsigned char * buffer;
    int buffer_size;

    ilBindImage(img);
    width  = ilGetInteger(IL_IMAGE_WIDTH);
    height = ilGetInteger(IL_IMAGE_HEIGHT);
    /* Alloc buffer for RBGA conversion */
    buffer_size = width * height * 4;
    buffer = malloc( buffer_size );
    if (buffer == NULL){
        fprintf(stderr, "Allocation error\n");
        return;
    }
    ilCopyPixels(0, 0, 0, width, height, 1, IL_RGBA, IL_UNSIGNED_BYTE, buffer);
    draw_RGB_buffer(buffer, 0, 0, width, height, true);    
    free( buffer );
}

