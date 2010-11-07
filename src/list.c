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
 * \file list.c
 * \author nullpointer
 * \brief basic list manipulation
 */

#include <stdio.h>
#include <stdlib.h>
#include "list.h"
#include "debug.h"

/**
 * \fn int get_list_count( struct list_object * list )
 * \brief retrieve the number of element in the list
 *
 * \param list the list
 *
 * \return number of element in the list
 */
int get_list_count( struct list_object * list ){
	int i = 0;

	while( list != NULL ){
		i++;
		list = list->next;
	}

	return i;
}


/**
 * \fn void * get_object_from_list( struct list_object * list, int n )
 * \brief retrieve the number of element in the list
 *
 * \param list the list
 * \param n index of the object
 *
 * \return the nth object of the list
 */
void * get_object_from_list( struct list_object * list, int n ){
	int i = 0;

	while( list != NULL ){
		if( i == n ) return list->object;
		i++;
		list = list->next;
	}

	return NULL;
}

/**
 * \fn struct list_object * new_element_list( void )
 * \brief create a new initialized list_object
 *
 * \return the new object
 */
struct list_object * new_element_list( void ){
	struct list_object * element;

	PRINTD( "new_element_list\n" );
	element = malloc( sizeof( struct list_object ) );
	if( element != NULL ){
		element->next = NULL;
		element->object = NULL;
	}

	return element;
}

/**
 * \fn void add_to_list( struct list_object ** list, void * object )
 * \brief Add an object to a list
 *
 * \param list the list where the object has to be added
 * \param object the object to add in the list
 */
void add_to_list( struct list_object ** list, void * object ){
	struct list_object * last;
	struct list_object * new_elt;

	PRINTD( "add_to_list\n" );

	if( *list == NULL ){
		new_elt = new_element_list();
		new_elt->object = object;
		*list = new_elt;
	}
	else{
		last = *list;
		while( last->next!= NULL ) last = last->next;

		new_elt = new_element_list();
		last->next = new_elt;
		new_elt->object = object;
	}
}

/**
 * \fn void add_to_list_sorted( struct list_object ** list, void *object, int (*compare)( void *, void * ) )
 * \brief Add an object in a sorted list
 *
 * \param list the list where the object has to be added
 * \param object the object to add in the list
 * \param compare the comparison function
 */
void add_to_list_sorted( struct list_object ** list, void *object, int (*compare)( void *, void * ) ){
	struct list_object * current_list;
	struct list_object * previous_list;
	struct list_object * new_elt;

	current_list = *list;
	previous_list = current_list;

	new_elt = new_element_list();
	new_elt->object = object;

	while( current_list != NULL ){
		if( compare( object, current_list->object ) < 0 ){
			if( current_list == *list ){
				new_elt->next = current_list;
				*list = new_elt;
			}
			else{
				previous_list->next = new_elt;
				new_elt->next = current_list;
			}
			return;
		}
		previous_list = current_list;
		current_list = current_list->next;
	}
	add_to_list( list, object );
}

/**
 * \fn void release_list( struct list_object * list, void (*release_object)(void *) )
 * \brief release a list
 *
 * \param list the list
 * \release_object the function to use to destroy an object. Can be NULL
 */
void release_list( struct list_object * list, void (*release_object)(void *) ){
	struct list_object * current;
	struct list_object * next;

	PRINTD( "release_list\n");

	if( list == NULL ) return;

	next = list;

	do{
		current = next;
		next = current->next;

		if( current->object != NULL){
			if( release_object == NULL ) free( current->object );
			else release_object( current->object );
		}

		free( current );

	}while( next != NULL );
}

