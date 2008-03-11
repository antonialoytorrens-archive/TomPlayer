/***************************************************************************
 *            engine.h
 *
 *  Mon Feb 11 22:30:35 2008
 *  Copyright  2008  nullpointer
 *  Email
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
 
#ifndef __ENGINE_H__
#define __ENGINE_H__


#ifndef BOOL
#define BOOL int
#define TRUE 1
#define FALSE 0
#endif

/* List of available commands */
#define CMD_EXIT_MENU               0
#define CMD_PAUSE                   1
#define CMD_STOP                    2
#define CMD_MUTE                    3
#define CMD_VOL_MOINS               4
#define CMD_VOL_PLUS                5
#define CMD_LIGHT_MOINS             6
#define CMD_LIGHT_PLUS              7
#define CMD_DELAY_MOINS             8
#define CMD_DELAY_PLUS              9
#define CMD_GAMMA_MOINS             10
#define CMD_GAMMA_PLUS              11
#define CMD_FORWARD                 12
#define CMD_BACKWARD                13


extern struct tomplayer_config config;
extern BOOL is_menu_showed;
extern BOOL is_mplayer_finished;
extern BOOL is_playing_video;
extern BOOL is_playing_audio;


char * get_file_extension( char * file );
BOOL is_video_file( char * file );
BOOL is_audio_file( char * file );
BOOL has_extension( char * file, char * extensions );

void send_command(const char * cmd );
void send_menu( char * cmd );

void show_menu( void );
void hide_menu( void );

void blit_video_menu( int fifo, struct skin_config * conf );

void * mplayer_thread(void *cmd);
void launch_mplayer( char * folder, char * filename, int pos );


void handle_mouse_event( int x, int y );
int get_command_from_xy( int x, int y, int * p );

/* This function is GUI library dependent, it's not defined in engine.c but in gui.c */
void gui_buffer_rgb(char * buffer,int width, int height, int x, int y);

#endif /* __ENGINE_H__ */
