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
#include "viewmeter.h"
#include "label.h"

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
  GUI_SCREEN_RESUME,
  GUI_SCREEN_CONFIGURATION,
  GUI_SCREEN_CONF_SCREEN_SAVER,
  GUI_SCREEN_CONF_DEFAULT_PATHS,
  GUI_SCREEN_CONF_FM_TRANSMITTER,
  GUI_SCREEN_CONF_DIAPO,
  GUI_SCREEN_SELECT_FOLDER,
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
                                                          "not_implemented.cfg",
                                                          "resume.cfg",
                                                          "config.cfg",
                                                          "screen.cfg",
                                                          "paths.cfg",
                                                          "fm.cfg",
                                                          "diapo.cfg",
                                                          "folder.cfg"
                                                         };

static bool quit = false;
static char  graphic_conf_folder[32];
static IDirectFB	      *dfb;
static IDirectFBDisplayLayer  *layer;
static int screen_width, screen_height;


static void play_audio_video(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt);
static void select_all_files(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt);
static void unselect_all_files(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt);
static void toggle_suffle(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt);  


static void launch_engine(const char * path, int pos, bool is_video){
  int fd, i ;
  char buffer[128];
  
  
  
  fd = open ("/tmp/start_engine.sh", O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
  if (fd >= 0){
    i = snprintf(buffer, sizeof(buffer) - 1,"./start_engine \"%s\" %i %s\n", path, pos, is_video?"VIDEO":"AUDIO"  );
    buffer[i] = '\0';
    write(fd,buffer,i);
    fsync(fd);
    close(fd);
  }
  return;
}

inline static const char * get_full_conf(enum gui_screens_type screen_type){
  static char buff[64];
  snprintf(buff, sizeof(buff) - 1, "%s%s",graphic_conf_folder , graphic_conf_files[screen_type]);
  return buff;    
}

/** utility function to blit a image */
static void blit_img(struct gui_control * ctrl, const char *ctrl_name){
  IDirectFBSurface * img_surf;
  const struct gui_control * img_ctrl;
  DFBRegion region;
  IDirectFBSurface * back_surf = gui_window_get_surface(ctrl->win);  
  img_ctrl =  gui_window_get_control(ctrl->win, ctrl_name);
  if (img_ctrl != NULL){
    img_surf = img_ctrl->obj;
    back_surf->Blit(back_surf,img_surf,NULL,ctrl->zone.x,ctrl->zone.y);
    region.x1 = img_ctrl->zone.x;
    region.y1 = img_ctrl->zone.y;
    region.x2 = img_ctrl->zone.x + img_ctrl->zone.w;
    region.y2 = img_ctrl->zone.y + img_ctrl->zone.h;
    back_surf->Flip(back_surf,&region,DSFLIP_WAITFORSYNC);
  }
  return;
}

/** Handle selection of an event */
static void handle_selection(struct gui_control *ctrl,  enum gui_event_type type){
    IDirectFBSurface * surf = gui_window_get_surface(ctrl->win);  
    
    if (type == GUI_EVT_SELECT){
        /* FIXME Add new image that represents the selected control */
        surf->SetColor(surf, 255,255,255,255);
        surf->DrawRectangle(surf, 
                            ctrl->zone.x , 
                            ctrl->zone.y , 
                            ctrl->zone.w , 
                            ctrl->zone.h );    
        surf->Flip(surf,NULL, DSFLIP_WAITFORSYNC);
    } else {
        /*
        if (ctrl->type == GUI_TYPE_CTRL_BUTTON){
            PRINTD("Cleaning button selection");
            blit_img(ctrl, ctrl->name);         
        } else {
            PRINTD("Cleaning NON button selection");
            */
            surf->SetColor(surf, 0,0,0,0);
            surf->DrawRectangle(surf, 
                            ctrl->zone.x , 
                            ctrl->zone.y , 
                            ctrl->zone.w , 
                            ctrl->zone.h );    
            surf->Flip(surf,NULL, DSFLIP_WAITFORSYNC);
        /*}*/
    }
}

/** Generic callback that destroy the windows which holds the control that has been clicked on */
static void quit_current_window(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    if (evt){
        gui_window_release(ctrl->win);
    } else {
        handle_selection(ctrl, type);
    }
    return;
}



/** Dispatch mouse event to a file selector controller (used by all the screens that holds a file selector) */
static void dispatch_fs_event(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
  fs_handle fs = ctrl->obj;

  if (evt)  {
    if (type == GUI_EVT_TS)
        fs_handle_click(fs, evt->ts.x , evt->ts.y );
    else {
        fs_handle_key(fs, evt->key);
    }
  } else {
     handle_selection(ctrl, type); 
  }   
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
    conf->surf->Flip(conf->surf,&region, DSFLIP_WAITFORSYNC);
    i -= 1;
    if (i < -w){
      i = screen_width;
    }
    usleep(10000);
  }
  return NULL;
}

/** Exit the about screen */
static void quit_about_window(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    if (evt){
        ((struct about_params *)ctrl->cb_param)->about_screen_exit = true ;
        pthread_join(((struct about_params *)ctrl->cb_param)->thread,NULL);
        gui_window_release(ctrl->win);     
    } 
    /* No selection for this screen */
    return;
}

/** Display the about screen */
static void enter_about(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
   char version_text[16]; 
   gui_window  win;  
   struct gui_control * txt_ctrl;
   int i,w;
   static struct about_params about_thread_params;
   
   if (evt) {
       win = gui_window_load(dfb, layer, get_full_conf(GUI_SCREEN_ABOUT)); 

       txt_ctrl = gui_window_get_control(win, "about_screen");
       if (txt_ctrl == NULL)
           return;
       txt_ctrl->cb_param = (int)&about_thread_params;
       gui_window_attach_cb(win, "about_screen", quit_about_window);

       txt_ctrl = gui_window_get_control(win, "text");
       if (txt_ctrl == NULL)
           return;
       about_thread_params.font = label_get_font(txt_ctrl->obj);
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
   } else {
       handle_selection(ctrl, type);
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
      rect.w = pos->w - 2 ;
      rect.h = rect.w/iratio;
      rect.x = 0 + 2  ;
      rect.y = (pos->h - rect.h) / 2;
    } else {
      rect.h = pos->h;
      rect.w = rect.h*iratio;
      rect.y = 0;
      rect.x = ( pos->w - rect.w) /2;
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


static void create_preview(fs_handle hdl,  enum  fs_events_type evt, const char * img_filename){
  static gui_window win_prev;
  IDirectFBWindow * win;
  IDirectFBSurface * s;
  const struct fs_config *conf; 
  int winx,winy;
  DFBRectangle pos;  

  if (win_prev !=NULL){
    gui_window_release(win_prev);
    win_prev = NULL;
  }

  if (evt == FS_EVT_SELECT){
    s = fs_get_preview_surface(hdl);  
    if (s != NULL){    
        s->GetSize (s, &pos.w,&pos.h);
        win = fs_get_window(hdl);
        if (win != NULL){
          win->GetPosition(win, &winx, &winy);
          conf = fs_get_config(hdl);
          pos.x =  conf->geometry.pos.x + winx;
          pos.y =  conf->geometry.pos.y + winy;
          win_prev = create_preview_window(img_filename, &pos);
        } 
    }
  }
}


/** Callback of video file selector 
  * Handle movie preview
  */
static void video_select_cb(fs_handle hdl, const char * c, enum  fs_events_type evt){
  static const char * cmd_mplayer_thumbnail = "DIR=`pwd` && cd /tmp && rm -f 00000001.png && $DIR/mplayer -ao null -vo png:z=0 -ss 5 -frames 1 \"%s\" 2> /dev/null ";
  int i ;
  char buffer[256];

  if (evt == FS_EVT_SELECT){
      i = snprintf( buffer, sizeof(buffer)-1,cmd_mplayer_thumbnail, c );
      if (i < 0){
        return;
      }
      buffer[i] = '\0';
      system(buffer) ;    
  }
  create_preview(hdl, evt, "/tmp/00000001.png");

}

/** Callback that displays video selection screen */
static void select_video(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    gui_window  win;
    fs_handle fs;
    const struct gui_control * fs_ctrl;
    struct gui_control *play_ctrl;

    /* TODO  Write a decent config module instead of grabbing a glocal conf that way !*/
    extern struct tomplayer_config config;

    if (evt){
        win = gui_window_load(dfb, layer, get_full_conf(GUI_SCREEN_VIDEO)); 
        gui_window_attach_cb(win, "file_selector", dispatch_fs_event);
        gui_window_attach_cb(win, "goback_button", quit_current_window);
        gui_window_attach_cb(win, "play_button",  play_audio_video);
        gui_window_attach_cb(win, "selall_button", select_all_files);
        gui_window_attach_cb(win, "nosel_button", unselect_all_files);   
        gui_window_attach_cb(win, "shuflle_on_button", toggle_suffle);     
        fs_ctrl =  gui_window_get_control(win, "file_selector");
        if (fs_ctrl != NULL){
        const struct fs_config * conf;

            fs = fs_ctrl->obj;
        fs_set_select_cb(fs, video_select_cb);
            fs_new_path(fs, config.video_folder, config.filter_video_ext);
        conf = fs_get_config(fs);
        if (!conf->options.multiple_selection){  
            /* Automatically select the first item if single  selection */
            fs_select(fs, 0);
        }	 
        }
        play_ctrl = gui_window_get_control(win, "play_button");
        if (play_ctrl != NULL){ 
            play_ctrl->cb_param = true;
        }
    } else {
        handle_selection(ctrl, type);
    }
}


/* SKINS SELECTION SCREENS (AUDIO and VIDEO) */

/** Update skin in tomplayer configuration file
 *
 * \return true on succes, false on failure 
 */
static bool update_skin( enum gui_screens_type type , const char * skin_filename){
  enum config_type conf_type; 
  bool ret = true; 

  PRINTDF( "select_skin %s\n", skin_filename);
  switch (type){
    case GUI_SCREEN_VIDEO_SKIN :
      conf_type = CONFIG_VIDEO;
      break;
    case GUI_SCREEN_AUDIO_SKIN :
      conf_type = CONFIG_AUDIO;
      break;
     default :
      return false;
  }

  ret = config_set_skin(conf_type, skin_filename);
  if (ret){
    ret = config_save();
  }
  return ret;
}


/** Callback of skin preview */
static void skin_select_cb(fs_handle hdl, const char * c, enum  fs_events_type evt){
  struct skin_config  skin_conf;

  if (evt == FS_EVT_SELECT){
      load_skin_from_zip(c , &skin_conf , false );
      unload_skin(  &skin_conf);
  }
  create_preview(hdl, evt,  ZIP_SKIN_BITMAP_FILENAME);
}



/** Callback on skin selection */
static void skin_selected(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    const char * file;
    DFBColor color = {255,255,255,255};
    gui_window win;
    const struct gui_control * ctrl_fs =  gui_window_get_control(ctrl->win, "file_selector");
    
    if (evt){
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
    } else {
        handle_selection(ctrl, type);
    }
}


/** Callback that displays the skin selection screen */
static void select_skin(struct gui_control *ctrl, enum gui_event_type evt_type, union gui_event* evt){
    gui_window  win;
    fs_handle fs;
    struct gui_control * fs_ctrl;
    struct gui_control * select_button;
    enum gui_screens_type type = ctrl->cb_param;

    if (evt) {
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
            char * basename;
            fs = fs_ctrl->obj;
            fs_set_select_cb(fs, skin_select_cb);     
            fs_new_path(fs, (type==GUI_SCREEN_AUDIO_SKIN) ? "./skins/audio": "./skins/video" , "^.*\\.zip$");
            basename = strrchr((type==GUI_SCREEN_AUDIO_SKIN) ? config.audio_skin_filename :config.video_skin_filename, '/');
            if (basename != NULL)
            fs_select_filename(fs, basename + 1);
        }

        select_button = gui_window_get_control(win, "select_button");    
        if (select_button  != NULL){
            select_button->cb_param=type;
            gui_window_attach_cb(win, "select_button", skin_selected);
        }
    } else {        
        handle_selection(ctrl, evt_type);
    }
}


/* DEFAULT PATHS CONFIGURATION */
static char *conf_video_path;
static char *conf_audio_path;

static void confpath_release_backup(void){
  if (conf_video_path != NULL){
    free(conf_video_path);
  }
  if (conf_audio_path != NULL){
    free(conf_audio_path);
  }
}

static void conf_paths_refresh(gui_window win){
  struct gui_control * label_ctrl;      
  label_ctrl = gui_window_get_control(win, "audio_path");
  if (label_ctrl != NULL){
    label_set_text(label_ctrl->obj, config.audio_folder);
  }
  label_ctrl = gui_window_get_control(win, "video_path");
  if (label_ctrl != NULL){
    label_set_text(label_ctrl->obj, config.video_folder);
  }
}

static void conf_path_new(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    const struct gui_control * fs_ctrl;
    gui_window defpath_guiwin;

    if (evt){
        fs_ctrl =  gui_window_get_control(ctrl->win, "select_folder");  
        if (fs_ctrl != NULL){        
            config_set_default_folder(ctrl->cb_param, fs_get_folder(fs_ctrl->obj));    
        }
        gui_window_release(ctrl->win);
        defpath_guiwin = gui_window_get_top();
        if (defpath_guiwin != NULL){
            conf_paths_refresh(defpath_guiwin);
        }
    } else {
        handle_selection(ctrl, type);
    }
}

static void conf_paths_choose(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    gui_window  win;  
    const struct gui_control * fs_ctrl;
    struct gui_control * ok_button;
    
    if (evt){
        win = gui_window_load(dfb, layer, get_full_conf(GUI_SCREEN_SELECT_FOLDER)); 
        gui_window_attach_cb(win,"cancel", quit_current_window);
        gui_window_attach_cb(win,"select_folder",dispatch_fs_event);
        fs_ctrl =  gui_window_get_control(win, "select_folder");  
        if (fs_ctrl != NULL){        
            if (ctrl->cb_param == CONFIG_AUDIO){
            fs_new_path(fs_ctrl->obj, config.audio_folder, "^$");    
            } else {
            fs_new_path(fs_ctrl->obj, config.video_folder, "^$");    
            }
        }
        ok_button = gui_window_get_control(win, "ok");  
        if (ok_button != NULL){        
            ok_button->cb_param = ctrl->cb_param;
            gui_window_attach_cb(win,"ok",conf_path_new);
        }
    } else {
        handle_selection(ctrl, type);
    }
}

static void conf_paths_cancel(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    if (evt) {
        config_set_default_folder(CONFIG_VIDEO, conf_video_path);    
        config_set_default_folder(CONFIG_AUDIO, conf_audio_path);    
        confpath_release_backup();
        gui_window_release(ctrl->win);  
    } else {
        handle_selection(ctrl, type);
    }
}

static void conf_paths_ok (struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    if (evt){
        confpath_release_backup();
        gui_window_release(ctrl->win);
    } else {
        handle_selection(ctrl, type);
    }
}

static void conf_default_paths(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    gui_window  win;
    struct gui_control * select_folder_button;
    
    if (evt){
        /* Keep paths to be able to resore them in case of cancel */
        conf_video_path = strdup(config.video_folder);
        conf_audio_path = strdup(config.audio_folder);
        
        win = gui_window_load(dfb, layer, get_full_conf(GUI_SCREEN_CONF_DEFAULT_PATHS)); 
        gui_window_attach_cb(win,"cancel",conf_paths_cancel);
        gui_window_attach_cb(win,"ok",conf_paths_ok);
        select_folder_button = gui_window_get_control(win,"choose_audio_path");
        if (select_folder_button != NULL){
            select_folder_button->cb_param = CONFIG_AUDIO;
            gui_window_attach_cb(win,"choose_audio_path", conf_paths_choose);
        }
        select_folder_button = gui_window_get_control(win,"choose_video_path");
        if (select_folder_button != NULL){
            select_folder_button->cb_param = CONFIG_VIDEO;
            gui_window_attach_cb(win,"choose_video_path", conf_paths_choose);
        }
        conf_paths_refresh(win);
    } else {
        handle_selection(ctrl, type);
    }
}


/*SCREEN SAVER */
static int screen_saver_toggle_nb = 0;
static void screen_saver_inc_delay(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    struct gui_control * vm_ctrl;

    if (evt) {
        vm_ctrl = gui_window_get_control(ctrl->win, "delay_value");
        if (vm_ctrl != NULL){    
            vm_inc(vm_ctrl->obj);
        }
    } else {
        handle_selection(ctrl, type);
    }
}

static void screen_saver_dec_delay(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    struct gui_control * vm_ctrl;
    
    if (evt) {
        vm_ctrl = gui_window_get_control(ctrl->win, "delay_value");
        if (vm_ctrl != NULL){    
            vm_dec(vm_ctrl->obj);
        }
    } else {
        handle_selection(ctrl, type);
    }
}

static void screen_saver_cancel(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    
    if (evt){
        /* Come back to the initial activation state */  
        if (screen_saver_toggle_nb & 1){
            config_toggle_screen_saver_state();
        }
        gui_window_release(ctrl->win);
    } else {
        handle_selection(ctrl, type);
    }
    return;
}

static void screen_saver_ok(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    struct gui_control * vm_ctrl;  
    double val; 

    if (evt){
        vm_ctrl = gui_window_get_control(ctrl->win, "delay_value");
        if (vm_ctrl != NULL){    
            val = vm_get_value(vm_ctrl->obj);
            /* Set new dleay (state has alreday been modified)*/
            config_set_screensaver_to((int)val);
        }
        gui_window_release(ctrl->win);
    } else {
        handle_selection(ctrl, type);
    }
    return;
}

static void screen_saver_toggle(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    struct gui_control * check;

    if (evt) {
        screen_saver_toggle_nb++;
        config_toggle_screen_saver_state();
        check = gui_window_get_control(ctrl->win, "check_screen_saver");
        if (check != NULL){
            if (config.enable_screen_saver){
                blit_img(check, "check_screen_saver");
            } else {
                blit_img(check, "no_check_screen_saver");
            }
        }
    } else {
        handle_selection(ctrl, type);
    }
    return;
}

static void config_screen_saver(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    gui_window  win;
    struct gui_control * vm_ctrl;
    struct gui_control * check;
    
    if (evt) {
        screen_saver_toggle_nb = 0;
        win = gui_window_load(dfb, layer, get_full_conf(GUI_SCREEN_CONF_SCREEN_SAVER)); 
        gui_window_attach_cb(win,"cancel",screen_saver_cancel);
        gui_window_attach_cb(win,"ok",screen_saver_ok);
        gui_window_attach_cb(win,"plus_delay", screen_saver_inc_delay);
        gui_window_attach_cb(win,"minus_delay", screen_saver_dec_delay);
        gui_window_attach_cb(win,"check_screen_saver", screen_saver_toggle);
        vm_ctrl = gui_window_get_control(win, "delay_value");
        if (vm_ctrl != NULL){
            vm_handle vm;
            vm = vm_ctrl->obj;
            vm_set_value(vm, config.screen_saver_to);
        }
        check = gui_window_get_control(win, "check_screen_saver");
        if (check != NULL){
            if (config.enable_screen_saver){
            blit_img(check, "check_screen_saver");
            } else {
            blit_img(check, "no_check_screen_saver");
            }
        }   
    } else {
        handle_selection(ctrl, type);
    }    
}

/* DIAPORAMA settings */
static int diapo_toggle_nb ;
static char *diapo_path;

static void diapo_paths_refresh(gui_window win){
  struct gui_control * label_ctrl;      
  label_ctrl = gui_window_get_control(win, "photo_path");
  if (label_ctrl != NULL){
    label_set_text(label_ctrl->obj, config.diapo.file_path);
  }
}

static void diapo_new_path(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    const struct gui_control * fs_ctrl;
    gui_window defpath_guiwin;

    if (evt) {
        fs_ctrl =  gui_window_get_control(ctrl->win, "select_folder");  
        if (fs_ctrl != NULL){        
            config_set_diapo_folder(fs_get_folder(fs_ctrl->obj));    
        }
        gui_window_release(ctrl->win);
        defpath_guiwin = gui_window_get_top();
        if (defpath_guiwin != NULL){
            diapo_paths_refresh(defpath_guiwin);
        }
    } else {
        handle_selection(ctrl, type);
    }
}

static void diapo_choose_path(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    gui_window  win;  
    const struct gui_control * fs_ctrl;
    struct gui_control * ok_button;

    if (evt) {
        win = gui_window_load(dfb, layer, get_full_conf(GUI_SCREEN_SELECT_FOLDER)); 
        gui_window_attach_cb(win,"cancel", quit_current_window);
        gui_window_attach_cb(win,"select_folder",dispatch_fs_event);
        fs_ctrl =  gui_window_get_control(win, "select_folder");  
        if (fs_ctrl != NULL){        
            fs_new_path(fs_ctrl->obj, config.diapo.file_path, "^$");      
        }
        gui_window_attach_cb(win,"ok", diapo_new_path);
    } else {
        handle_selection(ctrl, type);
    }
}

static void conf_diapo_release(void){
  if (diapo_path != NULL){
    free(diapo_path);
    diapo_path=NULL;
  }
}

static void diapo_inc_delay(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    struct gui_control * vm_ctrl;

    if (evt){
        vm_ctrl = gui_window_get_control(ctrl->win, "delay_value");
        if (vm_ctrl != NULL){    
            vm_inc(vm_ctrl->obj);
        }
    } else {
        handle_selection(ctrl, type);
    }
}

static void diapo_dec_delay(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    struct gui_control * vm_ctrl;

    if (evt){
        vm_ctrl = gui_window_get_control(ctrl->win, "delay_value");
        if (vm_ctrl != NULL){    
            vm_dec(vm_ctrl->obj);
        }
    } else {
        handle_selection(ctrl, type);
    }
}

static void diapo_toggle(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    struct gui_control * check;
    
    if (evt) {
        diapo_toggle_nb++;
        config_toggle_enable_diapo();
        check = gui_window_get_control(ctrl->win, "check_diapo");
        if (check != NULL){
            if (config.diapo_enabled){
            blit_img(check, "check_diapo");
            } else {
            blit_img(check, "no_check_diapo");
            }
        }
    } else {
        handle_selection(ctrl, type);
    }
    return;
}

static void diapo_cancel(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    if (evt){
        if (diapo_toggle_nb & 1){
            config_toggle_enable_diapo();
        }
        config_set_diapo_folder(diapo_path);
        conf_diapo_release();
        gui_window_release(ctrl->win);
    } else {
        handle_selection(ctrl, type);
    }
    return;
}


static void diapo_ok(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    struct gui_control * vm_ctrl;

    if (evt){
        vm_ctrl = gui_window_get_control(ctrl->win, "delay_value");
        if (vm_ctrl != NULL){
            config_set_diapo_delay((int)vm_get_value(vm_ctrl->obj));
        }
        conf_diapo_release();
        gui_window_release(ctrl->win);
    } else {
        handle_selection(ctrl, type);
    }
    return;
}

static void conf_diapo(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    gui_window  win;
    struct gui_control * vm_ctrl;
    struct gui_control * check;

    if (evt){
        diapo_toggle_nb = 0;
        diapo_path = strdup(config.diapo.file_path);
        win = gui_window_load(dfb, layer, get_full_conf(GUI_SCREEN_CONF_DIAPO)); 
        gui_window_attach_cb(win,"cancel",diapo_cancel);
        gui_window_attach_cb(win,"ok",diapo_ok);
        gui_window_attach_cb(win,"plus_delay", diapo_inc_delay);
        gui_window_attach_cb(win,"minus_delay", diapo_dec_delay);
        gui_window_attach_cb(win,"choose_photo_path", diapo_choose_path);
        vm_ctrl = gui_window_get_control(win, "delay_value");
        if (vm_ctrl != NULL){
            vm_handle vm;
            vm = vm_ctrl->obj;
            vm_set_value(vm, ((double)config.diapo.delay));
        }
        check = gui_window_get_control(win, "check_diapo");
        if (check != NULL){
            if (config.diapo_enabled){
            blit_img(check, "check_diapo");
            } else {
            blit_img(check, "no_check_diapo");
            }
            gui_window_attach_cb(win,"check_diapo", diapo_toggle);
        }   
        diapo_paths_refresh(win);  
    } else {
        handle_selection(ctrl, type);
    }
}

/* FM transmitter settings CONFIGUGARTION */
static int fm_transmitter_toggle_nb = 0;
static void fm_inc_freq(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    struct gui_control * vm_ctrl;
    
    if (evt){
        vm_ctrl = gui_window_get_control(ctrl->win, "freq_value");
        if (vm_ctrl != NULL){    
            vm_inc(vm_ctrl->obj);
        }
    } else {
        handle_selection(ctrl, type);
    }
}

static void fm_dec_freq(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    struct gui_control * vm_ctrl;
    
    if (evt) {
        vm_ctrl = gui_window_get_control(ctrl->win, "freq_value");
        if (vm_ctrl != NULL){    
            vm_dec(vm_ctrl->obj);
        }
    } else {
        handle_selection(ctrl, type);
    }
}

static void fm_cancel(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    if (evt){
        /* Come back to the initial activation state */
        if (fm_transmitter_toggle_nb & 1){
        config_toggle_fm_transmitter_state();
        }
        gui_window_release(ctrl->win);
    } else {
        handle_selection(ctrl, type);
    }
    return;
}

static void fm_ok(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    struct gui_control * vm_ctrl;  
    double val; 
    double freq;

    if (evt){
        vm_ctrl = gui_window_get_control(ctrl->win, "freq_value");
        if (vm_ctrl != NULL){    
        val = vm_get_value(vm_ctrl->obj);	    
        freq = config.fm_transmitter;
        if (val == (((double)config.fm_transmitter1) / (double)1000000.00 )){
        config_set_fm_frequency1(freq);
        } else {
        if (val == (((double)config.fm_transmitter2) / (double)1000000.00 )){
            config_set_fm_frequency2(freq);
        } else {
            config_set_fm_frequency2(config.fm_transmitter1);
            config_set_fm_frequency1(freq);
        }
        }
        /* Set new frequency (state has alreday been modified)*/
        config_set_fm_frequency( (int)(val * (double)1000000.00));

        }
        gui_window_release(ctrl->win);
    } else {
        handle_selection(ctrl, type);
    }
    return;
}

static void fm_toggle(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    struct gui_control * check;

    if (evt) {
        fm_transmitter_toggle_nb++;  
        config_toggle_fm_transmitter_state();
        check = gui_window_get_control(ctrl->win, "check_fm");
        if (check != NULL){
            if (config.enable_fm_transmitter){
                blit_img(check, "check_fm");
            } else {
                blit_img(check, "no_check_fm");
            }
        }
    } else {
        handle_selection(ctrl, type);
    }
    return;
}

static void fm_recall_fm(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    struct gui_control * vm_ctrl;  

    if (evt){
        vm_ctrl = gui_window_get_control(ctrl->win, "freq_value");
        if (vm_ctrl != NULL){
        vm_handle vm;
        vm = vm_ctrl->obj;
        if (ctrl->cb_param == 1) {
            vm_set_value(vm, ((double)config.fm_transmitter1) / (double)1000000.00 );
        } else {
            vm_set_value(vm, ((double)config.fm_transmitter2) / (double)1000000.00 );
        }
        }
    } else {
        handle_selection(ctrl, type);
    }
}

static void  conf_fm_tansmitter(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    gui_window  win;
    struct gui_control * vm_ctrl;
    struct gui_control * check;
    struct gui_control * recall_ctrl;
    char buffer_fm[16];

    if (evt){
        fm_transmitter_toggle_nb = 0;
        win = gui_window_load(dfb, layer, get_full_conf(GUI_SCREEN_CONF_FM_TRANSMITTER)); 
        gui_window_attach_cb(win,"cancel",fm_cancel);
        gui_window_attach_cb(win,"ok",fm_ok);
        gui_window_attach_cb(win,"plus_freq", fm_inc_freq);
        gui_window_attach_cb(win,"minus_freq", fm_dec_freq);
        gui_window_attach_cb(win,"check_fm", fm_toggle);
        gui_window_attach_cb(win,"fm1", fm_recall_fm);
        gui_window_attach_cb(win,"fm2", fm_recall_fm);
        recall_ctrl = gui_window_get_control(win, "fm1");
        if (recall_ctrl != NULL){	  
            recall_ctrl->cb_param = 1;  
            snprintf(buffer_fm, sizeof(buffer_fm),"%06.2f Mhz",(double)config.fm_transmitter1 / (double)1000000.00);	  
            label_set_text(recall_ctrl->obj,buffer_fm);  
        }
        recall_ctrl = gui_window_get_control(win, "fm2");
        if (recall_ctrl != NULL){
            recall_ctrl->cb_param = 2;
            snprintf(buffer_fm, sizeof(buffer_fm),"%06.2f Mhz",(double)config.fm_transmitter2 / (double)1000000.00);	  
            label_set_text(recall_ctrl->obj,buffer_fm);  
        }  
        vm_ctrl = gui_window_get_control(win, "freq_value");
        if (vm_ctrl != NULL){
            vm_handle vm;
            vm = vm_ctrl->obj;
            vm_set_value(vm, ((double)config.fm_transmitter) / (double)1000000.00 );
        }
        check = gui_window_get_control(win, "check_fm");
        if (check != NULL){
            if (config.enable_fm_transmitter){
            blit_img(check, "check_fm");
            } else {
            blit_img(check, "no_check_fm");
            }
        }   
    } else {
        handle_selection(ctrl, type);
    }
}

/* CONFIGURATION MENU */

static void  conf_toggle_small_text(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){  
    if (evt) {
        config_toggle_small_text_state();
        /* Display the check box according to the new small text config */
        if (config.enable_small_text){
            blit_img(ctrl, "check_small_text");
        } else {
            blit_img(ctrl, "no_check_small_text");
        }
    } else {
        handle_selection(ctrl, type);
    }
    return;
}

/* Effectively save the new configuration */
static void conf_save(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    if (evt) {
        gui_window_release(ctrl->win);
        config_save();  
    } else {
        handle_selection(ctrl, type);
    }
}

static void conf_cancel(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){    
    if (evt) {
        gui_window_release(ctrl->win);
        config_reload();
    }  else {
        handle_selection(ctrl, type);
    }
}

static void configuration_menu(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
  gui_window  win;
  struct gui_control * check;
  
  if (evt){
    gui_window_release(ctrl->win);
    win = gui_window_load(dfb, layer, get_full_conf(GUI_SCREEN_CONFIGURATION)); 


    gui_window_attach_cb(win,"config_path", conf_default_paths);
    gui_window_attach_cb(win,"config_saver",config_screen_saver);  
    gui_window_attach_cb(win,"check_small_text", conf_toggle_small_text);
    gui_window_attach_cb(win,"config_fm", conf_fm_tansmitter);
    gui_window_attach_cb(win,"config_diapo", conf_diapo);
    gui_window_attach_cb(win,"cancel",conf_cancel);
    gui_window_attach_cb(win,"ok",conf_save);

    /* Display the check box according to current small text config */
    check = gui_window_get_control(win, "check_small_text");
    if (check != NULL){
        if (config.enable_small_text){
        blit_img(check, "check_small_text");
        } else {
        blit_img(check, "no_check_small_text");
        }
    }   
  } else {
        handle_selection(ctrl, type);
  }
}


/* CHOOSE SETTINGS SCREEN */

/** Callback on choose settings screen */
static void choose_settings(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    gui_window  win;

    if (evt) {
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
        gui_window_attach_cb(win,"configuration",configuration_menu);
        gui_window_attach_cb(win,"cancel",quit_current_window);
    } else {
        handle_selection(ctrl, type);
    }
    return;
}



/* AUDIO SCREEN */

static bool shuffle_state = false;
/**Callback on audio and video selection*/
static void play_audio_video(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    FILE *fp;
    flenum list;
    const char * filename;

    if (evt){
        const struct gui_control * ctrl_fs =  gui_window_get_control(ctrl->win, "file_selector");
        int file_nb = 0;
        fs_handle fs = ctrl_fs->obj;    
        list =  fs_get_selection(fs);
        fp = fopen( "/tmp/playlist.m3u", "w+" );
        while ( (filename = flenum_get_next_file(list, shuffle_state)) != NULL)  {
            file_nb++;
            fprintf( fp, filename );
            fprintf( fp, "\n" );
        }
        flenum_release(list);
        fclose(fp);
        if (file_nb == 0){
            DFBColor color = {255,255,50,50};
            message_box("No file selected !", 24, &color, "./res/font/decker.ttf");
        } else {
            launch_engine("/tmp/playlist.m3u", 0, ctrl->cb_param);
            quit = true;
        }
    } else {
        handle_selection(ctrl, type);
    }
}


/** Callback on resume audio and video buttons */
static void resume_audio_video(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    int pos = 0;
    struct stat ftype;
    char filename[PATH_MAX - 32];    
    char mv_command[PATH_MAX];  
    
    if (evt){
        bool is_video = ctrl->cb_param;
        gui_window_release(ctrl->win);
        if (resume_get_file_infos(filename, PATH_MAX, &pos) != 0){
            DFBColor color = {255,255,50,50};
            message_box("No resume data...", 24, &color, "./res/font/decker.ttf");    	
            return;
        }  
        if( stat(filename, &ftype) != 0){
            DFBColor color = {255,255,50,50};
            message_box("Error no saved playlist...", 24, &color, "./res/font/decker.ttf");    
        }  else {
            snprintf(mv_command, sizeof(mv_command), "mv %s /tmp/playlist.m3u",filename);
            system(mv_command);	
            launch_engine("/tmp/playlist.m3u", pos, is_video);  	
            quit = true;
        }
    } else {
        handle_selection(ctrl, type);
    }
}

/** Callback on select all button */
static void select_all_files(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    if (evt){
        const struct gui_control * ctrl_fs =  gui_window_get_control(ctrl->win, "file_selector");
        fs_handle fs = ctrl_fs->obj;  
        fs_select_all(fs);
    } else {
        handle_selection(ctrl, type);
    }
}

/** Callback on unselect all button */
static void unselect_all_files(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    if (evt){
        const struct gui_control * ctrl_fs =  gui_window_get_control(ctrl->win, "file_selector");
        fs_handle fs = ctrl_fs->obj;          
        fs_unselect_all(fs);
    } else {
        handle_selection(ctrl, type);
    }
}

/** Callback on shuffle controller */
static void toggle_suffle(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){  
    if (evt){
        shuffle_state = !shuffle_state;
        if (shuffle_state){
            blit_img(ctrl, "shuflle_on_button");
        } else {
            blit_img(ctrl, "shuflle_off_button");
        }  
    } else {
        handle_selection(ctrl, type);
    }
    return;
}

/** Callback that displays audio selection screen */
static void select_audio(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    gui_window  win;
    fs_handle fs;
    const struct gui_control * fs_ctrl;
    struct gui_control *play_ctrl;
    
    /* TODO  Write a decent config module !*/
    extern struct tomplayer_config config;

    if (evt) {
        win = gui_window_load(dfb, layer, get_full_conf(GUI_SCREEN_AUDIO)); 
        gui_window_attach_cb(win, "file_selector", dispatch_fs_event);
        gui_window_attach_cb(win, "goback_button", quit_current_window);
        gui_window_attach_cb(win, "selall_button", select_all_files);
        gui_window_attach_cb(win, "nosel_button", unselect_all_files);
        gui_window_attach_cb(win, "play_button", play_audio_video);
        gui_window_attach_cb(win, "shuflle_on_button", toggle_suffle);
        fs_ctrl =  gui_window_get_control(win, "file_selector");
        if (fs_ctrl != NULL){
            fs = fs_ctrl->obj;
            /*fs_set_select_cb(fs, audio_select_cb);*/
            fs_new_path(fs, config.audio_folder, config.filter_audio_ext);
        }
        play_ctrl = gui_window_get_control(win, "play_button");
        if (play_ctrl != NULL){ 
            play_ctrl->cb_param = false;
        }   
    } else {
        handle_selection(ctrl, type);
    }
}


/* CHOOSE RESUME SCREEN */

/** Callback on choose resume screen */
static void resume(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    gui_window  win;
    struct gui_control *play_ctrl;

    if (evt){
        win = gui_window_load(dfb, layer, get_full_conf(GUI_SCREEN_RESUME)); 

        gui_window_attach_cb(win, "resume_video", resume_audio_video);
        gui_window_attach_cb(win, "resume_audio", resume_audio_video);
        gui_window_attach_cb(win, "cancel", quit_current_window);

        play_ctrl = gui_window_get_control(win, "resume_video");   
        if (play_ctrl != NULL){ 
            play_ctrl->cb_param = true;
        }   
        play_ctrl = gui_window_get_control(win, "resume_audio");   
        if (play_ctrl != NULL){ 
            play_ctrl->cb_param = false;
        }   
    } else {
        handle_selection(ctrl, type);
    }
    return;
}


/* MAIN SCREEN */
/** Callback on exit button */
static void quit_tomplayer(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    if (evt) {
        gui_window_release(ctrl->win);
        quit = true;
    } else {
        handle_selection(ctrl, type);
    }
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
   gui_window_attach_cb(win, "resume_button", resume);
   gui_window_attach_cb(win, "settings_button", choose_settings);
  return true;
}

/** Callback that displays main screen */
static void enter_main_screen(struct gui_control *ctrl, enum gui_event_type type, union gui_event* evt){
    if (evt){
        /* Dont want to keep the splash screen window => release it */
        gui_window_release(ctrl->win);
        load_main_screen();
    }
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
