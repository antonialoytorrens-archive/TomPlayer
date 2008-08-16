/*
   TI Davinci driver - C64X+ DSP Library

   (c) Copyright 2008  directfb.org
   (c) Copyright 2007  Telio AG

   Written by Denis Oliver Kropp <dok@directfb.org> and
              Olaf Dreesen <olaf@directfb.org>.

   All rights reserved.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public
   License along with this library; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __DAVINCI_C64X_H__
#define __DAVINCI_C64X_H__

#include <unistd.h>

#include <directfb.h>

#include <direct/messages.h>
#include <direct/trace.h>

#include <linux/c64x.h>

#define mb() __asm__ __volatile__ ("" : : : "memory")

/**********************************************************************************************************************/

typedef struct {
     int                 magic;

     int                 fd;
     c64xTaskControl    *ctl;
     void               *mem;

     c64xTask           *QueueL;
} DavinciC64x;

typedef struct {
     int                 magic;
     unsigned int        max_tasks;
     unsigned int        num_tasks;
     c64xTask           *tasks;
} DavinciC64xTasks;

typedef enum {
     C64X_TEF_NONE       = 0x0000,
     C64X_TEF_RESET      = 0x0001
} DavinciC64xEmitFlags;

/**********************************************************************************************************************/

DFBResult davinci_c64x_open    ( DavinciC64x *c64x );

DFBResult davinci_c64x_close   ( DavinciC64x *c64x );

DFBResult davinci_c64x_wait_low( DavinciC64x *c64x );

/**********************************************************************************************************************/

DFBResult davinci_c64x_tasks_init   ( DavinciC64xTasks *tasks,
                                      unsigned int      size );

DFBResult davinci_c64x_tasks_destroy( DavinciC64xTasks *tasks );

/**********************************************************************************************************************/

DFBResult davinci_c64x_emit_tasks( DavinciC64x          *c64x,
                                   DavinciC64xTasks     *tasks,
                                   DavinciC64xEmitFlags  flags );

/**********************************************************************************************************************/

static const char *state_names[] = { "DONE", "ERROR", "TODO", "RUNNING" };

static inline c64xTask *
c64x_get_task( DavinciC64x *c64x )
{
     c64xTaskControl *ctl   = c64x->ctl;
     uint32_t         idx   = ctl->QL_arm;
     uint32_t         next  = (idx + 1) & C64X_QUEUE_MASK;
     c64xTask        *task  = &c64x->QueueL[idx];
     int              loops = 0;
     uint32_t         idle  = 0;

     /* Wait for the entry (and next) to be processed by the DSP (rare case). */
     while (task->c64x_flags & C64X_FLAG_TODO || ctl->QL_dsp == next) {
          if (loops > 666 || (idle && ctl->idlecounter - idle > 666)) {
               c64xTask *dsp_task = &c64x->QueueL[ctl->QL_dsp];

               D_PERROR( "Davinci/C64X+: Blocked! [DSP %d / %d (%s), ARM %d / %d (%s)]\n",
                         ctl->QL_dsp,
                         (dsp_task->c64x_function >> 2) & 0x3fff,
                         state_names[dsp_task->c64x_function & 3],
                         ctl->QL_arm,
                         (task->c64x_function >> 2) & 0x3fff,
                         state_names[task->c64x_function & 3] );

               break;
          }

          idle = ctl->idlecounter;

          /* Queue is full, waiting 10-20ms should not be too bad. */
          if (loops++ > 10)
               usleep( 5000 );
     }

     return task;
}

static inline void
c64x_submit_task( DavinciC64x *c64x, c64xTask *task )
{
     c64xTaskControl *ctl  = c64x->ctl;
     uint32_t         idx  = ctl->QL_arm;
     uint32_t         next = (idx + 1) & C64X_QUEUE_MASK;

     mb();

     ctl->QL_arm = next;

     mb();
}

/**********************************************************************************************************************/

static inline void
davinci_c64x_wb_inv_range( DavinciC64x   *c64x,
                           unsigned long  start,
                           u32            length,
                           u32            func )
{
     c64xTask *task = c64x_get_task( c64x );

     task->c64x_arg[0] = start;
     task->c64x_arg[1] = length;
     task->c64x_arg[2] = func;

     task->c64x_function = C64X_WB_INV_RANGE | C64X_FLAG_TODO;

     c64x_submit_task( c64x, task );
}

static inline void
davinci_c64x_write_back_all( DavinciC64x *c64x )
{
     c64xTask *task = c64x_get_task( c64x );

     task->c64x_function = C64X_WRITE_BACK_ALL | C64X_FLAG_TODO;

     c64x_submit_task( c64x, task );
}

/**********************************************************************************************************************/

static inline void
davinci_c64x_load_block__L( DavinciC64xTasks *tasks,
                            unsigned long     words,
                            u32               num,
                            u32               flags )
{
     c64xTask *task = &tasks->tasks[tasks->num_tasks];

     D_ASSERT( tasks->num_tasks < tasks->max_tasks );

     task->c64x_arg[0] = words;
     task->c64x_arg[1] = num;
     task->c64x_arg[2] = flags;

     task->c64x_function = C64X_LOAD_BLOCK | C64X_FLAG_TODO;

     tasks->num_tasks++;
}

static inline void
davinci_c64x_load_block( DavinciC64x   *c64x,
                         unsigned long  words,
                         u32            num,
                         u32            flags )
{
     c64xTask *task = c64x_get_task( c64x );

     task->c64x_arg[0] = words;
     task->c64x_arg[1] = num;
     task->c64x_arg[2] = flags;

     task->c64x_function = C64X_LOAD_BLOCK | C64X_FLAG_TODO;

     c64x_submit_task( c64x, task );
}

static inline void
davinci_c64x_fetch_uyvy( DavinciC64x   *c64x,
                         unsigned long  dest,
                         unsigned long  source,
                         u32            pitch,
                         u32            height,
                         u32            flags )
{
     c64xTask *task = c64x_get_task( c64x );

     task->c64x_arg[0] = dest;
     task->c64x_arg[1] = source;
     task->c64x_arg[2] = pitch;
     task->c64x_arg[3] = height;
     task->c64x_arg[4] = flags;

     task->c64x_function = C64X_FETCH_UYVY | C64X_FLAG_TODO;

     c64x_submit_task( c64x, task );
}

static inline void
davinci_c64x_mc( DavinciC64x   *c64x,
                 unsigned long  dest,
                 u32            dpitch,
                 unsigned long  source0,
                 unsigned long  source1,
                 u32            spitch,
                 u32            height,
                 int            func )
{
     c64xTask *task = c64x_get_task( c64x );

     task->c64x_arg[0] = dest;
     task->c64x_arg[1] = dpitch;
     task->c64x_arg[2] = source0;
     task->c64x_arg[3] = source1;
     task->c64x_arg[4] = spitch;
     task->c64x_arg[5] = height;

     task->c64x_function = func | C64X_FLAG_TODO;

     c64x_submit_task( c64x, task );
}

static inline void
davinci_c64x_put_idct_uyvy_16x16__L( DavinciC64xTasks *tasks,
                                     unsigned long     dest,
                                     u32               pitch,
                                     u32               flags )
{
     c64xTask *task = &tasks->tasks[tasks->num_tasks];

     D_ASSERT( tasks->num_tasks < tasks->max_tasks );

     task->c64x_arg[0] = dest;
     task->c64x_arg[1] = pitch;
     task->c64x_arg[2] = flags;

     task->c64x_function = C64X_PUT_IDCT_UYVY_16x16 | C64X_FLAG_TODO;

     tasks->num_tasks++;
}

static inline void
davinci_c64x_put_idct_uyvy_16x16( DavinciC64x   *c64x,
                                  unsigned long  dest,
                                  u32            pitch,
                                  u32            flags )
{
     c64xTask *task = c64x_get_task( c64x );

     task->c64x_arg[0] = dest;
     task->c64x_arg[1] = pitch;
     task->c64x_arg[2] = flags;

     task->c64x_function = C64X_PUT_IDCT_UYVY_16x16 | C64X_FLAG_TODO;

     c64x_submit_task( c64x, task );
}

static inline void
davinci_c64x_put_mc_uyvy_16x16__L( DavinciC64xTasks *tasks,
                                   unsigned long     dest,
                                   u32               pitch,
                                   u32               flags )
{
     c64xTask *task = &tasks->tasks[tasks->num_tasks];

     D_ASSERT( tasks->num_tasks < tasks->max_tasks );

     task->c64x_arg[0] = dest;
     task->c64x_arg[1] = pitch;
     task->c64x_arg[2] = flags;

     task->c64x_function = C64X_PUT_MC_UYVY_16x16 | C64X_FLAG_TODO;

     tasks->num_tasks++;
}

static inline void
davinci_c64x_put_mc_uyvy_16x16( DavinciC64x   *c64x,
                                unsigned long  dest,
                                u32            pitch,
                                u32            flags )
{
     c64xTask *task = c64x_get_task( c64x );

     task->c64x_arg[0] = dest;
     task->c64x_arg[1] = pitch;
     task->c64x_arg[2] = flags;

     task->c64x_function = C64X_PUT_MC_UYVY_16x16 | C64X_FLAG_TODO;

     c64x_submit_task( c64x, task );
}

static inline void
davinci_c64x_put_sum_uyvy_16x16__L( DavinciC64xTasks *tasks,
                                    unsigned long     dest,
                                    u32               pitch,
                                    u32               flags )
{
     c64xTask *task = &tasks->tasks[tasks->num_tasks];

     D_ASSERT( tasks->num_tasks < tasks->max_tasks );

     task->c64x_arg[0] = dest;
     task->c64x_arg[1] = pitch;
     task->c64x_arg[2] = flags;

     task->c64x_function = C64X_PUT_SUM_UYVY_16x16 | C64X_FLAG_TODO;

     tasks->num_tasks++;
}

static inline void
davinci_c64x_put_sum_uyvy_16x16( DavinciC64x   *c64x,
                                 unsigned long  dest,
                                 u32            pitch,
                                 u32            flags )
{
     c64xTask *task = c64x_get_task( c64x );

     task->c64x_arg[0] = dest;
     task->c64x_arg[1] = pitch;
     task->c64x_arg[2] = flags;

     task->c64x_function = C64X_PUT_SUM_UYVY_16x16 | C64X_FLAG_TODO;

     c64x_submit_task( c64x, task );
}

static inline void
davinci_c64x_dva_begin_frame__L( DavinciC64xTasks *tasks,
                                 u32               pitch,
                                 unsigned long     current,
                                 unsigned long     past,
                                 unsigned long     future,
                                 u32               flags )
{
     c64xTask *task = &tasks->tasks[tasks->num_tasks];

     D_ASSERT( tasks->num_tasks < tasks->max_tasks );

     task->c64x_arg[0] = pitch;
     task->c64x_arg[1] = current;
     task->c64x_arg[2] = past;
     task->c64x_arg[3] = future;
     task->c64x_arg[4] = flags;

     task->c64x_function = C64X_DVA_BEGIN_FRAME | C64X_FLAG_TODO;

     tasks->num_tasks++;
}

static inline void
davinci_c64x_dva_begin_frame( DavinciC64x   *c64x,
                              u32            pitch,
                              unsigned long  current,
                              unsigned long  past,
                              unsigned long  future,
                              u32            flags )
{
     c64xTask *task = c64x_get_task( c64x );

     task->c64x_arg[0] = pitch;
     task->c64x_arg[1] = current;
     task->c64x_arg[2] = past;
     task->c64x_arg[3] = future;
     task->c64x_arg[4] = flags;

     task->c64x_function = C64X_DVA_BEGIN_FRAME | C64X_FLAG_TODO;

     c64x_submit_task( c64x, task );
}

static inline void
davinci_c64x_dva_motion_block__L( DavinciC64xTasks *tasks,
                                  unsigned long     macroblock )
{
     c64xTask *task = &tasks->tasks[tasks->num_tasks];

     D_ASSERT( tasks->num_tasks < tasks->max_tasks );

     task->c64x_arg[0] = macroblock;

     task->c64x_function = C64X_DVA_MOTION_BLOCK | C64X_FLAG_TODO;

     tasks->num_tasks++;
}

static inline void
davinci_c64x_dva_motion_block( DavinciC64x   *c64x,
                               unsigned long  macroblock )
{
     c64xTask *task = c64x_get_task( c64x );

     task->c64x_arg[0] = macroblock;

     task->c64x_function = C64X_DVA_MOTION_BLOCK | C64X_FLAG_TODO;

     c64x_submit_task( c64x, task );
}

/**********************************************************************************************************************/

static inline void
davinci_c64x_dva_idct( DavinciC64x   *c64x,
				   unsigned long  dest,
				   u32            pitch,
				   unsigned long  source )
{
     c64xTask *task = c64x_get_task( c64x );

     task->c64x_arg[0] = dest;
     task->c64x_arg[1] = pitch;
     task->c64x_arg[2] = source;

     task->c64x_function = C64X_DVA_IDCT | C64X_FLAG_TODO;

     c64x_submit_task( c64x, task );
}

/**********************************************************************************************************************/

static inline void
davinci_c64x_put_uyvy_16x16( DavinciC64x   *c64x,
                             unsigned long  dest,
                             u32            pitch,
                             unsigned long  source,
                             u32            flags )
{
     c64xTask *task = c64x_get_task( c64x );

     task->c64x_arg[0] = dest;
     task->c64x_arg[1] = pitch;
     task->c64x_arg[2] = source;
     task->c64x_arg[3] = flags;

     task->c64x_function = C64X_PUT_UYVY_16x16 | C64X_FLAG_TODO;

     c64x_submit_task( c64x, task );
}

static inline void
davinci_c64x_dither_argb__L( DavinciC64xTasks *tasks,
                             unsigned long     dst_rgb,
                             unsigned long     dst_alpha,
                             u32               dst_pitch,
                             unsigned long     source,
                             u32               src_pitch,
                             u32               width,
                             u32               height )
{
     c64xTask *task = &tasks->tasks[tasks->num_tasks];

     D_ASSERT( tasks->num_tasks < tasks->max_tasks );

     task->c64x_arg[0] = dst_rgb;
     task->c64x_arg[1] = dst_alpha;
     task->c64x_arg[2] = dst_pitch;
     task->c64x_arg[3] = source;
     task->c64x_arg[4] = src_pitch;
     task->c64x_arg[5] = width;
     task->c64x_arg[6] = height;

     task->c64x_function = C64X_DITHER_ARGB | C64X_FLAG_TODO;

     tasks->num_tasks++;
}

static inline void
davinci_c64x_dither_argb( DavinciC64x   *c64x,
                          unsigned long  dst_rgb,
                          unsigned long  dst_alpha,
                          u32            dst_pitch,
                          unsigned long  source,
                          u32            src_pitch,
                          u32            width,
                          u32            height )
{
     c64xTask *task = c64x_get_task( c64x );

     task->c64x_arg[0] = dst_rgb;
     task->c64x_arg[1] = dst_alpha;
     task->c64x_arg[2] = dst_pitch;
     task->c64x_arg[3] = source;
     task->c64x_arg[4] = src_pitch;
     task->c64x_arg[5] = width;
     task->c64x_arg[6] = height;

     task->c64x_function = C64X_DITHER_ARGB | C64X_FLAG_TODO;

     c64x_submit_task( c64x, task );
}

static inline void
davinci_c64x_fill_16__L( DavinciC64xTasks *tasks,
                         unsigned long     dest,
                         u32               pitch,
                         u32               width,
                         u32               height,
                         u32               value )
{
     c64xTask *task = &tasks->tasks[tasks->num_tasks];

     D_ASSERT( tasks->num_tasks < tasks->max_tasks );

     task->c64x_arg[0] = dest;
     task->c64x_arg[1] = pitch;
     task->c64x_arg[2] = width;
     task->c64x_arg[3] = height;
     task->c64x_arg[4] = value;

     task->c64x_function = C64X_FILL_16 | C64X_FLAG_TODO;

     tasks->num_tasks++;
}

static inline void
davinci_c64x_fill_16( DavinciC64x   *c64x,
                      unsigned long  dest,
                      u32            pitch,
                      u32            width,
                      u32            height,
                      u32            value )
{
     c64xTask *task = c64x_get_task( c64x );

     task->c64x_arg[0] = dest;
     task->c64x_arg[1] = pitch;
     task->c64x_arg[2] = width;
     task->c64x_arg[3] = height;
     task->c64x_arg[4] = value;

     task->c64x_function = C64X_FILL_16 | C64X_FLAG_TODO;

     c64x_submit_task( c64x, task );
}

static inline void
davinci_c64x_fill_32__L( DavinciC64xTasks *tasks,
                         unsigned long     dest,
                         u32               pitch,
                         u32               width,
                         u32               height,
                         u32               value )
{
     c64xTask *task = &tasks->tasks[tasks->num_tasks];

     D_ASSERT( tasks->num_tasks < tasks->max_tasks );

     task->c64x_arg[0] = dest;
     task->c64x_arg[1] = pitch;
     task->c64x_arg[2] = width;
     task->c64x_arg[3] = height;
     task->c64x_arg[4] = value;

     task->c64x_function = C64X_FILL_32 | C64X_FLAG_TODO;

     tasks->num_tasks++;
}

static inline void
davinci_c64x_fill_32( DavinciC64x   *c64x,
                      unsigned long  dest,
                      u32            pitch,
                      u32            width,
                      u32            height,
                      u32            value )
{
     c64xTask *task = c64x_get_task( c64x );

     task->c64x_arg[0] = dest;
     task->c64x_arg[1] = pitch;
     task->c64x_arg[2] = width;
     task->c64x_arg[3] = height;
     task->c64x_arg[4] = value;

     task->c64x_function = C64X_FILL_32 | C64X_FLAG_TODO;

     c64x_submit_task( c64x, task );
}

static inline void
davinci_c64x_blit_16__L( DavinciC64xTasks *tasks,
                         unsigned long     dest,
                         u32               dpitch,
                         unsigned long     src,
                         u32               spitch,
                         u32               width,
                         u32               height )
{
     c64xTask *task = &tasks->tasks[tasks->num_tasks];

     D_ASSERT( tasks->num_tasks < tasks->max_tasks );

     task->c64x_arg[0] = dest;
     task->c64x_arg[1] = dpitch;
     task->c64x_arg[2] = src;
     task->c64x_arg[3] = spitch;
     task->c64x_arg[4] = width;
     task->c64x_arg[5] = height;

     task->c64x_function = C64X_COPY_16 | C64X_FLAG_TODO;

     tasks->num_tasks++;
}

static inline void
davinci_c64x_blit_16( DavinciC64x   *c64x,
                      unsigned long  dest,
                      u32            dpitch,
                      unsigned long  src,
                      u32            spitch,
                      u32            width,
                      u32            height )
{
     c64xTask *task = c64x_get_task( c64x );

     task->c64x_arg[0] = dest;
     task->c64x_arg[1] = dpitch;
     task->c64x_arg[2] = src;
     task->c64x_arg[3] = spitch;
     task->c64x_arg[4] = width;
     task->c64x_arg[5] = height;

     task->c64x_function = C64X_COPY_16 | C64X_FLAG_TODO;

     c64x_submit_task( c64x, task );
}

static inline void
davinci_c64x_blit_32__L( DavinciC64xTasks *tasks,
                         unsigned long     dest,
                         u32               dpitch,
                         unsigned long     src,
                         u32               spitch,
                         u32               width,
                         u32               height )
{
     c64xTask *task = &tasks->tasks[tasks->num_tasks];

     D_ASSERT( tasks->num_tasks < tasks->max_tasks );

     task->c64x_arg[0] = dest;
     task->c64x_arg[1] = dpitch;
     task->c64x_arg[2] = src;
     task->c64x_arg[3] = spitch;
     task->c64x_arg[4] = width;
     task->c64x_arg[5] = height;

     task->c64x_function = C64X_COPY_32 | C64X_FLAG_TODO;

     tasks->num_tasks++;
}

static inline void
davinci_c64x_blit_32( DavinciC64x   *c64x,
                      unsigned long  dest,
                      u32            dpitch,
                      unsigned long  src,
                      u32            spitch,
                      u32            width,
                      u32            height )
{
     c64xTask *task = c64x_get_task( c64x );

     task->c64x_arg[0] = dest;
     task->c64x_arg[1] = dpitch;
     task->c64x_arg[2] = src;
     task->c64x_arg[3] = spitch;
     task->c64x_arg[4] = width;
     task->c64x_arg[5] = height;

     task->c64x_function = C64X_COPY_32 | C64X_FLAG_TODO;

     c64x_submit_task( c64x, task );
}

static inline void
davinci_c64x_stretch_32__L( DavinciC64xTasks *tasks,
                            unsigned long     dest,
                            u32               dpitch,
                            unsigned long     src,
                            u32               spitch,
                            u32               dw,
                            u32               dh,
                            u32               sw,
                            u32               sh,
                            const DFBRegion  *clip )
{
     c64xTask *task = &tasks->tasks[tasks->num_tasks];

     D_ASSERT( tasks->num_tasks < tasks->max_tasks );

     task->c64x_arg[0] = dest;
     task->c64x_arg[1] = src;
     task->c64x_arg[2] = dpitch   | (spitch   << 16);
     task->c64x_arg[3] = dh       | (dw       << 16);
     task->c64x_arg[4] = sh       | (sw       << 16);
     task->c64x_arg[5] = clip->x2 | (clip->y2 << 16);
     task->c64x_arg[6] = clip->x1 | (clip->y1 << 16);

     if (sw > dw && sh > dh)
          task->c64x_function = C64X_STRETCH_32_down | C64X_FLAG_TODO;
     else
          task->c64x_function = C64X_STRETCH_32_up | C64X_FLAG_TODO;

     tasks->num_tasks++;
}

static inline void
davinci_c64x_stretch_32( DavinciC64x     *c64x,
                         unsigned long    dest,
                         u32              dpitch,
                         unsigned long    src,
                         u32              spitch,
                         u32              dw,
                         u32              dh,
                         u32              sw,
                         u32              sh,
                         const DFBRegion *clip )
{
     c64xTask *task = c64x_get_task( c64x );

     task->c64x_arg[0] = dest;
     task->c64x_arg[1] = src;
     task->c64x_arg[2] = dpitch   | (spitch   << 16);
     task->c64x_arg[3] = dh       | (dw       << 16);
     task->c64x_arg[4] = sh       | (sw       << 16);
     task->c64x_arg[5] = clip->x2 | (clip->y2 << 16);
     task->c64x_arg[6] = clip->x1 | (clip->y1 << 16);

     if (sw > dw && sh > dh)
          task->c64x_function = C64X_STRETCH_32_down | C64X_FLAG_TODO;
     else
          task->c64x_function = C64X_STRETCH_32_up | C64X_FLAG_TODO;

     c64x_submit_task( c64x, task );
}

static inline void
davinci_c64x_blit_blend_32__L( DavinciC64xTasks *tasks,
                               u32               sub_func,
                               unsigned long     dest,
                               u32               dpitch,
                               unsigned long     src,
                               u32               spitch,
                               u32               width,
                               u32               height,
                               u32               argb,
                               u8                alpha )
{
     c64xTask *task = &tasks->tasks[tasks->num_tasks];

     D_ASSERT( tasks->num_tasks < tasks->max_tasks );

     task->c64x_arg[0] = dest;
     task->c64x_arg[1] = dpitch;
     task->c64x_arg[2] = src;
     task->c64x_arg[3] = spitch;
     task->c64x_arg[4] = width | (height << 16);
     task->c64x_arg[5] = argb;
     task->c64x_arg[6] = alpha;

     task->c64x_function = (sub_func << 16) | C64X_BLEND_32 | C64X_FLAG_TODO;

     tasks->num_tasks++;
}

static inline void
davinci_c64x_blit_blend_32( DavinciC64x   *c64x,
                            u32            sub_func,
                            unsigned long  dest,
                            u32            dpitch,
                            unsigned long  src,
                            u32            spitch,
                            u32            width,
                            u32            height,
                            u32            argb,
                            u8             alpha )
{
     c64xTask *task = c64x_get_task( c64x );

     task->c64x_arg[0] = dest;
     task->c64x_arg[1] = dpitch;
     task->c64x_arg[2] = src;
     task->c64x_arg[3] = spitch;
     task->c64x_arg[4] = width | (height << 16);
     task->c64x_arg[5] = argb;
     task->c64x_arg[6] = alpha;

     task->c64x_function = (sub_func << 16) | C64X_BLEND_32 | C64X_FLAG_TODO;

     c64x_submit_task( c64x, task );
}

static inline void
davinci_c64x_blit_keyed_16__L( DavinciC64xTasks *tasks,
                               unsigned long     dest,
                               u32               dpitch,
                               unsigned long     src,
                               u32               spitch,
                               u32               width,
                               u32               height,
                               u32               key,
                               u32               mask )
{
     c64xTask *task = &tasks->tasks[tasks->num_tasks];

     D_ASSERT( tasks->num_tasks < tasks->max_tasks );

     task->c64x_arg[0] = dest;
     task->c64x_arg[1] = (dpitch << 16) | (spitch & 0xffff);
     task->c64x_arg[2] = src;
     task->c64x_arg[3] = width;
     task->c64x_arg[4] = height;
     task->c64x_arg[5] = key;
     task->c64x_arg[6] = mask;

     task->c64x_function = C64X_COPY_KEYED_16 | C64X_FLAG_TODO;

     tasks->num_tasks++;
}

static inline void
davinci_c64x_blit_keyed_16( DavinciC64x   *c64x,
                            unsigned long  dest,
                            u32            dpitch,
                            unsigned long  src,
                            u32            spitch,
                            u32            width,
                            u32            height,
                            u32            key,
                            u32            mask )
{
     c64xTask *task = c64x_get_task( c64x );

     task->c64x_arg[0] = dest;
     task->c64x_arg[1] = (dpitch << 16) | (spitch & 0xffff);
     task->c64x_arg[2] = src;
     task->c64x_arg[3] = width;
     task->c64x_arg[4] = height;
     task->c64x_arg[5] = key;
     task->c64x_arg[6] = mask;

     task->c64x_function = C64X_COPY_KEYED_16 | C64X_FLAG_TODO;

     c64x_submit_task( c64x, task );
}

static inline void
davinci_c64x_blit_keyed_32__L( DavinciC64xTasks *tasks,
                               unsigned long     dest,
                               u32               dpitch,
                               unsigned long     src,
                               u32               spitch,
                               u32               width,
                               u32               height,
                               u32               key,
                               u32               mask )
{
     c64xTask *task = &tasks->tasks[tasks->num_tasks];

     D_ASSERT( tasks->num_tasks < tasks->max_tasks );

     task->c64x_arg[0] = dest;
     task->c64x_arg[1] = (dpitch << 16) | (spitch & 0xffff);
     task->c64x_arg[2] = src;
     task->c64x_arg[3] = width;
     task->c64x_arg[4] = height;
     task->c64x_arg[5] = key;
     task->c64x_arg[6] = mask;

     task->c64x_function = C64X_COPY_KEYED_32 | C64X_FLAG_TODO;

     tasks->num_tasks++;
}

static inline void
davinci_c64x_blit_keyed_32( DavinciC64x   *c64x,
                            unsigned long  dest,
                            u32            dpitch,
                            unsigned long  src,
                            u32            spitch,
                            u32            width,
                            u32            height,
                            u32            key,
                            u32            mask )
{
     c64xTask *task = c64x_get_task( c64x );

     task->c64x_arg[0] = dest;
     task->c64x_arg[1] = (dpitch << 16) | (spitch & 0xffff);
     task->c64x_arg[2] = src;
     task->c64x_arg[3] = width;
     task->c64x_arg[4] = height;
     task->c64x_arg[5] = key;
     task->c64x_arg[6] = mask;

     task->c64x_function = C64X_COPY_KEYED_32 | C64X_FLAG_TODO;

     c64x_submit_task( c64x, task );
}

#endif

