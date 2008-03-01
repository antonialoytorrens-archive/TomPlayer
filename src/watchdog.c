/***************************************************************************
 *
 *  
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "watchdog.h"


/** Periocally refresh the wdg */
static void  refresh_wdg(void){
  int fd;

  fd = open("/dev/watchdog", O_RDWR);
  
  if (fd < 0){
    fprintf(stderr,"unable to open watchdog device\n");
    return;
  }
  
  while (1){
    /*Write a single byte to refresh WDG */
    write(fd, "o",1);
    sleep(1);
  }  
}

/** Launch a process in charge of refreshing the wdg every seconds
*
*/
int main(int argc, char *argv[]) {  
  refresh_wdg();
  return 0;
}
