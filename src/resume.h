/**
 * \file resume.h
 * \author wolfgar
 * \brief Resume file handling
 *
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



#ifndef __TOMPLAYER_RESUME_H__
#define __TOMPLAYER_RESUME_H__
#include "engine.h"

/**
 * \struct video_settings
 * \brief structure to store video settings
 */
struct video_settings {
	int contrast;
	int brightness;
	float audio_delay;
	int volume; /* In fact mplayer property type is float but we always work with int on it*/
};


struct audio_settings {
	int volume; /* In fact mplayer property type is float but we always work with int on it*/
};


int resume_file_init(enum engine_mode);
int resume_write_pos(enum engine_mode mode, int value);
int resume_get_file_infos(enum engine_mode mode, char * filename, int len , int * pos);
int resume_get_audio_settings(struct audio_settings * settings);
int resume_get_video_settings(struct video_settings * settings);
int resume_set_audio_settings(const struct audio_settings * settings);
int resume_set_video_settings(const struct video_settings * settings);
int resume_save_playslist(enum engine_mode mode, const char * current_filename);

#endif
