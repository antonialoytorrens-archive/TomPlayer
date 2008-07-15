/***************************************************************************
 * Playlist utilities
 *
 *  Mon March 10 2008
 *  Copyright  2008  nullpointer
 *  Email nullpointer[at]lavabit[dot]com
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

/**
 * \file playlist.c
 * \author nullpointer
 * \brief playlist generation
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>


#include "config.h"
#include "engine.h"


/**
 * \fn bool generate_playlist( char * folder, char * filename )
 * \brief generate a playlist
 *
 * \param folder
 * \param filename filename of the playlist
 *
 * \return true on success, false on failure
 */
static bool generate_playlist( char * folder, char * filename )
{
    FILE * fp;
    struct dirent* dir_ent;
    DIR*   dir;
    struct stat ftype;
    char   fullpath [PATH_MAX + 1];

    if ((dir = opendir (folder)) == NULL)
         return false;

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

    return true;
}

/**
 * \fn static void get_line_file( FILE * fp, int n, char * buffer, int size  )
 * \brief seek to the specified line
 *
 * \param fp handle to the opened file
 * \param n the index of the line
 * \param buffer where to store the line
 * \param size size of the buffer string
 *
 */
static void get_line_file( FILE * fp, int n, char * buffer, int size  )
{
    int count = 0;

    rewind( fp );

    while( fgets( buffer, size, fp ) && count != n ) count++;
}


/**
 * \fn static void get_line_file( FILE * fp, int n, char * buffer, int size  )
 * \brief retrieve the number of line in a file
 *
 * \param fp handle to the opened file
 *
 * \return the number of line in the file
 */
static int get_nb_line_file( FILE * fp  )
{
    char buffer[1000];
    int count = 0;

    rewind( fp );

    while( fgets( buffer, sizeof( buffer ), fp ) )
        if( strlen( buffer ) > 0 ) count++;

    return count;

}

/**
 * \fn bool generate_random_playlist( char * folder, char * filename )
 * \brief generate a random playlist
 *
 * \param folder
 * \param filename filename of the playlist
 *
 * \return true on success, false on failure
 */
bool generate_random_playlist( char * folder, char * filename )
{
    FILE * fp, *fp_random;
    char filename_tmp[PATH_MAX+1];
    int count, i, nb;
    unsigned char * flags;
    char buffer[1000];

    strcpy( filename_tmp, filename );
    strcat( filename_tmp, ".tmp" );

    if( generate_playlist( folder, filename_tmp ) != true ) return false;


    fp_random = fopen( filename, "w+" );
    fp = fopen( filename_tmp, "r" );

    count = get_nb_line_file( fp );
    flags = (unsigned char * ) malloc( count * sizeof( unsigned char ) );
    if( flags == NULL ){
        fclose( fp );
        fclose( fp_random );
        return false;
    }

    memset( flags, 0, count * sizeof( unsigned char ) );

    nb = count;
    while( nb ){
        while( 1 ){
            i = rand() % count;
            if( flags[i] == 0 ) break;
        }

        flags[i] = 1;
        get_line_file( fp, i, buffer, sizeof( buffer ) );
        fputs( buffer, fp_random );
        nb--;
    }
    fclose( fp );
    fclose( fp_random );
    free( flags );
    unlink( filename_tmp );

    return true;
}
