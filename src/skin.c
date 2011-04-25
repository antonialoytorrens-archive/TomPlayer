/**
 * \file skin.c 
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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <zip.h>
#include <math.h>
#include <iniparser.h>

#include "widescreen.h"
#include "debug.h"
#include "skin.h"

#define CONF_FILENAME "/tmp/skin.conf"
#define SKIN_CONFIG_NAME "skin.conf"
#define WS_SKIN_CONFIG_NAME "ws_skin.conf"

/* Definition of section in the skin config file */
#define SECTION_GENERAL             "general"
#define SECTION_CONTROL_FMT_STR     "CONTROL_%d:%s"

/* Definition of keywords used in skin configuration file */
#define KEY_TYPE_CONTROL            "type"
#define KEY_CMD_CONTROL             "ctrl"
#define KEY_CMD_CONTROL2            "cmd"
#define KEY_CTRL_BITMAP_FILENAME    "bitmap"
#define KEY_SKIN_BMP                "image"
#define KEY_TEXT_COLOR              "text_color"
#define KEY_TEXT_X1                 "text_x1"
#define KEY_TEXT_Y1                 "text_y1"
#define KEY_TEXT_X2                 "text_x2"
#define KEY_TEXT_Y2                 "text_y2"
#define KEY_R_TRANSPARENCY          "r"
#define KEY_G_TRANSPARENCY          "g"
#define KEY_B_TRANSPARENCY          "b"
#define KEY_R_PROGRESSBAR           "pb_r"
#define KEY_G_PROGRESSBAR           "pb_g"
#define KEY_B_PROGRESSBAR           "pb_b"
#define KEY_CIRCULAR_CONTROL_X      "x"
#define KEY_CIRCULAR_CONTROL_Y      "y"
#define KEY_CIRCULAR_CONTROL_R      "r"
#define KEY_RECTANGULAR_CONTROL_X1  "x1"
#define KEY_RECTANGULAR_CONTROL_X2  "x2"
#define KEY_RECTANGULAR_CONTROL_Y1  "y1"
#define KEY_RECTANGULAR_CONTROL_Y2  "y2"
#define KEY_SEL_ORDER               "selection_order"
#define KEY_SEL_FIRST               "first_selection"
#define KEY_DISP_FILENAME           "display_filename"
#define KEY_TEXT_CONTROL_X          "x"
#define KEY_TEXT_CONTROL_Y          "y"
#define KEY_TEXT_CONTROL_COLOR      "color"
#define KEY_TEXT_CONTROL_SIZE       "size"

/* Current skin configuration */
static struct{
    struct skin_config config;      /*!< skin configuration */
    int cmd2idx[SKIN_CMD_MAX_NB];   /*!< Function --> ctrl index table */
    ILuint bitmap;                  /*!< DevIL background bitmap */
    int progress_bar_index;         /*!< index of progress bar object in controls table*/    
    ILuint bitmaps[MAX_SKIN_CONTROLS]; /*!< DevIL imgs associated to the controls */
} current_skin;

static const char * cmd_labels[SKIN_CMD_MAX_NB] = { "EXIT      ",
                                                    "PAUSE     ",
                                                    "STOP      ",
                                                    "MUTE      ",
                                                    "VOL -     ",
                                                    "VOL +     ",
                                                    "BRIGHT -  ",
                                                    "BRIGHT +  ",
                                                    "DELAY -   ",
                                                    "DELAY +   ",
                                                    "CONTRAST -",
                                                    "CONTRAST +",
                                                    "FORWARD   ",
                                                    "BACKWARD  ",
                                                    "NEXT      ",
                                                    "PREVIOUS  ",
                                                    "          ",
                                                    "          "
                                                  };
                                                
static int control_compare(const struct skin_control *c1, const struct skin_control * c2){
    struct skin_rectangular_shape zone1, zone2;
    
    zone1 = skin_ctrl_get_zone(c1);
    zone2 = skin_ctrl_get_zone(c2);    
    if ((zone1.y1  <= zone2.y2) &&
        (zone2.y1  <= zone1.y2)){
        /* If the two controls intersect horizontally the lefter is the lesser*/        
        return (zone1.x1 - zone2.x1);
    } else {
        /* Otherwise the upper the lesser */
        return (zone1.y1 - zone2.y1);        
    }
}

static void control_sort(struct skin_control *array, int length)  { 
    int i, j;  
    struct skin_control temp;
    int test; 

    for(i = length - 1; i > 0; i--){  
        test=0;  
        for(j = 0; j < i; j++){            
            if (control_compare(&array[j], &array[j+1]) > 0){
                temp = array[j];   
                array[j] = array[j+1];  
                array[j+1] = temp;  
                test=1;  
            }  
        }
        if(test==0) break; 
    }
} 

static void expand_bitmaps(const struct skin_config * skin_conf){
#ifdef WITH_DEVIL
  int i, frame_id;
  int new_w, new_h;

  ilBindImage(current_skin.bitmap);
  iluScale(WS_XMAX,WS_YMAX, 1);
  
  for( i = 0; i < skin_conf->nb; i++ ){
      if(current_skin.bitmaps[i] != 0 ){
          ilBindImage(current_skin.bitmaps[i]);
          frame_id = 0;
          new_w = ((int)(ilGetInteger(IL_IMAGE_WIDTH) *(1.0 * WS_XMAX ) / WS_NOXL_XMAX));
          new_h = ((int)(ilGetInteger(IL_IMAGE_HEIGHT)*(1.0 * WS_YMAX ) / WS_NOXL_YMAX));
          PRINTDF("%i - new w : %i new h : %i - num im :%i - num mipmaps : %i \n ", i, new_w,new_h, ilGetInteger(IL_NUM_IMAGES),ilGetInteger(IL_NUM_MIPMAPS));        
          if  (skin_conf->controls[i].cmd != SKIN_CMD_BATTERY_STATUS){
                  /* Scale animation seems to crash so exclude battery icons */
                  iluScale(new_w,new_h,1);
          }
      }
  }
#endif
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
			case SKIN_CONTROL_CIRCULAR:
				EXPAND_X(conf->controls[i].params.circ_icon.x);
				EXPAND_Y(conf->controls[i].params.circ_icon.y);
				EXPAND_Y(conf->controls[i].params.circ_icon.r);
				break;
			case SKIN_CONTROL_RECTANGULAR:
			case SKIN_CONTROL_PROGRESS_X:
			case SKIN_CONTROL_PROGRESS_Y:
				EXPAND_X(conf->controls[i].params.rect_icon.x1);
				EXPAND_X(conf->controls[i].params.rect_icon.x2);
				EXPAND_Y(conf->controls[i].params.rect_icon.y1);
				EXPAND_Y(conf->controls[i].params.rect_icon.y2);
				break;
      case SKIN_CONTROL_TEXT : 
        EXPAND_X(conf->controls[i].params.text.x);
        EXPAND_Y(conf->controls[i].params.text.y);          
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




/** Reset all value of the skin configuration structure
 *
 * \param conf skin configuration
 */
static void reset_skin_conf (void){
  int i;  
  memset(&current_skin, 0, sizeof(current_skin));  
  for( i = 0; i < SKIN_CMD_MAX_NB; i++ ){
    current_skin.cmd2idx[i] = -1;
  }
  current_skin.progress_bar_index = -1;
}

/** Load a skin configuration
 *
 * \param filename fullpath to the skin configuration file
 * \param conf skin configuration structure
 *
 * \return true on succes, false on failure
 */
static bool load_skin_config(char * filename){
    dictionary * ini ;
    char section_control[512];
    int i,j;
    char * s;
    struct skin_config * skin_conf = &current_skin.config;
    bool ret = true;

  
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
    skin_conf->selection_order = iniparser_getint(ini, SECTION_GENERAL":"KEY_SEL_ORDER, 0);
    skin_conf->first_selection = iniparser_getint(ini, SECTION_GENERAL":"KEY_SEL_FIRST, -1);
    skin_conf->display_filename = iniparser_getint(ini, SECTION_GENERAL":"KEY_DISP_FILENAME, 1);
    s = iniparser_getstring(ini, SECTION_GENERAL":"KEY_SKIN_BMP, NULL);
    if (s != NULL){
        skin_conf->bitmap_filename = strdup(s);
        if (skin_conf->bitmap_filename  == NULL){
            ret = false;
            goto error;
        }
    }
        
    
    /* Parse controls description */
    for(i = 0; i < MAX_SKIN_CONTROLS; i++){
        sprintf(section_control, SECTION_CONTROL_FMT_STR, i, KEY_TYPE_CONTROL);
        j = iniparser_getint(ini, section_control, -1);
        if (j < 0){
           PRINTDF( "Warning : no section  <%s>\n", section_control );
           break;
        } else {
            skin_conf->controls[i].type = j;
        }
        sprintf(section_control, SECTION_CONTROL_FMT_STR, i, KEY_CTRL_BITMAP_FILENAME);
        s = iniparser_getstring(ini, section_control, NULL);
        if (s != NULL){
            skin_conf->controls[i].bitmap_filename = strdup(s);
            if (skin_conf->controls[i].bitmap_filename == NULL){
                ret = false;
                goto error;
            }
        }
        sprintf( section_control, SECTION_CONTROL_FMT_STR, i, KEY_CMD_CONTROL );
        skin_conf->controls[i].cmd = iniparser_getint(ini, section_control, -1);
        if (skin_conf->controls[i].cmd < 0){
            sprintf( section_control, SECTION_CONTROL_FMT_STR, i, KEY_CMD_CONTROL2 );
            skin_conf->controls[i].cmd = iniparser_getint(ini, section_control, -1);
        }    
        switch (skin_conf->controls[i].type){
            case SKIN_CONTROL_CIRCULAR:
                sprintf( section_control, SECTION_CONTROL_FMT_STR, i, KEY_CIRCULAR_CONTROL_X );
                skin_conf->controls[i].params.circ_icon.x= iniparser_getint(ini, section_control, -1);
                sprintf( section_control, SECTION_CONTROL_FMT_STR, i, KEY_CIRCULAR_CONTROL_Y );
                skin_conf->controls[i].params.circ_icon.y = iniparser_getint(ini, section_control, -1);
                sprintf( section_control, SECTION_CONTROL_FMT_STR, i, KEY_CIRCULAR_CONTROL_R );
                skin_conf->controls[i].params.circ_icon.r = iniparser_getint(ini, section_control, -1);
                break;
            case SKIN_CONTROL_RECTANGULAR:
            case SKIN_CONTROL_PROGRESS_X:
            case SKIN_CONTROL_PROGRESS_Y:
                sprintf( section_control, SECTION_CONTROL_FMT_STR, i, KEY_RECTANGULAR_CONTROL_X1 );
                skin_conf->controls[i].params.rect_icon.x1 = iniparser_getint(ini, section_control, -1);
                sprintf( section_control, SECTION_CONTROL_FMT_STR, i, KEY_RECTANGULAR_CONTROL_X2 );
                skin_conf->controls[i].params.rect_icon.x2 = iniparser_getint(ini, section_control, -1);
                sprintf( section_control, SECTION_CONTROL_FMT_STR, i, KEY_RECTANGULAR_CONTROL_Y1 );
                skin_conf->controls[i].params.rect_icon.y1 = iniparser_getint(ini, section_control, -1);
                sprintf( section_control, SECTION_CONTROL_FMT_STR, i, KEY_RECTANGULAR_CONTROL_Y2 );
                skin_conf->controls[i].params.rect_icon.y2 = iniparser_getint(ini, section_control, -1);
                break;
            case SKIN_CONTROL_TEXT:
                sprintf( section_control, SECTION_CONTROL_FMT_STR, i, KEY_TEXT_CONTROL_X);
                skin_conf->controls[i].params.text.x = iniparser_getint(ini, section_control, -1);
                sprintf( section_control, SECTION_CONTROL_FMT_STR, i, KEY_TEXT_CONTROL_Y);
                skin_conf->controls[i].params.text.y = iniparser_getint(ini, section_control, -1);
                sprintf( section_control, SECTION_CONTROL_FMT_STR, i, KEY_TEXT_CONTROL_COLOR);
                skin_conf->controls[i].params.text.color = iniparser_getint(ini, section_control, 0xFFFFFF);
                sprintf( section_control, SECTION_CONTROL_FMT_STR, i, KEY_TEXT_CONTROL_SIZE);
                skin_conf->controls[i].params.text.size = iniparser_getint(ini, section_control, 12);                
                break;
            default:
                fprintf( stderr, "Type not defined correctly for %s\n", section_control );
                ret = false;
                goto error;
        }
    }
    
    /* Number of controls on the skin */
    skin_conf->nb = i;
    if (skin_conf->first_selection > skin_conf->nb){
        skin_conf->first_selection = -1;
    }
    
    /* Sort the controls */
    if (!skin_conf->selection_order){
        control_sort(skin_conf->controls, skin_conf->nb); 
    }
    
    /* Fill in the indexes fields */
    for (i = 0; i < skin_conf->nb; i++){
        /* Special case of progress bar for now - FIXME : generic handling - */
        if ((skin_conf->controls[i].type == SKIN_CONTROL_PROGRESS_X) ||
            (skin_conf->controls[i].type == SKIN_CONTROL_PROGRESS_Y)){
            current_skin.progress_bar_index = i;
        }
        /* Fill table cmd -> skin index */
        if ((skin_conf->controls[i].cmd >= 0) &&
            (skin_conf->controls[i].cmd < SKIN_CMD_MAX_NB)){
            current_skin.cmd2idx[skin_conf->controls[i].cmd] = i;
        }        
    }
    
error:
    iniparser_freedict(ini);
    return ret;
}


/** Release the current skin configuration
 *
 * \return true on succes, false on failure
 */
bool skin_release(void){
    int i;    
    struct skin_config * skin_conf = &current_skin.config;
#ifdef WITH_DEVIL       
    /* Unload different bitmap of the skin */
    if (current_skin.bitmap) 
        ilDeleteImages(1, &current_skin.bitmap);
    for(i = 0; i < skin_conf->nb; i++)
        if (current_skin.bitmaps[i]) 
            ilDeleteImages(1, &current_skin.bitmaps[i]);
#endif
    for(i = 0; i < skin_conf->nb; i++){
        free(skin_conf->controls[i].bitmap_filename);
    }
    free(skin_conf->bitmap_filename);
    reset_skin_conf();
    
    return true;
}


/** Initialize skin object from a zip skin file 
 *
 * \param filename      fullpath to the archive filename
 * \param load_bitmaps  Have bitmaps filename to be loaded as devil image 
 *
 * \return true on succes, false on failure
 */
bool skin_init(const char * filename, bool load_bitmaps ){
  int ws;
  int error;
  int expand_conf = false;
  struct zip * fp_zip;
  int return_code = false;
  int i;
  char cmd[200];
  struct skin_config * skin_conf = &current_skin.config;  
  
  error = 0;
  ws = ws_probe();
  reset_skin_conf();  
  
  /* Remove any residual bitmap temp file */
  unlink( ZIP_SKIN_BITMAP_FILENAME );
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
  
  if( load_skin_config(CONF_FILENAME) == false ){
    fprintf( stderr, "Error while loading config file <%s>\n", CONF_FILENAME );
    goto error;
  }

  if( expand_conf == true ) expand_config( skin_conf );

        if( unzip_file( fp_zip, skin_conf->bitmap_filename, ZIP_SKIN_BITMAP_FILENAME ) == false ){
                fprintf( stderr, "Error while unzipping <%s>\n", skin_conf->bitmap_filename );
                goto error;
        }

        if (load_bitmaps){  
          /* Loading of different bitmap of the skin */
          skin_load_bitmap(&current_skin.bitmap, ZIP_SKIN_BITMAP_FILENAME);
          for( i = 0; i < skin_conf->nb; i++ ){
              if(skin_conf->controls[i].bitmap_filename != NULL){
                  if (unzip_file(fp_zip, skin_conf->controls[i].bitmap_filename, ZIP_SKIN_BITMAP_FILENAME) == false){
                          fprintf(stderr, "Error while unzipping <%s>\n", skin_conf->controls[i].bitmap_filename);
                          goto error;
                  }
                  skin_load_bitmap(&current_skin.bitmaps[i], ZIP_SKIN_BITMAP_FILENAME);
                  PRINTDF("Loading %s - Image id :%i\n",skin_conf->controls[i].bitmap_filename, current_skin.bitmaps[i]);
              } else{
                  current_skin.bitmaps[i] = 0;
              }
          }
          if( expand_conf == true ){
            expand_bitmaps(skin_conf);
          }
        }

  return_code = true;

error:
    unlink( CONF_FILENAME );
    /* let the bitmap file to be able to use it after this call 
    unlink( ZIP_SKIN_BITMAP_FILENAME );*/
    zip_close( fp_zip );
    if (!return_code){
        skin_release(); 
    }
    return return_code;
}

const char * skin_cmd_2_txt(enum skin_cmd cmd){
    return cmd_labels[cmd];
}


struct skin_rectangular_shape skin_ctrl_get_zone(const struct skin_control *ctrl){
    struct skin_rectangular_shape zone;
    switch(ctrl->type){
        case SKIN_CONTROL_CIRCULAR :
            zone.x1 = ctrl->params.circ_icon.x - ctrl->params.circ_icon.r ;
            zone.y1 = ctrl->params.circ_icon.y - ctrl->params.circ_icon.r ;
            zone.x2 = ctrl->params.circ_icon.x + ctrl->params.circ_icon.r ;
            zone.y2 = ctrl->params.circ_icon.y + ctrl->params.circ_icon.r ;
            break;
        default :
            zone = ctrl->params.rect_icon;
            break;
    }
    return zone;
}

ILuint skin_get_background(void){
    return current_skin.bitmap;
}

ILuint skin_get_img(enum skin_cmd cmd){
    if (current_skin.cmd2idx[cmd] == -1) 
        return 0;    
    return current_skin.bitmaps[current_skin.cmd2idx[cmd]];
}

ILuint skin_get_pb_img(void){
    if (current_skin.progress_bar_index == -1)
        return 0;
    return current_skin.bitmaps[current_skin.progress_bar_index];
}


const struct skin_control* skin_get_ctrl(enum skin_cmd cmd){
    if (current_skin.cmd2idx[cmd] == -1) 
        return NULL;
    return &current_skin.config.controls[current_skin.cmd2idx[cmd]];
}

const struct skin_config *skin_get_config(void){
    return &current_skin.config;
}

const struct skin_control *skin_get_pb(void){
    if (current_skin.progress_bar_index == -1)
        return NULL;
    return &current_skin.config.controls[current_skin.progress_bar_index];
}

enum skin_cmd skin_get_cmd_from_xy(int x, int y, int * p){
    enum skin_cmd cmd;
    int i, distance;
    const struct skin_config * c = &current_skin.config;

    *p = -1;
   
    /* Init cmd !! */
    cmd = SKIN_CMD_EXIT_MENU;
    for( i = 0; i < c->nb; i++ ){
        switch( c->controls[i].type ){
            case SKIN_CONTROL_CIRCULAR:
                distance = sqrt( (x - c->controls[i].params.circ_icon.x ) * (x - c->controls[i].params.circ_icon.x ) + (y - c->controls[i].params.circ_icon.y ) * (y - c->controls[i].params.circ_icon.y ) );
                if( distance < c->controls[i].params.circ_icon.r ) cmd = c->controls[i].cmd;
                break;
            case SKIN_CONTROL_RECTANGULAR:
                if( c->controls[i].params.rect_icon.x1 < x && c->controls[i].params.rect_icon.x2 > x && c->controls[i].params.rect_icon.y1 < y && c->controls[i].params.rect_icon.y2 > y )
                    cmd = c->controls[i].cmd;
                break;
            case SKIN_CONTROL_PROGRESS_X:
                if( c->controls[i].params.rect_icon.x1 < x && c->controls[i].params.rect_icon.x2 > x && c->controls[i].params.rect_icon.y1 < y && c->controls[i].params.rect_icon.y2 > y ){
                    *p = ( 100 * ( x - c->controls[i].params.rect_icon.x1 ) )/( c->controls[i].params.rect_icon.x2 - c->controls[i].params.rect_icon.x1 );
                    if( *p >=100 ) *p=99;
                    cmd = c->controls[i].cmd;   
                }
                break;
            case SKIN_CONTROL_PROGRESS_Y:
                if( c->controls[i].params.rect_icon.x1 < x && c->controls[i].params.rect_icon.x2 > x && c->controls[i].params.rect_icon.y1 < y && c->controls[i].params.rect_icon.y2 > y ){
                    *p = ( 100 * ( y - c->controls[i].params.rect_icon.y1 ) )/( c->controls[i].params.rect_icon.y2 - c->controls[i].params.rect_icon.y1 );
                    if( *p >=100 ) *p=99;
                    cmd = c->controls[i].cmd;                    
                }
                break;
            case SKIN_CONTROL_TEXT:
                break;
            default:                    
                break;
        }
    }

    return cmd;
}

/** Load image in a DevIL bitmap
 *
 * \param bitmap_obj DevIL bitmap object
 * \param filename of the image
 *
 * \return true on success, false on failure
 * \note This function is more generic than skin but as this module is the primary user 
 * \note and as there is no better place for now, it remains here...
 */
bool skin_load_bitmap(ILuint * bitmap_obj, const char * filename){
#ifdef WITH_DEVIL
    ILint height, width;

    ilGenImages(1, bitmap_obj);
    ilBindImage(*bitmap_obj);
    if (!ilLoadImage(filename)) {
        fprintf(stderr, "Could not load image file %s.\nError : %s\n", filename, iluErrorString(ilGetError()));
        return false;
    }
    else{
      PRINTDF("Loading bitmap <%s>\n", filename);
    }
    /* Check that image is not too big - max about 1280x1024 - (otherwise ilConvertImage will kill the process!) */
    height = ilGetInteger(IL_IMAGE_HEIGHT);
    width =  ilGetInteger(IL_IMAGE_WIDTH);
    if ((height*width)> 5500000){
      PRINTDF("Image too big h*w = %ix%i\n", height,width);
      ilDeleteImages( 1, bitmap_obj);
      return false;
    }
    ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
    return true;
#else
  return false;
#endif
}
