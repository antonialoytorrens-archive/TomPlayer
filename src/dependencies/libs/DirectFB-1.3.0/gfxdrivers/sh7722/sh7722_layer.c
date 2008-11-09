#ifdef SH7722_DEBUG_LAYER
#define DIRECT_ENABLE_DEBUG
#endif


#include <config.h>

#include <stdio.h>

#include <sys/mman.h>

#include <asm/types.h>

#include <directfb.h>

#include <fusion/fusion.h>
#include <fusion/shmalloc.h>

#include <core/core.h>
#include <core/coredefs.h>
#include <core/coretypes.h>
#include <core/layers.h>
#include <core/palette.h>
#include <core/surface.h>
#include <core/surface_buffer.h>
#include <core/system.h>

#include <gfx/convert.h>

#include <misc/conf.h>

#include <direct/memcpy.h>
#include <direct/messages.h>

#include "sh7722.h"
#include "sh7722_types.h"
#include "sh7722_layer.h"
#include "sh7722_lcd.h"


D_DEBUG_DOMAIN( SH7722_Layer, "SH7722/Layer", "Renesas SH7722 Layers" );

/**********************************************************************************************************************/

static int
sh7722LayerDataSize()
{
     return sizeof(SH7722LayerData);
}

static int
sh7722RegionDataSize()
{
     return sizeof(SH7722RegionData);
}

static DFBResult
sh7722InitLayer( CoreLayer                  *layer,
                 void                       *driver_data,
                 void                       *layer_data,
                 DFBDisplayLayerDescription *description,
                 DFBDisplayLayerConfig      *config,
                 DFBColorAdjustment         *adjustment )
{
     SH7722DriverData *sdrv = driver_data;
     SH7722LayerData  *data = layer_data;

     D_DEBUG_AT( SH7722_Layer, "%s()\n", __FUNCTION__ );

     /* initialize layer data */
     data->layer = SH7722_LAYER_INPUT1 + sdrv->num_inputs++;

     /* set capabilities and type */
     description->caps = DLCAPS_SURFACE | DLCAPS_ALPHACHANNEL | DLCAPS_OPACITY |
                         DLCAPS_SCREEN_POSITION | DLCAPS_SRC_COLORKEY;

     description->type = DLTF_STILL_PICTURE | DLTF_GRAPHICS | DLTF_VIDEO;

     /* set name */
     snprintf( description->name, DFB_DISPLAY_LAYER_DESC_NAME_LENGTH, "Input %d", sdrv->num_inputs );

     /* fill out the default configuration */
     config->flags       = DLCONF_WIDTH       | DLCONF_HEIGHT |
                           DLCONF_PIXELFORMAT | DLCONF_BUFFERMODE | DLCONF_OPTIONS;
     config->width       = SH7722_LCD_WIDTH;
     config->height      = SH7722_LCD_HEIGHT;
     config->pixelformat = DSPF_RGB16;
     config->buffermode  = DLBM_FRONTONLY;
     config->options     = DLOP_ALPHACHANNEL;

     return DFB_OK;
}

static DFBResult
sh7722TestRegion( CoreLayer                  *layer,
                  void                       *driver_data,
                  void                       *layer_data,
                  CoreLayerRegionConfig      *config,
                  CoreLayerRegionConfigFlags *failed )
{
     SH7722DriverData           *sdrv = driver_data;
     SH7722DeviceData           *sdev = sdrv->dev;
     SH7722LayerData            *slay = layer_data;
     CoreLayerRegionConfigFlags  fail = 0;

     D_DEBUG_AT( SH7722_Layer, "%s()\n", __FUNCTION__ );

     if (config->options & ~SH7722_LAYER_SUPPORTED_OPTIONS)
          fail |= CLRCF_OPTIONS;

     switch (config->format) {
          case DSPF_LUT8:
               /* Indexed only for third input */
               if (slay->layer != SH7722_LAYER_INPUT3)
                    fail |= CLRCF_FORMAT;
               break;

          case DSPF_ARGB:
          case DSPF_RGB32:
          case DSPF_RGB24:
          case DSPF_RGB16:
               break;

          case DSPF_NV12:
          case DSPF_NV16:
               /* YUV only for first input */
               if (slay->layer != SH7722_LAYER_INPUT1)
                    fail |= CLRCF_FORMAT;
               break;

          default:
               fail |= CLRCF_FORMAT;
     }

     if (config->width  < 32 || config->width  > sdev->lcd_width)
          fail |= CLRCF_WIDTH;

     if (config->height < 32 || config->height > sdev->lcd_height)
          fail |= CLRCF_HEIGHT;

     if (config->dest.x >= sdev->lcd_width || config->dest.y >= sdev->lcd_height)
          fail |= CLRCF_DEST;

     if (config->dest.x < 0) {
          config->dest.x = 0;
// FIXME
//          fail |= CLRCF_DEST;
     }

     if (config->dest.y < 0) {
          config->dest.y = 0;
// FIXME
//          fail |= CLRCF_DEST;
     }

     if (failed)
          *failed = fail;

     if (fail)
          return DFB_UNSUPPORTED;

     return DFB_OK;
}

static DFBResult
sh7722AddRegion( CoreLayer             *layer,
                 void                  *driver_data,
                 void                  *layer_data,
                 void                  *region_data,
                 CoreLayerRegionConfig *config )
{
     SH7722RegionData *sreg = region_data;

     D_DEBUG_AT( SH7722_Layer, "%s()\n", __FUNCTION__ );

     sreg->config = *config;

     D_MAGIC_SET( sreg, SH7722RegionData );

     return DFB_OK;
}

static DFBResult
sh7722SetRegion( CoreLayer                  *layer,
                 void                       *driver_data,
                 void                       *layer_data,
                 void                       *region_data,
                 CoreLayerRegionConfig      *config,
                 CoreLayerRegionConfigFlags  updated,
                 CoreSurface                *surface,
                 CorePalette                *palette,
                 CoreSurfaceBufferLock      *lock )
{
     int               i, n;
     SH7722DriverData *sdrv = driver_data;
     SH7722DeviceData *sdev = sdrv->dev;
     SH7722RegionData *sreg = region_data;
     SH7722LayerData  *slay = layer_data;

     D_DEBUG_AT( SH7722_Layer, "%s()\n", __FUNCTION__ );

     D_MAGIC_ASSERT( sreg, SH7722RegionData );

     n = slay->layer - SH7722_LAYER_INPUT1;

     D_ASSERT( n >= 0 );
     D_ASSERT( n <= 2 );

     fusion_skirmish_prevail( &sdev->beu_lock );

     /* Wait for idle BEU. */
     BEU_Wait( sdrv, sdev );

     /* Update position? */
     if (updated & CLRCF_DEST) {
          /* Set horizontal and vertical offset. */
          SH7722_SETREG32( sdrv, BLOCR(n), (config->dest.y << 16) | config->dest.x );
     }

     /* Update size? */
     if (updated & (CLRCF_WIDTH | CLRCF_HEIGHT)) {
          int cw = config->width;
          int ch = config->height;

          if (config->dest.x + cw > SH7722_LCD_WIDTH)
               cw = SH7722_LCD_WIDTH - config->dest.x;

          if (config->dest.y + ch > SH7722_LCD_HEIGHT)
               ch = SH7722_LCD_HEIGHT - config->dest.y;

          /* Set width and height. */
          SH7722_SETREG32( sdrv, BSSZR(n), (ch << 16) | cw );
          SH7722_SETREG32( sdrv, BTPSR, (ch << 16) | cw );
     }

     /* Update surface? */
     if (updated & CLRCF_SURFACE) {
          CoreSurfaceBuffer *buffer = lock->buffer;

          /* Set buffer pitch. */
          SH7722_SETREG32( sdrv, BSMWR(n), lock->pitch );

          /* Set buffer offset (Y plane or RGB packed). */
          SH7722_SETREG32( sdrv, BSAYR(n), lock->phys );

          /* Set buffer offset (UV plane). */
          if (DFB_PLANAR_PIXELFORMAT(buffer->format)) {
               D_ASSUME( buffer->format == DSPF_NV12 || buffer->format == DSPF_NV16 );

               SH7722_SETREG32( sdrv, BSACR(n), lock->phys + lock->pitch * surface->config.size.h );
          }

          sreg->surface = surface;
     }

     /* Update format? */
     if (updated & CLRCF_FORMAT) {
          unsigned long tBSIFR = 0;
          unsigned long tBSWPR = BSWPR_MODSEL_EACH | (SH7722_GETREG32( sdrv, BSWPR ) & ~(7 << (n*8)));

          /* Set pixel format. */
          switch (config->format) {
               case DSPF_NV12:
                    tBSIFR |= CHRR_YCBCR_420 | BSIFR1_IN1TE_RGBYUV;
                    break;

               case DSPF_NV16:
                    tBSIFR |= CHRR_YCBCR_422 | BSIFR1_IN1TE_RGBYUV;
                    break;

               case DSPF_ARGB:
                    tBSIFR |= RPKF_ARGB;
                    break;

               case DSPF_RGB32:
                    tBSIFR |= RPKF_RGB32;
                    break;

               case DSPF_RGB24:
                    tBSIFR |= RPKF_RGB24;
                    break;

               case DSPF_RGB16:
                    tBSIFR |= RPKF_RGB16;
                    break;
 
               case DSPF_LUT8:
                    tBSIFR |= BSIFR3_MOD0_OSD | BSIFR3_MOD1_LUT;
                    break;

               default:
                    break;
          }

          /* Set swapping. */
          switch (config->format) {
               case DSPF_LUT8:
               case DSPF_NV12:
               case DSPF_NV16:
                    tBSWPR |= (BSWPR_INPUT_BYTESWAP |
                               BSWPR_INPUT_WORDSWAP |
                               BSWPR_INPUT_LONGSWAP) << (n*8);
                    break;

               case DSPF_RGB16:
                    tBSWPR |= (BSWPR_INPUT_WORDSWAP |
                               BSWPR_INPUT_LONGSWAP) << (n*8);
                    break;

               case DSPF_ARGB:
               case DSPF_RGB32:
               case DSPF_RGB24:
                    tBSWPR |= (BSWPR_INPUT_LONGSWAP) << (n*8);
                    break;

               default:
                    break;
          }

          SH7722_SETREG32( sdrv, BSIFR(n), tBSIFR );
          SH7722_SETREG32( sdrv, BSWPR, tBSWPR );
     }

     /* Update options or opacity? */
     if (updated & (CLRCF_OPTIONS | CLRCF_OPACITY | CLRCF_FORMAT)) {
          unsigned long tBBLCR0 = BBLCR0_LAY_123;

          /* Set opacity value. */
          tBBLCR0 &= ~(0xff << (n*8));
          tBBLCR0 |= ((config->options & CLRCF_OPACITY) ? config->opacity : 0xff) << (n*8);

          /* Enable/disable alpha channel. */
          if ((config->options & DLOP_ALPHACHANNEL) && DFB_PIXELFORMAT_HAS_ALPHA(config->format))
               tBBLCR0 |= BBLCR0_AMUX_BLENDPIXEL(n);
          else
               tBBLCR0 &= ~BBLCR0_AMUX_BLENDPIXEL(n);

          SH7722_SETREG32( sdrv, BBLCR0, tBBLCR0 );
     }

     /* Update CLUT? */
     if (updated & CLRCF_PALETTE && palette) {
          const DFBColor *entries = palette->entries;

          for (i=0; i<256; i++) {
               SH7722_SETREG32( sdrv, BCLUT(i), PIXEL_ARGB( entries[i].a,
                                                            entries[i].r,
                                                            entries[i].g,
                                                            entries[i].b ) );
          }
     }


     /* Enable or disable input. */
     if ((config->options & DLOP_OPACITY) && !config->opacity)
          sdev->input_mask &= ~(1 << n);
     else
          sdev->input_mask |= (1 << n);

     /* Choose parent input. */
     if (sdev->input_mask) {
          unsigned long tBBLCR1 = SH7722_GETREG32( sdrv, BBLCR1 ) & ~BBLCR1_PWD_INPUT_MASK;

          if (sdev->input_mask & 4)
               tBBLCR1 |= BBLCR1_PWD_INPUT3;
          else if (sdev->input_mask & 2)
               tBBLCR1 |= BBLCR1_PWD_INPUT2;
          else
               tBBLCR1 |= BBLCR1_PWD_INPUT1;

          SH7722_SETREG32( sdrv, BBLCR1, tBBLCR1 );
     }

     fusion_skirmish_dismiss( &sdev->beu_lock );

     sreg->config = *config;

     return DFB_OK;
}

static DFBResult
sh7722RemoveRegion( CoreLayer *layer,
                    void      *driver_data,
                    void      *layer_data,
                    void      *region_data )
{
     int               n;
     SH7722DriverData *sdrv = driver_data;
     SH7722DeviceData *sdev = sdrv->dev;
     SH7722LayerData  *slay = layer_data;

     D_DEBUG_AT( SH7722_Layer, "%s()\n", __FUNCTION__ );

     D_ASSERT( sdev != NULL );
     D_ASSERT( slay != NULL );

     n = slay->layer - SH7722_LAYER_INPUT1;

     D_ASSERT( n >= 0 );
     D_ASSERT( n <= 2 );

     fusion_skirmish_prevail( &sdev->beu_lock );

     /* Wait for idle BEU. */
     BEU_Wait( sdrv, sdev );

     sdev->input_mask &= ~(1 << n);

     /* Choose parent input. */
     if (sdev->input_mask) {
          unsigned long tBBLCR1 = SH7722_GETREG32( sdrv, BBLCR1 ) & ~BBLCR1_PWD_INPUT_MASK;

          if (sdev->input_mask & 4)
               tBBLCR1 |= BBLCR1_PWD_INPUT3;
          else if (sdev->input_mask & 2)
               tBBLCR1 |= BBLCR1_PWD_INPUT2;
          else
               tBBLCR1 |= BBLCR1_PWD_INPUT1;

          SH7722_SETREG32( sdrv, BBLCR1, tBBLCR1 );
     }

     /* Start operation! */
     BEU_Start( sdrv, sdev );

     fusion_skirmish_dismiss( &sdev->beu_lock );

     return DFB_OK;
}

static DFBResult
sh7722FlipRegion( CoreLayer             *layer,
                  void                  *driver_data,
                  void                  *layer_data,
                  void                  *region_data,
                  CoreSurface           *surface,
                  DFBSurfaceFlipFlags    flags,
                  CoreSurfaceBufferLock *lock )
{
     int                n;
     CoreSurfaceBuffer *buffer;
     SH7722DriverData  *sdrv = driver_data;
     SH7722DeviceData  *sdev = sdrv->dev;
     SH7722LayerData   *slay = layer_data;

     D_DEBUG_AT( SH7722_Layer, "%s()\n", __FUNCTION__ );

     D_ASSERT( surface != NULL );
     D_ASSERT( sdrv != NULL );
     D_ASSERT( sdev != NULL );
     D_ASSERT( slay != NULL );

     n = slay->layer - SH7722_LAYER_INPUT1;

     D_ASSERT( n >= 0 );
     D_ASSERT( n <= 2 );

     buffer = lock->buffer;
     D_ASSERT( buffer != NULL );

     fusion_skirmish_prevail( &sdev->beu_lock );

     /* Set buffer offset (Y plane or RGB packed). */
     SH7722_SETREG32( sdrv, BSAYR(n), lock->phys );

     /* Set buffer offset (UV plane). */
     if (DFB_PLANAR_PIXELFORMAT(buffer->format)) {
          D_ASSUME( buffer->format == DSPF_NV12 || buffer->format == DSPF_NV16 );

          SH7722_SETREG32( sdrv, BSACR(n), lock->phys + lock->pitch * surface->config.size.h );
     }

     /* Start operation! */
     BEU_Start( sdrv, sdev );

     fusion_skirmish_dismiss( &sdev->beu_lock );

     /* Wait for idle BEU? */
     if (flags & DSFLIP_WAIT)
          BEU_Wait( sdrv, sdev );

     dfb_surface_flip( surface, false );

     return DFB_OK;
}

static DFBResult
sh7722UpdateRegion( CoreLayer             *layer,
                    void                  *driver_data,
                    void                  *layer_data,
                    void                  *region_data,
                    CoreSurface           *surface,
                    const DFBRegion       *update,
                    CoreSurfaceBufferLock *lock )
{
     SH7722DriverData *sdrv = driver_data;
     SH7722DeviceData *sdev = sdrv->dev;

     D_DEBUG_AT( SH7722_Layer, "%s()\n", __FUNCTION__ );

     D_ASSERT( surface != NULL );
     D_ASSERT( sdrv != NULL );
     D_ASSERT( sdev != NULL );

     /* Start operation! */
     BEU_Start( sdrv, sdev );

     if (!(surface->config.caps & DSCAPS_FLIPPING))
          BEU_Wait( sdrv, sdev );

     return DFB_OK;
}

DisplayLayerFuncs sh7722LayerFuncs = {
     LayerDataSize:      sh7722LayerDataSize,
     RegionDataSize:     sh7722RegionDataSize,
     InitLayer:          sh7722InitLayer,

     TestRegion:         sh7722TestRegion,
     AddRegion:          sh7722AddRegion,
     SetRegion:          sh7722SetRegion,
     RemoveRegion:       sh7722RemoveRegion,
     FlipRegion:         sh7722FlipRegion,
     UpdateRegion:       sh7722UpdateRegion
};

