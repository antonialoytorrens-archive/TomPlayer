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


/** Adapt filename
*
*
*/
static void transform_filename(char * filename){
  char temp_buf[PATH_MAX];
  char * slash_pos;
  int slash_off;

  slash_pos=strrchr(filename,'/');
  if (slash_pos == NULL){ 
    slash_off = 0;
  } else {
    slash_off = (slash_pos - filename)+1;
  }

  memcpy(temp_buf, filename, slash_off);
  snprintf(&temp_buf[slash_off], PATH_MAX,"%s%s", WS_FILENAME_PREFIX, &filename[slash_off]);
  strncpy(filename, temp_buf, PATH_MAX);
}


/** Adapt controls coordinates 
*
*/
static void transform_controls(struct control ctl[]){
  int i;

  /* adapt coordinates */
  for (i=0; i<MAX_CONTROLS; i++){
    switch (ctl[i].type){
      case CIRCULAR_CONTROL :
        ctl[i].area.circular.x = ctl[i].area.circular.x*WS_XMAX/WS_NOXL_XMAX;
	ctl[i].area.circular.y = ctl[i].area.circular.y*WS_YMAX/WS_NOXL_YMAX;
	/* Most restrictive : not perfect , in fact on a ws, circles turn ellipses...	*/
	ctl[i].area.circular.r = ctl[i].area.circular.r*WS_YMAX/WS_NOXL_YMAX; 
      break;

      case RECTANGULAR_CONTROL :
      case PROGRESS_CONTROL_X :
      case PROGRESS_CONTROL_Y :
 	ctl[i].area.rectangular.x1 = ctl[i].area.rectangular.x1*WS_XMAX/WS_NOXL_XMAX;
	ctl[i].area.rectangular.y1 = ctl[i].area.rectangular.y1*WS_YMAX/WS_NOXL_YMAX;
	ctl[i].area.rectangular.x2 = ctl[i].area.rectangular.x2*WS_XMAX/WS_NOXL_XMAX;
	ctl[i].area.rectangular.y2 = ctl[i].area.rectangular.y2*WS_YMAX/WS_NOXL_YMAX;	
      break;

      default :
      break;
    }
  }

}


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

/** Modify configuration to be widescreen compliant
*
*\param[in,out]  conf the configuration to adapt for a widescreen
*
*\return None
*/
void ws_translate(struct gmplayer_config * conf){

  /* Prepend "ws_" to picture filenames */
  transform_filename(conf->bmp_loading_file);
  transform_filename(conf->bmp_exiting_file);
  transform_filename(conf->video_config.image_file);
  transform_filename(conf->audio_config.image_file);
  /* Adapt controls coordinates */
  transform_controls(conf->video_config.controls);
  transform_controls(conf->audio_config.controls);
  
}



