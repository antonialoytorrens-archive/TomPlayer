/**
 * \file config.h
 * \author nullpointer
 * \brief This module implements configuration reading
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


#ifndef __TOMPLAYER_CONFIG_H__
#define __TOMPLAYER_CONFIG_H__
#include <stdbool.h>
#include <linux/limits.h>
#include <IL/il.h>
#include <IL/ilu.h>

/* Definition of section in the config file */
#define SECTION_GENERAL             "general"
#define SECTION_VIDEO_SKIN          "video_skin"
#define SECTION_AUDIO_SKIN          "audio_skin"
#define SECTION_CONTROL_FMT_STR     "CONTROL_%d:%s"


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

/**
 * \def MAX_SKIN_CONTROLS
 * \brief define the max of control in a skin
 */
#define MAX_SKIN_CONTROLS	            20

/**
 * \def CONFIG_FILE
 * \brief fullpath to the main config file
 */
#define CONFIG_FILE                 "/tmp/tomplayer.ini"

/**
 * \enum SKIN_CMD
 * \brief define all available command in a skin
 */
enum SKIN_CMD{
	SKIN_CMD_EXIT_MENU = 0, 	/*!<exit skin */
    SKIN_CMD_PAUSE     = 1,		/*!<send a pause command to mplayer */
    SKIN_CMD_STOP      = 2,		/*!<send a stop command to mplayer */
    SKIN_CMD_MUTE      = 3,		/*!<send a mute command to mplayer */
    SKIN_CMD_VOL_MOINS = 4,		/*!<send a volume command to mplayer */
    SKIN_CMD_VOL_PLUS  = 5,		/*!<send a volume command to mplayer */
    SKIN_CMD_LIGHT_MOINS = 6,	/*!<send a light command to mplayer */
    SKIN_CMD_LIGHT_PLUS  = 7,	/*!<send a light command to mplayer */
    SKIN_CMD_DELAY_MOINS = 8,	/*!<send a delay+ command to mplayer */
    SKIN_CMD_DELAY_PLUS  = 9,	/*!<send a delay- command to mplayer */
    SKIN_CMD_GAMMA_MOINS = 10,	/*!<send a gamma+ command to mplayer */
    SKIN_CMD_GAMMA_PLUS  = 11,	/*!<send a gamma- command to mplayer */
    SKIN_CMD_FORWARD     = 12,	/*!<send a forward command to mplayer */
    SKIN_CMD_BACKWARD    = 13,	/*!<send a backward command to mplayer */
    SKIN_CMD_NEXT        = 14,	/*!<send a next command to mplayer */
    SKIN_CMD_PREVIOUS    = 15,	/*!<send a previous command to mplayer */
    SKIN_CMD_BATTERY_STATUS = 16,
    SKIN_CMD_ANIM,
    SKIN_CMD_MAX_NB
};




/**
 * \enum E_TYPE_SKIN_CONTROL
 * \brief define the shape of a skin control
 */
enum E_TYPE_SKIN_CONTROL {
    CIRCULAR_SKIN_CONTROL = 1,	/*!<circular control */
    RECTANGULAR_SKIN_CONTROL,	/*!<rectangular control */
    PROGRESS_SKIN_CONTROL_X,	/*!<horizontal progress bar control */
    PROGRESS_SKIN_CONTROL_Y		/*!<vertical progress bar control */
} ;

/**
 * \struct circular_skin_control
 * \brief structure of the circular skin control coordinates
 */
struct circular_skin_control {
    int x;	/*!<x center of the circular control */
    int y;	/*!<y center of the circular control */
    int r;	/*!<radius of the circular control */
} ;

/**
 * \struct rectangular_skin_control
 * \brief structure of the rectangular skin control coordinates
 */
struct rectangular_skin_control {
    int x1; /*!<left corner */
    int y1; /*!<upper corner */
    int x2; /*!< right corner */
    int y2; /*!<lower corner */
} ;


/**
 * \union skin_control_type
 * \brief union of a rectangular and a circular skin control
 */
union skin_control_type {
    struct circular_skin_control circular;
    struct rectangular_skin_control rectangular;
} ;

/**
 * \struct skin_control
 * \brief define the shape, position, bitmap and the command of a skin control
 */
struct skin_control{
    enum E_TYPE_SKIN_CONTROL type; /*< Type of skin control */
    enum SKIN_CMD cmd;						/*!< command associated*/
    union skin_control_type area;	/*!< location of skin control */
    char bitmap_filename[PATH_MAX]; /*!< Optional bitmap filename associated with the control */
    ILuint bitmap;					/*!< DevIL bitmap */
};


/**
 * \struct skin_config
 * \brief Skin configuration structure
 */
struct skin_config{
    int nb;	/*!< number of controls in the skin */
    struct skin_control controls[MAX_SKIN_CONTROLS]; /*!<array of control within the skin */
    int text_color; /*!< color of text */
    int r,g,b; /*!< color of the transparent color (when using bad image format such as bmp that does not hold this info) */
    int text_x1,text_y1,text_x2,text_y2; /*!< area where the a text can be displayed */
    char bitmap_filename[PATH_MAX]; /*!< filename of background bitmap */
    ILuint bitmap; /*!< DevIL background bitmap */
    int progress_bar_index; /*!< index of progress bar object in controls table*/
    int pb_r, pb_g, pb_b;/*!< progress bar color*/
    int bat_index; /*!< index of Battery object in controls table*/
    int cmd2idx[SKIN_CMD_MAX_NB];
};


/**
 * \struct tomplayer_config
 * \brief Main configuration structure
 */
struct tomplayer_config{
    char bitmap_loading_filename[PATH_MAX]; /*!<fullpath to the "loading" bitmap */
    char bitmap_exiting_filename[PATH_MAX]; /*!<fullpath to the "exiting" bitmap */
    ILuint bitmap_loading;					/*!<DevIL "loading" bitmap */
    ILuint bitmap_exiting;					/*!<DevIL "exiting" bitmap */

    char filter_video_ext[PATH_MAX];		/*!<List of supported video file extension */
    char filter_audio_ext[PATH_MAX];		/*!<List of supported audio file extension */
    char video_folder[PATH_MAX];			/*!<fullpath to the video folder */
    char audio_folder[PATH_MAX];			/*!<fullpath to the audio folder */
    char video_skin_filename[PATH_MAX];		/*!<fullpath to the video skin archive */
    char audio_skin_filename[PATH_MAX];		/*!<fullpath to the audio skin archive */
    struct skin_config video_config;		/*!<video skin config */
    struct skin_config audio_config;		/*!<audio skin config */
    int screen_saver_to;					/*!<screensaver timeout */
};




bool load_config( struct tomplayer_config * conf );
bool load_skin_config( char *, struct skin_config * );

#endif /* __TOMPLAYER_CONFIG_H__ */
