
/**
 * \file zip_skin.c
 * \author nullpointer
 * \brief handling of zipped skin
 *
 * $URL$
 * $Rev$
 * $Author:$
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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <zip.h>
#include "zip_skin.h"
#include "widescreen.h"
#include "engine.h"
#include "debug.h"


#define CONF_FILENAME		"/tmp/skin.conf"
#define BITMAP_FILENAME		"/tmp/bitmap"
#define SKIN_CONFIG_NAME	"skin.conf"
#define WS_SKIN_CONFIG_NAME	"ws_skin.conf"



/** Load image in a DevIL bitmap
 *
 * \param bitmap_obj DevIL bitmap object
 * \param filename of the image
 *
 * \return true on success, false on failure
 */
bool load_bitmap( ILuint * bitmap_obj, char * filename ){
    ilGenImages(1, bitmap_obj);
    ilBindImage(*bitmap_obj);
    if (!ilLoadImage(filename)) {
        fprintf(stderr, "Could not load image file %s.\nError : %s\n", filename, iluErrorString(ilGetError()));
        return false;
    }
    else{
    	PRINTDF("Loading bitmap <%s>\n", filename);
    }

    ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
    return true;
}


/** Expand configuration to fit in a widescreen
 *
 * \param conf configuration to be expanded
 */
static void expand_config( struct skin_config * conf ){
	int i;

	EXPAND_X(conf->text_x1);
	EXPAND_X(conf->text_x2);
	EXPAND_Y(conf->text_y1);
	EXPAND_Y(conf->text_y2);

	for( i = 0; i < conf->nb; i++){
		switch( conf->controls[i].type ){
			case CIRCULAR_SKIN_CONTROL:
				EXPAND_X(conf->controls[i].area.circular.x);
				EXPAND_Y(conf->controls[i].area.circular.y);
				EXPAND_Y(conf->controls[i].area.circular.r);
				break;
			case RECTANGULAR_SKIN_CONTROL:
			case PROGRESS_SKIN_CONTROL_X:
			case PROGRESS_SKIN_CONTROL_Y:
				EXPAND_X(conf->controls[i].area.rectangular.x1);
				EXPAND_X(conf->controls[i].area.rectangular.x2);
				EXPAND_Y(conf->controls[i].area.rectangular.y1);
				EXPAND_Y(conf->controls[i].area.rectangular.y2);
				break;
		}
	}
}

/** Unzip a file of an archive
 *
 * \param fp_zip handle to the opened zip file
 * \param filename_in filename in the archive
 * \param filename_out unzipped file
 *
 * \return true on succes, false on failure
 */
static bool unzip_file( struct zip * fp_zip, char * filename_in, char * filename_out ){
	struct zip_file * fp_zip_file;
	FILE * fp;
	unsigned char data[1000];
	int len;

	fp = fopen( filename_out, "wb" );
	if( fp == NULL ){
	    fprintf( stderr, "Unable to create file <%s>\n" , filename_out );
	    return false;
	}

	fp_zip_file = zip_fopen( fp_zip, filename_in, 0 );

	if( fp_zip_file == NULL ){
	    fprintf( stderr, "Unable to find <%s> in archive\n" , filename_in );
		fclose( fp );
		return false;
	}


	while( (len = zip_fread( fp_zip_file, data, sizeof( data ) ) ) )
		fwrite( data, sizeof( unsigned char ), len, fp );

	zip_fclose( fp_zip_file );
	fclose( fp );

	return true;
}

/** Load a zipped skin
 *
 * \param filename fullpath to the archive filename
 * \param skin_conf skin configuration structure
 *
 * \return true on succes, false on failure
 */
bool load_skin_from_zip( char * filename, struct skin_config * skin_conf ){
	int ws;
	int error;
	int expand_conf = false;
	struct zip * fp_zip;
	int return_code = false;
	int i, frame_id;
	int new_w, new_h;
	char cmd[200];

	ws = ws_probe();

    error = 0;
	fp_zip = zip_open( filename, ZIP_CHECKCONS, &error );

	if( error != 0 || fp_zip == NULL ){
	    fprintf( stderr, "Unable to load zip file <%s> (%d)\n" , filename, error );
	    return false;
	}

	/* Loading of config file */
	if( ws ){
		if( unzip_file( fp_zip, WS_SKIN_CONFIG_NAME, CONF_FILENAME ) == false ){
			fprintf( stderr, "No widescreen config in zip file <%s>\n", filename );
			if( unzip_file( fp_zip, SKIN_CONFIG_NAME, CONF_FILENAME ) == false ){
				fprintf( stderr, "Error while unzipping <%s>\n", SKIN_CONFIG_NAME );
				goto error;
			}
			expand_conf = true;
		}
	}
	else{
		if( unzip_file( fp_zip, SKIN_CONFIG_NAME, CONF_FILENAME ) == false ){
			fprintf( stderr, "Error while unzipping <%s>\n", SKIN_CONFIG_NAME );
			goto error;
		}
	}

    sprintf( cmd, "dos2unix %s", CONF_FILENAME );
    system( cmd );
	if( load_skin_config( CONF_FILENAME, skin_conf ) == false ){
		fprintf( stderr, "Error while loading config file <%s>\n", CONF_FILENAME );
		goto error;
	}

	if( expand_conf == true ) expand_config( skin_conf );

	/* Loading of bitmap file */
	if( unzip_file( fp_zip, skin_conf->bitmap_filename, BITMAP_FILENAME ) == false ){
		fprintf( stderr, "Error while unzipping <%s>\n", skin_conf->bitmap_filename );
		goto error;
	}

	/* Loading of different bitmap of the skin */
	load_bitmap( &skin_conf->bitmap, BITMAP_FILENAME );
	for( i = 0; i < skin_conf->nb; i++ ){
	    if( strlen( skin_conf->controls[i].bitmap_filename ) ){
	        if( unzip_file( fp_zip, skin_conf->controls[i].bitmap_filename, BITMAP_FILENAME ) == false ){
    			fprintf( stderr, "Error while unzipping <%s>\n", skin_conf->controls[i].bitmap_filename );
    			goto error;
    		}
	        load_bitmap( &skin_conf->controls[i].bitmap, BITMAP_FILENAME );
	        PRINTDF("Loading %s - Image id :%i\n",skin_conf->controls[i].bitmap_filename,skin_conf->controls[i].bitmap);
	    } else{
	    	skin_conf->controls[i].bitmap = 0;
	    }
	}

	if( expand_conf == true ){
        ilBindImage( skin_conf->bitmap );
        iluScale( WS_XMAX,WS_YMAX,1);

    	for( i = 0; i < skin_conf->nb; i++ ){
    	    if( skin_conf->controls[i].bitmap != 0 ){
    	        ilBindImage( skin_conf->controls[i].bitmap );
    	        frame_id = 0;
    	        new_w = ((int)(ilGetInteger(IL_IMAGE_WIDTH) *(1.0 * WS_XMAX ) / WS_NOXL_XMAX));
    	        new_h = ((int)(ilGetInteger(IL_IMAGE_HEIGHT)*(1.0 * WS_YMAX ) / WS_NOXL_YMAX));
    	        PRINTDF("%i - new w : %i new h : %i - num im :%i - num mipmaps : %i \n ", i, new_w,new_h, ilGetInteger(IL_NUM_IMAGES),ilGetInteger(IL_NUM_MIPMAPS));
    	        
    	        /*while (1){
    	        	ilBindImage( skin_conf->controls[i].bitmap );
    	        	if ( !ilActiveImage(frame_id)){
    	        		ilBindImage( skin_conf->controls[i].bitmap );
    	        		break;
    	        	}
    	        	PRINTD("frame id : %i\n", frame_id);
    	        	iluScale(new_w,new_h,1);
    	        	frame_id++;
    	       }    */
    	        if  (skin_conf->controls[i].cmd != SKIN_CMD_BATTERY_STATUS){
    	        	/* Scale animation seems to crash so exclude battery icons */
    	        	iluScale(new_w,new_h,1);
    	        }
    	    }
    	}
	}

	return_code = true;

error:
    unlink( CONF_FILENAME );
    unlink( BITMAP_FILENAME );
	zip_close( fp_zip );

	return return_code;
}

/** Release a skin configuration
 *
 * \param skin_conf skin configuration structure
 *
 * \return true on succes, false on failure
 */
bool unload_skin(  struct skin_config * skin_conf ){
    int i;

	/* Unload different bitmap of the skin */
	if( skin_conf->bitmap ) ilDeleteImages(1, &skin_conf->bitmap);
	for( i = 0; i < skin_conf->nb; i++ )
	    if( skin_conf->controls[i].bitmap ) ilDeleteImages(1, &skin_conf->controls[i].bitmap);

    memset( skin_conf, 0, sizeof( struct skin_config ) );

    return true;
}
