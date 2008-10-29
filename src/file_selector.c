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
 * $URL: $
 * $Rev: $
 * $Author: $
 * $Date: $
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
#include <linux/limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <directfb.h>
#include <pthread.h>
#include <regex.h>

#include "file_selector.h"
#include "debug.h"

/** File list used as an enumerator to provide file selection to the outside of the module */
struct _fl_handle {
  int entries_number;                          /**< Number of entries in filenames array */
  int current_idx;                             /**< Current index */
  char ** filenames;                           /**< Array holding filenames (full pathname) */
};

#define FILE_LIST_INC 32
/** Internal file list representation for the file selector */
struct file_list{
  int entries_number;                          /**< Number of entries in the following arrays...*/  
  char * basename;                             /**< Folder basename */
  int last_selected;                           /**< Index of last selected item */
  int max_entries_number;                      /**< Maximum entries that can be stored in the object */

  char ** filenames;                           /**< Array of entries_number elements containing the filenames (including folders) */
  bool * is_selected;                          /**< Array of entries_number elements indicating whether the corresponding file is selected or not*/
  bool * is_folder;                            /**< Array of entries_number elements indicating whether the corresponding file is a folder or a regular file*/
};

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
  struct file_list list;                       /**< Files list displayed by the object */
  pthread_t thread;                            /**< Thread that handles input events */
  bool end_asked;                              /**< Flag to ask for thread events termination */
  select_cb *prev_cb;                          /**< Selection callback */
  regex_t compiled_re_filter;                  /**< Compiled re filter to apply filter */
} ;

/** Return min between two int values
*/
static inline int  min(int x, int y ) {
  if (x<y) 
    return x; 
  else 
    return y;
}

/** Match string against a RE
 * \return true if match 
 */
static bool match(const char *string, const regex_t * re)
{
    return !regexec(re, string, (size_t) 0, NULL, 0);    
}

/** Concatene basename and filename
 * \warning no check : dest must be big enough 
 */
static inline void get_fullpath(fs_handle hdl, int i, char* dest){
  sprintf(dest,"%s/%s", hdl->list.basename, hdl->list.filenames[i]);
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


/** Release file list object */
static void fl_release(struct file_list *fl){  
  for (int i=0; i<fl->entries_number; i++){
    free(fl->filenames[i]);
  }
  free(fl->filenames);
  free(fl->is_selected);
  free(fl->is_folder);  
  free(fl->basename);  
  memset(fl, 0, sizeof(*fl));
  return;
}


/** Find an entry in file list 
 *
 * \return index of the entry if found or index where it should be added to let the list sorted
 */
static int fl_find_pos(struct file_list *fl, const char * filename, bool is_folder, bool * find){
  int i;  
  int ret_cmp = 1;
  *find= false;

  if (is_folder){
    i = fl->entries_number-1;
    while  ((i>=0) && (fl->is_folder[i])){ 
      ret_cmp = strcmp(fl->filenames[i],filename);
      if (ret_cmp<=0)
        break;
      i--;
    }
  } else{
    i = 0 ;
    
    while  ((i<fl->entries_number) && (!fl->is_folder[i])){ 
      ret_cmp = strcmp(fl->filenames[i],filename);
      if ( ret_cmp >= 0)
        break;
      i++;
    }
  }
  
  if (is_folder){  
      i++;  
  }

  if (ret_cmp ==0){
    *find = true;
  }
  return i;
}

/** Insert a slot in file list 
 *
 * \warning  No check on file list size (has to be big enough). Only called by :fl_add() that ensures this...
 */
static bool fl_shift(struct file_list *fl, int idx){
  int i ;
  for (i=fl->entries_number-1; i>=idx;i--){
     fl->is_folder[i+1] = fl->is_folder[i];
     fl->is_selected[i+1] = fl->is_selected[i];
     fl->filenames[i+1] = fl->filenames[i];
  }
  return true;
}

/** Add a filename to a file list
 *
 * Adds an entry to file lits. Itmay be a regular file or a folder.
 * File list is automatically allocated or reallocated to contain this new entry.
 *
 * \param[in] fl  file list object 
 * \param[in] filename  filename to add
 * \param[in] is_folder Explicit whether it is a folder or not
 *
 * \retval true success
 * \retval false Failure
 *
 * \note the file list is always sorted
*/
static bool fl_add(struct file_list *fl, char* filename, bool is_folder){
  int index;
  bool find;

  if (fl->max_entries_number <= fl->entries_number){
    fl->max_entries_number += FILE_LIST_INC;
    fl->filenames = realloc(fl->filenames, fl->max_entries_number * sizeof(*fl->filenames));
    fl->is_selected = realloc(fl->is_selected, fl->max_entries_number * sizeof(*fl->is_selected));
    fl->is_folder = realloc(fl->is_folder, fl->max_entries_number * sizeof(*fl->is_folder));
    if ((fl->filenames == NULL ) ||
        (fl->is_selected == NULL) ||
        ( fl->is_folder == NULL ) ) {
        return false;
    }
  }
  
  index = fl_find_pos(fl, filename, is_folder, &find);
  if (index < fl->entries_number){
    fl_shift(fl,index);
  }

  fl->entries_number++;
  fl->is_folder[index] = is_folder;
  fl->is_selected[index] = false;
  fl->filenames[index] = strdup(filename);
  return true;
}

/** Create a file list object 
 *
 * \param[in] path path from which the file list has to be built
 * \param[in] re compiled regular expression to use to filter filenames
 * \param[in] fl file list handle
 *
 * \retval true success
 * \retval false Failure
 */
static bool fl_create(const char * path, regex_t *re, struct file_list * fl){
  struct dirent* dir_ent;
  DIR*   dir;
  struct stat ftype;
  char   fullpath [PATH_MAX + 1];
  bool ret = true;

  if ((dir = opendir (path)) == NULL)
    return false;

  fl->basename = strdup(path);
  while ( (dir_ent = readdir ( dir )) != NULL ) {
          snprintf(fullpath,PATH_MAX,"%s/%s",path,  dir_ent->d_name);
          if (stat (fullpath, &ftype) < 0 ) {
            continue;
          }
          if (strcmp( dir_ent->d_name, ".")) {
            if ((S_ISDIR (ftype.st_mode)) ||
                (re == NULL) ||
                (match(dir_ent->d_name,re))){ 
              if (!fl_add(fl, dir_ent->d_name, S_ISDIR (ftype.st_mode))){
                ret = false;
                break;
              }
            }
          }
  }
  closedir (dir);
  return ret;
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
  fl_release(&hdl->list);
  
  if (hdl->config != NULL){
    free(hdl->config);
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
    /*hdl->destination->GetPixelFormat( hdl->destination, &dsc.pixelformat );*/
   
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
  int y = 0;
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
  max_idx = hdl->idx_first_displayed + min(hdl->nb_lines, hdl->list.entries_number - hdl->idx_first_displayed);

  /* Display zone content */
  for (int i=hdl->idx_first_displayed; i< max_idx; i++){  

    
    hdl->refresh_zone_surf->DrawString( hdl->refresh_zone_surf, 
                                        hdl->list.filenames[i],
                                        -1,
                                        hdl->file_zone.x - hdl->refresh_zone.x ,  
                                        y, 
                                        DSTF_TOPLEFT);
    if (hdl->list.is_selected[i]){
      display_obj(hdl,FS_ICON_CHECK,&point);
    }
    if (hdl->list.is_folder[i]){
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
  hdl->destination->Flip(hdl->destination, &region, 0);
  
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
  
  handle->config = malloc (sizeof *config);
  if (handle->config == NULL){
    goto error;
  }
  *handle->config = *config;
  
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


  dsc.flags = DSDESC_WIDTH | DSDESC_HEIGHT;
  dsc.width = handle->refresh_zone.w;
  dsc.height = handle->refresh_zone.h;
  if (dfb->CreateSurface(dfb, &dsc, &handle->refresh_zone_surf ) != DFB_OK) {
    goto error;
  }
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
    if (regcomp(&handle->compiled_re_filter, handle->config->folder.filter, REG_NOSUB | REG_EXTENDED)) {
       /* In case of error , compile a "*" RE  */
      regcomp(&handle->compiled_re_filter, "*", REG_NOSUB | REG_EXTENDED);
    }
  }

  /* Create file list */
  if (!fl_create(config->folder.pathname,  handle->config->folder.filter ? &handle->compiled_re_filter : NULL, &handle->list)){
    goto error;
  }
  
  /* initial refresh */
  refresh_display(handle);
    
  /* thread creation */
  if (config->options.events_thread){
    handle->end_asked = false;
    pthread_create(&handle->thread, NULL, thread, handle);
  }
    
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
  hdl->prev_cb = f;
  return true;  
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

  if( (x >= hdl->config->geometry.pos.x) && 
    (x <= (hdl->config->geometry.pos.x + hdl->config->geometry.pos.w )) && 
    (y >= hdl->config->geometry.pos.y) && 
    (y <= (hdl->config->geometry.pos.y + hdl->config->geometry.pos.h ))){

    
    if ((x >= hdl->arrow_list[FS_ICON_UP].x) &&
        (x <= (hdl->arrow_list[FS_ICON_UP].x + hdl->arrow_list[FS_ICON_UP].w)) &&
        (y >= hdl->arrow_list[FS_ICON_UP].y) &&
        (y <= (hdl->arrow_list[FS_ICON_UP].y + hdl->arrow_list[FS_ICON_UP].h))){
          /* Test scroll up */             
          hdl->idx_first_displayed -= hdl->nb_lines ; 
          if ((hdl->idx_first_displayed <= -1 ) && (hdl->idx_first_displayed > -hdl->nb_lines )) {
            hdl->idx_first_displayed = 0;                        
          }                   
          if (hdl->idx_first_displayed <= -hdl->nb_lines) {
            if (hdl->list.entries_number >  hdl->nb_lines){
              hdl->idx_first_displayed = hdl->list.entries_number -  hdl->nb_lines ;
            } else {
              hdl->idx_first_displayed = 0;
            }
          }
          refresh = true;
    }
    if ((x >= hdl->arrow_list[FS_ICON_DOWN].x) &&
        (x <= (hdl->arrow_list[FS_ICON_DOWN].x + hdl->arrow_list[FS_ICON_UP].w)) &&
        (y >= hdl->arrow_list[FS_ICON_DOWN].y) &&
        (y <= (hdl->arrow_list[FS_ICON_DOWN].y + hdl->arrow_list[FS_ICON_UP].h))){
        /* Test scroll down */                   
          hdl->idx_first_displayed += hdl->nb_lines;
          if (hdl->idx_first_displayed >= hdl->list.entries_number ){
              hdl->idx_first_displayed = 0 ;
          }                   
          refresh = true;
    }
    if ((x >= hdl->refresh_zone.x) &&
        (x <= (hdl->refresh_zone.x + hdl->refresh_zone.w)) &&
        (y >= hdl->refresh_zone.y) &&
        (y <= (hdl->refresh_zone.y + hdl->refresh_zone.h))) {
        /* Test a file selection */
        idx = hdl->idx_first_displayed + ((y - hdl->refresh_zone.y) / hdl->line_height);
        if (idx < hdl->list.entries_number) {
        if (!hdl->list.is_folder[idx]) {
          /* Regular file */
          fs_select(hdl,idx);        
          refresh = true;   
        } else {
            /* Folder */
          char * full_path = alloca(strlen(hdl->list.basename) + strlen(hdl->list.filenames[idx]) + 2);
          char * slash_index;
          strcpy(full_path, hdl->list.basename);
          if ((strcmp(hdl->list.filenames[idx],"..") == 0) &&
              ((slash_index = strrchr(full_path,'/')) != NULL) &&
              (strcmp(slash_index+1,"..")!=0)) {
              /* We cut the last folder when going up in a tree rather than concatening '..' => It simplifies pathnames */
              *slash_index = '\0';
          } else {                      
            strcat(full_path , "/");
            strcat(full_path, hdl->list.filenames[idx]);
          }
          fs_new_path(hdl,full_path);
        }
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
 *
 * \retval true success
 * \retval false Failure
 *
 */
bool fs_new_path(fs_handle hdl, const char * path){
  struct stat buf;
  
  /* Check path exists and is a folder */  
  if (stat(path, &buf) != 0)
    return false;
  if (!S_ISDIR(buf.st_mode))
    return false;  

  hdl->idx_first_displayed = 0;
  fl_release(&hdl->list);
  fl_create(path, hdl->config->folder.filter ? &hdl->compiled_re_filter : NULL, &hdl->list);
  /* TODO a deplacer ? => provoquer un appel de deselction ?*/
  if (hdl->preview_surf)
    clear_surface(hdl->preview_surf);
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

  if ( (idx < 0) || (idx >= hdl->list.entries_number)){
    return false;
  }

  if  (hdl->list.is_folder[idx])
    return false;

  if (hdl->config->options.multiple_selection){
    hdl->list.is_selected[idx] = !hdl->list.is_selected[idx];
  } else {
    hdl->list.is_selected[hdl->list.last_selected] = false;
    hdl->list.is_selected[idx] = true; 
    hdl->list.last_selected = idx;    
  }
  
  if (hdl->prev_cb != NULL) {
      char * full_path = malloc(strlen(hdl->list.basename) + strlen(hdl->list.filenames[idx]) + 2);
      get_fullpath (hdl,idx, full_path);
      clear_surface(hdl->preview_surf);
      hdl->prev_cb(hdl->preview_surf, full_path, hdl->list.is_selected[idx]);                  
      free(full_path);     
      hdl->destination->Blit(hdl->destination, hdl->preview_surf, NULL, hdl->preview_zone.x, hdl->preview_zone.y);
      hdl->destination->Flip(hdl->destination, NULL, 0);
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
  int i;

  if (!hdl->config->options.multiple_selection)
    return false;

  for(i=0; i<hdl->list.entries_number; i++){  
    if (!hdl->list.is_selected[i])
       fs_select(hdl, i);      
  }
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
  if (hdl->config->options.multiple_selection) {
    return NULL;
  }

  if (hdl->list.is_selected[hdl->list.last_selected]) {
      if ((strlen(hdl->list.basename) + strlen(hdl->list.filenames[hdl->list.last_selected]) + 2) > PATH_MAX){
        return NULL;
      }
      get_fullpath(hdl,hdl->list.last_selected,return_filename);      
      return return_filename;     
  }  
  return NULL;
}

/** Returns the number of selected items
 *
 * \param[in] hdl Handle of the fs object
 *
 * \return the number of selected files
 *
 * \warning Once again not so thread safe as file list is not protected. But again, dont really care here...
 */
int fs_get_selected_number(fs_handle hdl){
  int i, nb_selected;
  nb_selected = 0;
  
  for (i=0; i< hdl->list.entries_number; i++){
    if (hdl->list.is_selected[i]) nb_selected++;                 
  }

  return nb_selected;
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
  
  for (i=0; i < hdl->list.entries_number ; i++){
    if (strcmp(hdl->list.filenames[i], filename) == 0){
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
      (i >= hdl->list.entries_number))
    return false;
  hdl->idx_first_displayed = i;
  refresh_display(hdl);  
  return true;
}


/** Retrieve a selected files list objec
 *
 * \param[in] hdl Handle of the fs object
 *
 * \return the selected files list object containing the selected files or NULL if error
 */
fslist fs_get_selection(fs_handle hdl){
  struct _fl_handle *fl;
  int i,j;

  fl = malloc(sizeof(struct _fl_handle));  
  if (fl != NULL){
    fl->entries_number = fs_get_selected_number(hdl);
    fl->current_idx = 0;
    fl->filenames=malloc( fl->entries_number * sizeof(*fl->filenames) );
    if (fl->filenames == NULL){
      free(fl);
      fl=NULL;
    } else {
      j=0;
      for(i=0;i<fl->entries_number ;i++){
        while ((j<hdl->list.entries_number) && (!hdl->list.is_selected[j])){j++;}
        if (j<hdl->list.entries_number){
          fl->filenames[i] = malloc(strlen(hdl->list.basename) + strlen(hdl->list.filenames[j]) + 2);
          get_fullpath(hdl, j, fl->filenames[i]);       
          j++;
        }
      }
    }
  }
  return fl;
}

/** Get next file from selected files list
 *
 *\param[in] fl Handle on the selected file list
 *\param[in] is_random Explicit whether we want to retrieve filenames in a random order or not
 *
 *\return the next filename or NULL if no more filename 
 */
const char * fslist_get_next_file(fslist fl, bool is_random){
  int i = fl->current_idx;

  if (fl->current_idx >= fl->entries_number){
    return NULL;
  }
  
  if (is_random){
    int rand_offset;
    char * temp;

    /* Choose random filename */
    rand_offset = (fl->entries_number - fl->current_idx) *  (((double) rand())  / (((double)(RAND_MAX)) + 1.0));
    
    /* Switch random remaining filename and i-th item */
    temp = fl->filenames[i + rand_offset];  
    fl->filenames[i + rand_offset] = fl->filenames[i];
    fl->filenames[i] = temp;
  }
  fl->current_idx++;  
  return fl->filenames[i];
}

/** Release a selected files list object
 *
 * \param[in] fl Handle on the selected file list
 *
 * \retval true success
 * \retval false Failure
 */
bool fslist_release(fslist fl){
  int i;
  for (i=0; i<fl->entries_number; i++){
    free(fl->filenames[i]);
  }
  free(fl->filenames);
  
  free(fl);
  return true;
}
