/***************************************************************************
 *  19/06/2008
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


/*!
 * \file file_list.c
 * \brief file list utilities
 * \author nullpointer
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include "file_list.h"
#include "debug.h"
#include "engine.h"


/**
 * \fn int compare_file_elt( void * e1, void * e2 )
 * \brief Comparison of file_elt
 *
 * \param e1 first file_elt object
 * \param e1 second file_elt object
 */
int compare_file_elt( void * e1, void * e2 ){
	struct file_elt * f1;
	struct file_elt * f2;

	f1 = (struct file_elt *) e1;
	f2 = (struct file_elt *) e2;

	//PRINTDF( "compare_file_elt <%s> <%d> - <%s> <%d>\n", f1->name, f1->type, f2->name, f2->type );

	if( f1->type == f2->type ) return strcasecmp( f1->name, f2->name );
	else if( f1->type == TYPE_DIR ) return -1;
	else return 1;
}

/**
 * \fn void release_file_elt( void * e )
 * \brief release a file_elt object
 *
 * \param e adress of file_elt object to release
 */
void release_file_elt( void * e ){
	struct file_elt * f;

	PRINTD( "release_file_elt\n" );

	f = (struct file_elt *) e;
	if( f->name != NULL ) free( f->name );
}

/**
 * \fn bool fill_list_files( char * path, char * extension, struct list_object ** list_files )
 * \brief fill a list object with file_elt objects
 *
 * \param path base folder
 * \param extension list of extension of usable file
 * \param list_files list object to store file_elt
 *
 * \return true on success, false on failure
 */
bool fill_list_files( char * path, char * extension, struct list_object ** list_files ){
	struct dirent* dir_ent;
	DIR*   dir;
	struct stat ftype;
	char   fullpath [PATH_MAX + 1];
	struct file_elt * file_elt;

	PRINTD( "fill_list_files\n" );
	if ((dir = opendir (path)) == NULL)
		return false;


	while ( (dir_ent = readdir ( dir )) != NULL ) {
		strncpy (fullpath, path, PATH_MAX);
		strcat (fullpath, "/");
		strcat (fullpath, dir_ent->d_name);

		if (stat (fullpath, &ftype) < 0 ) continue;

		if (S_ISDIR (ftype.st_mode) && strcmp( dir_ent->d_name, ".") ){
			PRINTDF( "Add dir %s\n", dir_ent->d_name );

			file_elt = malloc( sizeof( struct file_elt ) );
			file_elt->name = strdup( dir_ent->d_name );
			file_elt->type = TYPE_DIR;

			add_to_list_sorted( list_files, file_elt, compare_file_elt );
		}
		else if (S_ISREG (ftype.st_mode) )
			if( has_extension(dir_ent->d_name, extension ) ){
				PRINTDF( "Add file %s\n", dir_ent->d_name );

				file_elt = malloc( sizeof( struct file_elt ) );
				file_elt->name = strdup( dir_ent->d_name );
				file_elt->type = TYPE_FILE;

				add_to_list_sorted( list_files, file_elt, compare_file_elt );
			}
	}

	closedir (dir);

	return true;
}

