/**
 * \file engine.c
 * \author nullpointer & wolfgar 
 * \brief This module implements all interactions with mplayer while audio or video are playing 
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


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <signal.h>
#include <math.h>
#include <time.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "config.h"
#include "engine.h"
#include "resume.h"
#include "widescreen.h"
#include "pwm.h"
#include "sound.h"
#include "power.h"
#include "debug.h"
#include "zip_skin.h"
/*#include "gui_control.h"*/
/*#include "window_context.h"*/
#include "file_list.h"
#include "gui.h"

/* Progress bar update period in us */
#define PB_UPDATE_PERIOD_US 250000
#define SCREEN_SAVER_ACTIVE (-1)

/*char * cmd_mplayer = "./mplayer -quiet -include ./conf/mplayer.conf -vf expand=%i:%i,bmovl=1:0:/tmp/mplayer-menu.fifo%s -ss %i -slave -input file=%s %s \"%s/%s\" > %s";*/
char * cmd_mplayer = "./mplayer -include ./conf/mplayer.conf -vf expand=%i:%i,bmovl=1:0:/tmp/mplayer-menu.fifo%s -ss %i -slave -input file=%s %s \"%s/%s\" > %s";


static char * fifo_command_name = "/tmp/mplayer-cmd.fifo";
static char * fifo_menu_name = "/tmp/mplayer-menu.fifo";
static char * fifo_stdout_name = "/tmp/mplayer-out.fifo";

static int fifo_command;
static int fifo_menu;
static int fifo_out;

struct tomplayer_config config;
bool is_menu_showed = false;
bool is_mplayer_finished = false;
bool is_playing_video = false;
bool is_playing_audio = false;
static bool is_paused = false;

/* Hold -1 if the file is not being seeked
 * or the percent that has to be reached*/
static int current_seek = -1;

/* To protect communication with mplayer -
 * update thread and gui event thread both perform requests...
 */
static pthread_mutex_t request_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t display_mutex = PTHREAD_MUTEX_INITIALIZER;

/* number of progress bar cycle without any activity on screen while screen in ON
 * if (no_user_interaction_cycles == SCREEN_SAVER_ACTIVE) then screen is OFF
 * and we do not count any longer */
static int no_user_interaction_cycles;

/* current settings */
static struct video_settings current_video_settings;
static struct audio_settings current_audio_settings;

extern char *strcasestr (const char *, const char *);
extern int coor_trans;

/* Image name of progress bar cursor */
static ILuint pb_cursor_id;
/* Image name of skin */
static ILuint skin_id;
/* Image name of battery cursor */
static ILuint bat_cursor_id;
/* Previous coord of progress bar cursor */
static struct{
		int x,y;
} prev_coords = {-1,-1};


static void display_RGB_to_fb(unsigned char * buffer, int x, int y, int w, int h, bool transparency);

int init_engine( void ){
	// Initialize DevIL.
	ilInit();
	iluInit();

	/* Will prevent any loaded image from being flipped dependent on its format */
	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_UPPER_LEFT);

    srand( time(NULL) );

    return true;
}

int release_engine( void ){
    ilShutDown();
    return true;
}

bool file_exist( char * file ){
	struct stat ftype;

	if( !stat( file, &ftype) ) return true;
	else return false;

}


void display_current_file( char * filename, struct skin_config *skin_conf, ILuint bitmap )
{
	// TODO retablir le comportement original
	display_image_to_fb( bitmap );
#if 0
	RECT rc;

    display_image_to_fb( bitmap );

    rc.left = skin_conf->text_x1;
    rc.right = skin_conf->text_x2;
    rc.top = skin_conf->text_y1;
    rc.bottom = skin_conf->text_y2;
    SetBkMode(HDC_SCREEN, BM_TRANSPARENT);
    SetTextColor(HDC_SCREEN, RGB2Pixel(HDC_SCREEN, skin_conf->text_color>>16, (skin_conf->text_color&0xFF00)>>8,skin_conf->text_color&0xFF));
    PRINTD("Display current file at x1 %i x2 %i y1 %i y2 %i color : %x name :%s\n",  rc.left, rc.right,rc.top,rc.bottom,skin_conf->text_color,filename );
    TextOut( HDC_SCREEN, rc.left, rc.top, filename );
    //DrawText (HDC_SCREEN, filename, -1, &rc, DT_CENTER );
#endif /* 0 */
}

void blit_video_menu( int fifo, struct skin_config * conf )
{
    char str[100];
    int height, width;
    unsigned char * buffer;
    int buffer_size;
    int i;

    ilBindImage(conf->bitmap);
    width  = ilGetInteger(IL_IMAGE_WIDTH);
    height = ilGetInteger(IL_IMAGE_HEIGHT);

    /* Alloc buffer for RBGA conversion */
    buffer_size = width * height * 4;
    buffer = malloc( buffer_size );
    if (buffer == NULL){
        fprintf(stderr, "Allocation error\n");
        return;
	}


    ilCopyPixels(0, 0, 0, width, height, 1, IL_RGBA, IL_UNSIGNED_BYTE, buffer);
    PRINTDF(" couleur transparence %d, %d, %d \n", conf->r,conf->g,conf->b);
    /* Aplly manually transparency */
    for (i=0; i < buffer_size; i+=4){
    	if ((buffer[i] == conf->r) &&
    		(buffer[i+1] == conf->g) &&
    		(buffer[i+2] == conf->b)){
    		buffer[i+3] = 0;
    	}
    }
    sprintf(str, "RGBA32 %d %d %d %d %d %d\n", width, height, 0, 0 , 0, 0);
    write(fifo_menu, str, strlen(str));
    write(fifo_menu, buffer,buffer_size);

    free( buffer );
}

/** Load bitmaps associated with controls
 * \note For now only Progress bar bitmap is involved */
static void init_ctrl_bitmaps(void){
	struct skin_config * c;

	prev_coords.x = -1;
	prev_coords.y = -1;

	if (is_playing_video == true )
		c = &config.video_config;
	else
		c = &config.audio_config;

    pb_cursor_id = c->controls[c->progress_bar_index].bitmap;
    bat_cursor_id = c->controls[c->bat_index].bitmap;
    skin_id = c->bitmap;

    /* Turn ON screen if it is not */
    pwm_resume();
}


/** Display a cursor over the skin
 *
 * \par cursor_id[in] Image name (Devil) to display
 * \par frame_id[in] Imagenumber to display in case of animation (0 if not used)
 * \par x[in] x coordinate where the image has to be displayed
 * \par y[in] y coordinate where the image has to be displayed
 *
 */
static void display_cursor_over_skin (ILuint cursor_id, ILuint frame_id, int x, int y ){
	ILuint tmp_skin_id = 0;
	ILuint tmp_cursor_id = 0;
	int height, width;
	int buffer_size;
	unsigned char * buffer;
	char str[100];

	/* Get cusor infos */
	ilBindImage(cursor_id);
	width  = ilGetInteger(IL_IMAGE_WIDTH);
	height = ilGetInteger(IL_IMAGE_HEIGHT);
	/* Alloc buffer for RBGA conversion */
	buffer_size = width * height * 4;
    buffer = malloc( buffer_size );
    if (buffer == NULL){
      fprintf(stderr, "Allocation error\n");
      return;
    }

	if (frame_id != 0){
		/* If we select a particular frame in an animation,
		 *  we have to copy that impage into a dedicated name id*/
		ilActiveImage(frame_id);
		PRINTDF("Frame id : %i active image : %i origin mode : %i\n",frame_id, ilGetInteger(IL_ACTIVE_IMAGE), ilGetInteger(IL_ORIGIN_MODE));
		ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
		tmp_cursor_id = ilCloneCurImage();
		cursor_id = tmp_cursor_id;
	}

    /* copy the relevant skin background zone */
	ilBindImage(skin_id);
    ilCopyPixels(x, y, 0, width, height, 1,
    			  IL_RGBA, IL_UNSIGNED_BYTE, buffer);
	ilGenImages(1, &tmp_skin_id);
	ilBindImage(tmp_skin_id);
	ilTexImage(width, height, 1, 4,IL_RGBA, IL_UNSIGNED_BYTE, buffer);
	/* Flip image because an ilTexImage is always LOWER_LEFT */
	iluFlipImage();

	/* Overlay cursor on background */
	ilOverlayImage(cursor_id,0,0,0);

	/* Update display zone */
	ilCopyPixels(0, 0, 0,
			 	width, height, 1,
			 	IL_RGBA, IL_UNSIGNED_BYTE, buffer);

	if( is_playing_video == true ) {
		sprintf(str, "RGBA32 %d %d %d %d %d %d\n", width, height, x, y , 0, 0);
		write(fifo_menu, str, strlen(str));
		write(fifo_menu, buffer,buffer_size);
	} else {
		/*gui_buffer_rgb(buffer,width, height, x, y);*/
		display_RGB_to_fb(buffer,x,y,width, height, true);
	}

	/* Cleanup */
	ilDeleteImages( 1, &tmp_skin_id);
	if (tmp_cursor_id != 0){
		ilDeleteImages( 1, &tmp_cursor_id);
	}
	free(buffer);
}



static void display_bat_state(bool force){
	static enum E_POWER_LEVEL previous_state = POWER_BAT_UNKNOWN;
	enum E_POWER_LEVEL state;
	struct skin_config * c;
	int x = -1;
	int y = -1;

	if (bat_cursor_id != 0){
		state = power_get_bat_state();
		if ((state != previous_state) ||
			(force == true )){
			if( is_playing_video == true )
		    	c = &config.video_config;
		    else {
		    	c = &config.audio_config;
		    }
			if (c->controls[c->bat_index].type == CIRCULAR_SKIN_CONTROL){
				x = c->controls[c->bat_index].area.circular.x - c->controls[c->bat_index].area.circular.r;
				y = c->controls[c->bat_index].area.circular.y - c->controls[c->bat_index].area.circular.r;
			}
			if (c->controls[c->bat_index].type == RECTANGULAR_SKIN_CONTROL){
				x = c->controls[c->bat_index].area.rectangular.x1;
				y = c->controls[c->bat_index].area.rectangular.y1;
			}
			PRINTDF("New battery state : %i \n", state);
			display_cursor_over_skin(bat_cursor_id,state,x,y);
			previous_state = state;
		}
	}
}



static void display_progress_bar(int current)
{
    char str[100];
    int i;
    int x,y;
    int height, width;
    unsigned char * buffer;
    int buffer_size;
    struct skin_config * c;

    if( is_playing_video == true )
      c = &config.video_config;
    else {
      c = &config.audio_config;
    }

    if (no_user_interaction_cycles == SCREEN_SAVER_ACTIVE){
    	/* No need to display progress bar as screen is off*/
    	return;
    }
    if (c->progress_bar_index < 0){
	/* No progress bar on skin nothing to do */
	return;
    }


    if (pb_cursor_id == 0) {
    	/* No cursor bitmap : just fill progress bar */
    	int step1;
    	unsigned char col1r, col1g, col1b;

    	height = c->controls[c->progress_bar_index].area.rectangular.y2 -  c->controls[c->progress_bar_index].area.rectangular.y1;
	    width =  c->controls[c->progress_bar_index].area.rectangular.x2 -  c->controls[c->progress_bar_index].area.rectangular.x1;
	    if ((height== 0) || (width == 0)){
	       /*dont care if progress bar not visible */
	       return;
	    }
	    buffer_size = height * width * 4;
	    buffer = malloc( buffer_size );
	    if (buffer == NULL){
	      fprintf(stderr, "Allocation error\n");
	      return;
	    }

		col1r = c->pb_r;
		col1g = c->pb_g;
		col1b = c->pb_b;
		step1 =  current * width / 100;

	    i = 0;
	    for( y = 0; y < height; y++ ){
	        for( x = 0; x < width ; x++ ){
	            if (x <= step1) {
	              buffer[i++] = (unsigned char )col1r;
	              buffer[i++] = (unsigned char )col1g;
	              buffer[i++] = (unsigned char )col1b;
	              buffer[i++] = 255;
	            } else {
	              buffer[i++] = 0;
	              buffer[i++] = 0;
	              buffer[i++] = 0;
	              buffer[i++] = 0;
	            }
		    }
	    }
	    if( is_playing_video == true ) {
	          sprintf(str, "RGBA32 %d %d %d %d %d %d\n", width, height, c->controls[c->progress_bar_index].area.rectangular.x1, c->controls[c->progress_bar_index].area.rectangular.y1, 0, 0);
	          write(fifo_menu, str, strlen(str));
	          write(fifo_menu, buffer,buffer_size);
	        } else {
/*	          gui_buffer_rgb(buffer,width, height, c->controls[c->progress_bar_index].area.rectangular.x1, c->controls[c->progress_bar_index].area.rectangular.y1);*/
	          display_RGB_to_fb(buffer, c->controls[c->progress_bar_index].area.rectangular.x1, c->controls[c->progress_bar_index].area.rectangular.y1, width, height,true);

	    }

    } else {
    	/* A cursor bitmap is available */
    	int new_x;
    	int erase_width;
    	int erase_x;

    	/* Get cusor infos and compute new coordinate */
    	ilBindImage(pb_cursor_id);
    	width  = ilGetInteger(IL_IMAGE_WIDTH);
    	height = ilGetInteger(IL_IMAGE_HEIGHT);
    	if (prev_coords.y == -1){
    		PRINTDF("New progress bar coord : y1 : %i - y2 : %i - h : %i\n",
    				c->controls[c->progress_bar_index].area.rectangular.y1,
    				c->controls[c->progress_bar_index].area.rectangular.y2,
    				height
    				);
    		prev_coords.y = c->controls[c->progress_bar_index].area.rectangular.y1 + (c->controls[c->progress_bar_index].area.rectangular.y2 -  c->controls[c->progress_bar_index].area.rectangular.y1) / 2 - height/2;
    		if (prev_coords.y < 0){
    			prev_coords.y = 0;
    		}
    		prev_coords.x = c->controls[c->progress_bar_index].area.rectangular.x1 - width/2;
    		if (prev_coords.x < 0){
    		    prev_coords.x = 0;
    		}
    	}
    	new_x = c->controls[c->progress_bar_index].area.rectangular.x1 + current * (c->controls[c->progress_bar_index].area.rectangular.x2 -  c->controls[c->progress_bar_index].area.rectangular.x1) / 100 - width/2;

    	/* Alloc buffer for RBGA conversion */
    	buffer_size = width * height * 4;
	    buffer = malloc( buffer_size );
	    if (buffer == NULL){
	      fprintf(stderr, "Allocation error\n");
	      return;
	    }

	    /* Restore Background */
	    ilBindImage(skin_id);
	    if (((new_x + width) < prev_coords.x)||
	    	((prev_coords.x + width) < new_x)){
	    	/* No intersection between new image and old image */
	    	erase_width = width;
	    	erase_x = prev_coords.x;
	    } else {
	    	if (prev_coords.x < new_x){
	    		/* New image is ahead of previous one with an intersection */
	    		erase_width = new_x-prev_coords.x;
	    		erase_x = prev_coords.x;
	    	} else {
	    		/* New image is behind previous one with an intersection */
	    		erase_width =  prev_coords.x - new_x;
	    		erase_x = new_x + width;
	    	}
	    }
	    ilCopyPixels(erase_x, prev_coords.y, 0,
	    	         erase_width, height , 1,
	    	    	 IL_RGBA, IL_UNSIGNED_BYTE, buffer);
	    PRINTDF("Progress bar - erase x : %i - prev y : %i - w : %i -h : %i - buffer : @%x \n",erase_x,prev_coords.y,erase_width,height,buffer);
	    if( is_playing_video == true ) {
	    	sprintf(str, "RGBA32 %d %d %d %d %d %d\n", erase_width, height, erase_x, prev_coords.y , 0, 0);
		    write(fifo_menu, str, strlen(str));
		    write(fifo_menu, buffer,4*erase_width*height);
        } else {
   	        /*gui_buffer_rgb(buffer,erase_width, height, erase_x, prev_coords.y);*/
   	        display_RGB_to_fb(buffer, erase_x, prev_coords.y, erase_width, height,true);
	    }

		/* Display cursor at its new position after overlaying cursor bitmap on skin background */
	    display_cursor_over_skin (pb_cursor_id, 0, new_x, prev_coords.y);

	    prev_coords.x = new_x;
    }
    free( buffer );
}

/** Flush any data from mplayer stdout
*/
static void flush_stdout(void){
  char buffer[200];
  int status;
  if (fifo_out>0) {
    status = fcntl(fifo_out, F_GETFL);
    fcntl(fifo_out,F_SETFL, status | O_NONBLOCK);
    while ( read(fifo_out, buffer, sizeof(buffer)) > 0);
    fcntl(fifo_out,F_SETFL, status & (~O_NONBLOCK));
  }
  return;
}

/** Read a line from mplayer stdout
*
*\param buffer the buffer where the line has to be stored
*\param len the size of the buffer
*
*\retval >0 : line sucessfully read, the len of the line is returned
*\retval -1 : An error occured
*/
static int read_line_from_stdout(char * buffer, int len){
  int total_read = 0;
  fd_set rfds;
  struct timeval tv;

  do {
    if  (total_read>=len){
      return -1;
    }
    FD_ZERO(&rfds);
    FD_SET(fifo_out, &rfds);
    tv.tv_sec = 0;
    tv.tv_usec = 300000;
    if (select(fifo_out+1, &rfds, NULL, NULL, &tv) == 0){
      /* Time out */
      return -1;
    }

    if (read(fifo_out, &buffer[total_read], 1) != 1){
      return -1;
    }
    total_read ++;
  }while (buffer[total_read-1] != '\n' );

  if (total_read < len){
    buffer[total_read]=0;
  } else {
    return -1;
  }
  return total_read;
}

/** retrieve an int value from mplayer stdout
*
* \param val[out]
*\retval 0 : OK
*\retval -1 : KO
*/
static int get_int_from_stdout(int *val){
  char * value_pos=NULL;
  char buffer[200];

  if (read_line_from_stdout(buffer, sizeof(buffer)) > 0){
    value_pos=strrchr(buffer,'=');
    if (value_pos == NULL){
      /*FIXME*/
      fprintf(stderr, "error parsing output : %s", buffer);
      return -1;
    }
    if (sscanf(value_pos+1, "%i", val) == 1){
      return 0;
    } else {
      return -1;
    }
  } else {
    return -1;
  }
}


/** retrieve any RAW string from mplayer stdout
*
* \param val[out]
*\retval 0 : OK
*\retval -1 : KO
*/
static int get_string_from_stdout(char *val){

	char buffer[200];
  	//PRINTD("get_string_from_stdout %s\n","");
    if (read_line_from_stdout(buffer, sizeof(buffer)) > 0){
        strcpy( val, buffer );
       // PRINTD( "Fichier courant <%s>\n",buffer );
        return 0;
    }

    return -1;
}

/** retrieve an float value from mplayer stdout
*
* \param val[out]
*\retval 0 : OK
*\retval -1 : KO
*/
static int get_float_from_stdout(float *val){
  char * value_pos=NULL;
  char buffer[200];

  if (read_line_from_stdout(buffer, sizeof(buffer)) > 0){
    value_pos=strrchr(buffer,'=');
    if (value_pos == NULL){
      /*FIXME*/
      fprintf(stderr, "error parsing output : %s", buffer);
      return -1;
    }
    if (sscanf(value_pos+1, "%f", val) == 1){
      return 0;
    } else {
      return -1;
    }
  } else {
    return -1;
  }
}

/** Send a command to mplayer and return any line from stdout
*
* \param cmd the command to send
* \param val[out] the string returned by mplayer
*
* \reval 0 : OK
* \reval -1 : An error occured
*
*/
static int send_command_wait_string(const char * cmd, char *val ){
  int nb_try = 0;
  int res = 0;
  //PRINTD("send_command_wait_string : %s \n", cmd);
  pthread_mutex_lock(&request_mutex);
  send_command(cmd);
  do {
    res = get_string_from_stdout(val);
    nb_try++;
  }while (( res == -1) && (nb_try < 5));
  pthread_mutex_unlock(&request_mutex);
  return res;
}

/** Send a command to mplayer and wait for an int answer
*
* \param cmd the command to send
* \param val[out] the int value returned by mplayer
*
* \reval 0 : OK
* \reval -1 : An error occured
*
*/
static int send_command_wait_int(const char * cmd, int *val ){
  int nb_try = 0;
  int res = 0;

  pthread_mutex_lock(&request_mutex);
  send_command(cmd);
  do {
    res = get_int_from_stdout(val);
    nb_try++;
  }while (( res == -1) && (nb_try < 5));
  pthread_mutex_unlock(&request_mutex);
  return res;
}


/** Send a command to mplayer and wait for a float answer
*
* \param cmd the command to send
* \param val[out] the float value returned by mplayer
*
* \reval 0 : OK
* \reval -1 : An error occured
*
*/
static int send_command_wait_float(const char * cmd, float *val ){
  int nb_try = 0;
  int res = 0;

  pthread_mutex_lock(&request_mutex);
  send_command(cmd);
  do {
    res = get_float_from_stdout(val);
    nb_try++;
  }while (( res == -1) && (nb_try < 5));
  pthread_mutex_unlock(&request_mutex);
  return res;
}


/** Return the current file position as a percent
*/
static int get_file_position_percent(void){
	int val = 0;
    if (send_command_wait_int(" get_property percent_pos\n", &val) == 0){
    	return val;
    } else {
    	/* Return 0 if command failed */
    	return 0;
    }
}

/** Return the current file been playing
*/
static bool get_current_file_name(char * filename){
	//PRINTD("get_current_file_name %s\n","");
    if (send_command_wait_string(" get_file_name\n", filename) == 0){
    	return true;
    }
    return false;
}

/** Return the current file position in seconds
*/
static int get_file_position_seconds(void){
  int val = 0;
  if (send_command_wait_int(" get_property time_pos\n",&val) == 0){
	  return val;
  } else {
  	/* Return 0 if command failed */
  	return 0;
  }
}


/** Ask mplayer for the curent video settings
 */
static int get_audio_settings( struct audio_settings * settings){

	return send_command_wait_int(" get_property volume\n", &settings->volume);

}

/** Ask mplayer for the curent audio settings
 */
static int get_video_settings( struct video_settings * settings){
	int res = 0;

	res  = send_command_wait_int(" get_property brightness\n", &settings->brightness);
	//fprintf(stderr,"lumi : %i\n res :%i\n",settings->brightness,res);
	res |= send_command_wait_int(" get_property contrast\n", &settings->contrast);
	//fprintf(stderr,"contrast : %i\n res :%i\n",settings->contrast,res);
	res |= send_command_wait_float(" get_property audio_delay\n", &settings->audio_delay);
	//fprintf(stderr,"delay : %f\n res :%i\n",settings->audio_delay,res);
	res |= send_command_wait_int(" get_property volume\n", &settings->volume);
	//fprintf(stderr,"volume  : %i\n res :%i\n",settings->volume,res);

	return res;
}

/** Set audio settings
 */
static void set_audio_settings(const struct audio_settings * settings){
	char buffer[256];

	snprintf(buffer, sizeof(buffer),"volume  %i 1\n",settings->volume);
	send_command(buffer);

	return;
}


/** Set audio settings
 */
static void set_video_settings(const struct video_settings * settings){
	char buffer[256];

	snprintf(buffer, sizeof(buffer),"audio_delay  %f 1\n",settings->audio_delay);
	send_command(buffer);
	snprintf(buffer, sizeof(buffer),"contrast  %i 1\n",settings->contrast);
	send_command(buffer);
	snprintf(buffer, sizeof(buffer),"brightness  %i 1\n",settings->brightness);
	send_command(buffer);
	snprintf(buffer, sizeof(buffer),"volume  %i 1\n",settings->volume);
	send_command(buffer);

	return;
}

char * get_file_extension( char * file ){
    return strrchr( file, '.');
}

bool has_extension( char * file, char * extensions ){
    char * ext;

    ext = get_file_extension( file );

    if( ext == NULL ) return false;
    if( strlen( ext ) > 0 )
        if( strcasestr( extensions, ext ) != NULL ) return true;
    return false;

}

bool is_skin_file( char * file ){
    return has_extension( file, ".zip" );
}

bool is_video_file( char * file ){
    return has_extension( file, config.filter_video_ext );
}

bool is_audio_file( char * file ){
    return has_extension( file, config.filter_audio_ext );}


void send_raw_command( const char * cmd ){
	PRINTDF ("Raw Commande envoy�e : %s",cmd);
	write( fifo_command, cmd, strlen(cmd));
}

/* Send a comand to mplayer
 *
 * This function automatically prepends prefix for pause correctness
 *
 * */
void send_command( const char * cmd ){
	char full_cmd [256];
	int len;
	len  = snprintf(full_cmd, sizeof(full_cmd), "%s %s", (is_paused ?"pausing " :"") , cmd);
	PRINTDF ("Commande envoy�e : %s",full_cmd);
    write( fifo_command, full_cmd, len );
}

void send_menu( char * cmd ){
    write( fifo_menu, cmd, strlen( cmd ) );
}

void show_menu( void ){
    is_menu_showed = true;
    send_menu( "SHOW\n" );
}

void hide_menu( void ){
    is_menu_showed = false;
    send_menu( "HIDE\n" );
}

static void quit_mplayer(void){
	int pos;

	if ( is_playing_video == true ){
	  is_paused=false;
	  pos = get_file_position_seconds();
	  if (pos > 0){
	    resume_write_pos(pos);
	  }
	}
	send_raw_command( "quit\n" );
}


void *anim_thread(void * param){
	int anim_idx ;
	int nb_frame,width,height;
	int current_num, x, y;
	unsigned char * buffer;

	anim_idx = config.audio_config.cmd2idx[SKIN_CMD_ANIM];

	 if (( is_playing_audio == true ) &&
	     ( anim_idx !=-1)){

		 ilBindImage(config.audio_config.controls[anim_idx].bitmap);
		 nb_frame = ilGetInteger(IL_NUM_IMAGES);
		 width  = ilGetInteger(IL_IMAGE_WIDTH);
		 height = ilGetInteger(IL_IMAGE_HEIGHT);
		 PRINTDF(" anim : frames : %i - WxH : %ix%i \n", nb_frame, width,height);
		 if (config.audio_config.controls[anim_idx].type == CIRCULAR_SKIN_CONTROL){
			x = config.audio_config.controls[anim_idx].area.circular.x - config.audio_config.controls[anim_idx].area.circular.r;
			y = config.audio_config.controls[anim_idx].area.circular.y - config.audio_config.controls[anim_idx].area.circular.r;
		 }
		 if (config.audio_config.controls[anim_idx].type == RECTANGULAR_SKIN_CONTROL){
			x = config.audio_config.controls[anim_idx].area.rectangular.x1;
			y = config.audio_config.controls[anim_idx].area.rectangular.y1;
		 }
		 /*
		 for (current_num=0; current_num<nb_frame; current_num++){
			 ilActiveImage(current_num);
			 ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);
		 }
		 */
		 current_num = 0;
		 while (is_mplayer_finished == false){
			 /*PRINTD("New anim frame : %i \n", current_num);*/
			 pthread_mutex_lock(&display_mutex);
			 ilBindImage(config.audio_config.controls[anim_idx].bitmap);
			 ilActiveImage(current_num);
			 ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);
	 		 buffer = ilGetData();
			 display_RGB_to_fb(buffer,x,y,width, height, false);
			 pthread_mutex_unlock(&display_mutex);

			 current_num++;
			 if (current_num >=nb_frame){
				 current_num=0;
			 }
			 usleep(100000);
		 }

	 }
	 return NULL;
}


/** Thread that updates peridocally OSD if needed
*
* \note it also flushes mplayer stdout
*/
void * update_thread(void *cmd){
  int pos ;
  int screen_saver_to_cycles;
  char current_filename[200];
  char old_current_filename[200];
  char *p;

  old_current_filename[0] = 0;

  /* Timout in cycles before turning OFF screen while playing audio */
  screen_saver_to_cycles = ((config.screen_saver_to * 1000000) / PB_UPDATE_PERIOD_US);

  PRINTDF("starting thread update...\n screen saver to : %i - cycle : %i \n",config.screen_saver_to, screen_saver_to_cycles );

  while (is_mplayer_finished == false){
    flush_stdout();


    if (((is_menu_showed == true) ||  ( is_playing_video == false ))  &&
       (is_paused == false)) {
    	 pthread_mutex_lock(&display_mutex);
      /* Update progress bar */
      pos = get_file_position_percent();
      if (pos >= 0){
        display_progress_bar(pos);
      } else {
         fprintf(stderr, "error while trying to retrieve current position\n");
      }

      /* Display battery state */
      display_bat_state(false);


      /* DO Not send periodic commands to mplayer while in pause because it unlocks the pause for a brief delay */
      if( is_playing_audio == true ){
          if( get_current_file_name( current_filename ) == true ){
              if( !strncmp( "ANS_FILENAME='", current_filename, strlen( "ANS_FILENAME='" ) ) ){
                  p=current_filename + strlen( "ANS_FILENAME='" );
                  p[strlen(p)-2] = 0;
                  if( strcmp( p, old_current_filename ) ){
                      strcpy( old_current_filename, p);
                      // TODO retablir l'affichage du nom du fichier
                      //display_current_file( p, &config.audio_config, config.audio_config.bitmap );
                      display_bat_state(true);
                  }
              }
          }
      }
      pthread_mutex_unlock(&display_mutex);
    }


	/* Handle screen saver */
	if ((is_playing_video == false) &&
		(no_user_interaction_cycles != SCREEN_SAVER_ACTIVE)){
		no_user_interaction_cycles++;
		if ((screen_saver_to_cycles != 0) &&
			(no_user_interaction_cycles >= screen_saver_to_cycles)){
			no_user_interaction_cycles = SCREEN_SAVER_ACTIVE;
			pwm_off();
		}
	}

	/* Check for headphones presence to turn on/off internal speaker */
	snd_check_headphone();

	/* Handle power button*/
	if (power_is_off_button_pushed() == true){
		quit_mplayer();
	}


    usleep(PB_UPDATE_PERIOD_US);
  }

  pthread_exit(NULL);
}


void * mplayer_thread(void *cmd){
    pthread_t t;

    is_mplayer_finished = false;
    pthread_create(&t, NULL, update_thread, NULL);
    pthread_create(&t, NULL, anim_thread, NULL);
    system( (char *) cmd );
    is_mplayer_finished = true;
    printf("\nmplayer has exited\n");
    /* Save settings to resume file */
    if ( is_playing_video == true ){
	   	resume_set_video_settings(&current_video_settings);
    } else {
    	resume_set_audio_settings(&current_audio_settings);
    }
    pthread_exit(NULL);
}

void launch_mplayer( char * folder, char * filename, int pos ){
    pthread_t t;
    char cmd[500];
    int resume_pos;
    char file[PATH_MAX+1];
    char rotated_param[10];
    char playlist_param[10];
    struct video_settings v_settings;
    struct audio_settings a_settings;

    // TODO Rotated screen
#if 0
    if( coor_trans != 0 ){
    	strcpy(rotated_param, ",rotate=1" );
    } else {
    	rotated_param[0] = 0;
    }
#else
    rotated_param[0] = 0;
#endif
    if ( has_extension( filename, ".m3u" ) ) {
    	strcpy(playlist_param, "-playlist" );
    } else {
    	playlist_param[0] = 0;
    }

    if( strlen( folder ) > 0 ){
    	sprintf( file, "%s/%s", folder,filename );
    } else {
    	strcpy( file, filename );
    }

    /* Dont want to be killed by SIGPIPE */
    signal (SIGPIPE, SIG_IGN);

    sprintf( cmd, "rm -rf %s %s %s", fifo_menu_name,fifo_command_name, fifo_stdout_name);
    system( cmd );

    mkfifo( fifo_command_name, 0700 );
    mkfifo( fifo_menu_name, 0700 );
    mkfifo( fifo_stdout_name, 0700);

    fifo_command = open( fifo_command_name, O_RDWR );
    fifo_menu = open( fifo_menu_name, O_RDWR );
    fifo_out = open(fifo_stdout_name, O_RDWR);

    is_menu_showed = false;
    is_playing_video = false;
    is_playing_audio = false;
    is_paused = false;
    no_user_interaction_cycles = 0;

    if ( is_video_file( filename ) ) {
      is_playing_video = true;
      resume_file_init(file);
      if (pos > 5){
        resume_pos = pos - 5;
      } else {
        resume_pos = 0;
      }
    }
    else {
      is_playing_audio = true;
      resume_pos = 0;
    }

    init_ctrl_bitmaps();

    /*sprintf( cmd, cmd_mplayer, (ws_probe()? WS_YMAX : WS_NOXL_YMAX), rotated_param, resume_pos, fifo_command_name, playlist_param, folder , filename, fifo_stdout_name );*/
    sprintf( cmd, cmd_mplayer, (ws_probe()? WS_XMAX : WS_NOXL_XMAX), (ws_probe()? WS_YMAX : WS_NOXL_YMAX), rotated_param, resume_pos, fifo_command_name, playlist_param, folder , filename, fifo_stdout_name );
    PRINTDF("Mplayer command line : %s \n", cmd);

    pthread_create(&t, NULL, mplayer_thread, cmd);

    usleep( 500000 );

    if( is_video_file( filename ) ){
    	blit_video_menu( fifo_menu, &config.video_config );
        /* Restore video settings */
        if (resume_get_video_settings(&v_settings) == 0){
        	set_video_settings(&v_settings);
        	current_video_settings = v_settings;
        } else {
        	 if (get_video_settings(&v_settings) == 0){
        		current_video_settings = v_settings;
        	 } else {
        		 memset (&current_video_settings,0,sizeof(current_video_settings));
        	 }
        }
    } else {
    	/* Restore audio settings */
    	if (resume_get_audio_settings(&a_settings) == 0){
    		set_audio_settings(&a_settings);
    		current_audio_settings = a_settings;
    	} else {
    		 if (get_audio_settings(&a_settings) == 0){
        		current_audio_settings = a_settings;
        	 } else {
        		 memset (&current_audio_settings,0,sizeof(current_audio_settings));
        	 }
    	}
    }
    display_bat_state(true);

}


void handle_mouse_event( int x, int y )
{
    int p;
    char buffer[200];
    struct video_settings v_settings;
    struct audio_settings a_settings;
    bool update_settings = false;


    if (no_user_interaction_cycles == SCREEN_SAVER_ACTIVE){
    	/* If screen is OFF, turn it ON */
    	pwm_resume();
    	no_user_interaction_cycles = 0;
    	/*Do not handle event as we touch the screen while it was OFF*/
    	return;
    }
    no_user_interaction_cycles = 0;

    if( is_menu_showed == false && is_playing_video == true){
        show_menu();
        return;
    }

    switch( get_command_from_xy( x, y, &p ) ){
        case SKIN_CMD_PAUSE:
        	send_raw_command("pause\n");
        	is_paused ^=  true;
            break;
        case SKIN_CMD_STOP:
        	quit_mplayer();
            break;
        case SKIN_CMD_MUTE:
            send_command( "mute\n" );
            break;
        case SKIN_CMD_VOL_PLUS:
            if( p == -1 ) send_command( "volume 1 0\n" );
            else{
                sprintf( buffer, "volume %d 1\n",p );
                send_command( buffer );
            }
            update_settings = true;
        break;
        case SKIN_CMD_VOL_MOINS:
            if( p == -1 ) send_command( "volume -1 0\n" );
            else{
                sprintf( buffer, "volume %d 1\n",p );
                send_command( buffer );
            }
            update_settings = true;
            break;

        case SKIN_CMD_LIGHT_PLUS:
            send_command( "brightness +5\n" );
            update_settings = true;
            break;
        case SKIN_CMD_LIGHT_MOINS:
            send_command( "brightness -5\n" );
            update_settings = true;
            break;
        case SKIN_CMD_DELAY_PLUS:
            send_command( "audio_delay  +0.1\n" );
            update_settings = true;
            break;
        case SKIN_CMD_DELAY_MOINS:
            send_command( "audio_delay  -0.1\n" );
            update_settings = true;
            break;
        case SKIN_CMD_GAMMA_PLUS:
            send_command( "contrast +5\n" );
            update_settings = true;
            break;
        case SKIN_CMD_GAMMA_MOINS:
            send_command( "contrast -5\n" );
            update_settings = true;
            break;
        case SKIN_CMD_FORWARD:
            if( p == -1 ) send_command( "seek 10 0\n" );
            else{
                sprintf( buffer, "seek %d 1\n",p );
                send_command( buffer );
                fprintf(stderr,"send command : %s\n",buffer);
            }
            break;
        case SKIN_CMD_BACKWARD:
            if( p == -1 ) send_command( "seek -10 0\n" );
            else{
                sprintf( buffer, "seek %d 1\n",p );
                send_command( buffer );
            }
            break;
        case SKIN_CMD_PREVIOUS:
            send_command( "pt_step -1" );
            break;
        case SKIN_CMD_NEXT:
            send_command( "pt_step 1" );
            break;
        case SKIN_CMD_BATTERY_STATUS:
        	break;
        case SKIN_CMD_EXIT_MENU:
        default:
            if( is_playing_video == true ) hide_menu();
    }


    /* Update in-memory settings */
    if (update_settings == true) {
	    if ( is_playing_video == true ){
		    if (get_video_settings(&v_settings) == 0){
		    	current_video_settings = v_settings;
		    }
	    } else {
		  	if (get_audio_settings( &a_settings) == 0){
		  		current_audio_settings = a_settings;
		  	}
	    }
    }

}


int get_command_from_xy( int x, int y, int * p ){
    int i,distance, cmd;
    struct skin_config * c;

    *p = -1;

    if( is_playing_video == true ) c = &config.video_config;
    else c = &config.audio_config;

    /* Init cmd !! */
    cmd = SKIN_CMD_EXIT_MENU;
    for( i = 0; i < c->nb; i++ ){
        switch( c->controls[i].type ){
            case CIRCULAR_SKIN_CONTROL:
                distance = sqrt( (x - c->controls[i].area.circular.x ) * (x - c->controls[i].area.circular.x ) + (y - c->controls[i].area.circular.y ) * (y - c->controls[i].area.circular.y ) );
                if( distance < c->controls[i].area.circular.r ) cmd = c->controls[i].cmd;
                break;
            case RECTANGULAR_SKIN_CONTROL:
                if( c->controls[i].area.rectangular.x1 < x && c->controls[i].area.rectangular.x2 > x && c->controls[i].area.rectangular.y1 < y && c->controls[i].area.rectangular.y2 > y )
                    cmd = c->controls[i].cmd;
                break;
            case PROGRESS_SKIN_CONTROL_X:
                if( c->controls[i].area.rectangular.x1 < x && c->controls[i].area.rectangular.x2 > x && c->controls[i].area.rectangular.y1 < y && c->controls[i].area.rectangular.y2 > y ){
                    *p = ( 100 * ( x - c->controls[i].area.rectangular.x1 ) )/( c->controls[i].area.rectangular.x2 - c->controls[i].area.rectangular.x1 );
                    if( *p >=100 ) *p=99;
                    cmd = c->controls[i].cmd;
		    current_seek = *p;

                }
                break;
            case PROGRESS_SKIN_CONTROL_Y:
                if( c->controls[i].area.rectangular.x1 < x && c->controls[i].area.rectangular.x2 > x && c->controls[i].area.rectangular.y1 < y && c->controls[i].area.rectangular.y2 > y ){
                    *p = ( 100 * ( y - c->controls[i].area.rectangular.y1 ) )/( c->controls[i].area.rectangular.y2 - c->controls[i].area.rectangular.y1 );
                    if( *p >=100 ) *p=99;
                    cmd = c->controls[i].cmd;
                    current_seek = *p;


                }
                break;
            default:
                    cmd = SKIN_CMD_EXIT_MENU;
                break;
        }
    }

    return cmd;
}


/** Display a RGB or RGBA buffer to frame buffer
 */
static void display_RGB_to_fb(unsigned char * buffer, int x, int y, int w, int h, bool transparency){
	static unsigned short * fb_mmap;
	static struct fb_var_screeninfo screeninfo;
	unsigned short * buffer16;
	int buffer_size;
	int fb;
	int i ;

	/* Alloc buffer for RBG conversion */
	buffer_size = w * h * 2 ;
	buffer16 = malloc( buffer_size );
	if (buffer16 == NULL){
	    fprintf(stderr, "Allocation error\n");
	    return;
	}


	if (fb_mmap == NULL){
		fb = open( getenv( "FRAMEBUFFER" ), O_RDWR);
		fb_mmap = mmap(NULL, buffer_size, PROT_READ|PROT_WRITE,MAP_SHARED, fb, 0);
	    ioctl (fb, FBIOGET_VSCREENINFO, &screeninfo);
	    close(fb);
	}

	/*if (coor_trans == 0){*/
		for( i = 0; i < (w * h); i++ ){
			if (transparency == false ){
				/* Initial buffer is RGB24 */
				buffer16[i] =  ( (buffer[3*i] & 0xF8)  <<  8 | /* R 5 bits*/
					     (buffer[3*i+1] & 0xFC) << 3 | /* G 6 bits */
						 (buffer[3*i+2]>>3));          /* B 5 bits*/
			} else {
				/* Initial buffer is RGBA*/
				buffer16[i] =  ( (buffer[4*i] & 0xF8) << 8 | /* R 5 bits*/
								   (buffer[4*i+1] & 0xFC) << 3 | /* G 6 bits */
								   (buffer[4*i+2]>>3));          /* B 5 bits*/
				/*TODO gerer transparence*/
			}
		}

	if (fb_mmap != NULL){
		int j;
		// TODO Rotated screen
#if 0
		if (coor_trans == 0){
#endif /* 0 */
			for (i=y; i<y+h; i++){
				for(j=x; j<x+w; j++){
					fb_mmap[j+(i*screeninfo.xres)]=buffer16[((i-y)*w)+(j-x)];
				}
			}
#if 0
		}else{
			/* Magic combination for inverted coordinates */
			for (i=y; i<y+h; i++){
				for(j=x; j<x+w; j++){
					fb_mmap[j*screeninfo.xres+i]=buffer16[(-i+y+h-1)*w+(j-x)];
				}
			}
		}
#endif /* 0 */
	}
	free( buffer16 );

}

void display_image_to_fb(ILuint img){
    int height, width;
    unsigned char  *buffer24;
    int buffer_size;

    ilBindImage(img);
    width  = ilGetInteger(IL_IMAGE_WIDTH);
    height = ilGetInteger(IL_IMAGE_HEIGHT);

    /* Alloc buffer for RBG conversion */
    buffer_size = width * height * 3;
    buffer24 = malloc( buffer_size );
    if (buffer24 == NULL){
        fprintf(stderr, "Allocation error\n");
        return;
	}
    ilCopyPixels(0, 0, 0, width, height, 1, IL_RGB, IL_UNSIGNED_BYTE, buffer24);
    display_RGB_to_fb(buffer24,0,0,width, height, false);
    free( buffer24 );
}
