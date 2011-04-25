/**
 * \file skin_display.c
 * \brief Handle skin display and the update of the controls  
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

#include <time.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "widescreen.h"
#include "power.h"
#include "skin.h"
#include "track.h"
#include "font.h"
#include "play_int.h"
#include "debug.h"
#include "draw.h"
#include "gps.h"
#include "skin_display.h"

#define COLOR_R(x) ((x & 0xFF0000) >> 16)
#define COLOR_G(x) ((x & 0x00FF00) >> 8)
#define COLOR_B(x)  (x & 0x0000FF)

static void draw_track_text(const char * text) {               
    const struct skin_config * skin_conf;
    int img_width, img_height;
    struct font_color  color ;
     
    skin_conf = skin_get_config();
    img_width = (skin_conf->text_x2 - skin_conf->text_x1) ;
    img_height = (skin_conf->text_y2 - skin_conf->text_y1);
    color.r = COLOR_R(skin_conf->text_color);
    color.g = COLOR_G(skin_conf->text_color);
    color.b = COLOR_B(skin_conf->text_color);
    draw_text(text, skin_conf->text_x1, skin_conf->text_y1, img_width, img_height, &color, 0);
}

static void refresh_filename(void){  
  char displayed_name[300];
  const char *filename;
  const struct track_tags * tags;
  
  if (skin_get_config()->display_filename == 0)
      return;
  filename = track_get_current_filename();
  tags = track_get_tags();
  if (tags->title != NULL){
    if (tags->artist != NULL){
        snprintf(displayed_name, sizeof(displayed_name),"%s - %s", tags->artist, tags->title);
    } else {
        snprintf(displayed_name, sizeof(displayed_name),"%s", tags->title);
    }
    displayed_name[sizeof(displayed_name)-1] = 0;
    draw_track_text(displayed_name);    
  } else {
    draw_track_text(filename);       
  }
}

static const char *get_string_tag_ctrl(const struct track_tags * tags, enum skin_cmd cmd){
    switch (cmd){
        case SKIN_CMD_TEXT_ARTIST :
            return tags->artist;
        case SKIN_CMD_TEXT_TRACK :
            return tags->track;
        case SKIN_CMD_TEXT_ALBUM :
            return tags->album;
        case SKIN_CMD_TEXT_TITLE :
            return tags->title;
        case SKIN_CMD_TEXT_YEAR :
            return tags->year;
        case SKIN_CMD_TEXT_COMMENT :
            return tags->comment;
        case SKIN_CMD_TEXT_GENRE :
            return tags->genre;           
        default :
            return NULL;
    }
}

static void display_txt_ctrl(const struct skin_control *ctrl, const char* string){
    int text_width, text_height, orig;
    struct font_color color;        
    int screen_width, screen_height;

    if ((string == NULL) || (strlen(string) == 0)){
        return;
    }
    
    if (ctrl != NULL){
        if (ctrl->params.text.size != -1)
            font_change_size(ctrl->params.text.size);
        font_get_size(string, &text_width, &text_height, &orig);
        if (ctrl->params.text.size != -1)
            font_restore_default_size();  
        if (ctrl->params.text.color != -1){
            color.r = 0xFF;
            color.g = 0xFF;
            color.b = 0xFF;
        } else {
            color.r = COLOR_R(ctrl->params.text.color);
            color.g = COLOR_G(ctrl->params.text.color);
            color.b = COLOR_B(ctrl->params.text.color);
        }
        ws_get_size(&screen_width, &screen_height);
        
        /* Truncate height and width if needed */
        if ((text_width + ctrl->params.text.x) > screen_width){
            text_width = screen_width - ctrl->params.text.x - 1;
        }
        if ((text_height + ctrl->params.text.y) > screen_height){
            text_height = screen_height - ctrl->params.text.y -1;
        }
        draw_text(string, 
                  ctrl->params.text.x, ctrl->params.text.y,
                  text_width, text_height, &color, 
                  (ctrl->params.text.size != -1)?ctrl->params.text.size:0);
    }
    return;
}

static void refresh_tags_infos(void){
    const struct skin_control *ctrl;
    const struct track_tags *tags;          
    enum skin_cmd cmd;
    
    tags = track_get_tags();
    
    /* Handle text tags */
    for (cmd = SKIN_CMD_TAGS_FIRST; cmd <= SKIN_CMD_TAGS_LAST; cmd++){
        ctrl = skin_get_ctrl(cmd);
        if (ctrl != NULL)
            display_txt_ctrl(ctrl, get_string_tag_ctrl(tags, cmd));
    }
    
    /* Handle coverart */    
    ctrl = skin_get_ctrl(SKIN_CMD_COVERART);
    if (ctrl != NULL){        
        ILuint default_cover;
        int width, height;
        
        default_cover = skin_get_img(SKIN_CMD_COVERART);
        if (default_cover != 0){
            if (tags->coverart == 0){
                /*No coverart for this file - Display default one*/
                draw_cursor(default_cover, 0, ctrl->params.text.x, ctrl->params.text.y);
            } else {
                /* Resize the embedded cover and display it */      
                ilBindImage(default_cover);
                width  = ilGetInteger(IL_IMAGE_WIDTH);
                height = ilGetInteger(IL_IMAGE_HEIGHT);
                ilBindImage(tags->coverart);
                iluScale(width, height, 1);
                draw_cursor(tags->coverart, 0, ctrl->params.text.x, ctrl->params.text.y);
            }
        }
    }
    
}

static int track_get_string(char * buffer, size_t len, int time, bool is_hour){        
    int curr_hour, curr_min, curr_sec;
    int ret;
    char sign[2] = {0,0};
    
    curr_sec = abs(time);    
    curr_hour = curr_sec / 3600;
    curr_min = curr_sec / 60;
    curr_sec = curr_sec % 60;
    if (time < 0)
        sign[0] = '-';
    if (is_hour){        
        ret = snprintf(buffer, len, "%s%02i:%02i:%02i", sign, curr_hour, curr_min, curr_sec);
    } else {
        ret = snprintf(buffer, len, "%s%02i:%02i", sign, curr_min, curr_sec);
    }
    buffer[len - 1] = 0;
    return ret;    
}

/* Percent param may appear as a redundant param but it is not : 
   It is handled as an independant params to set the cursor on progress bar
   (the underlying issue is that mplayer may reports wrong pos for VBR streams)  */
static void display_progress_bar(int current, int length, int percent)
{
    /* Previous coord of progress bar cursor */
    static struct{
        int x,y;
        int pos, length; 
    } pb_prev_val = {-1, -1, 0, 0};
    
    int i, x, y, height, width;
    unsigned char * buffer;
    int buffer_size;       
    struct font_color color;
    char displayed_text[16];
    int text_height, text_width, orig; 
    struct skin_rectangular_shape pb_zone;
    const struct skin_control *pb = skin_get_pb();
    
    if (pb == NULL){
        /* No progress bar on skin nothing to do */
        return;
    }
    
    if ((pb_prev_val.pos == current) && 
        (pb_prev_val.length == length)){
        /* current position in seconds since last call have not been modified
         * It is useless to redraw...*/
        return;
    }
    pb_prev_val.pos = current;
    pb_prev_val.length = length;
    pb_zone = skin_ctrl_get_zone(pb);
    

    /* Display current time in file at the beginig of progress bar */    
    height = pb_zone.y2 - pb_zone.y1;
    color.r = 255;
    color.g = 255;
    color.b = 255; 
    if ((height - 4) > 0){
        /* Display current track time */
        track_get_string(displayed_text, sizeof(displayed_text), current, !(length < 3600));
        font_change_size(height-4);        
        font_get_size(displayed_text, &text_width, &text_height, &orig);
        font_restore_default_size();  
        draw_text(displayed_text, pb_zone.x1, pb_zone.y1 - text_height,
                          text_width, text_height, &color, height-4);
        /* Display remaining track time */                      
        current = current - length;
        track_get_string(displayed_text, sizeof(displayed_text), current, !(length < 3600));
        font_change_size(height-4);        
        font_get_size(displayed_text, &text_width, &text_height, &orig);
        font_restore_default_size();  
        draw_text(displayed_text,  pb_zone.x2 - text_width, pb_zone.y1 - text_height,
                          text_width, text_height, &color, height-4);        
    }
                       
    
    if (skin_get_pb_img() == 0) {
        /* No cursor bitmap : just fill progress bar */
        int step1;
        unsigned char col1r, col1g, col1b;
        
        width =  pb_zone.x2 - pb_zone.x1;
        if ((height== 0) || (width <= 0)){
            /*dont care if progress bar not visible */
            return;
        }
        buffer_size = height * width * 4;
        buffer = malloc( buffer_size );
        if (buffer == NULL){
            fprintf(stderr, "Allocation error\n");
            return;
        }    
        col1r = skin_get_config()->pb_r;
        col1g = skin_get_config()->pb_g;
        col1b = skin_get_config()->pb_b;
        step1 =  percent * width / 100;

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
        draw_RGB_buffer(buffer, pb_zone.x1, pb_zone.y1, width, height, true);
    } else {
        /* A cursor bitmap is available */
        int new_x;
        int erase_width;
        int erase_x;
                          
        /* Get cusor infos and compute new coordinate */
        ilBindImage(skin_get_pb_img());
        width  = ilGetInteger(IL_IMAGE_WIDTH);
        height = ilGetInteger(IL_IMAGE_HEIGHT);
        if (pb_prev_val.y == -1){
            PRINTDF("New progress bar coord : y1 : %i - y2 : %i - h : %i\n",
                    pb_zone.y1, pb_zone.y2, height);
            pb_prev_val.y = pb_zone.y1 + (pb_zone.y2 - pb_zone.y1) / 2  - height/2;                           
            if (pb_prev_val.y < 0){
                pb_prev_val.y = 0;
            }
            pb_prev_val.x = pb_zone.x1 - width / 2;
            if (pb_prev_val.x < 0){
                pb_prev_val.x = 0;
            }
        }
        new_x = pb_zone.x1 +
                percent * (pb_zone.x2 - pb_zone.x1) / 100 - width / 2;

        /* Alloc buffer for RBGA conversion */
        buffer_size = width * height * 4;
        buffer = malloc( buffer_size );
        if (buffer == NULL){
            fprintf(stderr, "Allocation error\n");
            return;
        }

        /* Restore Background */
        ilBindImage(skin_get_background());
        erase_width = width;
        erase_x = pb_prev_val.x;        
        ilCopyPixels(erase_x, pb_prev_val.y, 0,
                     erase_width, height , 1,
                     IL_RGBA, IL_UNSIGNED_BYTE, buffer);
        /*PRINTDF("Progress bar - erase x : %i - prev y : %i - w : %i -h : %i - buffer : @%x \n",erase_x,prev_coords.y,erase_width,height,buffer);*/
        draw_RGB_buffer(buffer, erase_x, pb_prev_val.y, erase_width, height, true);

        /* Display cursor at its new position after overlaying cursor bitmap on skin background */
        draw_cursor(skin_get_pb_img(), 0, new_x, pb_prev_val.y);
        pb_prev_val.x = new_x;
    }
    free(buffer);
}

static void refresh_progress_bar(void){
    int pos;    
    int length;       
    int percent;
    
    pos = playint_get_file_position_seconds();    
    length = track_get_tags()->length;            
    if ((pos >= 0) && (length > 0)) {          
        /* BUG : pos may be plain wrong coz mplayer does not correctly handle VBR */
        /* Dont know how to fix correctly : Just Avoid crazy figures for now */
        if (pos <= length){
            percent = pos * 100 / length;
        } else {
            pos = length;
            percent = 100;
        }
        display_progress_bar(pos, length, percent);
    }
}

static void refresh_battery_status(bool force){
    static enum E_POWER_LEVEL previous_state = POWER_BAT_UNKNOWN;
    enum E_POWER_LEVEL state;
    struct skin_rectangular_shape  zone;
    const struct skin_control * bat = skin_get_ctrl(SKIN_CMD_BATTERY_STATUS);

    PRINTDF("Display bat \n");
    if (bat != NULL){
        state = power_get_bat_state();
        if ((state != previous_state) ||
            (force == true )){
            zone = skin_ctrl_get_zone(bat);
            PRINTDF("New battery state : %i \n", state);
            draw_cursor(skin_get_img(SKIN_CMD_BATTERY_STATUS), state,zone.x1, zone.y1);
            previous_state = state;
        }
    }
}

static const char *get_string_gps_ctrl(struct gps_data *info, enum skin_cmd cmd){
    static char buff_text[32];   

    switch (cmd){
        case SKIN_CMD_TEXT_LAT :
            snprintf(buff_text,sizeof(buff_text),"%02i %02i", info->lat_deg, info->lat_mins);    
            break;
        case SKIN_CMD_TEXT_LONG :
            snprintf(buff_text,sizeof(buff_text),"%02i %02i", info->long_deg, info->long_mins);         
            break;            
        case SKIN_CMD_TEXT_ALT :
            snprintf(buff_text,sizeof(buff_text),"%04i m", info->alt_cm / 100); 
            break;            
        case SKIN_CMD_TEXT_SPEED :
            snprintf(buff_text,sizeof(buff_text),"%03i km/h", info->speed_kmh);
            break;
        default :
            return NULL;
    }
    return &buff_text[0];
}

static void refresh_gps_infos(void){
    const struct skin_control *ctrl;
    enum skin_cmd cmd;
    struct gps_data info;
    
    gps_get_data(&info);
    /* Handle GPS text tags */
    for (cmd = SKIN_CMD_GPS_FIRST; cmd <= SKIN_CMD_GPS_LAST; cmd++){
        ctrl = skin_get_ctrl(cmd);
        if (ctrl != NULL){            
            display_txt_ctrl(ctrl, get_string_gps_ctrl(&info, cmd));
        }
    }
    return;
}

static void refresh_time(void){
    char buff_text[32];
    time_t curr_time;
    struct tm * ptm;   
    const struct skin_control *ctrl;

    ctrl = skin_get_ctrl(SKIN_CMD_TEXT_TIME);
    if (ctrl != NULL){
        time(&curr_time);
        ptm = localtime(&curr_time);
        snprintf(buff_text, sizeof(buff_text), "%02d : %02d",ptm->tm_hour, ptm->tm_min);
        display_txt_ctrl(ctrl, buff_text);
    }
    return;
}

static double get_uptime(void){    
    int fd;
    char buffer[64];
    double secs = 0.0;
    
    fd = open("/proc/uptime", O_RDONLY);
    if (fd > 0){
        if (read(fd, buffer, sizeof(buffer)) > 0){
            sscanf(buffer, "%lf", &secs);
        }
        close(fd);
    }
    return secs;
}

static void refresh_uptime(void){
    const struct skin_control *ctrl;
    int hours, mins;
    double secs;
    char buffer[16];
    
    ctrl = skin_get_ctrl(SKIN_CMD_TEXT_UPTIME);
    if (ctrl != NULL){
        secs = get_uptime();
        hours = floor(secs / (double)3600.0);
        mins = floor((secs - ((double)hours * (double)3600.0))/(double)60.0);
        snprintf(buffer, sizeof(buffer), "%02i:%02i", hours, mins);
        display_txt_ctrl(ctrl, buffer);
    }    
}

void skin_display_refresh(enum skin_display_update type){        

    if (type == SKIN_DISPLAY_NEW_TRACK){
        /* We have to redraw the background for video on new track event
           coz mplayer does not keep overlay from one track to the other...
           For audio, it enables not to care about erasing tags and filename */
        draw_img(skin_get_background());
        refresh_tags_infos();   
        refresh_filename();
        refresh_battery_status(true);
    }
    
    /* Common treatments for SKIN_DISPLAY_NEW_TRACK and  SKIN_DISPLAY_PERIODIC */   
    refresh_progress_bar();
    refresh_battery_status(false);
    refresh_gps_infos();
    refresh_time();
    refresh_uptime();
    
    return;
}
