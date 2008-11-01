/**
 * \file screens.c
 * \author nullpointer & wolfgar 
 * \brief This module implements GUI programmatic behavior.
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


#include <directfb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "dictionary.h"
#include "iniparser.h"
#include "debug.h"
#include "window.h"
#include "engine.h"
#include "file_selector.h"
#include "zip_skin.h"
#include "config.h"

enum gui_screens_type {
  GUI_SCREEN_MAIN,
  GUI_SCREEN_SELECT_SETTINGS,
  GUI_SCREEN_AUDIO_SKIN,
  GUI_SCREEN_VIDEO_SKIN,
  GUI_SCREEN_AUDIO,
  GUI_SCREEN_VIDEO,
  GUI_SCREEN_ABOUT,
  GUI_SCREEN_SPLASH,
  GUI_SCREEN_NOT_IMPLEMENTED,
  GUI_SCREEN_MAX
};

#define CFG_FOLDER "./conf"

static const char * graphic_conf_files[GUI_SCREEN_MAX] = {"main.cfg",
                                                          "settings.cfg",
                                                          "skin_audio.cfg",
                                                          "skin_video.cfg",
                                                          "audio.cfg",
                                                          "video.cfg",
                                                          "about.cfg",
                                                          "splash.cfg",
						          "not_implemented.cfg"
                                                         };

static bool quit = false;
static char  graphic_conf_folder[32];
static IDirectFB	      *dfb;
static IDirectFBDisplayLayer  *layer;
static int screen_width, screen_height;


inline static const char * get_full_conf(enum gui_screens_type screen_type){
  static char buff[64];
  snprintf(buff, sizeof(buff) - 1, "%s%s",graphic_conf_folder , graphic_conf_files[screen_type]);
  return buff;    
}


/** Generic callback that destroy the windows wwhich holds the control that has been clicked on */
static void quit_current_window(struct gui_control * ctrl, int x, int y){
   gui_window_release(ctrl->win);
   return;
}

/** Diplay the "not yet implemented error message */
static void display_not_implemented(struct gui_control * ctrl, int x, int y){
   gui_window  win;
   win = gui_window_load(dfb, layer, get_full_conf(GUI_SCREEN_NOT_IMPLEMENTED)); 
   gui_window_attach_cb(win, "any_part", quit_current_window);   
}


/** Dispatch mouse event to a file selector controller (used by all the screens that holds a file selector) */
static void dispatch_fs_event(struct gui_control * ctrl, int x, int y){
  fs_handle fs = ctrl->obj;
  fs_handle_click(fs, x , y );
}


/** Display a message box */
static gui_window message_box(const char *msg, int height, const DFBColor * color, const char * font_name){

const char * text_win_tpl="\
[general]\n\
opacity=200\n\
x=%i\n\
y=%i\n\
w=%i\n\
h=%i\n\
[control_00]\n\
type=1\n\
msg=%s\n\
x=5\n\
y=5\n\
font_height=%i\n\
font=%s\n\
r=%i\n\
g=%i\n\
b=%i\n\
a=%i\n\
[control_01]\n\
type=3\n\
x=0\n\
y=0\n\
w=%i\n\
h=%i\n\
name=any_part\n\
";

  int fd, i, w;
  char buffer[512];
  DFBFontDescription desc;
  IDirectFBFont * font;	
  gui_window win;

  desc.flags = DFDESC_HEIGHT;
  desc.height = height;
  if (dfb->CreateFont(dfb, font_name, &desc, &font ) != DFB_OK){
    return NULL;
  } 
  font->GetStringWidth (font, msg, -1, &w);
  font->Release(font);
  fd = open("/tmp/tmp.cfg", O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
  
  if (fd >= 0) {
    
    i = snprintf(buffer, sizeof(buffer) -1, text_win_tpl, (screen_width-w-10)/2, (screen_height - height -10)/2,
                                                           w+10, height+10,msg,height,font_name,
                                                           color->r,color->g,color->b,color->a,
                                                           w+10, height+10 );
    if (i>= 0){
      buffer[i] = '\0';
      write(fd,buffer, i);
      win = gui_window_load(dfb, layer, "/tmp/tmp.cfg");
      gui_window_attach_cb(win, "any_part", quit_current_window);   
    }
    close(fd);
  }
  return win;
}



/* ABOUT SCREEN */

/** Display the about screen */
static void enter_about(struct gui_control * ctrl, int x, int y){
   gui_window  win;
   win = gui_window_load(dfb, layer, get_full_conf(GUI_SCREEN_ABOUT)); 
   gui_window_attach_cb(win, "about_screen", quit_current_window);      
   return;
}


/* SKINS SELECTION SCREENS (AUDIO and VIDEO) */

/** Update skin in tomplayer configuration file
 *
 * \return true on succes, false on failure
 *  \todo Should be moved to config module  TODO 
 */
static bool update_skin( enum gui_screens_type type , const char * skin_filename){
  /* Correct iniparser bug in naming function iniparser_setstring/iniparser_set */
  extern int iniparser_set(dictionary * ini, char * entry, char * val);
  #define iniparser_setstring iniparser_set

  struct skin_config * conf = NULL;
  char * original_skin_filename = NULL;
  char * config_skin_filename_key = NULL;
  char cmd[200];
  dictionary * ini ;
  bool ret = true;
  FILE * fp;
  
  PRINTDF( "select_skin %s\n", skin_filename);

  switch (type){
    case GUI_SCREEN_VIDEO_SKIN :
      conf = &config.video_config;
      original_skin_filename = config.video_skin_filename;
      config_skin_filename_key = SECTION_VIDEO_SKIN":"KEY_SKIN_FILENAME;
      break;
    case GUI_SCREEN_AUDIO_SKIN :
      conf = &config.audio_config;
      original_skin_filename = config.audio_skin_filename;
      config_skin_filename_key = SECTION_AUDIO_SKIN":"KEY_SKIN_FILENAME;
      break;
     default :
      return false;
  }

  if( conf != NULL ){
    unload_skin( conf );
    if( load_skin_from_zip( skin_filename, conf ) == false ){
      PRINTD("Error unable to load this skin\n");
      load_skin_from_zip( original_skin_filename, conf );
      ret = false;
    }
    else{
      ini = iniparser_load(CONFIG_FILE);
      if( ini == NULL ){
              PRINTD( "Unable to save main configuration\n" );
              ret = false;
              goto error;
      }
      fp = fopen( CONFIG_FILE, "w+" );
      if( fp == NULL ){
              PRINTD( "Unable to open main config file\n" );
              ret = false;
              goto error;
      }
      iniparser_setstring( ini, config_skin_filename_key, NULL );
      iniparser_setstring( ini, config_skin_filename_key, skin_filename );
      iniparser_dump_ini( ini, fp );
      fclose( fp );

      sprintf( cmd, "cp -f %s ./conf/tomplayer.ini", CONFIG_FILE );
      system( cmd );
      system( "unix2dos ./conf/tomplayer.ini" );
error:
      iniparser_freedict(ini);
    }
  }

  return ret;
}

/** Callback on skin selection */
static void skin_selected(struct gui_control * ctrl, int x, int y){
  const char * file;
  DFBColor color = {255,255,255,255};
  gui_window win;
  const struct gui_control * ctrl_fs =  gui_window_get_control(ctrl->win, "file_selector");
  fs_handle fs = ctrl_fs->obj;  
  file = fs_get_single_selection(fs);
  if (file != NULL){
    if (update_skin(ctrl->cb_param, file)){
      win = message_box("Configuration updated...", 24, &color, "./res/font/decker.ttf");
    } else {
      win = message_box("Error : Cannot update !", 24, &color, "./res/font/decker.ttf");
    }
    sleep(2);
    gui_window_release(win);
    gui_window_release(ctrl->win);
  } else {
    win = message_box("No Selection !", 24, &color, "./res/font/decker.ttf");
  }
}


/** Callback that displays the skin selection screen */
static void select_skin(struct gui_control * ctrl, int x, int y){
   gui_window  win;
   fs_handle fs;
   struct gui_control * fs_ctrl;
   struct gui_control * select_button;
   enum gui_screens_type type = ctrl->cb_param;

   switch (type){
    case GUI_SCREEN_VIDEO_SKIN :
    case GUI_SCREEN_AUDIO_SKIN :
      break;
     default :
      return;
   }

   gui_window_release(ctrl->win);
   win = gui_window_load(dfb, layer, get_full_conf((type==GUI_SCREEN_AUDIO_SKIN)? GUI_SCREEN_AUDIO_SKIN : GUI_SCREEN_VIDEO_SKIN)); 
   gui_window_attach_cb(win, "file_selector", dispatch_fs_event);
   gui_window_attach_cb(win, "goback_button", quit_current_window);   
   fs_ctrl =  gui_window_get_control(win, "file_selector");
   if (fs_ctrl != NULL){
     fs = fs_ctrl->obj;
     /*fs_set_select_cb(fs, audio_select_cb);*/
     fs_new_path(fs, (type==GUI_SCREEN_AUDIO_SKIN) ? "./skins/audio/": "./skins/video/" , "^.*\\.zip$");
   }

   select_button = gui_window_get_control(win, "select_button");
   if (select_button  != NULL){
     select_button->cb_param=type;
     gui_window_attach_cb(win, "select_button", skin_selected);
   }
}

/* CHOOSE SETTINGS SCREEN */

/** Callback on choose settings screen */
static void choose_settings(struct gui_control * ctrl, int x, int y){
   gui_window  win;
   win = gui_window_load(dfb, layer, get_full_conf(GUI_SCREEN_SELECT_SETTINGS)); 
   struct gui_control * choose_button;

   choose_button = gui_window_get_control(win, "skin_video");
   if (choose_button  != NULL){
     choose_button->cb_param=GUI_SCREEN_VIDEO_SKIN;
     gui_window_attach_cb(win, "skin_video", select_skin);
   }
   choose_button = gui_window_get_control(win, "skin_audio");
   if (choose_button  != NULL){
     choose_button->cb_param=GUI_SCREEN_AUDIO_SKIN;
     gui_window_attach_cb(win, "skin_audio", select_skin);
   }
   return;
}


/* VIDEO SELECTION SCREEN */

/** Callback of video file selector 
  * Handle movie preview
  */
static void video_select_cb(fs_handle hdl, const char * c, enum  fs_events_type evt){

const char * img_win_tpl="\
[general]\n\
opacity=255\n\
x=%i\n\
y=%i\n\
w=%i\n\
h=%i\n\
detached=1\n\
[control_00]\n\
type=2\n\
x=0\n\
y=0\n\
w=%i\n\
h=%i\n\
image=%s\n\
";
  static const char * cmd_mplayer_thumbnail = "cd /tmp && rm -f 00000001.png && mplayer -ao null -vo png:z=0 -ss 10 -frames 1 \"%s\"";
  static gui_window win_prev;
  IDirectFBImageProvider *provider;
  IDirectFBWindow * win;
  IDirectFBSurface * s;
  DFBSurfaceDescription idsc;
  const struct fs_config *conf; 
  DFBRectangle rect;
  double iratio, sratio;
  int w,h,fd,winx,winy,i;
  char buffer[512];

  if (win_prev !=NULL){
    gui_window_release(win_prev);
    win_prev = NULL;
  }

  s = fs_get_preview_surface(hdl);  
  if (s != NULL){
    if (evt == FS_EVT_SELECT){
      i = snprintf( buffer, sizeof(buffer)-1,cmd_mplayer_thumbnail, c );
      if (i < 0){
        return;
      }
      buffer[i] = '\0';
      system(buffer) ;
      /*unzip_file( c, "skin.bmp", "00000001.bmp" );*/
      if (dfb->CreateImageProvider (dfb,  "/tmp/00000001.png", &provider) ==DFB_OK ) {
        provider->GetSurfaceDescription (provider, &idsc);
        iratio = idsc.width / idsc.height;
        s->GetSize (s, &w,&h);
        sratio = w/h;
        if (iratio > sratio) {
          rect.w = w;
          rect.h = rect.w/iratio;
          rect.x = 0;
          rect.y = (h - rect.h) / 2;
        } else {
          rect.h = h;
          rect.w = rect.h*iratio;
          rect.y = 0;
          rect.x =( w - rect.w) /2;
        }
        provider->Release (provider);
      }
      fd = open("/tmp/tmp.cfg", O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
      win = fs_get_window(hdl);
      if ((fd >= 0) && (win != NULL)){
        win->GetPosition(win, &winx, &winy);
        conf = fs_get_config(hdl);
        i = snprintf(buffer, sizeof(buffer) -1, img_win_tpl,rect.x + conf->geometry.pos.x + winx ,rect.y + conf->geometry.pos.y + winy ,rect.w,rect.h,rect.w,rect.h,"/tmp/00000001.png");
        if (i>= 0){
          buffer[i] = '\0';
          write(fd,buffer, i);
          win_prev = gui_window_load(dfb, layer, "/tmp/tmp.cfg");
        }
        close(fd);
      }
    } 
  }
}


/** Callback on video selection */
static void play_video(struct gui_control * ctrl, int x, int y){
  const char * file;
  const struct gui_control * ctrl_fs =  gui_window_get_control(ctrl->win, "file_selector");
  fs_handle fs = ctrl_fs->obj;  
  file = fs_get_single_selection(fs);
  
  if (file != NULL){
    DFBColor color = {255,255,255,255};
    message_box("Will launch Video...", 24, &color, "./res/font/decker.ttf");
    printf ("About to play %s \n", file);
    /* FIXME MPLAYER launch_mplayer("", file, 0 );*/
  } else {
    printf("No file selected !\n");
  }
}


/** Callback that displays video selection screeen */
static void select_video(struct gui_control * ctrl, int x, int y){
   gui_window  win;
   fs_handle fs;
   const struct gui_control * fs_ctrl;

   /* TODO  Write a decent config module !*/
   extern struct tomplayer_config config;

   win = gui_window_load(dfb, layer, get_full_conf(GUI_SCREEN_VIDEO)); 
   gui_window_attach_cb(win, "file_selector", dispatch_fs_event);
   gui_window_attach_cb(win, "goback_button", quit_current_window);
   gui_window_attach_cb(win, "play_button",  play_video);
   fs_ctrl =  gui_window_get_control(win, "file_selector");
   if (fs_ctrl != NULL){
     fs = fs_ctrl->obj;
     fs_set_select_cb(fs, video_select_cb);
     fs_new_path(fs, config.video_folder, config.filter_video_ext);
   }
}

/* AUDIO SCREEN */

static bool shuffle_state = true;
/**Callback on audio selection*/
static void play_audio(struct gui_control * ctrl, int x, int y){
  FILE *fp;
  fslist list;
  const char * filename;
  const struct gui_control * ctrl_fs =  gui_window_get_control(ctrl->win, "file_selector");
  

  fs_handle fs = ctrl_fs->obj;  
  DFBColor color = {255,255,255,255};
  message_box("Will play audio...", 24, &color, "./res/font/decker.ttf");
  list =  fs_get_selection(fs);
  fp = fopen( "/tmp/playlist.m3u", "w+" );
  while ( (filename = fslist_get_next_file(list, shuffle_state)) != NULL)  {
    fprintf( fp, filename );
    fprintf( fp, "\n" );
  }
  fslist_release(list);
  fclose(fp);
 /* FIXME MPLAYER launch_mplayer( "","/tmp/playlist.m3u", 0 );*/
}

/** Callback on select all button */
static void select_all_files(struct gui_control * ctrl, int x, int y){
  const struct gui_control * ctrl_fs =  gui_window_get_control(ctrl->win, "file_selector");
  fs_handle fs = ctrl_fs->obj;  
  fs_select_all(fs);
}

/** Callback on unselect all button */
static void unselect_all_files(struct gui_control * ctrl, int x, int y){
  const struct gui_control * ctrl_fs =  gui_window_get_control(ctrl->win, "file_selector");
  fs_handle fs = ctrl_fs->obj;  
  fs_unselect_all(fs);
}

/** Callback on shuffle controller */
static void toggle_suffle(struct gui_control * ctrl, int x, int y){
  IDirectFBSurface * img_surf;
  const struct gui_control * img_ctrl;
  DFBRegion region;
  IDirectFBSurface * back_surf = gui_window_get_surface(ctrl->win);

  shuffle_state = !shuffle_state;
  if (shuffle_state){
     img_ctrl =  gui_window_get_control(ctrl->win, "shuflle_on_button");
  } else {
     img_ctrl =  gui_window_get_control(ctrl->win, "shuflle_off_button");
  }
  img_surf = img_ctrl->obj;
  back_surf->Blit(back_surf,img_surf,NULL,ctrl->zone.x,ctrl->zone.y);
  region.x1 = img_ctrl->zone.x;
  region.y1 = img_ctrl->zone.y;
  region.x2 = img_ctrl->zone.x + img_ctrl->zone.w;
  region.y2 = img_ctrl->zone.y + img_ctrl->zone.h;
  back_surf->Flip(back_surf,&region,0);
  return;
}

/** Callback that displays audio selection screen */
static void select_audio(struct gui_control * ctrl, int x, int y){
   gui_window  win;
   fs_handle fs;
   const struct gui_control * fs_ctrl;

   /* TODO  Write a decent config module !*/
   extern struct tomplayer_config config;

   win = gui_window_load(dfb, layer, get_full_conf(GUI_SCREEN_AUDIO)); 
   gui_window_attach_cb(win, "file_selector", dispatch_fs_event);
   gui_window_attach_cb(win, "goback_button", quit_current_window);
   gui_window_attach_cb(win, "selall_button", select_all_files);
   gui_window_attach_cb(win, "nosel_button", unselect_all_files);
   gui_window_attach_cb(win, "play_button", play_audio);
   gui_window_attach_cb(win, "shuflle_on_button", toggle_suffle);
   fs_ctrl =  gui_window_get_control(win, "file_selector");
   if (fs_ctrl != NULL){
     fs = fs_ctrl->obj;
     /*fs_set_select_cb(fs, audio_select_cb);*/
     fs_new_path(fs, config.audio_folder, config.filter_audio_ext);
   }
}


/* MAIN SCREEN */
/** Callback on exit button */
static void quit_tomplayer(struct gui_control * ctrl, int x, int y){
   gui_window_release(ctrl->win);
   quit = true;
}

/** Callback that displays main screen */
static void enter_main_screen(struct gui_control * ctrl, int x, int y){
   /* We go there from splash screen */
   gui_window  win;
   /* Dont want to keep the splash screen window => release it */
   gui_window_release(ctrl->win);
   win = gui_window_load(dfb, layer, get_full_conf(GUI_SCREEN_MAIN)); 
   gui_window_attach_cb(win, "exit_button", quit_tomplayer);
   gui_window_attach_cb(win, "video_button", select_video);
   gui_window_attach_cb(win, "about_button", enter_about);
   gui_window_attach_cb(win, "audio_button", select_audio);
   gui_window_attach_cb(win, "resume_button", display_not_implemented);
   gui_window_attach_cb(win, "settings_button", choose_settings);
   return;
}


/* SPLASH SCREEN */
/** Display splash screen */
static bool load_first_screen(void){
  gui_window  win;
  win = gui_window_load(dfb, layer, get_full_conf(GUI_SCREEN_SPLASH));
  if (win == NULL){
    return false;
  }
  gui_window_attach_cb(win, "splash_screen", enter_main_screen);
  return true;
  
}

/* -- External functions -- */

/** Initialize screens module and displays first screen */
bool screen_init(IDirectFB  *v_dfb, IDirectFBDisplayLayer  * v_layer){
  int temp;	
  DFBSurfaceDescription dsc;	
  IDirectFBSurface * primary;
  struct stat stats;

  
  dfb = v_dfb;
  layer = v_layer;

  dsc.flags = DSDESC_CAPS;
  dsc.caps = DSCAPS_PRIMARY;
  if (dfb->CreateSurface( dfb, &dsc, &primary ) != DFB_OK ){
    return false;
  }

  if (primary->GetSize( primary, &screen_width, &screen_height ) !=DFB_OK) {
      primary->Release(primary);
      return false;
  }

  if( screen_width < screen_height ){ 
          temp = screen_height;
          screen_height = screen_width;
          screen_width = temp;
  }

  primary->Release(primary);
  snprintf(graphic_conf_folder, sizeof(graphic_conf_folder) - 1, "%s/%i_%i/", CFG_FOLDER, screen_width, screen_height);
  if (stat(graphic_conf_folder,&stats) != 0){
    PRINTDF("No available folder for your screen : %i x %i \n",screen_width,screen_height );
    return false;
  }

  return load_first_screen();
}

/** Returns whether user has clicked on exit or not */
bool screen_is_end_asked(void){
  return quit;
}
