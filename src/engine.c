/***************************************************************************
 *  Sun Jan  6 14:15:55 2008
 *  Copyright  2008  nullpointer
 *  Email
 *
 *  14.02.08 : wolfgar - Add progress bar handling
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


#include "config.h"
#include "engine.h"

char * cmd_mplayer = "./mplayer -quiet -include mplayer.conf -slave -input file=%s \"%s/%s\" > %s";
static char * fifo_command_name = "/tmp/mplayer-cmd.fifo";
static char * fifo_menu_name = "/tmp/mplayer-menu.fifo";
static char * fifo_stdout_name = "/tmp/mplayer-out.fifo";

static int fifo_command;
static int fifo_menu;
static int fifo_out;

struct tomplayer_config config;
BOOL is_menu_showed = FALSE;
BOOL is_mplayer_finished = FALSE;
BOOL is_playing_video = FALSE;
BOOL is_playing_audio = FALSE;
static BOOL is_paused = FALSE;

/* Hold -1 if the file is not being seeked 
 * or the percent that has to be reached*/
static int current_seek = -1;

extern char *strcasestr (const char *, const char *);

static void display_progress_bar(int current)
{

    char str[100];
    int i;
    int x,y;  
    int height, width;
    unsigned char * buffer;
    struct skin_config * c;

    int step1,step2;
    unsigned char col1r, col1g, col1b;
    unsigned char col2r, col2g, col2b;
        
            
    if( is_playing_video == TRUE ) 
      c = &config.video_config;
    else 
      c = &config.audio_config;

    if (c->progress_bar_index < 0){
	/* No progress bar on skin nothing to do */
	return;
    }
    height = c->controls[c->progress_bar_index].area.rectangular.y2 -  c->controls[c->progress_bar_index].area.rectangular.y1;
    width =  c->controls[c->progress_bar_index].area.rectangular.x2 -  c->controls[c->progress_bar_index].area.rectangular.x1;
    if ((height== 0) || (width == 0)){
       /*dont care if progress bar not visible */
       return;
    }

    

    buffer = malloc( height * width * 4 );           
    if (buffer == NULL){
      fprintf(stderr, "Allocation error\n");
      return;
    }

/* FIXME test if useful on large movie : disable for now...
    if (current_seek != -1){
	if (current_seek > current){ 	  
	  col1r = c->pb_r;
          col1g = c->pb_g;
          col1b = c->pb_b;
	  col2r = SEEK_POS_COLOR_R;
          col2g = SEEK_POS_COLOR_G;
          col2b = SEEK_POS_COLOR_B;
          step1 = current * width / 100;
          step2 = current_seek * width / 100;
        } else {
          col1r = SEEK_POS_COLOR_R;
          col1g = SEEK_POS_COLOR_G;
          col1b = SEEK_POS_COLOR_B;
	  col2r = c->pb_r;
          col2g = c->pb_g;
          col2b = c->pb_b;
          step2 = current * width / 100;
          step1 = current_seek * width / 100;
        }    	
    } else {
*/
      col1r = c->pb_r;
      col1g = c->pb_g;
      col1b = c->pb_b;
      step1 = step2 = current * width / 100;
  //  }

/*fprintf(stderr, "pbr : %i , pbg %i  pbb : %i \n", c->pb_r, c->pb_g, c->pb_b);*/
    i = 0;
    for( y = 0; y < height; y++ ){
        for( x = 0; x < width ; x++ ){           
            if (x <= step1) {
              buffer[i++] = (unsigned char )col1r;
              buffer[i++] = (unsigned char )col1g;
              buffer[i++] = (unsigned char )col1b;
              buffer[i++] = 255;
            } else {
            if ( (x > step1) && (x <= step2)){
              buffer[i++] = (unsigned char )col2r;
              buffer[i++] = (unsigned char )col2g;
              buffer[i++] = (unsigned char )col2b;
              buffer[i++] = 255;               
            } else {
              buffer[i++] = 0;
              buffer[i++] = 0;
              buffer[i++] = 0;
              buffer[i++] = 0;
            }
	    }
        }
    }
    if( is_playing_video == TRUE ) {
      sprintf(str, "RGBA32 %d %d %d %d %d %d\n", width, height, c->controls[c->progress_bar_index].area.rectangular.x1, c->controls[c->progress_bar_index].area.rectangular.y1, 0, 0);
      write(fifo_menu, str, strlen(str));
      write(fifo_menu, buffer, height * width * 4);
    } else {    
      gui_buffer_rgb(buffer,width, height, c->controls[c->progress_bar_index].area.rectangular.x1, c->controls[c->progress_bar_index].area.rectangular.y1);
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

  do {
    if  (total_read>=len){
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
*
*/
int get_int_from_stdout(void){
  char * value_pos=NULL;
  char buffer[200];
  int value;

  if (read_line_from_stdout(buffer, sizeof(buffer)) > 0){
    value_pos=strrchr(buffer,'=');
    if (value_pos == NULL){
      /*FIXME*/
      fprintf(stderr, "error parsing output : %s", buffer);
      return -1;
    }
    if (sscanf(value_pos+1, "%i", &value) == 1){
      return value;
    } else {
      return -1;
    }
  } else {
    return -1;
  }


}


/** Send a command to mplayer and wait for a positive int answer
*
* \param cmd the command to send 
* 
* \reval >0 : The int returned by mplayer
* \reval -1 : An error occured 
*
*/
static int send_command_wait_int(const char * cmd){
  int value; 
  int nb_try = 0;

  send_command(cmd);
  do {
    value = get_int_from_stdout();
    nb_try++;
  }while (( value == -1) && (nb_try < 25));
  return value;  
}

/** Return the current file position as a percent 
*/
static int get_file_position_percent(void){  
  return send_command_wait_int("pausing_keep get_property percent_pos\n");
}

/** Return the current file position in seconds 
*/
static int get_file_position_seconds(void){  
  return send_command_wait_int("pausing_keep get_property time_pos\n");
}


char * get_file_extension( char * file ){
    return strrchr( file, '.');
}
BOOL is_video_file( char * file ){
    char * ext;
    
    ext = get_file_extension( file );
    
    if( ext == NULL ) return FALSE;
    if( strlen( ext ) > 0 )
        if( strcasestr( config.filter_video_ext, ext ) != NULL ) return TRUE;
    return FALSE;
}

BOOL is_audio_file( char * file ){
    char * ext;
    
    ext = get_file_extension( file );
    
    if( ext == NULL ) return FALSE;
    if( strlen( ext ) > 0 )
        if( strcasestr( config.filter_audio_ext, ext ) != NULL ) return TRUE;
    return FALSE; 
}


void send_command( const char * cmd ){
    write( fifo_command, cmd, strlen( cmd ) );
}

void send_menu( char * cmd ){
    write( fifo_menu, cmd, strlen( cmd ) );
}

void show_menu( void ){
    is_menu_showed = TRUE;
    send_menu( "SHOW\n" );
}

void hide_menu( void ){
    is_menu_showed = FALSE;
    send_menu( "HIDE\n" );
}

/** Thread that updates peridocally OSD if needed
*
* \note it also flushes mplayer stdout 
*/
void * update_thread(void *cmd){
  int pos ;

  fprintf(stderr, "starting thread update ...\n");
  
  while (is_mplayer_finished == FALSE){
    flush_stdout();

    if (((is_menu_showed == TRUE) ||  ( is_playing_video == FALSE ))  && 
       (is_paused == FALSE)) {
      pos = get_file_position_percent();
      if (pos >= 0){
        display_progress_bar(pos);  
      } else {
         fprintf(stderr, "error while trying to retrieve current position\n");
      }

    }

    
    
 
/* FIXME check if useful     
   if (current_seek != -1){
      if (get_file_position_percent() == current_seek){
	current_seek = -1;
      }
    }
*/   
    usleep(250000);    
  }
  
  pthread_exit(NULL);
}


void * mplayer_thread(void *cmd){
    pthread_t t;

    is_mplayer_finished = FALSE; 
    pthread_create(&t, NULL, update_thread, NULL);   
    system( (char *) cmd );
    is_mplayer_finished = TRUE;
    pthread_exit(NULL);
}

void launch_mplayer( char * filename ){
    pthread_t t;
    char cmd[500];   

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
    
    is_menu_showed = FALSE;
    is_playing_video = FALSE;
    is_playing_audio = FALSE;
    
    if( is_video_file( filename ) ){
        is_playing_video = TRUE;
    }
    else{
        is_playing_audio = TRUE;
    }
    
    sprintf( cmd, cmd_mplayer, fifo_command_name, config.folder , filename, fifo_stdout_name );  
    pthread_create(&t, NULL, mplayer_thread, cmd);
    
    usleep( 500000 );
      
    if( is_video_file( filename ) ) 
        blit_video_menu( fifo_menu, &config.video_config );
	
}


void handle_mouse_event( int x, int y )
{
    int p;
    char buffer[200];
    
    if( is_menu_showed == FALSE && is_playing_video == TRUE){
        show_menu();        
        return;
    }
    
    switch( get_command_from_xy( x, y, &p ) ){
        case CMD_PAUSE:
            send_command("pause\n");
	    is_paused ^=  TRUE;
            break;
        case CMD_STOP:
            send_command( "quit\n" );
            break;
        case CMD_MUTE:
            send_command( "mute\n" );
            break;
        case CMD_VOL_PLUS:
            if( p == -1 ) send_command( "volume 1 0\n" );
            else{
                sprintf( buffer, "volume %d 1\n",p );
                send_command( buffer );
            }
        break;
        case CMD_VOL_MOINS:
            if( p == -1 ) send_command( "volume -1 0\n" );
            else{
                sprintf( buffer, "volume %d 1\n",p );
                send_command( buffer );
            }
            break;
        case CMD_LIGHT_PLUS:
            send_command( "brightness +5\n" );
            break;
        case CMD_LIGHT_MOINS:
            send_command( "brightness -5\n" );
            break; 
        case CMD_DELAY_PLUS:
            send_command( "audio_delay  +0.1\n" );
            break;
        case CMD_DELAY_MOINS:
            send_command( "audio_delay  -0.1\n" );
            break;
        case CMD_GAMMA_PLUS:
            send_command( "contrast +5\n" );
            break;
        case CMD_GAMMA_MOINS:
            send_command( "contrast -5\n" );
            break; 
        case CMD_FORWARD:
            if( p == -1 ) send_command( "seek 10 0\n" );
            else{
                sprintf( buffer, "seek %d 1\n",p );
                send_command( buffer );
            }
            break;
        case CMD_BACKWARD:
            if( p == -1 ) send_command( "seek -10 0\n" );
            else{
                sprintf( buffer, "seek %d 1\n",p );
                send_command( buffer );
            }
            break;
        case CMD_EXIT_MENU:
        default:
            if( is_playing_video == TRUE ) hide_menu();
    }
}


int get_command_from_xy( int x, int y, int * p ){
    int i,distance, cmd;
    struct skin_config * c;
        
    *p = -1;
    
    if( is_playing_video == TRUE ) c = &config.video_config;
    else c = &config.audio_config;

    
    for( i = 0; i < c->nb; i++ ){
        switch( c->controls[i].type ){
            case CIRCULAR_CONTROL:
                distance = sqrt( (x - c->controls[i].area.circular.x ) * (x - c->controls[i].area.circular.x ) + (y - c->controls[i].area.circular.y ) * (y - c->controls[i].area.circular.y ) );
                if( distance < c->controls[i].area.circular.r ) cmd = c->controls[i].cmd;
                break;
            case RECTANGULAR_CONTROL:
                if( c->controls[i].area.rectangular.x1 < x && c->controls[i].area.rectangular.x2 > x && c->controls[i].area.rectangular.y1 < y && c->controls[i].area.rectangular.y2 > y )
                    cmd = c->controls[i].cmd;
                break;
            case PROGRESS_CONTROL_X:
                if( c->controls[i].area.rectangular.x1 < x && c->controls[i].area.rectangular.x2 > x && c->controls[i].area.rectangular.y1 < y && c->controls[i].area.rectangular.y2 > y ){
                    *p = ( 100 * ( x - c->controls[i].area.rectangular.x1 ) )/( c->controls[i].area.rectangular.x2 - c->controls[i].area.rectangular.x1 );
                    if( *p >=100 ) *p=99;
                    cmd = c->controls[i].cmd;
		    current_seek = *p;
                }
                break;
            case PROGRESS_CONTROL_Y:
                if( c->controls[i].area.rectangular.x1 < x && c->controls[i].area.rectangular.x2 > x && c->controls[i].area.rectangular.y1 < y && c->controls[i].area.rectangular.y2 > y ){
                    *p = ( 100 * ( y - c->controls[i].area.rectangular.y1 ) )/( c->controls[i].area.rectangular.y2 - c->controls[i].area.rectangular.y1 );
                    if( *p >=100 ) *p=99;
                    cmd = c->controls[i].cmd;
                    current_seek = *p;
                }
                break;
            default:
                    cmd = CMD_EXIT_MENU;
                break;
        }
    }
    
    return cmd;
}
