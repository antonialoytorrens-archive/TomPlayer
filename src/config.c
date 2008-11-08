/***************************************************************************
 *            tomplayer_config.c
 *
 *  Sun Jan  6 14:15:55 2008
 *  Copyright  2008  nullpointer
 *  Email nullpointer[at]lavabit[dot]com
 *
 * 14.02.08 wolfgar : Add progress bar index detection, progress bar colour config and ws adaptation
 *
 *
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

/*!
 * \file config.c
 * \brief main configuration loading
 * \author nullpointer
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <dictionary.h>
#include <iniparser.h>

#include "debug.h"
#include "config.h"
#include "zip_skin.h"
#include "engine.h"

#include "widescreen.h"

/**
 * \def SCREEN_SAVER_TO_S
 * \brief Timeout in seconds before turning OFF screen while playing audio
 */
#define SCREEN_SAVER_TO_S 6

/**
 * \def DEFAULT_FOLDER
 * \brief default folder is the specified one doesn't exist
 */
#define DEFAULT_FOLDER "/mnt"

struct tomplayer_config config;
/**
 * \fn static void display_config( struct tomplayer_config * conf )
 * \brief Show configuration parameters
 *
 * \param conf main configuration structure
 */
static void display_config( struct tomplayer_config * conf ){
	PRINTDF( "   - bitmap_loading_filename : %s\n", conf->bitmap_loading_filename );
    PRINTDF( "   - bitmap_exiting_filename : %s\n", conf->bitmap_exiting_filename );
	PRINTDF( "   - filter_video_ext : %s\n", conf->filter_video_ext );
    PRINTDF( "   - filter_audio_ext : %s\n", conf->filter_audio_ext );
    PRINTDF( "   - video_folder : %s\n", conf->video_folder );
    PRINTDF( "   - audio_folder : %s\n", conf->audio_folder );
    PRINTDF( "   - video_skin_filename : %s\n", conf->video_skin_filename );
    PRINTDF( "   - audio_skin_filename : %s\n", conf->audio_skin_filename );
    PRINTDF( "   - screen_saver_to : %d\n", conf->screen_saver_to );
}

/**
 * \fn void reset_skin_conf (struct skin_config * conf)
 * \brief reset all value of the skin configuration structure
 *
 * \param conf skin configuration
 */
void reset_skin_conf (struct skin_config * conf){
	int i;
	/* FIXME Rajouter lib�ration dynamique */
	memset(conf, 0, sizeof(*conf));
	for( i = 0; i < SKIN_CMD_MAX_NB; i++ ){
	  conf->cmd2idx[i] = -1;
	}
}

/**
 * \fn void reset_skin_conf (struct skin_config * conf)
 * \brief load a skin configuration
 *
 * \param filename fullpath to the skin configuration file
 * \param conf skin configuration structure
 *
 * \return true on succes, false on failure
 */
bool load_skin_config( char * filename, struct skin_config * skin_conf ){
	dictionary * ini ;
    char section_control[PATH_MAX + 1];
    int i,j;
    char * s;
    bool ret = true;


    reset_skin_conf(skin_conf);
    skin_conf->progress_bar_index = -1;

	ini = iniparser_load(filename);
	if (ini == NULL) {
		PRINTDF( "Unable to load config file %s\n", filename);
		return false ;
	}

	i = iniparser_getint(ini, SECTION_GENERAL":"KEY_TEXT_COLOR, 0xFF0000);
        if( i < 0  ){
      	 PRINTD("No text color\n");
        } else{
	 skin_conf->text_color = i;
    	 PRINTDF("Read txt color : 0x%x  \n",skin_conf->text_color);
        }

	skin_conf->text_x1 = iniparser_getint(ini, SECTION_GENERAL":"KEY_TEXT_X1, 0);
	skin_conf->text_x2 = iniparser_getint(ini, SECTION_GENERAL":"KEY_TEXT_X2, 0);
	skin_conf->text_y1 = iniparser_getint(ini, SECTION_GENERAL":"KEY_TEXT_Y1, 0);
	skin_conf->text_y2 = iniparser_getint(ini, SECTION_GENERAL":"KEY_TEXT_Y2, 0);

	skin_conf->r = iniparser_getint(ini, SECTION_GENERAL":"KEY_R_TRANSPARENCY, 0);
	skin_conf->g = iniparser_getint(ini, SECTION_GENERAL":"KEY_G_TRANSPARENCY, 0);
	skin_conf->b = iniparser_getint(ini, SECTION_GENERAL":"KEY_B_TRANSPARENCY, 0);

	skin_conf->pb_r = iniparser_getint(ini, SECTION_GENERAL":"KEY_R_PROGRESSBAR, 0);
	skin_conf->pb_g = iniparser_getint(ini, SECTION_GENERAL":"KEY_G_PROGRESSBAR, 0);
	skin_conf->pb_b = iniparser_getint(ini, SECTION_GENERAL":"KEY_B_PROGRESSBAR, 0);

	s = iniparser_getstring(ini, SECTION_GENERAL":"KEY_SKIN_BMP, NULL);
	if( s != NULL ) strcpy( skin_conf->bitmap_filename, s ); // FIXME replace by a strdup


    for( i = 0; i < MAX_SKIN_CONTROLS; i++ ){
		sprintf( section_control, SECTION_CONTROL_FMT_STR, i, KEY_TYPE_CONTROL );
		j = iniparser_getint(ini, section_control, -1);
        if( j < 0  ){
           PRINTDF( "Warning : no section  <%s>\n", section_control );
           break;
        }
		else skin_conf->controls[i].type = j;

        /* Read filename to load
         *
         * Wolf  : suppress dynamic allocation is not such a good idea : it wastes about 80Ko
         * But now,  with mulitple config during the same session, a conf cleanup is needed (in unload_skin ?  Todo later)...
         */
		sprintf( section_control, SECTION_CONTROL_FMT_STR, i, KEY_CTRL_BITMAP_FILENAME );
		s = iniparser_getstring(ini, section_control, NULL);
		if( s != NULL ) strcpy( skin_conf->controls[i].bitmap_filename, s );


		sprintf( section_control, SECTION_CONTROL_FMT_STR, i, KEY_CMD_CONTROL );
		skin_conf->controls[i].cmd = iniparser_getint(ini, section_control, -1);
        if( skin_conf->controls[i].cmd < 0  ){
			sprintf( section_control, SECTION_CONTROL_FMT_STR, i, KEY_CMD_CONTROL2 );
			skin_conf->controls[i].cmd = iniparser_getint(ini, section_control, -1);
		}

        if (skin_conf->controls[i].cmd == SKIN_CMD_BATTERY_STATUS){
        	PRINTDF("Battery conf index :%i - filename : %s\n",i,skin_conf->controls[i].bitmap_filename);
        	skin_conf->bat_index = i;
        }

        /* Fill table cmd -> skin index */
        if ((skin_conf->controls[i].cmd >= 0) &&
        	(skin_conf->controls[i].cmd < SKIN_CMD_MAX_NB)){
        	skin_conf->cmd2idx[skin_conf->controls[i].cmd] = i;
        }

        switch( skin_conf->controls[i].type ){
            case CIRCULAR_SKIN_CONTROL:
				sprintf( section_control, SECTION_CONTROL_FMT_STR, i, KEY_CIRCULAR_CONTROL_X );
				skin_conf->controls[i].area.circular.x= iniparser_getint(ini, section_control, -1);
				sprintf( section_control, SECTION_CONTROL_FMT_STR, i, KEY_CIRCULAR_CONTROL_Y );
				skin_conf->controls[i].area.circular.y = iniparser_getint(ini, section_control, -1);
				sprintf( section_control, SECTION_CONTROL_FMT_STR, i, KEY_CIRCULAR_CONTROL_R );
				skin_conf->controls[i].area.circular.r = iniparser_getint(ini, section_control, -1);
                break;
            case RECTANGULAR_SKIN_CONTROL:
            case PROGRESS_SKIN_CONTROL_X:
            case PROGRESS_SKIN_CONTROL_Y:
				sprintf( section_control, SECTION_CONTROL_FMT_STR, i, KEY_RECTANGULAR_CONTROL_X1 );
				skin_conf->controls[i].area.rectangular.x1= iniparser_getint(ini, section_control, -1);
				sprintf( section_control, SECTION_CONTROL_FMT_STR, i, KEY_RECTANGULAR_CONTROL_X2 );
				skin_conf->controls[i].area.rectangular.x2 = iniparser_getint(ini, section_control, -1);
				sprintf( section_control, SECTION_CONTROL_FMT_STR, i, KEY_RECTANGULAR_CONTROL_Y1 );
				skin_conf->controls[i].area.rectangular.y1 = iniparser_getint(ini, section_control, -1);
				sprintf( section_control, SECTION_CONTROL_FMT_STR, i, KEY_RECTANGULAR_CONTROL_Y2 );
				skin_conf->controls[i].area.rectangular.y2 = iniparser_getint(ini, section_control, -1);

				if ((skin_conf->controls[i].type == PROGRESS_SKIN_CONTROL_X) ||
					(skin_conf->controls[i].type == PROGRESS_SKIN_CONTROL_Y)){
					skin_conf->progress_bar_index = i;
				}
                break;
            default:
                fprintf( stderr, "Type not defined correctly for %s\n", section_control );
                ret = false;
                goto error;
        }
    }

    skin_conf->nb = i;

error:
    iniparser_freedict(ini);
    return ret;
}

/**
 * \fn bool load_config( struct tomplayer_config * conf )
 * \brief load the main configuration and skin configuration
 *
 * \param conf main configuration structure
 *
 * \return true on succes, false on failure
 */
bool load_config( struct tomplayer_config * conf ){
	dictionary * ini ;
    struct stat folder_stats;
	char *s;

	PRINTD( "Loading main configuration\n");

    memset( conf, 0, sizeof( struct tomplayer_config ) );

    ini = iniparser_load(CONFIG_FILE);
	if (ini == NULL) {
		PRINTDF( "Unable to load main configuration file <%s>\n", CONFIG_FILE );
		return false ;
	}

	s = iniparser_getstring(ini, SECTION_GENERAL":"KEY_LOADING_BMP, NULL);
	PRINTDF( "%s\n",s);
	if( s != NULL ) strcpy( conf->bitmap_loading_filename, s );

	s = iniparser_getstring(ini, SECTION_GENERAL":"KEY_EXITING_BMP, NULL);
	if( s != NULL ) strcpy( conf->bitmap_exiting_filename, s );

	s = iniparser_getstring(ini, SECTION_GENERAL":"KEY_VIDEO_FILE_DIRECTORY, NULL);
	if( s != NULL ) strcpy( conf->video_folder, s );
    if (stat(conf->video_folder, &folder_stats) != 0){
    	strcpy(conf->video_folder,DEFAULT_FOLDER);
    }

	s = iniparser_getstring(ini, SECTION_GENERAL":"KEY_AUDIO_FILE_DIRECTORY, NULL);
	if( s != NULL ) strcpy( conf->audio_folder, s );
    if (stat(conf->audio_folder, &folder_stats) != 0){
    	strcpy(conf->audio_folder,DEFAULT_FOLDER);
    }


	s = iniparser_getstring(ini, SECTION_GENERAL":"KEY_FILTER_VIDEO_EXT, NULL);
	if( s != NULL ) strcpy( conf->filter_video_ext, s );
	s = iniparser_getstring(ini, SECTION_GENERAL":"KEY_FILTER_AUDIO_EXT, NULL);
	if( s != NULL ) strcpy( conf->filter_audio_ext, s );

	conf->screen_saver_to = iniparser_getint(ini, SECTION_GENERAL":"KEY_SCREEN_SAVER_TO, -1);
    if (conf->screen_saver_to < 0 ){
    	conf->screen_saver_to = SCREEN_SAVER_TO_S;
    }

    s = iniparser_getstring(ini, SECTION_VIDEO_SKIN":"KEY_SKIN_FILENAME, NULL);
	if( s != NULL ) strcpy( conf->video_skin_filename, s );

	s = iniparser_getstring(ini, SECTION_AUDIO_SKIN":"KEY_SKIN_FILENAME, NULL);
	if( s != NULL ) strcpy(conf->audio_skin_filename, s );

    iniparser_freedict(ini);

	display_config( conf );
    if( ws_probe() ) ws_translate( conf );

/* Not used any longer
    load_bitmap( &conf->bitmap_loading, conf->bitmap_loading_filename );
    load_bitmap( &conf->bitmap_exiting, conf->bitmap_exiting_filename );
*/
  


    return true;
}

