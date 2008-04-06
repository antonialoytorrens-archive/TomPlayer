#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <zip.h>
#include "zip_skin.h"
#include "widescreen.h"

#define CONF_FILENAME		"/tmp/skin.conf"
#define BITMAP_FILENAME		"/tmp/bitmap"
#define SKIN_CONFIG_NAME	"skin.conf"
#define WS_SKIN_CONFIG_NAME	"ws_skin.conf"

int load_bitmap( ILuint * bitmap_obj, char * filename ){
    ilGenImages(1, bitmap_obj);	

    ilBindImage(*bitmap_obj);
    if (!ilLoadImage(filename)) {
        fprintf(stderr, "Could not load image file %s.\n", filename);
        return FALSE;
    }	
		
    ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
		
    return TRUE;
}

static void expand_config( struct skin_config * conf ){
	int i;

	EXTEND_X(conf->text_x1);
	EXTEND_X(conf->text_x2);
	EXTEND_Y(conf->text_y1);
	EXTEND_Y(conf->text_y1);
	
	for( i = 0; i < conf->nb; i++){
		switch( conf->controls[i].type ){
			case CIRCULAR_CONTROL:
				EXTEND_X(conf->controls[i].area.circular.x);
				EXTEND_Y(conf->controls[i].area.circular.y);
				EXTEND_Y(conf->controls[i].area.circular.r);
				break;
			case RECTANGULAR_CONTROL:
			case PROGRESS_CONTROL_X:
			case PROGRESS_CONTROL_Y:
				EXTEND_X(conf->controls[i].area.rectangular.x1);
				EXTEND_X(conf->controls[i].area.rectangular.x2);
				EXTEND_Y(conf->controls[i].area.rectangular.y1);
				EXTEND_Y(conf->controls[i].area.rectangular.y1);
				break;
		}
	}
}

int unzip_file( struct zip * fp_zip, char * filename_in, char * filename_out ){
	struct zip_file * fp_zip_file;
	FILE * fp;
	unsigned char data[1000];
	int len;

	fp = fopen( filename_out, "wb" ); 
	if( fp == NULL ){
	    fprintf( stderr, "Unable to create file <%s>\n" , filename_out );
	    return FALSE;
	}

	fp_zip_file = zip_fopen( fp_zip, filename_in, 0 );

	if( fp_zip_file == NULL ){
	    fprintf( stderr, "Unable to find <%s> in archive\n" , filename_in );
		fclose( fp );
		return FALSE;
	}

	
	while( (len = zip_fread( fp_zip_file, data, sizeof( data ) ) ) )
		fwrite( data, sizeof( unsigned char ), len, fp );

	zip_fclose( fp_zip_file );
	fclose( fp );

	return TRUE;
}

int load_skin_from_zip( char * filename, struct skin_config * skin_conf ){
	int ws;
	int error;
	int expand_conf = FALSE;
	struct zip * fp_zip;
	int return_code = FALSE;
	int i;
	char cmd[200];

	ws = ws_probe();

    error = 0;
	fp_zip = zip_open( filename, ZIP_CHECKCONS, &error );

	if( error != 0 || fp_zip == NULL ){
	    fprintf( stderr, "Unable to load zip file <%s> (%d)\n" , filename, error );
	    return FALSE;
	}

	/* Loading of config file */
	if( ws ){
		if( unzip_file( fp_zip, WS_SKIN_CONFIG_NAME, CONF_FILENAME ) == FALSE ){
			fprintf( stderr, "No widescreen config in zip file <%s>\n", filename );
			if( unzip_file( fp_zip, SKIN_CONFIG_NAME, CONF_FILENAME ) == FALSE ){
				fprintf( stderr, "Error while unzipping <%s>\n", SKIN_CONFIG_NAME );
				goto error;
			}
			expand_conf = TRUE;
		}
	}
	else{
		if( unzip_file( fp_zip, SKIN_CONFIG_NAME, CONF_FILENAME ) == FALSE ){
			fprintf( stderr, "Error while unzipping <%s>\n", SKIN_CONFIG_NAME );
			goto error;
		}
	}

    sprintf( cmd, "dos2unix %s", CONF_FILENAME );
    system( cmd );
	if( load_skin_config( CONF_FILENAME, skin_conf ) == FALSE ){
		fprintf( stderr, "Error while loading config file <%s>\n", CONF_FILENAME );
		goto error;
	}

	if( expand_conf == TRUE ) expand_config( skin_conf );

	/* Loading of bitmap file */
	if( unzip_file( fp_zip, skin_conf->bitmap_filename, BITMAP_FILENAME ) == FALSE ){
		fprintf( stderr, "Error while unzipping <%s>\n", skin_conf->bitmap_filename );
		goto error;
	}
	
	/* Loading of different bitmap of the skin */
	load_bitmap( &skin_conf->bitmap, BITMAP_FILENAME );
	for( i = 0; i < skin_conf->nb; i++ )
	    if( strlen( skin_conf->controls[i].bitmap_filename ) ){
	        if( unzip_file( fp_zip, skin_conf->controls[i].bitmap_filename, BITMAP_FILENAME ) == FALSE ){
    			fprintf( stderr, "Error while unzipping <%s>\n", skin_conf->controls[i].bitmap_filename );
    			goto error;
    		}
	        load_bitmap( &skin_conf->controls[i].bitmap, BITMAP_FILENAME );
	    }

	if( expand_conf == TRUE ){
        ilBindImage( skin_conf->bitmap );
        iluScale( WS_XMAX,WS_YMAX,1);
    	for( i = 0; i < skin_conf->nb; i++ )
    	    if( skin_conf->controls[i].bitmap != 0 ){
    	        ilBindImage( skin_conf->controls[i].bitmap );
    	        iluScale( ilGetInteger(IL_IMAGE_WIDTH) * (1.0*WS_XMAX/WS_NOXL_XMAX),ilGetInteger(IL_IMAGE_HEIGHT) * (1.0*WS_YMAX/WS_NOXL_YMAX) ,1);
    	    }
	}
	
	return_code = TRUE;

error:
    unlink( CONF_FILENAME );
    unlink( BITMAP_FILENAME );
	zip_close( fp_zip );

	return return_code;
}	

	
int unload_skin(  struct skin_config * skin_conf ){
    int i;
	/* Unload different bitmap of the skin */
	if( skin_conf->bitmap ) ilDeleteImages(1, &skin_conf->bitmap);
	for( i = 0; i < skin_conf->nb; i++ )
	    if( skin_conf->controls[i].bitmap ) ilDeleteImages(1, &skin_conf->controls[i].bitmap);

    memset( skin_conf, 0, sizeof( struct skin_config ) );
    
    return TRUE;   
}
