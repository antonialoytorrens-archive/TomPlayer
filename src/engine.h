/**
 * \file engine.h
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

#ifndef __ENGINE_H__
#define __ENGINE_H__

#include <stdbool.h>
#include "skin.h"

enum eng_mode{MODE_VIDEO, MODE_AUDIO, MODE_UNKNOWN};
      
int  eng_init(bool);
int  eng_release(void);
void eng_play(char * filename, int pos);
int  eng_ask_menu(void);
void eng_handle_cmd(int cmd, int p);
int  eng_select_ctrl(const struct skin_control * ctrl, bool state);
enum eng_mode eng_get_mode(void);

#endif /* __ENGINE_H__ */
