/**
 * \file file_selector.c
 * \author Stephan Rafin
 *
 * This module implements a directfb based file selector object.
 *
 * This file selector aims at being used with a touchscreen.
 *
 * It has the following features : 
 *    \li Enable navigation (of course)
 *    \li Filtering facilities
 *    \li One or multiple file(s) selection (Multiple selection possible only in the same folder)
 *    \li Preview facility 
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

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <directfb.h>
#include <pthread.h>
#include <regex.h>

#include "file_list.h"
#include "file_selector.h"
#include "debug.h"


/** Internal data describing the file selector */
struct fs_data {
  IDirectFB  * dfb;                            /**< Main dfb interface */
  IDirectFBWindow * win;                       /**< Window containing the object */  
  struct fs_config * config;                   /**< Configuration of the object */
  IDirectFBSurface * destination;              /**< Surface used to display the object */
  IDirectFBEventBuffer * evt_buffer;           /**< Input event used by the object*/
  IDirectFBSurface* icon_surf[FS_MAX_ICON_ID]; /**< surface rendering icons displayed in the object */ 
  DFBRectangle arrow_list[2];                  /**< List of control zone that have to react to a click (scroll arrows) */
  DFBRectangle file_zone;                      /**< Zone containing the filenames in the object */  
  DFBRectangle refresh_zone;                   /**< Zone to be refreshed (both file zone and check or folder icons)*/  
  DFBRectangle preview_zone;                   /**< Preview zone */
  IDirectFBSurface * refresh_zone_surf;        /**< Surface in which filenames & associated icons are drawed */    
  IDirectFBSurface * preview_surf;             /**< Surface in which preview is drawed (if any) */
  IDirectFBFont *font;                         /**< Font used to display filenames */
  int idx_first_displayed;                     /**< Index of the first displayed filename */  
  int nb_lines;                                /**< Number of displayed lines */
  int line_height;                             /**< Height of a single line : font + interline...*/
  file_list list;                      	       /**< Files list displayed by the object */
  pthread_t thread;                            /**< Thread that handles input events */
  bool end_asked;                              /**< Flag to ask for thread events termination */
  select_cb *prev_cb;                          /**< callback funtion */
  regex_t compiled_re_filter;                  /**< Compiled re filter to apply filter */
  int selected_item ;                          /**< Currently selected item */
} ;



/** Return min between two int values
*/
static inline int  min(int x, int y ) {
  if (x<y) 
    return x; 
  else 
    return y;
}


/** Clear a surface
 * \param[in] surf DirectFb surface to clear
 */
static inline bool clear_surface(IDirectFBSurface* surf){
  int width, height;
  /* Clear zone */
  surf->GetSize(surf,&width, &height);
  surf->SetColor(surf, 0x0, 0x0, 0x0, 0xFF);  
  surf->FillRectangle (surf, 0, 0, width, height);
  return true;
  
}


/**Helper that releases a config object */
static void free_config(struct fs_config * conf){
  int i ;

  if (conf == NULL)
    return;
  for (i=0;i<FS_MAX_ICON_ID;i++){
    free (conf->graphics.filename[i]);
  }
  free (conf->graphics.font);
  if (conf->folder.filter != NULL){
    free(conf->folder.filter);
  }
  free(conf->folder.pathname);
  free(conf);
}

/**Helper that performs a deep copy of config object */
static bool copy_config(const struct fs_config * in, struct fs_config ** out2 ){
  int i ;
  struct fs_config * out;

  *out2 = out = calloc(1,sizeof(struct fs_config));
  if (out == NULL){
    return false;
  }
  *out = *in;
  for (i=0;i<FS_MAX_ICON_ID;i++){
    out->graphics.filename[i] = strdup(in->graphics.filename[i]);
    if (out->graphics.filename[i] == NULL){
      goto error;
    }
  }
  out->graphics.font = strdup(in->graphics.font);
  if (out->graphics.font  == NULL){
    goto error;
  }
  if (in->folder.filter != NULL){
    out->folder.filter = strdup(in->folder.filter);
    if (out->folder.filter == NULL){
      goto error;
    }
  }
  if (in->folder.pathname != NULL){
    out->folder.pathname = strdup(in->folder.pathname);
    if (out->folder.pathname == NULL) {
      goto error;
    }
  }
  return true;

error:
  free_config(out);
  *out2 = NULL;
  return false;
}

/** Release a fs object 
 *
 *\param[in] hdl Handle of the fs object
 *
 *\return true if function succeeded. false otherwise
 */
static bool release(fs_handle hdl) {
  int i ;

  if (hdl == NULL){
    return false;  
  }

  /* Notify the object is released */
  if (hdl->prev_cb != NULL){
    hdl->prev_cb(hdl, "", FS_EVT_RELEASE);
  }

  /* Wait for events thread to terminate */
  hdl->end_asked = true;
  if (hdl->thread){
    pthread_join(hdl->thread,NULL);
  }


  if (hdl->destination != NULL) {
    hdl->destination->Release(hdl->destination);
  } 
  if (hdl->evt_buffer != NULL){
      hdl->evt_buffer->Release(hdl->evt_buffer);
  }
  
  for (i =0; i< FS_MAX_ICON_ID; i++){
    if (hdl->icon_surf[i] != NULL){
      hdl->icon_surf[i]->Release(hdl->icon_surf[i]);
      hdl->icon_surf[i] = NULL;
    }
  }
  if (hdl->font != NULL){
    hdl->font->Release(hdl->font);
  }
  if (hdl->refresh_zone_surf != NULL) {
    hdl->refresh_zone_surf->Release(hdl->refresh_zone_surf);
  }  
  if (hdl->preview_surf != NULL){
    hdl->preview_surf->Release(hdl->preview_surf);
  }
  if (hdl->config->folder.filter != NULL){
    regfree(&hdl->compiled_re_filter);
  }
  fl_release(hdl->list);
  hdl->list = NULL;
  
  if (hdl->config != NULL){
    free_config(hdl->config);    
  }
  /* These two ones are allocated outside of the module : It's not up to us to release them */
  hdl->dfb = NULL;
  hdl->win = NULL;
  
  free(hdl);

  return true;
}


/** Display an icon in the file decriptor object
 *
 *\param[in] hdl Handle of the fs object
 *\param[in] id ID of the icon to display
 *\param[in] Point where to display the icon. If NULL, the icon is displayed to its default position...
 *
 *\return true if function succeeded. false otherwise
 *
 *\note On First call, files are read and rendered to newly allocated surfaces. Then surfaces are directly blitted where they have to 
 */
static bool display_obj(fs_handle hdl, enum fs_icon_ids id, DFBPoint * point){
  
  DFBPoint lpoint;
  IDirectFBSurface * dest_surf;
  


  if ( (id <0) || (id >= FS_MAX_ICON_ID) ) {
    return false;
  }

  if (hdl->icon_surf[id] == NULL){

    IDirectFBImageProvider *provider;
    DFBSurfaceDescription dsc;  
    
    if (hdl->dfb->CreateImageProvider (hdl->dfb,  hdl->config->graphics.filename[id], &provider) !=DFB_OK ){
      return false;
    }

    if (provider->GetSurfaceDescription (provider, &dsc) != DFB_OK){
      return false;
    }

    /* odd value give stange results so lets stay on even values...*/
    dsc.width &=0xFFFFFFFE;

   if (hdl->dfb->CreateSurface (hdl->dfb, &dsc, &hdl->icon_surf[id]) != DFB_OK) {
      provider->Release (provider);
      return false;
    }

    provider->RenderTo (provider, hdl->icon_surf[id], NULL);
    provider->Release (provider);
  }

  if (point == NULL){
    int h,w;
    h=w=0;
    hdl->icon_surf[id]->GetSize(hdl->icon_surf[id],&w,&h) ;

    switch (id){
      case FS_ICON_UP:
        lpoint.x = hdl->config->geometry.pos.x +  hdl->config->geometry.pos.w -  w;
        lpoint.y = hdl->config->geometry.pos.y ;
        hdl->arrow_list[id].x = lpoint.x;
        hdl->arrow_list[id].y = lpoint.y;
        hdl->arrow_list[id].w = w;
        hdl->arrow_list[id].h = h;
        break;
      case FS_ICON_DOWN:
        lpoint.x =  hdl->config->geometry.pos.x +  hdl->config->geometry.pos.w - w;
        lpoint.y =  hdl->config->geometry.pos.y +  hdl->config->geometry.pos.h - h;
        hdl->arrow_list[id].x = lpoint.x;
        hdl->arrow_list[id].y = lpoint.y;
        hdl->arrow_list[id].w = w;
        hdl->arrow_list[id].h = h;
        break;
      default :
        lpoint.x = lpoint.y = 0; 
    }
  } else {
    lpoint = *point;
  }

  switch (id){
      case FS_ICON_CHECK : /* Volontary no break */
      case FS_ICON_FOLDER :
        dest_surf = hdl->refresh_zone_surf;
        break;
      default :
        dest_surf = hdl->destination;
        break;
  }
  
  if (dest_surf != NULL){
    dest_surf->Blit (dest_surf, hdl->icon_surf[id], NULL, lpoint.x, lpoint.y);

  }
  return true;
}




/** Refresh the fs object
 *
 *\param[in] hdl Handle of the fs object
 *
 * \retval true success
 * \retval false Failure
 *
 */
static bool refresh_display(fs_handle hdl){
  char *filename;  
  int len;
  int i,y = 0;
  DFBPoint point;
  int max_idx;
  DFBRegion region;

  clear_surface( hdl->refresh_zone_surf);

  /* Set color */
  hdl->refresh_zone_surf->SetColor( hdl->refresh_zone_surf,
                                       hdl->config->graphics.font_color.r,
                                       hdl->config->graphics.font_color.g,
                                       hdl->config->graphics.font_color.b,
                                       hdl->config->graphics.font_color.a);

  point.x = 0;
  point.y = 0;
  max_idx = hdl->idx_first_displayed + min(hdl->nb_lines, fl_get_entries_nb(hdl->list)- hdl->idx_first_displayed);

  /* Display zone content */
  for (i=hdl->idx_first_displayed; i< max_idx; i++){  

    if (i != hdl->selected_item){
        hdl->refresh_zone_surf->DrawString( hdl->refresh_zone_surf, 
                                            fl_get_filename(hdl->list, i),                                        
                                            -1,
                                            hdl->file_zone.x - hdl->refresh_zone.x ,  
                                            y, 
                                            DSTF_TOPLEFT);
    } else {
        len = strlen(fl_get_filename(hdl->list, i));       
        /*2 addtional characters + terminal 0*/
        filename = malloc(len+3);
        memcpy (&filename[2],fl_get_filename(hdl->list, i), len+1);        
        filename[0] = '>';
        filename[1] = ' ';
        hdl->refresh_zone_surf->DrawString( hdl->refresh_zone_surf, 
                                            filename,                                        
                                            -1,
                                            hdl->file_zone.x - hdl->refresh_zone.x ,  
                                            y, 
                                            DSTF_TOPLEFT);
        free(filename);
    }
    if (fl_is_selected(hdl->list,i)) {
      display_obj(hdl,FS_ICON_CHECK,&point);
    }
    if (fl_is_folder(hdl->list,i)){
      display_obj(hdl,FS_ICON_FOLDER,&point);
    }
    y += hdl->line_height ;
    point.y = y;
  }
  
  hdl->destination->Blit(hdl->destination, hdl->refresh_zone_surf, NULL, hdl->refresh_zone.x, hdl->refresh_zone.y);
  region.x1 = hdl->refresh_zone.x;
  region.y1 = hdl->refresh_zone.y;
  region.x2 = region.x1 + hdl->refresh_zone.w;
  region.y2 = region.y1 + hdl->refresh_zone.h;
  hdl->destination->Flip(hdl->destination, &region, DSFLIP_WAITFORSYNC);
  
  return true;

}





/** Thread that handles input events 
*/
static void * thread(void *param){
  static int x,y;
  int win_x, win_y;
  fs_handle hdl = param;
  DFBInputEvent evt; 


  while(!hdl->end_asked){
      hdl->evt_buffer->WaitForEventWithTimeout( hdl->evt_buffer, 0, 500 );
      if (hdl->end_asked) break;
      while (hdl->evt_buffer->GetEvent( hdl->evt_buffer, DFB_EVENT(&evt)) == DFB_OK) {
	if (evt.type == DIET_AXISMOTION) {
		if (evt.flags & DIEF_AXISABS) {
			switch (evt.axis) {
				case DIAI_X:
					x = evt.axisabs;
					break;
				case DIAI_Y:
					y = evt.axisabs;
					break;
				default :
				  break;
			}
		}		
	}
	else if (evt.type == DIET_BUTTONPRESS ){	
	  hdl->win->GetPosition(hdl->win, &win_x, &win_y);
          fs_handle_click(hdl, x - win_x,y -win_y);
        }
      }      
  }
  return NULL;
}



static int item_selection(fs_handle hdl, int idx){
    bool refresh = false;
    if (!fl_is_folder(hdl->list,idx)) {
        /* Regular file */
        fs_select(hdl,idx);        
        refresh = true;   
    } else {
        /* Folder */
        char * full_path = alloca(strlen(fl_get_basename(hdl->list)) + strlen(fl_get_filename(hdl->list,idx)) + 2);
        char * slash_index;
        strcpy(full_path, fl_get_basename(hdl->list));
        if ((strcmp(fl_get_filename(hdl->list,idx),"..") == 0) &&
            ((slash_index = strrchr(full_path,'/')) != NULL) &&
            (strcmp(slash_index+1,"..")!=0)) {
            /* We cut the last folder when going up in a tree rather than concatening '..' => It simplifies pathnames */
            if (slash_index == full_path){  
            /* Specific case where we are at root */
            *(slash_index +1)= '\0'; 
            } else {
            *slash_index = '\0';
            }
        } else {                      
            strcat(full_path , "/");
            strcat(full_path, fl_get_filename(hdl->list,idx));
        }
        fs_new_path(hdl,full_path, NULL);
    }
    return refresh;
}

/** Scroll down */
static bool scroll_down(fs_handle hdl){
    int entries_nb = fl_get_entries_nb(hdl->list);
    
    hdl->idx_first_displayed += hdl->nb_lines;
    if (hdl->idx_first_displayed >= entries_nb ){
        hdl->idx_first_displayed = 0 ;
    }     
    return true;
}

/** Scroll up */
static bool scroll_up(fs_handle hdl){
    int entries_nb = fl_get_entries_nb(hdl->list);
    
    /* Test scroll up */             
    hdl->idx_first_displayed -= hdl->nb_lines ; 
    if ((hdl->idx_first_displayed <= -1 ) && (hdl->idx_first_displayed > -hdl->nb_lines )) {
        hdl->idx_first_displayed = 0;                        
    }                   
    if (hdl->idx_first_displayed <= -hdl->nb_lines) {    
        if ( entries_nb >  hdl->nb_lines){
            hdl->idx_first_displayed = entries_nb -  hdl->nb_lines ;
        } else {
            hdl->idx_first_displayed = 0;
        }
    }
    return true;
}


/** Create a file selector object
 *
 * \param[in] dfb DirectFb interface object
 * \param[in] win DirectFb window that will hold the file selector
 * \param[in] config File selector configuration
 *
 * \return a file selector handle or NULL in case of error.   
 * \warning  dfb and win pointers have to stay valid for the whole life of the object as they are not copied...
 */
fs_handle fs_create (IDirectFB  * dfb, IDirectFBWindow * win, const struct fs_config * config){
  fs_handle  handle ;
  DFBFontDescription font_dsc;
  DFBSurfaceDescription dsc;  
  int w1,h1,w2,h2;
  int shift_y;

  handle = calloc(1,sizeof(struct fs_data));
  if (handle == NULL)   
      goto error;
  handle->dfb = dfb;
  handle->win = win;

  if (!copy_config(config, &handle->config)) {
    goto error;
  }
  
  if (win->GetSurface(win,&handle->destination) != DFB_OK){
    return false;
  }

  /*clear_surface(handle->destination);*/
  srand(time(NULL));
  
  if (config->options.events_thread){
  if (  dfb->CreateInputEventBuffer ( dfb,
  	                               DICAPS_AXES | DICAPS_BUTTONS ,
  	                               DFB_FALSE, 
                                       &handle->evt_buffer ) != DFB_OK) { 
      goto error;  
  }
  }


  /* Display scroll Arrows */
  if (!display_obj(handle, FS_ICON_UP,NULL))
      goto error;
  if (!display_obj(handle, FS_ICON_DOWN,NULL))
    goto error;
    


  /* perform sanity checks : arrows icons must have same sizes... */ 
  handle->icon_surf[FS_ICON_UP]->GetSize(handle->icon_surf[FS_ICON_UP],&w1,&h1);
  handle->icon_surf[FS_ICON_DOWN]->GetSize(handle->icon_surf[FS_ICON_DOWN],&w2,&h2);
  if ((h1 != h2) ||
      (w1 != w2)) {
    goto error;
  }
  /* Update file and refresh zone that do not contain scroll arrows */
  handle->file_zone.w = config->geometry.pos.w - w1;
  handle->refresh_zone.w =  handle->file_zone.w;
 
  /* Load check and folder icons */
  if (!display_obj(handle,FS_ICON_CHECK,NULL))
    goto error;
  if (!display_obj(handle,FS_ICON_FOLDER,NULL ))
    goto error;
  /* perform sanity checks : check and folder icons must have same sizes... */ 
  handle->icon_surf[FS_ICON_CHECK]->GetSize(handle->icon_surf[FS_ICON_CHECK],&w1,&h1);
  handle->icon_surf[FS_ICON_FOLDER]->GetSize(handle->icon_surf[FS_ICON_FOLDER],&w2,&h2);
  if ((h1 != h2) ||
      (w1 != w2)) {
    goto error;
  }
  /* Update file_zone that does not contain these icons*/
  handle->file_zone.x = config->geometry.pos.x + w1;
  handle->file_zone.w -= w1;
  /*Font will have same height as these icons */ 
  font_dsc.height =  h1;


  handle->refresh_zone.x = config->geometry.pos.x; 
  /* Reserve preview zone if needed */ 
  if (config->options.preview_box){
    handle->preview_zone.w = (config->geometry.preview_width_ratio *  config->geometry.pos.w) / 100;
    handle->preview_zone.h = config->geometry.pos.h;
    handle->preview_zone.x = config->geometry.pos.x;
    handle->preview_zone.y = config->geometry.pos.y;
    handle->file_zone.x += handle->preview_zone.w;
    handle->file_zone.w -= handle->preview_zone.w;
    handle->refresh_zone.x += handle->preview_zone.w;
    handle->refresh_zone.w -= handle->preview_zone.w;
  }

  /* Compute number of viewable lines given font height and interflines... */
  handle->line_height = (font_dsc.height * 110) / 100;
  handle->nb_lines = config->geometry.pos.h / handle->line_height ;
  shift_y = (config->geometry.pos.h - (handle->nb_lines * handle->line_height)) / 2;

  /* End up initializing file and refresh zone */
  handle->file_zone.y = shift_y + config->geometry.pos.y;  
  handle->file_zone.h = config->geometry.pos.h - shift_y;   
  handle->refresh_zone.y = config->geometry.pos.y + shift_y ; 
  handle->refresh_zone.h = config->geometry.pos.h - shift_y ; 

  handle->idx_first_displayed = 0;

  /* Init font */
  font_dsc.flags = DFDESC_HEIGHT;
  if (dfb->CreateFont(dfb, config->graphics.font, &font_dsc, &handle->font) != DFB_OK) {
    goto error;
  }


  dsc.flags = DSDESC_WIDTH | DSDESC_HEIGHT/* | DSDESC_CAPS*/;
  dsc.width = handle->refresh_zone.w;
  dsc.height = handle->refresh_zone.h;
  /*dsc.caps = DSCAPS_DOUBLE;*/
  if (dfb->CreateSurface(dfb, &dsc, &handle->refresh_zone_surf ) != DFB_OK) {
    goto error;
  }
  /* Set encodings to latin 1 ISO 8859-1 instead of UTF-8 */
  handle->font->SetEncoding(handle->font,DTEID_OTHER);
  handle->refresh_zone_surf->SetFont (handle->refresh_zone_surf, handle->font);
 
  /* Create preview surface if needed */
  if (config->options.preview_box){
    dsc.flags = DSDESC_WIDTH | DSDESC_HEIGHT;
    dsc.width = handle->preview_zone.w;
    dsc.height = handle->preview_zone.h;
    if (dfb->CreateSurface(dfb, &dsc, &handle->preview_surf ) != DFB_OK) {
      goto error;
    }
  }
 
  /* Create RE for filtering */
  if (handle->config->folder.filter != NULL) {
    if (regcomp(&handle->compiled_re_filter, handle->config->folder.filter, REG_NOSUB | REG_EXTENDED | REG_ICASE )) {
        /* In case of error , no filter ! */
        free(handle->config->folder.filter);
        handle->config->folder.filter = NULL;
    }
  }

  /* Create file list */
  if (config->folder.pathname != NULL){
    handle->list = fl_create(config->folder.pathname,  
			     handle->config->folder.filter ? &handle->compiled_re_filter : NULL,
			     handle->config->options.multiple_selection );
    if ( handle->list == NULL){
      goto error;
    }
  }
 

  /* initial refresh */
/*  refresh_display(handle);*/
 
  /* thread creation */
  if (config->options.events_thread){
    handle->end_asked = false;
	pthread_create(&handle->thread, NULL, thread, handle);
  }
  
  handle->selected_item = 0;
  return handle;

error :
  release(handle); 
  return NULL;

}

/** Release a file selector object
 *
 *\param[in] hdl Handle of the fs object
 *
 *\return true if function succeeded. false otherwise
 */
bool fs_release(fs_handle hdl){
  return release(hdl);
}


/** Set the selection callback
 * \param[in] hdl Handle of the fs object
 * \retval true success
 * \retval false Failure
 */
bool fs_set_select_cb(fs_handle hdl, select_cb * f){
  if (hdl->config->options.preview_box){
	hdl->prev_cb = f;
	return true;  
  } else {
	return false;
  }
}


void fs_handle_key(fs_handle hdl, DFBInputDeviceKeyIdentifier id){    
    bool refresh = false;
    
    switch(id){    
    case DIKI_DOWN:
        if  (hdl->selected_item < (fl_get_entries_nb(hdl->list) - 1))
            hdl->selected_item++;
        refresh = true;
        break;
    case DIKI_UP:
        if (hdl->selected_item > 0)
            hdl->selected_item--;
        refresh = true;
        break;
    case DIKI_LEFT:
        refresh = scroll_up(hdl);
        hdl->selected_item = hdl->idx_first_displayed;
        break;
    case DIKI_RIGHT:
        refresh = scroll_down(hdl);
        hdl->selected_item = hdl->idx_first_displayed;
        break;
    default: 
        refresh = item_selection(hdl, hdl->selected_item);
        break;    
    }    
    if (hdl->selected_item >= (hdl->idx_first_displayed + hdl->nb_lines)){
        hdl->idx_first_displayed++;
    }
    if (hdl->selected_item < hdl->idx_first_displayed){
        hdl->idx_first_displayed--;
    }
    if (refresh){
        refresh_display(hdl);
    }          
}



/** Handle click on the object
 *
 * \param[in] x x coordonate of the click (relative to the window)
 * \param[in] y y coordonate of the click (relative to the window)
 *
 */
void fs_handle_click(fs_handle hdl,int x, int y){
  int idx; 
  bool refresh = false;
  int entries_nb = fl_get_entries_nb(hdl->list);

  if( (x >= hdl->config->geometry.pos.x) && 
    (x <= (hdl->config->geometry.pos.x + hdl->config->geometry.pos.w )) && 
    (y >= hdl->config->geometry.pos.y) && 
    (y <= (hdl->config->geometry.pos.y + hdl->config->geometry.pos.h ))){

    
    if ((x >= hdl->arrow_list[FS_ICON_UP].x) &&
        (x <= (hdl->arrow_list[FS_ICON_UP].x + hdl->arrow_list[FS_ICON_UP].w)) &&
        (y >= hdl->arrow_list[FS_ICON_UP].y) &&
        (y <= (hdl->arrow_list[FS_ICON_UP].y + hdl->arrow_list[FS_ICON_UP].h))){          
          /* Scroll up */
          refresh = scroll_up(hdl);
    }
    
    if ((x >= hdl->arrow_list[FS_ICON_DOWN].x) &&
        (x <= (hdl->arrow_list[FS_ICON_DOWN].x + hdl->arrow_list[FS_ICON_UP].w)) &&
        (y >= hdl->arrow_list[FS_ICON_DOWN].y) &&
        (y <= (hdl->arrow_list[FS_ICON_DOWN].y + hdl->arrow_list[FS_ICON_UP].h))){
          /* Scroll down */                             
          refresh = scroll_down(hdl);
    }
    if ((x >= hdl->refresh_zone.x) &&
        (x <= (hdl->refresh_zone.x + hdl->refresh_zone.w)) &&
        (y >= hdl->refresh_zone.y) &&
        (y <= (hdl->refresh_zone.y + hdl->refresh_zone.h))) {
        /* Test a file selection */
        idx = hdl->idx_first_displayed + ((y - hdl->refresh_zone.y) / hdl->line_height);
        if (idx < entries_nb) {
            refresh = item_selection(hdl, idx);
        }
    }
  }
  if (refresh){
    refresh_display(hdl);
  }        
  return;
}

/** Change current folder for file selector object 
 *
 * \param[in] hdl Handle of the fs object
 * \param[in] path New path 
 * \param[in] filter Filter to apply. Empty string means no filter. NULL means do not change existing filter
 *
 * \retval true success
 * \retval false Failure
 *
 */
bool fs_new_path(fs_handle hdl, const char * path, const char * filter){
  struct stat buf;

  /* Check path exists and is a folder */
  if (stat(path, &buf) != 0)
    return false;
  if (!S_ISDIR(buf.st_mode))
    return false;  

  if (filter != NULL){
    if (hdl->config->folder.filter != NULL){
      free(hdl->config->folder.filter);
      regfree(&hdl->compiled_re_filter);
    }

    if (strlen(filter) > 0){
      hdl->config->folder.filter = strdup(filter);
      if (regcomp(&hdl->compiled_re_filter, filter, REG_NOSUB | REG_EXTENDED | REG_ICASE )) {
        free(hdl->config->folder.filter);
        hdl->config->folder.filter = NULL;
      }
    } else {
      hdl->config->folder.filter = NULL;
    }
  }
  fs_unselect_all(hdl);
  hdl->idx_first_displayed = 0;
  fl_release(hdl->list);
  hdl->list = fl_create(path, hdl->config->folder.filter ? &hdl->compiled_re_filter : NULL,  hdl->config->options.multiple_selection );
  hdl->selected_item = 0;
  refresh_display(hdl);
  return true;
}

/** Select an item in file selector given its index
 *
 * \param[in] hdl Handle of the fs object
 * \param[in] idx Index of the file to select 
 *
 * \retval true success
 * \retval false Failure
 *
 * \note In multiple selection mode, selecting an already selected item, removes it from the selection  !
 */
bool fs_select(fs_handle hdl, int idx) {
  bool new_state = true;

  if (!fl_select_by_pos(hdl->list, idx, &new_state)){
    return false;
  }

  if ( (hdl->prev_cb != NULL)&& (new_state) ){
      char * full_path = malloc(strlen(fl_get_basename(hdl->list)) + strlen(fl_get_filename(hdl->list,idx)) + 2);
      /* Refresh here coz callback can take some time */
      refresh_display(hdl);
      strcpy(full_path, fl_get_basename(hdl->list));
      strcat(full_path,"/");
      strcat(full_path, fl_get_filename(hdl->list,idx));      
      clear_surface(hdl->preview_surf);
      hdl->prev_cb(hdl, full_path, fl_is_selected(hdl->list,idx)?FS_EVT_SELECT:FS_EVT_UNSELECT);                  
      free(full_path);
      hdl->destination->Blit(hdl->destination, hdl->preview_surf, NULL, hdl->preview_zone.x, hdl->preview_zone.y);
      hdl->destination->Flip(hdl->destination, NULL, DSFLIP_WAITFORSYNC);
  }

  return true;
}

/** Select all the regular files in the file selector
 *
 * \param[in] hdl Handle of the fs object 
 *
 * \retval true success
 * \retval false Failure
 *
 * \note Of course, the file selector has to be in multiple selection mode
 */
bool fs_select_all(fs_handle hdl) {
  
  fl_select_all(hdl->list);
  refresh_display(hdl);
  return true;
}


/** Unselect any selected files in the file selector
*
* \note This function is also called before releasing file list (on folder change for instance)
*/
bool  fs_unselect_all(fs_handle hdl){
  int i;
  char * full_path;

  for(i=0; i<fl_get_entries_nb(hdl->list); i++){  
    if (fl_is_selected(hdl->list,i)) {
      if (hdl->prev_cb != NULL){
        full_path = alloca(strlen(fl_get_basename(hdl->list)) + strlen(fl_get_filename(hdl->list,i)) + 2);
	strcpy(full_path,fl_get_basename(hdl->list));
	strcat(full_path,"/");
	strcat(full_path,fl_get_filename(hdl->list,i));        
        hdl->prev_cb(hdl,full_path,FS_EVT_UNSELECT);
      }
      fl_unselect_by_pos(hdl->list,i);
      }
  }
  refresh_display(hdl); 
  return true;
}

/** Get selected file for a single selector
 *
 * \param[in] hdl Handle of the fs object
 *
 * \return the selected filename or NULL if no file is selected
 *
 * \note This call will fail on an multiple selection enabled object
 * \warning Not thread safe (and potential race if user is changing folder while calling this) but who cares (not a use case)
 *
 */
const char * fs_get_single_selection(fs_handle hdl){
  static char return_filename[PATH_MAX];
  const char * file;

  file = fl_get_single_selection(hdl->list);
  if (file == NULL){
    return NULL;
  }

  if ((strlen(fl_get_basename(hdl->list)) + strlen(file) + 2) > PATH_MAX){
    return NULL;
  }  
  strcpy(return_filename, fl_get_basename(hdl->list));
  strcat(return_filename,"/");
  strcat(return_filename, file);
  return return_filename;       
}


/** Select a given filename in file selector and returns its index
 *
 * \param[in] hdl Handle of the fs object
 * \param[in] filename Filename to select
 *
 * \return The filename index if filenmae found. -1 if filename is not found.
 *
 */
int fs_select_filename(fs_handle hdl, const char * filename){
  int i;
  
  for (i=0; i <fl_get_entries_nb(hdl->list) ; i++){
    if (strcmp(fl_get_filename(hdl->list,i), filename) == 0){
      fs_select(hdl, i);
      return i;
    }
  }
  return -1;
}


/** Set first displayed item in file selector
 *
 * \param[in] hdl Handle of the fs object
 * \param[in] i   Index of the filename to dispaly as first item in file selector
 *
 * \retval true success
 * \retval false Failure
 */
bool fs_set_first_displayed_item(fs_handle hdl, int i){
  if ((i < 0) || 
      (i >= fl_get_entries_nb(hdl->list)))
    return false;
  hdl->idx_first_displayed = i;
  refresh_display(hdl);  
  return true;
}


/** Retrieve a selected files list objec
 *
 * \param[in] hdl Handle of the fs object
 *
 * \return the selected files list enumerator containing the selected files or NULL if error
 */
flenum   fs_get_selection(fs_handle hdl){
  return fl_get_selection(hdl->list);
}

/** Retrieve the preview surface associated with the file selector object
*/
IDirectFBSurface * fs_get_preview_surface(fs_handle hdl){
  if (hdl == NULL){
    return NULL;
  } else {
    return hdl->preview_surf;
  }
}


/** Retrieve the window that hold the  file selector object
*/
IDirectFBWindow * fs_get_window(fs_handle hdl){
  if (hdl == NULL){
    return NULL;
  } else {
    return hdl->win;
  }
}

/** Retrieve the configuration associated to the  file selector object
*/
const struct fs_config * fs_get_config(fs_handle hdl){
  if (hdl == NULL){
    return NULL;
  } else {
    return hdl->config;
  }
}

/** retrieve the current folder */
const char * fs_get_folder(fs_handle hdl){
  return fl_get_basename(hdl->list);
}

