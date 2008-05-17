/***************************************************************************
 *            config.h
 *
 *  Sun Jan  6 14:15:55 2008
 *  Copyright  2008  nullpointer
 *  Email
 ****************************************************************************/
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

#ifndef __TOMPLAYER_CONFIG_H__
#define __TOMPLAYER_CONFIG_H__

#include <linux/limits.h>
#include <IL/il.h>
#include <IL/ilu.h>

#define MAX_CONTROLS	            20

/* List of available commands */
enum {
	CMD_EXIT_MENU = 0,
    CMD_PAUSE     = 1,
    CMD_STOP      = 2,
    CMD_MUTE      = 3,
    CMD_VOL_MOINS = 4,
    CMD_VOL_PLUS  = 5,
    CMD_LIGHT_MOINS = 6,
    CMD_LIGHT_PLUS  = 7,
    CMD_DELAY_MOINS = 8,
    CMD_DELAY_PLUS  = 9,
    CMD_GAMMA_MOINS = 10,
    CMD_GAMMA_PLUS  = 11,
    CMD_FORWARD     = 12,
    CMD_BACKWARD    = 13,
    CMD_NEXT        = 14,
    CMD_PREVIOUS    = 15,
    CMD_BATTERY_STATUS = 16,
    CMD_ANIM,
    CMD_MAX_NB
};

#define CONFIG_FILE                 "/tmp/tomplayer.ini"

/* Definition of section in the config file */
#define SECTION_GENERAL             "general"
#define SECTION_VIDEO_SKIN          "video_skin"
#define SECTION_AUDIO_SKIN          "audio_skin"
#define SECTION_CONTROL_FMT_STR     "CONTROL_%d"


#define KEY_SKIN_FILENAME           "filename"
#define KEY_LOADING_BMP             "loading"
#define KEY_EXITING_BMP             "exiting"


#define KEY_FILTER_VIDEO_EXT        "filter_video"
#define KEY_FILTER_AUDIO_EXT        "filter_audio"
#define KEY_VIDEO_FILE_DIRECTORY    "video_dir"
#define KEY_AUDIO_FILE_DIRECTORY    "audio_dir"

#define KEY_SKIN_BMP                "image"
#define KEY_SKIN_CONF               "conf"

#define KEY_TEXT_X1                 "text_x1"
#define KEY_TEXT_Y1                 "text_y1"
#define KEY_TEXT_X2                 "text_x2"
#define KEY_TEXT_Y2                 "text_y2"

#define KEY_R_TRANSPARENCY          "r"
#define KEY_G_TRANSPARENCY          "g"
#define KEY_B_TRANSPARENCY          "b"


#define KEY_R_PROGRESSBAR           "pb_r"
#define KEY_G_PROGRESSBAR	    	"pb_g"
#define KEY_B_PROGRESSBAR           "pb_b"

#define KEY_SCREEN_SAVER_TO			"screen_saver_to"

#define KEY_TYPE_CONTROL            "type"
#define KEY_CMD_CONTROL             "ctrl"
#define KEY_CMD_CONTROL2            "cmd"

#define KEY_CIRCULAR_CONTROL_X      "x"
#define KEY_CIRCULAR_CONTROL_Y      "y"
#define KEY_CIRCULAR_CONTROL_R      "r"

#define KEY_RECTANGULAR_CONTROL_X1  "x1"
#define KEY_RECTANGULAR_CONTROL_X2  "x2"
#define KEY_RECTANGULAR_CONTROL_Y1  "y1"
#define KEY_RECTANGULAR_CONTROL_Y2  "y2"

#define KEY_TEXT_COLOR "text_color"

#define KEY_CTRL_BITMAP_FILENAME "bitmap"

/* Define the shape of control */
enum E_TYPE_CONTROL {
    CIRCULAR_CONTROL = 1,
    RECTANGULAR_CONTROL,
    PROGRESS_CONTROL_X,
    PROGRESS_CONTROL_Y
} ;

struct circularControl {
    int x,y,r;
} ;

struct rectangularControl {
    int x1,y1,x2,y2;
} ;


union control_type {
    struct circularControl circular;
    struct rectangularControl rectangular;
} ;

struct control{
    enum E_TYPE_CONTROL type;
    int cmd;
    union control_type area;		
    char bitmap_filename[PATH_MAX]; // Optional bitmap filename associated with the control
    ILuint bitmap;
};


struct skin_config{
    int nb;
    struct control controls[MAX_CONTROLS];
    int r,g,b;    
    int text_x1,text_y1,text_x2,text_y2;
    char bitmap_filename[PATH_MAX];
    ILuint bitmap;
    char conf_file[PATH_MAX];    
    int progress_bar_index; /**< index of progress bar object in controls table*/    
    int pb_r, pb_g, pb_b;
    int bat_index; /**< index of Battery object in controls table*/    
    int text_color;
    int cmd2idx[CMD_MAX_NB];
};



struct tomplayer_config{
    char bitmap_loading_filename[PATH_MAX];
    char bitmap_exiting_filename[PATH_MAX];
    ILuint bitmap_loading;
    ILuint bitmap_exiting;    
    
    char filter_video_ext[PATH_MAX];
    char filter_audio_ext[PATH_MAX];
    char video_folder[PATH_MAX];
    char audio_folder[PATH_MAX];
    char video_skin_filename[PATH_MAX];
    char audio_skin_filename[PATH_MAX];
    struct skin_config video_config;
    struct skin_config audio_config;
    int screen_saver_to;
};




int load_config( struct tomplayer_config * conf );
int load_skin_config( char *, struct skin_config * );

#endif /* __TOMPLAYER_CONFIG_H__ */
