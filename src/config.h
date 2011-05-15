/**
 * \file config.h 
 * \brief This module implements configuration access
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


#ifndef __TOMPLAYER_CONFIG_H__
#define __TOMPLAYER_CONFIG_H__
#include <stdbool.h>
#include "log.h"

/** FIFO Where key inputs can be read from */
#define KEY_INPUT_FIFO "/tmp/key_fifo"

/** fullpath to the main config file*/
#define CONFIG_FILE  "/tmp/tomplaye.ini"

/** audio or video configuration type */
enum config_type {
  CONFIG_AUDIO,
  CONFIG_VIDEO
};

/** Speaker configuration type */
enum config_int_speaker_type{
  CONF_INT_SPEAKER_AUTO = 0,
  CONF_INT_SPEAKER_NO,
  CONF_INT_SPEAKER_ALWAYS,
  CONF_INT_SPEAKER_MAX
};


/** FM transmitter configuration identifier */
enum config_fm_type { 
  CONFIG_FM_DEFAULT, 
  CONFIG_FM_SAV1,
  CONFIG_FM_SAV2
};

bool config_init(void);
bool config_save(void);
void config_free(void);
void config_reload(void);

/* GET accessors */
const char  *config_get_folder(enum config_type type);
const char  *config_get_skin_filename(enum config_type type);
const char  *config_get_ext(enum config_type type);
bool         config_get_auto_resume(void);
int          config_get_screen_saver_to(void);
bool         config_get_screen_saver(void);
unsigned int config_get_fm(enum config_fm_type);
bool         config_get_fm_activation(void);
bool         config_get_small_text_activation(void);
bool         config_get_diapo_activation(void);
bool         config_get_video_preview(void);
enum config_int_speaker_type config_get_speaker(void);
const struct diapo_config *config_get_diapo(void);
enum log_level config_get_log_level(void);

/* SET accessors */
bool config_set_skin_filename(enum config_type type, const char * filename);
bool config_set_default_folder(enum config_type type, const char * folder);
bool config_set_screensaver_to(int delay);
bool config_set_fm_frequency(enum config_fm_type, unsigned int freq);
bool config_set_int_speaker(enum config_int_speaker_type mode);
bool config_toggle_enable_diapo(void);
bool config_set_diapo_folder(const char *folder);
bool config_set_diapo_delay(int delay);
bool config_toggle_screen_saver_state(void);
bool config_toggle_fm_transmitter_state(void);
bool config_toggle_small_text_state(void);
bool config_toggle_auto_resume(void);
    
#endif /* __TOMPLAYER_CONFIG_H__ */
