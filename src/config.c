/***************************************************************************
 *            tomplayer_config.c
 *
 *  Sun Jan  6 14:15:55 2008
 *  Copyright  2008  nullpointer
 *  Email
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
 
#include <string.h>
#include "config.h"


#ifdef USE_MINIMIG

#include <minigui/common.h>
#include <minigui/minigui.h>
#include "widescreen.h"


int load_skin_config( char * filename, struct skin_config * skin_conf ){
    GHANDLE gh_config;
    char section_control[PATH_MAX + 1];
    int i;

    skin_conf->progress_bar_index = -1;
    gh_config = LoadEtcFile( filename );
    
    if( gh_config == ETC_FILENOTFOUND ){
        fprintf( stderr, "Unable to load file <%s>\n", filename );
        return FALSE;
    }
    
    /* GetValueFromEtc( gh_config, SECTION_GENERAL, KEY_SKIN_BMP,  skin_conf->image_file, PATH_MAX ); */
    
    GetIntValueFromEtc( gh_config, SECTION_GENERAL, KEY_TEXT_X1, &(skin_conf->text_x1) );
    GetIntValueFromEtc( gh_config, SECTION_GENERAL, KEY_TEXT_X2, &(skin_conf->text_x2) );
    GetIntValueFromEtc( gh_config, SECTION_GENERAL, KEY_TEXT_Y1, &(skin_conf->text_y1) );
    GetIntValueFromEtc( gh_config, SECTION_GENERAL, KEY_TEXT_Y2, &(skin_conf->text_y2) );
    
    GetIntValueFromEtc( gh_config, SECTION_GENERAL, KEY_R_TRANSPARENCY, &(skin_conf->r) );
    GetIntValueFromEtc( gh_config, SECTION_GENERAL, KEY_G_TRANSPARENCY, &(skin_conf->g) );
    GetIntValueFromEtc( gh_config, SECTION_GENERAL, KEY_B_TRANSPARENCY, &(skin_conf->b) );

    GetIntValueFromEtc( gh_config, SECTION_GENERAL, KEY_R_PROGRESSBAR, &(skin_conf->pb_r) );
    GetIntValueFromEtc( gh_config, SECTION_GENERAL, KEY_G_PROGRESSBAR, &(skin_conf->pb_g) );
    GetIntValueFromEtc( gh_config, SECTION_GENERAL, KEY_B_PROGRESSBAR, &(skin_conf->pb_b) );

    
    for( i = 0; i < MAX_CONTROLS; i++ ){
        sprintf( section_control, SECTION_CONTROL_FMT_STR, i );
        if( GetIntValueFromEtc( gh_config, section_control, KEY_TYPE_CONTROL, (int *) &(skin_conf->controls[i].type) ) != ETC_OK  ){
           fprintf( stderr, "Warning : no section  <%s>\n", section_control );
           break;
        }
        GetIntValueFromEtc( gh_config, section_control, KEY_CMD_CONTROL, &skin_conf->controls[i].cmd );       
        switch( skin_conf->controls[i].type ){
            case CIRCULAR_CONTROL:
                GetIntValueFromEtc( gh_config, section_control, KEY_CIRCULAR_CONTROL_X, &skin_conf->controls[i].area.circular.x );
                GetIntValueFromEtc( gh_config, section_control, KEY_CIRCULAR_CONTROL_Y, &skin_conf->controls[i].area.circular.y );
                GetIntValueFromEtc( gh_config, section_control, KEY_CIRCULAR_CONTROL_R, &skin_conf->controls[i].area.circular.r );
                break;
            case RECTANGULAR_CONTROL:
            case PROGRESS_CONTROL_X:
            case PROGRESS_CONTROL_Y:
                GetIntValueFromEtc( gh_config, section_control, KEY_RECTANGULAR_CONTROL_X1, &skin_conf->controls[i].area.rectangular.x1 );
                GetIntValueFromEtc( gh_config, section_control, KEY_RECTANGULAR_CONTROL_X2, &skin_conf->controls[i].area.rectangular.x2 );
                GetIntValueFromEtc( gh_config, section_control, KEY_RECTANGULAR_CONTROL_Y1, &skin_conf->controls[i].area.rectangular.y1 );
                GetIntValueFromEtc( gh_config, section_control, KEY_RECTANGULAR_CONTROL_Y2, &skin_conf->controls[i].area.rectangular.y2 );

		if ((skin_conf->controls[i].type == PROGRESS_CONTROL_X) ||
		    (skin_conf->controls[i].type == PROGRESS_CONTROL_Y)){
		  skin_conf->progress_bar_index = i;		
                }
                break;
            default:
                fprintf( stderr, "Type not defined correctly for %s\n", section_control );
                return FALSE;
        }
    }
    
    skin_conf->nb = i;
    
    UnloadEtcFile( gh_config );

    return TRUE;
}

int load_config( struct tomplayer_config * conf ){
    GHANDLE gh_config;
    
    memset( conf, 0, sizeof( struct tomplayer_config ) );
        
    gh_config = LoadEtcFile( CONFIG_FILE );
    
    if( gh_config == ETC_FILENOTFOUND ){
        fprintf(stderr, "Unable to load main configuration file <%s>\n", CONFIG_FILE );
        return FALSE;
    }
    
    GetValueFromEtc( gh_config, SECTION_GENERAL, KEY_LOADING_BMP,  conf->bmp_loading_file, PATH_MAX );
    GetValueFromEtc( gh_config, SECTION_GENERAL, KEY_EXITING_BMP,  conf->bmp_exiting_file, PATH_MAX );
    GetValueFromEtc( gh_config, SECTION_GENERAL, KEY_FILE_DIRECTORY,  conf->folder, PATH_MAX );
    /*GetValueFromEtc( gh_config, SECTION_GENERAL, KEY_AUDIO_FILE_DIRECTORY,conf->audio_folder, PATH_MAX);*/
    GetValueFromEtc( gh_config, SECTION_GENERAL, KEY_FILTER_VIDEO_EXT,  conf->filter_video_ext, PATH_MAX );
    GetValueFromEtc( gh_config, SECTION_GENERAL, KEY_FILTER_AUDIO_EXT,  conf->filter_audio_ext, PATH_MAX );

    GetValueFromEtc( gh_config, SECTION_VIDEO_SKIN, KEY_SKIN_BMP,  conf->video_config.image_file, PATH_MAX );
    GetValueFromEtc( gh_config, SECTION_VIDEO_SKIN, KEY_SKIN_CONF,  conf->video_config.conf_file, PATH_MAX );

    GetValueFromEtc( gh_config, SECTION_AUDIO_SKIN, KEY_SKIN_BMP,  conf->audio_config.image_file, PATH_MAX );
    GetValueFromEtc( gh_config, SECTION_AUDIO_SKIN, KEY_SKIN_CONF,  conf->audio_config.conf_file, PATH_MAX );
    
    UnloadEtcFile( gh_config );
    
    /* Configuration adpatation for widescreen */
    if (ws_probe()){
	ws_translate(conf);	
    }


    load_skin_config( conf->video_config.conf_file, &conf->video_config );
    load_skin_config( conf->audio_config.conf_file, &conf->audio_config );
    

    return TRUE;
}

#endif /* USE_MINIMIG */
