  /**
 * \file window.c
 * \author nullpointer & wolfgar
 * \brief This module provides window creation and handling facilities 
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


#include "dictionary.h"
#include "iniparser.h"
#include "debug.h"
#include "file_selector.h"
#include "list.h"
#include "window.h"


#define DEFAULT_FONT_HEIGHT 15

/** Data related to a window object  */
struct _gui_window{
        IDirectFB * dfb;                       /**< Dfb interafce */
        IDirectFBDisplayLayer  *layer;         /**< dfb layer related to the current window object */
        IDirectFBWindow * win;                 /**< dfb window related to the current window object */      
	IDirectFBSurface * background_surface; /**< dfb window surface */                
	struct list_object * controls;         /**< list of controls */        
        IDirectFBFont * font ;
        DFBColor color;                        /**< Text color */ 
        bool is_detached;                      /**< If detahed, the window is not hndled in the window stack and does not receives events */
};

/**/
static int screen_height, screen_width;
static bool is_rotated = false;

/* Keep track of the different created windows as a stack model */
#define GUI_WINDOW_MAX_NB 4

static struct {
  gui_window winlist[GUI_WINDOW_MAX_NB];
  int current_win;
} win_stack =  { .current_win = -1 };


static inline char * get_key(int ctrl_id,const char * ctrl_param){
  static char key[200];
  snprintf(key, sizeof(key) -1,"control_%02d:%s", ctrl_id, ctrl_param);
  return key;
}

/** Retrieve screen size to use them as default values when window sizes are not set */
static void probe_screen( IDirectFB *dfb){
  DFBSurfaceDescription dsc;
  IDirectFBSurface * primary;

  dsc.flags = DSDESC_CAPS;
  dsc.caps = DSCAPS_PRIMARY | DSCAPS_FLIPPING;
  if (dfb->CreateSurface( dfb, &dsc, &primary ) != DFB_OK ){
    return;
  }
  primary->GetSize( primary, &screen_width, &screen_height );
  if (screen_width<screen_height){
     is_rotated = true;
  }
  primary->Release(primary); 
}

/** Load a font  
 *
 * \param filename of the font
 * \param height size of the font
 *
 * \return DirectFB font or NULL if error
 */
static IDirectFBFont * load_font(gui_window win, char * filename, int height ){
	DFBFontDescription desc;
	IDirectFBFont * font;	

	PRINTDF( "Loading font <%s> <%d>\n", filename, height );
	desc.flags = DFDESC_HEIGHT;
	desc.height = height;
	if (win->dfb->CreateFont( win->dfb, filename, &desc, &font ) != DFB_OK){
          return NULL;
        } else {
      	  return font;
        }
}

/** Load an image to a DirectFB surface of a control
 *
 * \param filename of the bitmap
 *
 * \return DirectFB surface, NULL if error
 */
static IDirectFBSurface * load_image_to_surface(struct gui_control * ctrl, char * filename ){	
	IDirectFBImageProvider *provider;
	DFBSurfaceDescription dsc;
	IDirectFBSurface * surface = NULL;
	PRINTDF( "load_image_to_surface <%s>\n", filename );

	if (ctrl->win->dfb->CreateImageProvider( ctrl->win->dfb, filename, &provider ) != DFB_OK)
          return NULL;

	if ((provider->GetSurfaceDescription (provider, &dsc) == DFB_OK) &&
	    ( ctrl->win->dfb->CreateSurface( ctrl->win->dfb, &dsc, &surface ) == DFB_OK) ){
          if ((ctrl->zone.w != -1) &&
              (ctrl->zone.w != -1)){
           provider->RenderTo( provider, surface, &ctrl->zone ) ;
          } else {
	   provider->RenderTo( provider, surface, NULL ) ;
           ctrl->zone.w = dsc.width;
           ctrl->zone.h = dsc.height;
          }
        }
	provider->Release( provider );
        
	return surface;
}

static  bool init_window(gui_window win,  dictionary * ini){
  DFBRectangle zone;
  char* s;
  DFBWindowDescription  desc;     
  IDirectFBImageProvider *provider;
  int opacity, font_height, tmp;

  zone.x = iniparser_getint(ini, "general:x", 0);
  zone.y = iniparser_getint(ini, "general:y", 0);  
  zone.w = iniparser_getint(ini, "general:w", screen_width-zone.x);
  zone.h = iniparser_getint(ini, "general:h", screen_height-zone.y);
  if (is_rotated) { 
    tmp =  zone.y;
    zone.y = zone.x;
    zone.x = screen_width-zone.h -tmp;
  }
  win->color.r = iniparser_getint(ini, "general:r", 0);
  win->color.g = iniparser_getint(ini, "general:g", 0);
  win->color.b = iniparser_getint(ini, "general:b", 0);
  win->color.a = iniparser_getint(ini, "general:a", 0xFF);
  s = iniparser_getstring(ini, "general:background", NULL);
  

  desc.flags = ( DWDESC_POSX | DWDESC_POSY |
                 DWDESC_WIDTH | DWDESC_HEIGHT );
  desc.posx   = zone.x;
  desc.posy   = zone.y;
  desc.width  = zone.w;
  desc.height = zone.h;
  

  if (  (win->layer->CreateWindow (win->layer, &desc, &win->win)  != DFB_OK )  ||
        (win->win->GetSurface(win->win,&win->background_surface) != DFB_OK) ){
      return false;
    }
  if (! is_rotated){
    win->win->LowerToBottom(win->win);
  }
  
  
    win->background_surface->SetBlittingFlags(win->background_surface,DSBLIT_BLEND_ALPHACHANNEL);
  
 
  if (s != NULL) {  
    win->dfb->CreateImageProvider( win->dfb, s, &provider );
    provider->RenderTo( provider, win->background_surface, NULL ) ;
    provider->Release(provider);
  } else {
    win->background_surface->SetColor(win->background_surface, 0,0,0,0xFF);
    win->background_surface->FillRectangle(win->background_surface,0,0, desc.width,desc.height);
  }
  
  font_height = iniparser_getint(ini, "general:font_height", DEFAULT_FONT_HEIGHT);
  s = iniparser_getstring(ini, "general:font", NULL);
  if( s != NULL ) win->font = load_font (win, s, font_height );
  if (win->font != NULL){
    win->background_surface->SetColor(win->background_surface,win->color.r,  win->color.g, win->color.b, win->color.a);
    win->background_surface->SetFont(win->background_surface, win->font);
  }
  opacity = iniparser_getint(ini, "general:opacity", 0xFF);  
  if (is_rotated){
    win->win->SetOpacity(win->win, 255 );
    win->win->SetRotation(win->win, 270);
  } else {
    win->win->SetOpacity(win->win, opacity);
  }

  win->is_detached = iniparser_getint(ini, "general:detached", 0);
  return true; 
}

static void draw_text(struct gui_control * ctrl,  dictionary * ini , int num_control ){
  DFBColor color;
  int font_height;
  IDirectFBFont * font ;
  char * s;
  gui_window  window = ctrl->win;

  
  font = NULL;
  font_height = iniparser_getint(ini, get_key(num_control,"font_height"), DEFAULT_FONT_HEIGHT);
  s = iniparser_getstring(ini, get_key(num_control,"font"), NULL);
  if( s != NULL ) font = load_font (window, s, font_height );
  color.r = iniparser_getint(ini, get_key(num_control,"r"), 0);
  color.g = iniparser_getint(ini, get_key(num_control,"g"), 0);
  color.b = iniparser_getint(ini, get_key(num_control,"b"), 0);
  color.a = iniparser_getint(ini, get_key(num_control,"a"), 0xFF);
  s = iniparser_getstring(ini, get_key(num_control,"msg"),  NULL);
  if (font == NULL){
    window->background_surface->GetFont(window->background_surface, &font);
  }
  if (( s!=NULL) && (font != NULL)){
    window->background_surface->SetColor(window->background_surface,color.r,  color.g, color.b, color.a);
    window->background_surface->SetFont(window->background_surface, font);
    window->background_surface->DrawString( window->background_surface , s, -1,ctrl->zone.x, ctrl->zone.y, DSTF_TOPLEFT);
    font->GetHeight(font, &ctrl->zone.h);
    font->GetStringWidth(font, s, -1, &ctrl->zone.w);  	
    ctrl->obj = font;
    /*font->Release(font);*/
    /* Restore default font */
    if (window->font){
      window->background_surface->SetColor(window->background_surface,window->color.r,  window->color.g, window->color.b, window->color.a);
      window->background_surface->SetFont(window->background_surface, window->font);
    } 
  }
}


static  fs_handle load_fs_ctrl(struct gui_control * ctrl,  dictionary * ini , int num_control ){	
  #define RES_FOLDER "./res/icon/"
  #define FONT_FOLDER "./res/font/"

  char * s;
  struct fs_config conf = {
                            .graphics = { .filename = {RES_FOLDER "scroll_up_0.png",
                                                       RES_FOLDER "scroll_down_0.png" ,
                                                       RES_FOLDER "check_0.png",
                                                       RES_FOLDER "folder_0.png"},
                                          .font = FONT_FOLDER "decker.ttf",
                                          .font_color = {.a=0xff , .r=188, .g =133 , .b =215 },
                                        },

                            .geometry = { .preview_width_ratio = 30, },

                            .options ={ .preview_box = true,
                                        .multiple_selection = true,
                                        .events_thread = false
                            },

                            .folder = {
                              .filter= NULL /*"^.*\\.(avi|zip)$"*/,
                              .pathname=NULL,
                            }
                           };

    conf.geometry.pos = ctrl->zone;
    s = iniparser_getstring(ini, get_key(num_control,"font"), NULL);
    if (s != NULL){
      conf.graphics.font = s;
    }
    conf.graphics.font_color.a = iniparser_getint(ini, get_key(num_control,"a"), 0xFF);
    conf.graphics.font_color.r = iniparser_getint(ini, get_key(num_control,"r"), 188);
    conf.graphics.font_color.g = iniparser_getint(ini, get_key(num_control,"g"), 133);
    conf.graphics.font_color.b = iniparser_getint(ini, get_key(num_control,"b"), 215);
    conf.options.multiple_selection = iniparser_getint(ini, get_key(num_control,"multiple_select"), 0);
    conf.geometry.preview_width_ratio = iniparser_getint(ini,get_key(num_control,"prev_ratio"),0);
    if  ((conf.geometry.preview_width_ratio>0) && 
        (conf.geometry.preview_width_ratio<100)){
      conf.options.preview_box = true;
    } else {
      conf.options.preview_box = false;
    }
    return fs_create (ctrl->win->dfb, ctrl->win->win, &conf);
}

/** Loading of a window configuration
 *
 * \param filename window configuration file
 * \param window structure where to store configuration
 *
 * \return true on success, false on failure
 */
static bool load_window_config( const char * filename, gui_window  window ){
        dictionary * ini ;
        char * s;
        int i;
        bool return_code = false;
        struct gui_control * control;
        int num_control = 0;

        PRINTDF( "load_window_config <%s>\n", filename );
        ini = iniparser_load(filename);
        if (ini == NULL) {
                PRINTDF( "Unable to load config file %s\n", filename);
                return false ;
        }

        /* init the window itself */
        init_window(window,ini);

        /* Init each control in the window */
        while( true ){                
                i = iniparser_getint(ini, get_key(num_control, "type"), -1);
                if( i < 0 ) break;
                if ((i < GUI_TYPE_CTRL_TEXT) || (i >= GUI_TYPE_CTRL_MAX_NB )){
                        PRINTDF( "Control type unknown for control #%d\n", num_control );
                        goto end;
                }
                control = ( struct gui_control * ) calloc(1, sizeof( struct gui_control ) );	
                if (control == NULL){
                  PRINTDF( " Control Allocation failed %s \n","" );
                  goto end;
                }
                control->type = i;               
                control->win = window;                
                control->zone.x = iniparser_getint(ini, get_key(num_control,"x"), 0);
                control->zone.y = iniparser_getint(ini, get_key(num_control,"y"), 0);
                control->zone.w = iniparser_getint(ini, get_key(num_control,"w"), -1);
                control->zone.h = iniparser_getint(ini, get_key(num_control,"h"), -1);
                s = iniparser_getstring(ini, get_key(num_control,"name"), NULL);
                if( s != NULL ) control->name = strdup( s );

                switch(control->type){
                  case GUI_TYPE_CTRL_TEXT:
                    draw_text(control,ini, num_control);
                    break;
                  case GUI_TYPE_CTRL_BUTTON :
                    s = iniparser_getstring(ini, get_key(num_control,"image"), NULL);
                    if( s != NULL ) {
                      control->obj = load_image_to_surface(control, s);
                      window->background_surface->Blit( window->background_surface, control->obj, NULL, control->zone.x, control->zone.y );
                      /*window->background_surface->Flip(window->background_surface,NULL,0);                      */
                      /*window->background_surface->Blit( window->background_surface, control->obj, NULL, control->zone.x, control->zone.y );*/
                    }
                    break;
                  case GUI_TYPE_CTRL_CLICKABLE_ZONE :
                    /* Nothing else todo */
                    break;
	          case GUI_TYPE_CTRL_FILESELECTOR :
                    control->obj =  load_fs_ctrl(control,ini, num_control);
                    break;
                  default :
                    break;
                }
                add_to_list( &window->controls, control );
                num_control++;
        }

        return_code = true;
end:
        iniparser_freedict(ini);
        return return_code;
}


/** 
 * Loading of a window and initialization
 *
 * \param[in] filename window configuration file 
 *
 * \return Handle to the created object, NULL if error
 */
gui_window  gui_window_load(IDirectFB  *dfb, IDirectFBDisplayLayer *layer, const char * filename){
  gui_window  window;
  bool return_code;


  if (screen_height == 0){
    probe_screen( dfb);
  }

  if ( (win_stack.current_win + 1)>= GUI_WINDOW_MAX_NB) {
       PRINTDF( "No more window object can be created while trying to instanciate %s\n", filename );
  }

  PRINTDF( "load_window <%s>\n", filename );
  window = calloc(1,  sizeof( *window));
  if( window == NULL ){
      PRINTDF( "Unable to allocate memory for creating window %s\n", filename );
      return NULL;
  }
  window->dfb = dfb;
  window->layer = layer;

  return_code = load_window_config( filename, window );

  if( return_code != true ){
          PRINTDF( "Unable to load  window config\n%s", filename );
          return NULL;
  }
  if (! is_rotated)  {
    window->win->RaiseToTop(window->win);
  }
  window->background_surface->Flip(window->background_surface,NULL,0/*DSFLIP_WAITFORSYNC*/);
  window->background_surface->Blit(window->background_surface,window->background_surface,NULL,0,0);
  window->background_surface->Flip(window->background_surface,NULL,0/*DSFLIP_WAITFORSYNC*/);

  if (!window->is_detached){
    win_stack.current_win += 1;
    win_stack.winlist[win_stack.current_win] = window;
  }
  return window;
}



/** 
 * Release a window 
 *
 * \param window window handle  
 *
 * \warning Attempting to release a window which is not the last created (and thus the active one) will fail
 */
bool gui_window_release(gui_window window){

	struct list_object * list_controls;
	struct gui_control * control;
        
	
        PRINTD("unload_window\n");

        if( window != NULL ){
            /* Remove the window from the stack */    
            if (!window->is_detached){
              if (win_stack.winlist[win_stack.current_win] != window) {             
               return false;
              }
              win_stack.winlist[win_stack.current_win] = NULL;
              win_stack.current_win -= 1;  
            }
            /* Perform clean*/
            if (window->background_surface != NULL) {
              window->background_surface->Release( window->background_surface );
            }
            if (window->font != NULL) {
              window->font->Release( window->font );
            }
            if (window->win  != NULL){
                window->win->Destroy(window->win);
                window->win->Release( window->win);
            }
            list_controls = window->controls;
            while( list_controls != NULL ){
                    control = (struct gui_control *) list_controls->object;
                    free(control->name);
                    switch(control->type){
                      case GUI_TYPE_CTRL_TEXT :
                        ((IDirectFBFont * )control->obj)->Release((IDirectFBFont * )control->obj);
                        break;
                      case GUI_TYPE_CTRL_CLICKABLE_ZONE :  
                        /* No specific underlying object to free */
                        break;
                      case GUI_TYPE_CTRL_BUTTON :
                        if (control->obj != NULL)
                          ((IDirectFBSurface *)control->obj)->Release((IDirectFBSurface *)control->obj);
                        break;	
	              case GUI_TYPE_CTRL_FILESELECTOR :
                          if (control->obj != NULL){
                            fs_release(control->obj);
                          }
                        break;
                      default :
                        break;
                    }
                    list_controls = list_controls->next;
            }
            release_list( window->controls, NULL );
            window->controls = NULL;
	    free( window );  
        } else {  
          return false;
        }
  return true;
}

void gui_window_refresh(void){
  int i;
  for (i=win_stack.current_win; i>=0; i--){
    win_stack.winlist[i]->background_surface->Flip(win_stack.winlist[i]->background_surface,NULL, DSFLIP_WAITFORSYNC);
  }
}

void gui_window_release_all(void){
  int i;
  for (i=win_stack.current_win; i>=0; i--){
    gui_window_release(win_stack.winlist[i]);
  }
}

struct gui_control * gui_window_get_control(gui_window win, const char * name){
  struct list_object * list_controls;
  struct gui_control * control;
  struct gui_control * ret = NULL;

  list_controls = win->controls;
  while( list_controls != NULL ){
    control = (struct gui_control *) list_controls->object;
    
    if  ((control->name != NULL) && (strcmp(control->name, name) == 0)) {
      ret = control;
      break;
    }
    list_controls = list_controls->next;
  }
  return ret;
}

void gui_window_attach_cb(gui_window win,const char * name, gui_control_cb cb){
  struct list_object * list_controls;
  struct gui_control * control;  

  list_controls = win->controls;
  while( list_controls != NULL ){
    control = (struct gui_control *) list_controls->object;
    if ((control->name != NULL) && (strcmp(control->name, name)==0)) {
      control->cb = cb;
      break;
    }
    list_controls = list_controls->next;
  }
  return;
}

void gui_window_handle_click(int  x, int y){
  struct list_object * list_controls;
  struct gui_control * control;  
  int win_x, win_y;	
  gui_window win;

  if (win_stack.current_win < 0){
    return;
  }
  win = win_stack.winlist[win_stack.current_win];
  win->win->GetPosition(win->win, &win_x, &win_y);


  if (is_rotated){
#ifdef NATIVE
    int w,h;
    int tmp;
    win->win->GetSize(win->win, &w, &h);
    tmp= x;
    x = y;
    y = h - tmp;
    x-=win_y;
    y+=win_x;
#else
    int w,h;
    win->win->GetSize(win->win, &w, &h);
    x-=win_y;
    y= y - (screen_width -h -win_x);
#endif
  } else {  
    x-=win_x;
    y-=win_y;
  }

  list_controls = win->controls;
  while( list_controls != NULL ){
    control = (struct gui_control *) list_controls->object; 
    if ((control->zone.x <= x) &&
        (control->zone.y <= y) &&
        ((control->zone.x + control->zone.w) >= x) &&
        ((control->zone.y + control->zone.h) >= y) 
        ) {      
        if( control->cb != NULL){
          control->cb(control, x, y);
	  /* exit on first callback : needed as the cb may destroy the control list ! */
	  break;
        }
    }    
    list_controls = list_controls->next;
  }
  return;
}

/*
void gui_window_get_pos(gui_window win, int *x, int* y){
  *x=*y=0;
  win->win->GetPosition(win->win,x,y);
}
*/

/** Retrieve the window surface
*/
IDirectFBSurface * gui_window_get_surface(gui_window win){
  return win->background_surface;  
}
