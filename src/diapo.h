/**
 * \file diapo.h
 * \author Stephan Rafin
 *
 * This module implements a slide show engine
 *
 * $URL: $
 * $Rev: $
 * $Author: $
 * $Date: $
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

#ifndef __DIAPO_H__
#define __DIAPO_H__

#include <stdbool.h>
 
struct diapo_config{
   const char *file_path; /**< Path to the images */
   const char *filter;    /**< Regex *filter */
   unsigned int delay;    /**< Delay between two images in seconds */
};

bool diapo_init(const  struct diapo_config * conf );
bool diapo_resume (void);
bool diapo_stop (void);
void diapo_release(void);

#endif
