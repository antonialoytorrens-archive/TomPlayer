 /***************************************************************************
 *  
 *  Calibrate the TomTom with factory settings from flash
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

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <barcelona/Barc_ts.h>

static MATRIX cal_data = {
	.An = 1075057140,
	.Bn = 1074339800,
	.Cn = 0,
	.Dn = 73808,
	.En = 0,
	.Fn = 1075057140,
	.Divider = 1074270160,
	.xMin = 0,    // Stored on FLASH address 0x10c
	.xMax = 1023, // Stored on FLASH address 0x110
	.yMin = 0,    // Stored on FLASH address 0x104
	.yMax = 1023, // Stored on FLASH address 0x108
};

int main (){
  
  int fd_cal;
  int tsfd;
  int bytes_read;
  int temp;
  
  tsfd = open("/dev/" TS_DEVNAME, O_RDWR);
  if (tsfd < 0){
      perror ("Opening touchscreen failed ");
      exit (1);
  }

  if (ioctl(tsfd,TS_GET_CAL,&cal_data) != 0){
      perror ("Unable to retrieve ts calibration data ");
      exit (1);
  }  
/*
  printf("A : %li, B : %li, C : %li, D : %li , E : %li, F : %li, div : %li \n",cal_data.An,cal_data.Bn,cal_data.Cn,cal_data.Dn,cal_data.En,cal_data.Fn,cal_data.Divider);
  printf("xMin : %i, xMax : %i, yMin :  %i, yMax : %i\n",cal_data.xMin, cal_data.xMax, cal_data.yMin, cal_data.yMax);
*/
  fd_cal = open("/mnt/flash/sysfile/cal",O_RDONLY); 
  if (fd_cal < 0)
  {
      perror ("Open calibration file Failed ");
      exit (1);
  }
  bytes_read = read (fd_cal,&cal_data.xMin,sizeof(cal_data) - offsetof(MATRIX,xMin));
/*  printf("%i calibration bytes read \n", bytes_read);*/
  printf("xMin : %i, xMax : %i, yMin :  %i, yMax : %i\n",cal_data.xMin, cal_data.xMax, cal_data.yMin, cal_data.yMax);

  /* Don't know why but but swapping ymin and ymax seems necessary ! */
  temp = cal_data.yMin;
  cal_data.yMin = cal_data.yMax;
  cal_data.yMax = temp;

  if (ioctl(tsfd,TS_SET_CAL,&cal_data) != 0){
    perror ("Setting calibration failed ");
    exit (1);  
  }
  /* Tell driver ts is calibrated , seems to be useless... */
  ioctl(tsfd,TS_SET_RAW_OFF);

  close (fd_cal);
  close (tsfd);
  return 0;
}
