/**
 * \file screens.c
 * \author nullpointer & wolfgar 
 * \brief This module implements GUI programmatic behavior.
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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#include "dictionary.h"
#include "iniparser.h"
#include "debug.h"
#include "window.h"
#include "engine.h"
#include "file_selector.h"
#include "zip_skin.h"
#include "config.h"
#include "version.h"
#include "resume.h"

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


/** About Thread parameters structure */
struct about_params{
    IDirectFBSurface * surf;
    IDirectFBFont * font;
    DFBPoint pos;
    pthread_t thread;
    bool about_screen_exit;
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


static void launch_engine(const char * path, int pos){
  FILE * fp;
  fp = fopen("./start_engine.sh", "w+");
  if (fp != NULL){
    fprintf(fp,"./start_engine \"%s\" %i\n", path, pos);
    fclose(fp);
  }
  return;
}

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
opacity=160\n\
x=%i\n\
y=%i\n\
w=%i\n\
h=%i\n\
[control_00]\n\
type=1\n\
msg=%s\n\
x=15\n\
y=3\n\
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
    
    i = snprintf(buffer, sizeof(buffer) -1, text_win_tpl, (screen_width-w-30)/2, (screen_height - height -10)/2,
                                                           w+30, height+10,msg,height,font_name,
                                                           color->r,color->g,color->b,color->a,
                                                           w+30, height+10 );
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


/** About screen thread that handles the scroll */
static void * about_thread(void *param){
  const char * about_text = "Tomplayer a multimedia player for TomTom GPS - Code by nullpointer (Patrick Bruyere) and Wolfgar (Stephan Rafin) - Graphics by Flavien and Daniel Clermont - Many thanks to all users and contributors - Meet us at http://www.tomplayer.net - This is free software under GPL";

  struct about_params * conf = param;
  int w, i ;
  DFBRegion region;
  
  region.x1 = conf->pos.x ;
  region.y1 = conf->pos.y ;
  region.x2 = screen_width ;
  region.y2 = screen_height;
  conf->surf->SetColor(conf->surf, 200,200,200,0xFF);
  conf->font->GetStringWidth ( conf->font, about_text, -1, &w);   
  i= screen_width;
  while(!conf->about_screen_exit) {
    conf->surf->SetColor(conf->surf, 0,0,0,0xFF);
    conf->surf->FillRectangle(conf->surf,conf->pos.x,conf->pos.y,  screen_width , screen_height-conf->pos.y);
    conf->surf->SetColor( conf->surf, 200,200,200,0xFF);
    conf->surf->DrawString( conf->surf,about_text, -1, i, conf->pos.y, DSTF_TOPLEFT);
    conf->surf->Flip(conf->surf,&region, 0/*DSFLIP_WAITFORSYNC*/);
    i -= 1;
    if (i < -w){
      i = screen_width;
    }
    usleep(10000);
  }
  return NULL;
}

/** Exit the about screen */
static void quit_about_window(struct gui_control * ctrl, int x, int y){
  ((struct about_params *)ctrl->cb_param)->about_screen_exit = true ;
  pthread_join(((struct about_params *)ctrl->cb_param)->thread,NULL);
  gui_window_release(ctrl->win);
  return;
}

/** Display the about screen */
static void enter_about(struct gui_control * ctrl, int x, int y){   
   /* FIXME recup propre de la version */
   char version_text[16]; 
   gui_window  win;  
   struct gui_control * txt_ctrl;
   int i,w;
   static struct about_params about_thread_params;

   win = gui_window_load(dfb, layer, get_full_conf(GUI_SCREEN_ABOUT)); 

   txt_ctrl = gui_window_get_control(win, "about_screen");
   if (txt_ctrl == NULL)
    return;
   txt_ctrl->cb_param = &about_thread_params;
   gui_window_attach_cb(win, "about_screen", quit_about_window);

   txt_ctrl = gui_window_get_control(win, "text");
   if (txt_ctrl == NULL)
    return;
   about_thread_params.font = txt_ctrl->obj;
   about_thread_params.surf = gui_window_get_surface(win);   
   about_thread_params.surf->SetFont(about_thread_params.surf, about_thread_params.font);
   i=snprintf(version_text,sizeof(version_text)-1 , "V %s", VERSION);
   if (i< 0) {
     i=0;
   }
   version_text[i]=0;
   about_thread_params.font->GetStringWidth (about_thread_params.font, version_text, -1, &w);
   about_thread_params.surf->SetColor(about_thread_params.surf, 100,100,100,0xFF);
   about_thread_params.surf->DrawString( about_thread_params.surf,version_text, -1, screen_width - w -10, txt_ctrl->zone.y - 30, DSTF_TOPLEFT);
   about_thread_params.surf->Flip(about_thread_params.surf,NULL, 0);
   about_thread_params.surf->DrawString( about_thread_params.surf,version_text, -1, screen_width - w -10, txt_ctrl->zone.y - 30, DSTF_TOPLEFT);
   about_thread_params.surf->Flip( about_thread_params.surf, NULL, DSFLIP_WAITFORSYNC);

   about_thread_params.pos.x = txt_ctrl->zone.x;
   about_thread_params.pos.y = txt_ctrl->zone.y;
   about_thread_params.about_screen_exit = false;

   pthread_create(&about_thread_params.thread, NULL, about_thread, &about_thread_params);
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
/*   unload_skin( conf );
    if( load_skin_from_zip( skin_filename, conf ) == false ){
      PRINTD("Error unable to load this skin\n");
      load_skin_from_zip( original_skin_filename, conf );
      ret = false;
    }
    else{*/
    /* FIXME add a check of skin sanity */
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
    /*}*/
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
    win = message_box("No File selected !", 24, &color, "./res/font/decker.ttf");
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

/** Display an opaque preview window at pos from an image */
static gui_window create_preview_window(const char * img_filename, const DFBRectangle * pos){
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
  gui_window win;
  DFBSurfaceDescription idsc;
  IDirectFBImageProvider *provider;
  win = NULL;
  DFBRectangle rect;
  double iratio, sratio;
  char buffer[512];
  int fd, i;

  if (dfb->CreateImageProvider (dfb,  img_filename, &provider) ==DFB_OK ) {
    provider->GetSurfaceDescription (provider, &idsc);
    iratio = idsc.width / idsc.height;
    sratio = pos->w/pos->h;
    if (iratio > sratio) {
      rect.w = pos->w;
      rect.h = rect.w/iratio;
      rect.x = 0;
      rect.y = (pos->h - rect.h) / 2;
    } else {
      rect.h = pos->h;
      rect.w = rect.h*iratio;
      rect.y = 0;
      rect.x =( pos->w - rect.w) /2;
    }
    provider->Release (provider);
  } else {
    return NULL;
  }

  fd = open("/tmp/tmp.cfg", O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
  if (fd >= 0){
    i = snprintf(buffer, sizeof(buffer) -1, img_win_tpl,rect.x + pos->x, rect.y + pos->y ,rect.w,rect.h,rect.w,rect.h,img_filename);
    if (i>= 0){
      buffer[i] = '\0';
      write(fd,buffer, i);
      win = gui_window_load(dfb, layer, "/tmp/tmp.cfg");
    }
    close(fd);
  }
  return win ;
}

/** Callback of video file selector 
  * Handle movie preview
  */
static void video_select_cb(fs_handle hdl, const char * c, enum  fs_events_type evt){
  static const char * cmd_mplayer_thumbnail = "DIR=`pwd` && cd /tmp && rm -f 00000001.png && $DIR/mplayer_png -ao null -vo png:z=0 -ss 10 -frames 1 \"%s\"";
  static gui_window win_prev;
  IDirectFBWindow * win;
  IDirectFBSurface * s;
  const struct fs_config *conf; 
  int winx,winy,i;
  DFBRectangle pos;
  char buffer[256];

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
      s->GetSize (s, &pos.w,&pos.h);
      win = fs_get_window(hdl);
      if (win != NULL){
        win->GetPosition(win, &winx, &winy);
        conf = fs_get_config(hdl);
        pos.x =  conf->geometry.pos.x + winx;
        pos.y =  conf->geometry.pos.y + winy;
        win_prev = create_preview_window("/tmp/00000001.png", &pos);
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
    /*DFBColor color = {255,255,255,255};
    message_box("Will launch Video...", 24, &color, "./res/font/decker.ttf");
    printf ("About to play %s \n", file);*/
    /*launch_mplayer("", file, 0 );*/
    launch_engine(file, 0 );
    quit = true;
  } else {
    DFBColor color = {255,255,50,50};    
    message_box("No file selected !", 24, &color, "./res/font/decker.ttf");
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

/** Callback that resumes video  */
static void resume_video( struct gui_control * ctrl, int x, int y){ 
    int pos = 0;
    char filename[PATH_MAX];

    if (resume_get_file_infos(filename, PATH_MAX, &pos) != 0){
        DFBColor color = {255,255,50,50};
        message_box("No resume data...", 24, &color, "./res/font/decker.ttf");    	
    }
    else{
    	/*display_current_file( filename, &config.video_config, config.bitmap_loading );*/
        /*launch_mplayer( "", filename, pos ); */
        launch_engine( filename, pos );
        quit = true;
    }
    return;
}

/* AUDIO SCREEN */

static bool shuffle_state = true;
/**Callback on audio selection*/
static void play_audio(struct gui_control * ctrl, int x, int y){
  FILE *fp;
  fslist list;
  const char * filename;
  const struct gui_control * ctrl_fs =  gui_window_get_control(ctrl->win, "file_selector");
  int file_nb = 0;
  

  fs_handle fs = ctrl_fs->obj;    
  list =  fs_get_selection(fs);
  fp = fopen( "/tmp/playlist.m3u", "w+" );
  while ( (filename = fslist_get_next_file(list, shuffle_state)) != NULL)  {
    file_nb++;
    fprintf( fp, filename );
    fprintf( fp, "\n" );
  }
  fslist_release(list);
  fclose(fp);
  if (file_nb == 0){
    DFBColor color = {255,255,50,50};
    message_box("No file selected !", 24, &color, "./res/font/decker.ttf");
  } else {
  /*DFBColor color = {255,255,255,255};
  message_box("Will play audio...", 24, &color, "./res/font/decker.ttf");*/
    /*launch_mplayer( "","/tmp/playlist.m3u", 0 );*/
    launch_engine("/tmp/playlist.m3u", 0);
    quit = true;
  }
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
  back_surf->Flip(back_surf,&region,DSFLIP_WAITFORSYNC);
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


static bool load_main_screen(void){
   /* We go there from splash screen */
   gui_window  win;
   win = gui_window_load(dfb, layer, get_full_conf(GUI_SCREEN_MAIN)); 
   if (win == NULL){
    return false;
   }
   gui_window_attach_cb(win, "exit_button", quit_tomplayer);
   gui_window_attach_cb(win, "video_button", select_video);
   gui_window_attach_cb(win, "about_button", enter_about);
   gui_window_attach_cb(win, "audio_button", select_audio);
   gui_window_attach_cb(win, "resume_button", resume_video);
   gui_window_attach_cb(win, "settings_button", choose_settings);
  return true;
}

/** Callback that displays main screen */
static void enter_main_screen(struct gui_control * ctrl, int x, int y){
   /* Dont want to keep the splash screen window => release it */
   gui_window_release(ctrl->win);
   load_main_screen();
   return;
}



/* SPLASH SCREEN */
/** Display splash screen */
static bool load_splash_screen(void){
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
bool screen_init(IDirectFB  *v_dfb, IDirectFBDisplayLayer  * v_layer, bool splash_wanted){
  int temp;	
  DFBSurfaceDescription dsc;	
  IDirectFBSurface * primary;
  struct stat stats;

  
  dfb = v_dfb;
  layer = v_layer;

  dsc.flags = DSDESC_CAPS;
  dsc.caps = DSCAPS_PRIMARY | DSCAPS_FLIPPING;
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
  if (splash_wanted){
    return load_splash_screen();
  } else {
    return load_main_screen();    
  }
}

/** Returns whether user has clicked on exit or not */
bool screen_is_end_asked(void){
  return quit;
}
