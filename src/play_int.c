/**
 * \file play_int.c 
 * \brief This module implements all interactions with mplayer 
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

#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>

#include "widescreen.h"
#include "debug.h"
#include "play_int.h"

#ifdef NATIVE
#define MPLAYER_CMD_FMT "mplayer -quiet -vf expand=%i:%i,bmovl=1:0:/tmp/mplayer-menu.fifo%s -slave -input file=%s %s \"%s\" > %s 2> /dev/null"
#else
/* quiet option is mandatory to be able  to parse correctly mplayer output */
#define MPLAYER_CMD_FMT  "./mplayer -quiet -include ./conf/mplayer.conf -vf expand=%i:%i,bmovl=1:0:/tmp/mplayer-menu.fifo%s -slave -input file=%s %s \"%s\" > %s 2> /dev/null"
#endif
#define FIFO_COMMAND_NAME "/tmp/mplayer-cmd.fifo"
#define FIFO_MENU_NAME "/tmp/mplayer-menu.fifo"
#define FIFO_STDOUT_NAME "/tmp/mplayer-out.fifo"

/* FIFO fds */
static int fifo_command;
static int fifo_menu;
static int fifo_out;

/* mplayer pause state */
static bool is_paused = false;
/* mplayer running state */
static bool is_running = false;

/* mutex that protects request/reply exchanges with mplayer 
   from multiple threads (Update thread and GUI events handling thread)*/   
static pthread_mutex_t request_mutex = PTHREAD_MUTEX_INITIALIZER;

static char * get_file_extension(char * file){
    return strrchr( file, '.');
}

static bool has_extension(char * file, char * extensions){
    char * ext;

    ext = get_file_extension( file );
    if( ext == NULL ) return false;
    if( strlen( ext ) > 0 )
        if( strcasestr( extensions, ext ) != NULL ) return true;
    return false;
}


/** Wait for mplayer to output on its stdout
 * \param timeout timeout in ms
 * \retval  0 Data available
 * \retval -1 Timeout occured
 */
int playint_wait_output(int timeout){
  fd_set rfds;
  struct timeval tv;
  long long to_us = timeout * 1000;
 
  FD_ZERO(&rfds);
  FD_SET(fifo_out, &rfds);
  tv.tv_sec  = to_us / 1000000;
  tv.tv_usec = to_us % 1000000; 
  if (select(fifo_out+1, &rfds, NULL, NULL, &tv) <= 0){      
      /*perror("select stdout");
      printf("stout is %d\n", fifo_out);*/
      /* Time out */
      return -1;
  }
  return 0;
}

/** Flush any data from mplayer stdout */
void playint_flush_stdout(void){
  char buffer[200];
  int status;
  /*The Flush has to lock mutex to avoid to grab an answer from another thread */
  pthread_mutex_lock(&request_mutex);
  if (fifo_out>0) {
    status = fcntl(fifo_out, F_GETFL);
    fcntl(fifo_out,F_SETFL, status | O_NONBLOCK);
    while ( read(fifo_out, buffer, sizeof(buffer)) > 0){
        PRINTDF("Flushing %s\n", buffer);
    }
    fcntl(fifo_out,F_SETFL, status & (~O_NONBLOCK));
  }
  pthread_mutex_unlock(&request_mutex);
  return;
}

static void send_raw_command( const char * cmd ){
    PRINTDF ("Raw sent command : %s",cmd);
    write( fifo_command, cmd, strlen(cmd));
}

/* Send a comand to mplayer
 *
 * This function automatically prepends prefix for pause correctness
 *
 * */
static void send_command( const char * cmd ){
    char full_cmd [256];
    int len;
    len  = snprintf(full_cmd, sizeof(full_cmd), "%s %s", (is_paused ?"pausing " :"") , cmd);
    PRINTDF ("sent command : %s",full_cmd);
    write(fifo_command, full_cmd, len);
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
  
  int eof_idx;
  int read_bytes;
  bool eof_found = false;
  static char remaining_buffer[2048];
  static int idx = 0;
  
  
  do {    
    for (eof_idx = 0; eof_idx < idx; eof_idx++){
        if (remaining_buffer[eof_idx] == '\n'){
            //PRINTDF("EOF found at %d / %d\n", eof_idx+1, total_read);
            eof_found = true;
            break ;            
        }
    }
    if (eof_found) 
        break;
    if (playint_wait_output(300) < 0){
      /* Time out */
      PRINTDF("Timeout on stdout\n");
      return -1;
    }
    read_bytes = read(fifo_out, &remaining_buffer[idx], sizeof(remaining_buffer) - idx);
    if (read_bytes <= 0){
        PRINTDF("Error while reading from mplayer FIFO : %d - errno : %d\n", read_bytes, errno);
        return -1;
    }        
    idx += read_bytes;    
  } while (idx < sizeof(remaining_buffer) );
  
  
  
  if (eof_found){
    /* An EOLine has been found */
    if ((eof_idx + 1) <= len){
        memcpy(buffer, remaining_buffer, (eof_idx + 1));
        buffer[eof_idx] = 0;        
    } 
    memmove(remaining_buffer, &remaining_buffer[eof_idx + 1], idx - eof_idx -1);
    idx -= (eof_idx + 1);
    if ((eof_idx + 1) <= len){
        //PRINTDF("Read OK from mplayer : %s\n", buffer);
        return eof_idx;
    } else {
        PRINTDF("Read KO from mplayer (too long) \n");
        return -1;
    }
  }
  
  if (idx >= sizeof(remaining_buffer)){
      /* Abnormal case ; we have filled the whole buffer and not found an EOL - Flush everything */
      idx = 0;   
  }
  PRINTDF(" Fatal error on read_line_from_stdout exit\n");
  return -1;
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
    PRINTDF("Reading : %s", buffer);
    value_pos=strrchr(buffer,'=');
    if (value_pos == NULL){
      /*FIXME*/
      fprintf(stderr, "error parsing output : %s\n", buffer);
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
*\retval >0 : OK, string length
*\retval -1 : KO
*/
static int get_string_from_stdout(char *val, size_t len){
    int nb_chars;
    
    nb_chars = read_line_from_stdout(val, len);
    //PRINTDF("get_string_from_stdout : %d\n", nb_chars);
    if (nb_chars > 0){
        PRINTDF( "Line read from mplayer : %s\n", val);
        return nb_chars;
    }
    return -1;
}

/** retrieve an float value from mplayer stdout
*
* \param[out] val
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
* \param[in]  cmd the command to send
* \param[out] val the string returned by mplayer
*
* \reval >0 : OK, string length
* \reval -1 : An error occured
*
*/
static int send_command_wait_string(const char * cmd, char *val, size_t len, const char *pattern){
  int nb_try = 0;
  int res = 0;
  PRINTDF("send_command_wait_string : %s \n", cmd);
  pthread_mutex_lock(&request_mutex);
  send_command(cmd);
  do {
    res = get_string_from_stdout(val, len);
    if (strstr(val, pattern) == NULL)
        res = -1;
    nb_try++;
  }while ((res == -1) && (nb_try < 30) && (is_running));
  PRINTDF("send_command_wait_string : %d - %s\n", res, val);
  pthread_mutex_unlock(&request_mutex);
  return res;
}

/** Send a command to mplayer and wait for an int answer
*
* \param[in]  cmd the command to send
* \param[out] val the int value returned by mplayer
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
  } while ((res == -1) && (nb_try < 5) && (is_running));
  pthread_mutex_unlock(&request_mutex);
  return res;
}


/** Send a command to mplayer and wait for a float answer
*
* \param[in]  cmd the command to send
* \param[out] val the float value returned by mplayer
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
  }while (( res == -1) && (nb_try < 5) && (is_running));
  pthread_mutex_unlock(&request_mutex);
  return res;
}

int playint_get_file_length(void){
  int val = 0;
    if (send_command_wait_int(" get_property length\n", &val) == 0){
      return val;
    } else {
      /* Return 0 if command failed */
      return 0;
    }
}

static int extract_mplayer_answer(char *buffer, int len, const char* pattern, bool quote){
    int extract_len = -1;
    int pattern_len = strlen(pattern);
        
    if(!strncmp(pattern, buffer, pattern_len)){
        /* Remove ANS pattern and final quote */   
        extract_len = len - pattern_len;
        if (quote)
           extract_len -= 1;
        memmove(buffer, buffer + pattern_len, extract_len);
        buffer[extract_len] = 0;
        //PRINTDF("Extracted name : %s\n", buffer);        
    } 
    return extract_len;
}

/** Return the current file been playing */
int playint_get_filename(char *buffer, size_t len){
    #define FILENAME_ANS_PATTERN "ANS_FILENAME='"
  
    int nb_chars;   
    
    nb_chars = send_command_wait_string(" get_file_name\n", buffer, len, FILENAME_ANS_PATTERN);    
    if (nb_chars > 0){
        return extract_mplayer_answer(buffer, nb_chars, FILENAME_ANS_PATTERN, true);        
    }      
    return -1;
}

int playint_get_path(char *buffer, size_t len){
    #define PATH_ANS_PATTERN "ANS_path="
  
    int nb_chars;   
    
    nb_chars = send_command_wait_string("get_property path\n", buffer, len, PATH_ANS_PATTERN);    
    if (nb_chars > 0){
        return extract_mplayer_answer(buffer, nb_chars, PATH_ANS_PATTERN, false);        
    }      
    return -1;
}

/** Return the artist from the current file (from ID tag) */
int playint_get_artist(char *buffer, size_t len){
    #define ARTIST_ANS_PATTERN "ANS_META_ARTIST='"
  
    int nb_chars;   
    nb_chars = send_command_wait_string(" get_meta_artist\n", buffer, len, ARTIST_ANS_PATTERN);    
    if (nb_chars > 0){
        return extract_mplayer_answer(buffer, nb_chars, ARTIST_ANS_PATTERN, true);        
    }
    return -1;
}

/** Return the title from the current file (from ID tag) */
int playint_get_title(char *buffer, size_t len){
    #define ALBUM_ANS_PATTERN "ANS_META_TITLE='"
    int nb_chars;   
    nb_chars = send_command_wait_string(" get_meta_title\n", buffer, len, ALBUM_ANS_PATTERN);    
    if (nb_chars > 0){
        return extract_mplayer_answer(buffer, nb_chars, ALBUM_ANS_PATTERN, true);        
    }
    return -1;
}

/** Return the current file position in seconds
*/
int playint_get_file_position_seconds(void){  
  int val = 0;
  if (is_paused)
    return -1;  
  if (send_command_wait_int(" get_property time_pos\n",&val) == 0){
    return val;
  } else {
    /* Return 0 if command failed */
    return 0;
  }
}

/** Return the current file position in percent
*/
int playint_get_file_position_percent(void){  
  int val = 0;
  if (is_paused)
    return -1;  
  if (send_command_wait_int(" get_property percent_pos\n",&val) == 0){
    return val;
  } else {
    /* Return 0 if command failed */
    return 0;
  }
}

/** Ask mplayer for the curent video settings */ 
int playint_get_audio_settings(struct audio_settings * settings){
  return send_command_wait_int(" get_property volume\n", &settings->volume);
}

/** Ask mplayer for the curent audio settings
 */
int playint_get_video_settings( struct video_settings * settings){
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
void playint_set_audio_settings(const struct audio_settings * settings){
  char buffer[256];

  snprintf(buffer, sizeof(buffer),"volume  %i 1\n",settings->volume);
  send_command(buffer);

  return;
}


/** Set video settings
 */
void playint_set_video_settings(const struct video_settings * settings){
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

static void send_menu( char * cmd){
    write(fifo_menu, cmd, strlen(cmd));
}

void playint_menu_write(const unsigned char *buffer, size_t len){
    write(fifo_menu, buffer, len);
}
    
void playint_menu_show( void ){    
    send_menu("SHOW\n");
}

void playint_menu_hide( void ){    
    send_menu("HIDE\n");
}

void playint_osd(const char *text, int to){
    char buffer[128];
    if (strlen(text) > (sizeof(buffer) - 28))
        return;
    snprintf(buffer, sizeof(buffer), "osd_show_text \"%s\" %d\n", text, to);
    buffer[sizeof(buffer)-1] = 0;
    send_command(buffer);   
}


void playint_display_time(void){
  char buff_time[40];
  time_t curr_time;
  struct tm * ptm;   
  
  time(&curr_time);  
  ptm= localtime(&curr_time);
  snprintf(buff_time,sizeof(buff_time),"osd_show_text \"Time : %02d:%02d\" 2000\n",ptm->tm_hour, ptm->tm_min);
  buff_time[sizeof(buff_time)-1]=0; 
  send_command(buff_time); 
}

void playint_pause(void){
    send_raw_command("pause\n");
    is_paused ^=  true;
}

bool playint_is_paused(void){
    return is_paused;
}
    
void playint_quit(void){ 
  is_paused=false;
  send_raw_command( "quit\n" );
}

void playint_seek(int val, enum playint_seek type){
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "seek %d %d\n", val, type);
    buffer[sizeof(buffer)-1] = 0;
    send_command(buffer); 
}

void playint_mute(void){
    send_command("mute\n");
}

void playint_vol(int val, enum playint_vol type){
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "volume %d %d\n", val, type);
    buffer[sizeof(buffer)-1] = 0;
    send_command(buffer); 
}

void playint_bright(int step){
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "brightness %d\n", step);
    buffer[sizeof(buffer)-1] = 0;
    send_command(buffer);   
}

void playint_contrast(int step){
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "contrast %d\n", step);
    buffer[sizeof(buffer)-1] = 0;
    send_command(buffer);   
}

void playint_skip(int step){
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "pt_step %d\n", step);
    buffer[sizeof(buffer)-1] = 0;
    send_command(buffer);       
}

void playint_delay(double step){
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "audio_delay %.1f\n", step);
    buffer[sizeof(buffer)-1] = 0;
    send_command(buffer);     
}

/** \note This function blocks and returns only when mplayer is dead */
void playint_run(char * filename){
    char cmd[500]; 
    char rotated_param[10];
    char playlist_param[10];

    if(ws_are_axes_inverted() != 0){
      strcpy(rotated_param, ",rotate=1" );
    } else {
      rotated_param[0] = 0;
    }
    if (has_extension(filename, ".m3u")) {
      strcpy(playlist_param, "-playlist" );
    } else {
      playlist_param[0] = 0;
    }
    snprintf(cmd, sizeof(cmd), MPLAYER_CMD_FMT, (ws_probe()? WS_XMAX : WS_NOXL_XMAX), 
            (ws_probe()? WS_YMAX : WS_NOXL_YMAX), rotated_param, 
            FIFO_COMMAND_NAME, playlist_param, filename, FIFO_STDOUT_NAME);
    cmd[sizeof(cmd)-1] = 0;
    PRINTDF("Mplayer command line : %s \n", cmd);      
    system((char *)cmd);    
    is_running = false;
}

bool playint_is_running(void){
    return is_running;
}

void playint_init(void){
    /* (re)create Fifos which are used to communicate between tomplayer and mplayer */    
    unlink(FIFO_COMMAND_NAME);
    mkfifo(FIFO_COMMAND_NAME, 0700);
    unlink(FIFO_MENU_NAME);
    mkfifo(FIFO_MENU_NAME, 0700);
    unlink(FIFO_STDOUT_NAME);
    mkfifo(FIFO_STDOUT_NAME, 0700);
    fifo_command = open(FIFO_COMMAND_NAME, O_RDWR);
    fifo_menu = open(FIFO_MENU_NAME, O_RDWR);
    fifo_out = open(FIFO_STDOUT_NAME, O_RDWR);
    is_paused = false;
    /* is_running is set to true before real launch of mplayer 
       coz only the value false is meaningfull for callers 
       to playint_is_running and default value must be true 
       to avoid premature exits*/
    is_running = true;
}
