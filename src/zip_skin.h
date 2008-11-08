/**
 * \file zip_skin.h
 * \author nullpointer
 * \brief handling of zipped skin
 *
 * $URL:$
 * $Rev:$
 * $Author:$
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

#ifndef __ZIP_SKIN_H__
#define __ZIP_SKIN_H__

#include "config.h"

#define ZIP_SKIN_BITMAP_FILENAME "/tmp/bitmap"

bool load_bitmap( ILuint * , char * );
bool load_skin_from_zip(const char * , struct skin_config * , bool );
bool unload_skin(  struct skin_config *  );

#endif /* __ZIP_SKIN_H__ */
