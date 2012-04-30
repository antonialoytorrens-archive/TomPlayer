/**
 * \file file_list.c
 * \author Stephan Rafin
 *
 * This module implements a selection file list object 
 *
 * It holds the filenames that match a pattern from a given folder
 * A state (selected) is associated with each regular file 
 * The file list can be in multiple or single selection mode
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



#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include "file_list.h"

/** File list used as an enumerator to provide file selection to the outside of the module */
struct _fl_handle {
  int entries_number;                          /**< Number of entries in filenames array */
  int current_idx;                             /**< Current index */
  char ** filenames;                           /**< Array holding filenames (full pathname) */
};

#define FILE_LIST_INC 32
/** Internal file list representation for the file selector 
  *  \note Do not use linked list to implement file list as there is no modification once the list is created
  */
struct file_list{
  bool multiple_select;                        /**< Is multiple selection allowed*/
  int entries_number;                          /**< Number of entries in the following arrays...*/  
  char * basename;                             /**< Folder basename */
  int last_selected;                           /**< Index of last selected item */
  int max_entries_number;                      /**< Maximum entries that can be stored in the object */

  char ** filenames;                           /**< Array of entries_number elements containing the filenames (including folders) */
  bool * is_selected;                          /**< Array of entries_number elements indicating whether the corresponding file is selected or not*/
  bool * is_folder;                            /**< Array of entries_number elements indicating whether the corresponding file is a folder or a regular file*/
};


/** Match string against a RE
 * \return true if match 
 */
static bool match(const char *string, const regex_t * re)
{
    return !regexec(re, string, (size_t) 0, NULL, 0);    
}


/** Find an entry in file list 
 *
 * \return index of the entry if found or index where it should be added to let the list sorted
 */
static int fl_find_pos(struct file_list *fl, const char * filename, bool is_folder, bool * find){
  int i;  
  int ret_cmp = 1;
  *find= false;

  if (!is_folder){
    i = fl->entries_number-1;
    while  ((i>=0) && (!fl->is_folder[i])){ 
      ret_cmp = strcmp(fl->filenames[i],filename);
      if (ret_cmp<=0)
        break;
      i--;
    }
  } else{
    i = 0 ;
    
    while  ((i<fl->entries_number) && (fl->is_folder[i])){ 
      ret_cmp = strcmp(fl->filenames[i],filename);
      if ( ret_cmp >= 0)
        break;
      i++;
    }
  }
  
  if (!is_folder){  
      i++;  
  }

  if (ret_cmp ==0){
    *find = true;
  }
  return i;
}

/** Insert a slot in file list 
 *
 * \warning  No check on file list size (has to be big enough). Only called by :fl_add() that ensures this... 
 */
static bool fl_shift(struct file_list *fl, int idx){
  int i ;
  for (i=fl->entries_number-1; i>=idx;i--){
     fl->is_folder[i+1] = fl->is_folder[i];
     fl->is_selected[i+1] = fl->is_selected[i];
     fl->filenames[i+1] = fl->filenames[i];
  }
  return true;
}

/** Add a filename to a file list
 *
 * Adds an entry to file lits. Itmay be a regular file or a folder.
 * File list is automatically allocated or reallocated to contain this new entry.
 *
 * \param[in] fl  file list object 
 * \param[in] filename  filename to add
 * \param[in] is_folder Explicit whether it is a folder or not
 *
 * \retval true success
 * \retval false Failure
 *
 * \note the file list is always sorted
*/
static bool fl_add(struct file_list *fl, char* filename, bool is_folder){
  int index;
  bool find;

  if (fl->max_entries_number <= fl->entries_number){
    fl->max_entries_number += FILE_LIST_INC;
    fl->filenames = realloc(fl->filenames, fl->max_entries_number * sizeof(*fl->filenames));
    fl->is_selected = realloc(fl->is_selected, fl->max_entries_number * sizeof(*fl->is_selected));
    fl->is_folder = realloc(fl->is_folder, fl->max_entries_number * sizeof(*fl->is_folder));
    if ((fl->filenames == NULL ) ||
        (fl->is_selected == NULL) ||
        ( fl->is_folder == NULL ) ) {
        return false;
    }
  }
  
  index = fl_find_pos(fl, filename, is_folder, &find);
  if (index < fl->entries_number){
    fl_shift(fl,index);
  }

  fl->entries_number++;
  fl->is_folder[index] = is_folder;
  fl->is_selected[index] = false;
  fl->filenames[index] = strdup(filename);
  return true;
}

/** Create a file list object 
 *
 * \param[in] path path from which the file list has to be built
 * \param[in] re compiled regular expression to use to filter filenames
 * \param[in] fl file list handle
 *
 * \return a file list or NULL if an rerror occured  
 */
file_list fl_create(const char * path, regex_t *re, bool mul){
  struct dirent* dir_ent;
  DIR*   dir;
  struct stat ftype;
  struct file_list * fl;
  char   fullpath [PATH_MAX + 1];
  
  fl = calloc(1, sizeof(*fl));
  if (fl == NULL) {
    return fl;
  }

  fl->multiple_select = mul;
  if ((dir = opendir (path)) == NULL)
    goto out_error;

  fl->basename = strdup(path);
  while ( (dir_ent = readdir ( dir )) != NULL ) {
          snprintf(fullpath,PATH_MAX,"%s/%s",path,  dir_ent->d_name);
          if (stat (fullpath, &ftype) < 0 ) {
            continue;
          }
          if (strcmp( dir_ent->d_name, ".")) {
            if ((S_ISDIR (ftype.st_mode)) ||
                (re == NULL) ||
                (match(dir_ent->d_name,re))){ 
              if (!fl_add(fl, dir_ent->d_name, S_ISDIR (ftype.st_mode))){
                goto out_error;                                
              }
            }
          }
  }
  closedir (dir);
  return fl;

  out_error:
    free(fl);
    return NULL;
}


/** Release file list object */
void fl_release(file_list fl){  
  int i ;

  if (fl == NULL) return; 
  for (i=0; i<fl->entries_number; i++){
    free(fl->filenames[i]);
  }
  free(fl->filenames);
  free(fl->is_selected);
  free(fl->is_folder);  
  free(fl->basename);  
  free(fl);
  return;
}

bool fl_select_by_pos(file_list fl, int idx, bool * change){
  
  if (fl == NULL) return false;
  if (change != NULL)
    *change = true;

  if ( (idx < 0) || (idx >= fl->entries_number)){
    return false;
  }
  if  (fl->is_folder[idx])
    return false;

  if ( fl->multiple_select ){
    fl->is_selected[idx] = !fl->is_selected[idx];
  } else {
    if (fl->is_selected[idx]){
      if (change != NULL)
        *change = false;
    } else {
      fl->is_selected[fl->last_selected] = false;
      fl->is_selected[idx] = true; 
      fl->last_selected = idx;    
    }
  }
  return true;
}

bool fl_select_all(file_list fl){
  int i;

  if (fl == NULL) return false;
  if (!fl->multiple_select)
    return false;

  for(i=0; i<fl->entries_number; i++){  
    if (!fl->is_selected[i])
      fl_select_by_pos(fl, i, NULL);       
  }

  return true;
}


const char * fl_get_single_selection(file_list fl){

  if (fl == NULL) return NULL;
  if (fl->multiple_select) {
    return NULL;
  }
  if (fl->is_selected[fl->last_selected]) {
    return fl->filenames[fl->last_selected];
  } else {
     return NULL;
  }     
}

bool fl_unselect_by_pos(file_list fl, int i){

  if (fl == NULL) return false;
  if ( (i < 0) || (i >= fl->entries_number)){
    return false;
  }
  fl->is_selected[i] = false;
  return true;
}


bool fl_is_selected(file_list fl, int i){
  if (fl == NULL) return false;
  return fl->is_selected[i];
}

bool fl_is_folder(file_list fl, int i){
  if (fl == NULL) return false;
  return fl->is_folder[i];
}

int fl_get_entries_nb(file_list fl){
  if (fl == NULL) return 0;
  return fl->entries_number;
}

const char * fl_get_filename(file_list fl, int i){
  if (fl == NULL) return NULL;
  return fl->filenames[i];
}

const char * fl_get_basename(file_list fl){
  if (fl == NULL) return NULL;
  return fl->basename;
}

/** Returns the number of selected items
 *
 * \param[in] fl Handle of the fl object
 *
 * \return the number of selected files
 *
 * \warning No thread safe as file list is not protected. But again, dont really care here...
 */
int fl_get_selected_number(const file_list fl){
  int i, nb_selected;
  nb_selected = 0;
  
  if (fl == NULL) return 0;
  for (i=0; i< fl->entries_number; i++){
    if (fl->is_selected[i]) nb_selected++;                 
  }

  return nb_selected;
}

/** Retrieve an enumerator from the file list object
 *
 * \param[in] fl Handle of the fl object
 *
 * \return the enumerator files list object containing the selected files or NULL if error
 */
flenum fl_get_selection(file_list fl){
  struct _fl_handle * fle;
  int i,j;

  if (fl == NULL) return NULL;
  fle = malloc(sizeof(struct _fl_handle));  
  if (fle != NULL){
    fle->entries_number = fl_get_selected_number(fl);
    fle->current_idx = 0;
    fle->filenames=malloc( fle->entries_number * sizeof(*fle->filenames) );
    if (fle->filenames == NULL){
      free(fle);
      fle=NULL;
    } else {
      j=0;
      for(i=0;i<fle->entries_number ;i++){
        while ((j<fl->entries_number) && (!fl->is_selected[j])){j++;}
        if (j<fl->entries_number){
          fle->filenames[i] = malloc(strlen(fl->basename) + strlen(fl->filenames[j]) + 2);
          sprintf(fle->filenames[i],"%s/%s",fl->basename,fl->filenames[j]);          
          j++;
        }
      }
    }
  }
  return fle;
}



/** Get next file from enumerator files list 
 *
 *\param[in] fl Handle on the enumerator files list  
 *\param[in] is_random Explicit whether we want to retrieve filenames in a random order or not
 *
 *\return the next filename or NULL if no more filename 
 */
const char * flenum_get_next_file(flenum fl, bool is_random){
  int i = fl->current_idx;

  if (fl == NULL) return NULL;
  if (fl->current_idx >= fl->entries_number){
    return NULL;
  }
  
  if (is_random){
    int rand_offset;
    char * temp;

    /* Choose random filename */
    rand_offset = (fl->entries_number - fl->current_idx) *  (((double) rand())  / (((double)(RAND_MAX)) + 1.0));
    
    /* Switch random remaining filename and i-th item */
    temp = fl->filenames[i + rand_offset];  
    fl->filenames[i + rand_offset] = fl->filenames[i];
    fl->filenames[i] = temp;
  }
  fl->current_idx++;  
  return fl->filenames[i];
}

/** Release a enumerator files list  object 
 *
 * \param[in] fl Handle on the selected file list
 *
 * \retval true success
 * \retval false Failure
 */
bool flenum_release(flenum fl){
  int i;

  if (fl == NULL) return false;
  for (i=0; i<fl->entries_number; i++){
    free(fl->filenames[i]);
  }
  free(fl->filenames);
  
  free(fl);
  return true;
}

