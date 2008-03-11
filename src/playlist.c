/***************************************************************************
 * Playlist utilities
 *
 *  Mon March 10 2008
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
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <minigui/common.h>
#include <minigui/minigui.h>

#include "config.h"
#include "engine.h"

BOOL generate_playlist( char * folder, char * filename )
{
    FILE * fp;
    struct dirent* dir_ent;
    DIR*   dir;
    struct stat ftype;
    char   fullpath [PATH_MAX + 1];

    if ((dir = opendir (folder)) == NULL)
         return FALSE;
    
    fp = fopen( filename, "w+" );

    while ( (dir_ent = readdir ( dir )) != NULL ) {
        strncpy (fullpath, folder, PATH_MAX);
        strcat (fullpath, "/");
        strcat (fullpath, dir_ent->d_name);
        
        if (stat (fullpath, &ftype) < 0 ) {
            continue;
        }

        if (S_ISREG (ftype.st_mode) ) 
            if( has_extension(dir_ent->d_name, config.filter_audio_ext ) ){
                fprintf( fp, fullpath );
                fprintf( fp, "\n" );
            }
    }

    closedir (dir);
    fclose(fp);

    return TRUE;
}

