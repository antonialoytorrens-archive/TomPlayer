/***************************************************************************
 *
 *  14.02.08 : wolfgar - Widescreen Handling 
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



#include <linux/fb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "widescreen.h"
#include "config.h"


/** Returns wether the device is equiped with a widescreen or not
*
*\retval 0 no widescreen 
*\retval 1 widescreen present
*/
int ws_probe(void){
  char * fb_dev;
  int fb_fd;
  struct fb_var_screeninfo screeninfo;
  int is_ws = 0;


  fb_dev = getenv("FRAMEBUFFER");
  if (fb_dev == NULL){
    fb_dev = "/dev/fb";
  } 

  fb_fd = open(fb_dev, O_RDONLY);
  if (fb_fd < 0){
    fprintf(stderr,"Error while trying to open frame buffer device %s \n", fb_dev);
    fprintf(stderr,"Assuming no widescreen \n");
  } else {
    if (ioctl (fb_fd, FBIOGET_VSCREENINFO, &screeninfo) != 0){
      fprintf(stderr,"Error while trying to get info on frame buffer device %s \n", fb_dev);
      fprintf(stderr,"Assuming no widescreen \n");
    } else {    
      if (screeninfo.xres == WS_XMAX){
        is_ws = 1;
      }
    }
  }

  return is_ws;
}
