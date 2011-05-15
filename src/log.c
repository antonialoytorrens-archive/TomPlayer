/**
 * \file log.c
 * \brief This module implements logging functions
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

#include <stdio.h>
#include <stdarg.h>

#include "config.h"

#define LOG_FILENAME "tomlog.txt"

static FILE * log_file;

static const char * log_labels[]={
    "",
    "error",
    "warning",
    "info",
    "debug",
    "verbose"
};

static const char * lvl_2_str(enum log_level lvl){
    if ((lvl >= 0) && (lvl < LOG_LVL_NB))
        return log_labels[lvl];
    else 
        return "";
}

int log_init(void){
    log_file = fopen(LOG_FILENAME, "a+");
    return !(log_file == NULL);
}

int log_write(enum log_level lvl, const char * str, ...){
    va_list ap;
    int ret = 0;
    
    if (log_file == NULL)
        return -1;
    if (lvl <= config_get_log_level()){
        ret = fprintf(log_file, "%s : ", lvl_2_str(lvl));
        va_start(ap, str);
        ret += vfprintf(log_file, str, ap);
        va_end(ap);  
        fprintf(log_file, "\n");
        fflush(log_file);       
    }
    return ret;
}

int log_release(void){
    fclose(log_file);
    log_file = NULL;
    return 0;
}
