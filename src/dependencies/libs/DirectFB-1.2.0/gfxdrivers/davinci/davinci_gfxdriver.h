/*
   TI Davinci driver - Graphics Driver

   (c) Copyright 2007  Telio AG

   Written by Denis Oliver Kropp <dok@directfb.org>

   Code is derived from VMWare driver.

   (c) Copyright 2001-2008  The world wide DirectFB Open Source Community (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __DAVINCI_GFXDRIVER_H__
#define __DAVINCI_GFXDRIVER_H__

#include <sys/ioctl.h>
#include <davincifb.h>

#include <core/surface_buffer.h>

#include "davincifb.h"

#include "davinci_c64x.h"


typedef struct {
     /* validation flags */
     int                       v_flags;

     /* cached/computed values */
     void                     *dst_addr;
     unsigned long             dst_phys;
     unsigned int              dst_size;
     unsigned long             dst_pitch;
     DFBSurfacePixelFormat     dst_format;
     unsigned long             dst_bpp;

     void                     *src_addr;
     unsigned long             src_phys;
     unsigned long             src_pitch;
     DFBSurfacePixelFormat     src_format;
     unsigned long             src_bpp;

     unsigned long             source_mult;

     unsigned long             fillcolor;

     int                       blit_blend_sub_function;
     int                       draw_blend_sub_function;

     DFBColor                  color;
     unsigned long             color_argb;
     unsigned long             colorkey;

     DFBSurfaceBlittingFlags   blitting_flags;

     DFBRegion                 clip;

     /** Add shared data here... **/
     struct fb_fix_screeninfo  fix[4];

     CoreSurfacePool          *osd_pool;
     CoreSurfacePool          *video_pool;

     bool                      synced;
} DavinciDeviceData;


typedef struct {
     int                       num;
     int                       fd;
     void                     *mem;
     int                       size;
} DavinciFB;

typedef struct {
     DavinciDeviceData        *ddev;

     CoreDFB                  *core;

     CoreScreen               *screen;
     CoreLayer                *osd;
     CoreLayer                *video;

     DavinciFB                 fb[4];

     DavinciC64x               c64x;
     bool                      c64x_present;

     DavinciC64xTasks          tasks;
} DavinciDriverData;


static inline DFBResult
davincifb_pan_display( const DavinciFB             *fb,
                       struct fb_var_screeninfo    *var,
                       const CoreSurfaceBufferLock *lock,
                       DFBSurfaceFlipFlags          flags,
                       int                          x,
                       int                          y )
{
     int ret;

     if (lock) {
#ifdef FBIO_SET_START
          CoreSurfaceBuffer   *buffer = lock->buffer;
          struct fb_set_start  set_start;

          /* physical mode */
          set_start.offset   = -1;
          set_start.sync     = (flags & DSFLIP_ONSYNC) ? 1 : 0;

          /* life's so easy */
          set_start.physical = lock->phys + DFB_BYTES_PER_LINE( buffer->format, x ) + y * lock->pitch;

          ret = ioctl( fb->fd, FBIO_SET_START, &set_start );
          if (ret < 0)
               D_DEBUG( "FBIO_SET_START (0x%08lx, sync %llu) failed!\n",
                         set_start.physical, set_start.sync );

          if (ret == 0) {
               if (flags & DSFLIP_WAIT)
                    ioctl( fb->fd, FBIO_WAITFORVSYNC );

               return DFB_OK;
          }

          /* fallback */
#endif
          var->xoffset = x;                  /* poor version */
          var->yoffset = y + lock->offset / lock->pitch;
     }
     else {
          var->xoffset = x;
          var->yoffset = y;
     }

     var->activate = /*(flags & DSFLIP_ONSYNC) ? FB_ACTIVATE_VBL :*/ FB_ACTIVATE_NOW;

     ret = ioctl( fb->fd, FBIOPAN_DISPLAY, var );
     if (ret)
          D_PERROR( "Davinci/FB: FBIOPAN_DISPLAY (fb%d - %d,%d) failed!\n",
                    fb->num, var->xoffset, var->yoffset );

     if (flags & DSFLIP_WAIT)
          ioctl( fb->fd, FBIO_WAITFORVSYNC );

     return DFB_OK;
}

#endif
