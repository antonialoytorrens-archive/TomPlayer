/**
 * \file skin.h 
 * \brief Handle skins configuration
 *
 * $URL$
 * $Rev$
 * $Author$
 * $Date$
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

#ifndef __SKIN_H__
#define __SKIN_H__
#include <IL/ilu.h>

#define ZIP_SKIN_BITMAP_FILENAME "/tmp/bitmap"

/** max number of controls in a skin */
#define MAX_SKIN_CONTROLS             30

/** Function associated to a control */ 
enum skin_cmd{
    /* Begin of commands for mplayer */ 
    SKIN_CMD_EXIT_MENU      = 0, /*!<exit skin */
    SKIN_CMD_PAUSE          = 1, /*!<send a pause command to mplayer */
    SKIN_CMD_STOP           = 2, /*!<send a stop command to mplayer */
    SKIN_CMD_MUTE           = 3, /*!<send a mute command to mplayer */
    SKIN_CMD_VOL_MOINS      = 4, /*!<send a volume command to mplayer */
    SKIN_CMD_VOL_PLUS       = 5, /*!<send a volume command to mplayer */
    SKIN_CMD_LIGHT_MOINS    = 6, /*!<send a light command to mplayer */
    SKIN_CMD_LIGHT_PLUS     = 7, /*!<send a light command to mplayer */
    SKIN_CMD_DELAY_MOINS    = 8, /*!<send a delay+ command to mplayer */
    SKIN_CMD_DELAY_PLUS     = 9, /*!<send a delay- command to mplayer */
    SKIN_CMD_GAMMA_MOINS    = 10,/*!<send a gamma+ command to mplayer */
    SKIN_CMD_GAMMA_PLUS     = 11,/*!<send a gamma- command to mplayer */
    SKIN_CMD_FORWARD        = 12,/*!<send a forward command to mplayer */
    SKIN_CMD_BACKWARD       = 13,/*!<send a backward command to mplayer */
    SKIN_CMD_NEXT           = 14,/*!<send a next command to mplayer */
    SKIN_CMD_PREVIOUS       = 15,/*!<send a previous command to mplayer */
    /* Others */ 
    SKIN_CMD_BATTERY_STATUS = 16,
    SKIN_CMD_ANIM           = 17,
    SKIN_CMD_TEXT_UPTIME    = 18, 
    SKIN_CMD_TEXT_TIME      = 19, 
    /* Begin of GPS info */
    SKIN_CMD_TEXT_LAT       = 20,  
    SKIN_CMD_TEXT_LONG      = 21, 
    SKIN_CMD_TEXT_ALT       = 22,
    SKIN_CMD_TEXT_SPEED     = 23,
    /* Begin of tags related info */    
    SKIN_CMD_TEXT_ARTIST    = 30,
    SKIN_CMD_TEXT_TRACK     = 31,
    SKIN_CMD_TEXT_ALBUM     = 32,
    SKIN_CMD_TEXT_TITLE     = 33,
    SKIN_CMD_TEXT_YEAR      = 34,
    SKIN_CMD_TEXT_COMMENT   = 35,
    SKIN_CMD_TEXT_GENRE     = 36,
    SKIN_CMD_COVERART       = 37,       
    /* Misc */
    SKIN_CMD_CURRENT_POS    = 40,
    SKIN_CMD_REMAINING_TIME = 41,
    SKIN_CMD_TEXT_DATE      = 42, /**< Date in ISO format  YY/MM/DD */
    SKIN_CMD_TEXT_DAY       = 43, /**< Day of the month as a number */
    SKIN_CMD_TEXT_MONTH     = 44, /**< Month name */
    SKIN_CMD_TEXT_WEEKDAY   = 45, /**< Day of the week as a name */
    SKIN_CMD_MAX_NB
};

#define SKIN_CMD_TAGS_FIRST  SKIN_CMD_TEXT_ARTIST
#define SKIN_CMD_TAGS_LAST   SKIN_CMD_TEXT_GENRE
#define SKIN_CMD_GPS_FIRST   SKIN_CMD_TEXT_LAT 
#define SKIN_CMD_GPS_LAST    SKIN_CMD_TEXT_SPEED

/** Define the shape of a skin control */ 
enum skin_control_type {
    SKIN_CONTROL_CIRCULAR = 1, /*!<circular control */
    SKIN_CONTROL_RECTANGULAR,  /*!<rectangular control */
    SKIN_CONTROL_PROGRESS_X,   /*!<horizontal progress bar control */
    SKIN_CONTROL_PROGRESS_Y,   /*!<vertical progress bar control */
    SKIN_CONTROL_TEXT          /*!<Text control */
} ;


/** Test skin descriptor */
struct skin_text_control {
    int x;                 /*!<x upper left coordinate of the text */
    int y;                 /*!<y upper left coordinate of the text */
    int size;              /*!<Font size, -1 for default */
    int color;             /*!<Text color, -1 for default */
} ;

/** Circular skin control coordinates */
struct skin_circular_shape {
    int x;  /*!<x center of the circular control */
    int y;  /*!<y center of the circular control */
    int r;  /*!<radius of the circular control */
} ;

/** Rectangular skin control coordinates */
struct skin_rectangular_shape {
    int x1; /*!<left corner */
    int y1; /*!<upper corner */
    int x2; /*!< right corner */
    int y2; /*!<lower corner */
} ;


/**
 * \struct skin_control
 * \brief define the shape, position, bitmap and the command of a skin control
 */
struct skin_control{
    char *bitmap_filename;            /*!< Optional bitmap filename associated with the control */
    enum skin_cmd cmd;                /*!< Function associated*/
    enum skin_control_type type;      /*!< Type of skin control */    
    union{
      struct skin_circular_shape circ_icon;
      struct skin_rectangular_shape rect_icon;
      struct skin_text_control text;
    } params;                         /*!< parameters depending on the type */    
};


/**
 * \struct skin_config
 * \brief Skin configuration structure
 */ 
struct skin_config{
    int nb;                                          /*!< number of controls in the skin */
    struct skin_control controls[MAX_SKIN_CONTROLS]; /*!<array of control within the skin */    
    int r,g,b;                                       /*!< color of the transparent color (when using image format such as bmp that does not hold this info) */
    int text_color;                                  /*!< color of filename text - deprecated - */
    int text_x1,text_y1,text_x2,text_y2;             /*!< area where the filename text can be displayed - deprecated - */
    char *bitmap_filename;                           /*!< filename of background bitmap */    
    int pb_r, pb_g, pb_b;                            /*!< progress bar filling color*/        
    int selection_order;                             /*!< if 0 an heuristic is used for navigation order otherwise the control description order is kept */
    int first_selection;                             /*!< Default selected  control */    
    int display_filename;                            /*!< Indicates whether the old filename text has to be displayed on the skin. 
                                                          It is common to set it to 0 with new skins which display tag infos*/
};

bool   skin_init(const char * filename, bool load_bitmaps);
bool   skin_release(void);
const struct skin_config *skin_get_config(void);
ILuint skin_get_background(void);
ILuint skin_get_img(enum skin_cmd);
ILuint skin_get_pb_img(void);
const struct skin_control *skin_get_ctrl(enum skin_cmd);
const struct skin_control *skin_get_pb(void);

const char * skin_cmd_2_txt(enum skin_cmd);
struct skin_rectangular_shape skin_ctrl_get_zone(const struct skin_control *ctrl);
enum skin_cmd skin_get_cmd_from_xy(int x, int y, int *p);

bool skin_load_bitmap(ILuint * bitmap_obj, const char * filename);
#endif
