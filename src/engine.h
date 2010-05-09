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
#include <stdbool.h>
#include "config.h"


extern struct tomplayer_config config;
extern bool is_mplayer_finished;

int init_engine( void );
int release_engine( void );

void send_command(const char * cmd );
void send_menu( char * cmd );

void show_menu( void );
void hide_menu( void );

void handle_mouse_event( int x, int y );
int get_command_from_xy( int x, int y, int * p );
void display_image_to_fb( ILuint  );
void launch_mplayer( char * , char * , int, bool );

#endif /* __ENGINE_H__ */
