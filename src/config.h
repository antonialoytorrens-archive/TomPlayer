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
#include "diapo.h"

/* FIFO Where key inputs can be read from */
#define KEY_INPUT_FIFO "/tmp/key_fifo"


enum config_int_speaker_type{
	CONF_INT_SPEAKER_AUTO = 0,
	CONF_INT_SPEAKER_NO,
	CONF_INT_SPEAKER_ALWAYS,
	CONF_INT_SPEAKER_MAX
};

/**
 * \def MAX_SKIN_CONTROLS
 * \brief define the max of control in a skin
 */
#define MAX_SKIN_CONTROLS	            20

/**
 * \def CONFIG_FILE
 * \brief fullpath to the main config file
 */
#define CONFIG_FILE                 "/tmp/tomplaye.ini"

enum config_type {
  CONFIG_AUDIO,
  CONFIG_VIDEO
};

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
    enum E_TYPE_SKIN_CONTROL type;      /*!< Type of skin control */
    enum SKIN_CMD cmd;			/*!< command associated*/
    union skin_control_type area;	/*!< location of skin control */
    char bitmap_filename[PATH_MAX];     /*!< Optional bitmap filename associated with the control */
    ILuint bitmap;			/*!< DevIL bitmap */
};


/**
 * \struct skin_config
 * \brief Skin configuration structure
 */ /*!< index of Battery object in controls table*/
struct skin_config{
    int nb;	                                     /*!< number of controls in the skin */
    struct skin_control controls[MAX_SKIN_CONTROLS]; /*!<array of control within the skin */
    int text_color;                      /*!< color of text */
    int r,g,b;                           /*!< color of the transparent color (when using image format such as bmp that does not hold this info) */
    int text_x1,text_y1,text_x2,text_y2; /*!< area where the a text can be displayed */
    char bitmap_filename[PATH_MAX]; /*!< filename of background bitmap */
    ILuint bitmap;                  /*!< DevIL background bitmap */
    int progress_bar_index;         /*!< index of progress bar object in controls table*/
    int pb_r, pb_g, pb_b;           /*!< progress bar color*/
    int bat_index;                  /*!< index of Battery object in controls table*/
    int cmd2idx[SKIN_CMD_MAX_NB];   /*!< command to skin control index table */
    int selection_order;            /*!< if 0 an heuristic is used for navigation order otherwise the control description order is kept */
    int first_selection;            /*!< Default selected  control */    
};


/**
 * \struct tomplayer_config
 * \brief Main configuration structure
 */
struct tomplayer_config{
    char filter_video_ext[PATH_MAX];		/*!<List of supported video file extension */
    char filter_audio_ext[PATH_MAX];		/*!<List of supported audio file extension */
    char video_folder[PATH_MAX];			/*!<fullpath to the video folder */
    char audio_folder[PATH_MAX];			/*!<fullpath to the audio folder */
    char video_skin_filename[PATH_MAX];		/*!<fullpath to the video skin archive */
    char audio_skin_filename[PATH_MAX];		/*!<fullpath to the audio skin archive */
    struct skin_config video_config;		/*!<video skin config */
    struct skin_config audio_config;		/*!<audio skin config */
    int screen_saver_to;			        /*!<screensaver timeout */
    int enable_screen_saver;                /*!<Enable Screen saver */
    unsigned int fm_transmitter;            /*!<FM transmitter frequency in HZ  */
    unsigned int fm_transmitter1;           /*!<First FM transmitter frequency backup  */
    unsigned int fm_transmitter2;           /*!<Second FM transmitter frequency backup */ 
    int enable_fm_transmitter;              /*!<Enable FM transmitter*/ 
    int diapo_enabled;                      /*!<Enable Diaporama */     
    struct diapo_config diapo;              /*!<Diaporama Config */
    int enable_small_text;                  /*!<Enable samll text in file selector*/
    int int_speaker; 						/*!<Internal speaker configuration*/
    int video_preview;						/*!<Enable video preview*/    
};




bool load_config( struct tomplayer_config * conf );
bool load_skin_config( char *, struct skin_config *);
bool config_save(void);
bool config_set_skin(enum config_type type, const char * filename);
bool config_set_default_folder(enum config_type type, const char * folder);
bool config_set_screensaver_to(int delay);
bool config_set_fm_frequency(int freq);
bool config_set_fm_frequency1(int freq);
bool config_set_fm_frequency2(int freq);
bool config_set_int_speaker(int mode);
bool config_toggle_enable_diapo(void);
bool config_set_diapo_folder(const char *folder);
bool config_set_diapo_delay(int delay);
bool config_toggle_screen_saver_state(void);
bool config_toggle_fm_transmitter_state(void);
bool config_toggle_small_text_state(void);
void config_free(void);
void config_reload(void);
const char * skin_cmd_2_txt(enum SKIN_CMD);

struct rectangular_skin_control control_get_zone(const struct skin_control *ctrl);
    
#endif /* __TOMPLAYER_CONFIG_H__ */
