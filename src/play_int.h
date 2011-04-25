/**
 * \file play_int.h
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


#ifndef __PLAY_INT_H__

#include "resume.h"

enum playint_seek{
    PLAYINT_SEEK_REL,
    PLAYINT_SEEK_PERCENT,
    PLAYINT_SEEK_ABS,
};    

enum playint_vol{
    PLAYINT_VOL_REL,
    PLAYINT_VOL_ABS,
};    


void playint_init(void);
void playint_run(char *);
bool playint_is_running(void);
void playint_quit(void);
int  playint_wait_output(int timeout);
void playint_flush_stdout(void);
void playint_seek(int val, enum playint_seek type);
int  playint_get_artist(char *buffer, size_t len);
int  playint_get_title(char *buffer, size_t len);
int  playint_get_file_position_seconds(void);
int  playint_get_file_position_percent(void);
void playint_set_audio_settings(const struct audio_settings * settings);
void playint_set_video_settings(const struct video_settings * settings);
int  playint_get_audio_settings( struct audio_settings * settings);
int  playint_get_video_settings( struct video_settings * settings);
int  playint_get_file_length(void);
int  playint_get_filename(char *buffer, size_t len);
int  playint_get_path(char *buffer, size_t len);
void playint_mute(void);
void playint_pause(void);
bool playint_is_paused(void);
void playint_vol(int val, enum playint_vol type);
void playint_bright(int step);
void playint_contrast(int step);
void playint_skip(int step);
void playint_delay(double step);
void playint_osd(const char *text, int to);
void playint_display_time(void);


void playint_menu_show(void);
void playint_menu_hide(void);
void playint_menu_write(const unsigned char *buffer, size_t len);

      
#endif
