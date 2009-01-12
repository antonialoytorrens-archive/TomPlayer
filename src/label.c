/**
 * \file label.c 
 * \brief This module provides label creation and handling facilities
 *
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

#include <directfb.h>
#include "debug.h"
#include "label.h"


struct _label_state{
  struct label_config conf;  
  IDirectFBFont * font ;  
  IDirectFBSurface * surf;
  char * msg;
};


static bool copy_config(label_handle state, const struct label_config * config){
   state->conf = *config;
   state->conf.name = NULL; /* Dont keep filename */
   return true; 
}


static bool init_surf(label_handle hdl, int w, int h){
  DFBSurfaceDescription surf_dsc;

  surf_dsc.flags = DSDESC_WIDTH | DSDESC_HEIGHT;
  surf_dsc.width =  w;
  surf_dsc.height = h;  
  
  if (hdl->conf.dfb->CreateSurface(hdl->conf.dfb, &surf_dsc, &hdl->surf ) != DFB_OK) {    
    return false;
  }
  hdl->surf->SetColor(hdl->surf, hdl->conf.font_color.r, hdl->conf.font_color.g, hdl->conf.font_color.b,0xFF);
  hdl->surf->SetFont(hdl->surf, hdl->font);
  return true;
}

/** Release a label object
*/
void label_release(label_handle hdl){
  if (hdl == NULL){
    return;
  }
  if (hdl->msg != NULL){
    free(hdl->msg);
  }
  if (hdl->font != NULL){
    hdl->font->Release(hdl->font);
  }
  if (hdl->surf != NULL) {
    hdl->surf->Release(hdl->surf);
  }
  free(hdl);  
}


/** Create a label object
 *
 * \warnig dfb and win reference passed in config must remain valid during the life of the label object
 */
label_handle label_create(const struct label_config * config){  
  DFBFontDescription desc;  
  label_handle state;

  state = calloc(1,sizeof(*state));
  if (state == NULL){
    PRINTDF("Allocation failed\n");
    return false;
  }
  copy_config(state, config);  

  desc.flags = DFDESC_HEIGHT;
  desc.height = config->height;
  if (config->dfb->CreateFont( config->dfb, config->name, &desc, &state->font ) != DFB_OK){
          label_release(state);
          return false;
  }
  if (config->pos.w != -1){
     if (init_surf(state, config->pos.w, config->pos.h) == false){
         label_release(state);
         return false;
     }
  }

  return state;
}

/** Set text associated to label and refresh 
 *
 */
bool label_set_text(label_handle hdl, const char * msg){
  IDirectFBSurface * win_surf;

  if (hdl->msg != NULL){
    free (hdl->msg);
  }
  if (hdl->surf == NULL){   
    int w;
    /* Auto size of surface on first call to set_text */
    hdl->font->GetStringWidth(hdl->font, msg, -1, &w); 
    /* (+10)is an empiric correction coz a font needs more than its height to be displayed */
    if (init_surf(hdl, w, hdl->conf.height + 10) == false) {
      return false;
    }
    hdl->conf.pos.w = w;
    hdl->conf.pos.h = hdl->conf.height + 10;
  }
  hdl->msg = strdup(msg);
  hdl->surf->SetColor(hdl->surf, 0x0, 0x0, 0x0, 0xFF);  
  hdl->surf->FillRectangle (hdl->surf, 0, 0, hdl->conf.pos.w, hdl->conf.pos.h);
   hdl->surf->SetColor(hdl->surf, hdl->conf.font_color.r, hdl->conf.font_color.g, hdl->conf.font_color.b,hdl->conf.font_color.a); 
  hdl->surf->DrawString( hdl->surf ,msg, -1, 0, 0, DSTF_TOPLEFT);  
  hdl->surf->Flip(hdl->surf, NULL, DSFLIP_WAITFORSYNC);
  if (hdl->conf.win->GetSurface(hdl->conf.win, &win_surf) != DFB_OK){
    PRINTDF("Error while getting window surface !\n");
  }
  win_surf->Blit(win_surf, hdl->surf, NULL, hdl->conf.pos.x,hdl->conf.pos.y );
  win_surf->Flip(win_surf, NULL, DSFLIP_WAITFORSYNC);
  /* Release the ref */
  win_surf->Release(win_surf);
  return true;
}

IDirectFBFont * label_get_font(label_handle hdl){
  return hdl->font;
}
