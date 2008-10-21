
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <directfb.h>
#include "file_selector.h"

#define RES_FOLDER "./res/"

static struct fs_config config = 
{
  .graphics = { .filename = {RES_FOLDER "scroll_up_0.png",
                             RES_FOLDER "scroll_down_0.png" ,
                             RES_FOLDER "check_0.png",
                             RES_FOLDER "folder_0.png"},
                .font = RES_FOLDER "decker.ttf",
                .font_color = {.a=0xff , .r=0xff, .g =0x00 , .b =0x00 },
              },

  .geometry = { .x = 0,
                .y = 0,
                .height = 200,
                .width = 300,     
                .preview_width_ratio = 20,
  },
  
  .options ={ .preview_box = false,
              .multiple_selection = true,
  },


  .folder = {
    .filter=NULL,
    .pathname=".",
  } 

};


static IDirectFB *dfb = NULL;
static IDirectFBSurface *primary = NULL;

static int screen_width  = 0;
static int screen_height = 0;


#define DFBCHECK(x...)                                         \
  {                                                            \
    DFBResult err = x;                                         \
                                                               \
    if (err != DFB_OK)                                         \
      {                                                        \
        fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ ); \
        DirectFBErrorFatal( #x, err );                         \
      }                                                        \
  }

void cb(IDirectFBSurface * s, const char * c){
  printf("coucou %s\n",c);  
}

int main (int argc, char **argv)
{

	 

  DFBSurfaceDescription dsc;
  fs_handle hdl;

  DFBCHECK (DirectFBInit (&argc, &argv));
  DFBCHECK (DirectFBCreate (&dfb));

  DFBCHECK (dfb->SetCooperativeLevel (dfb, DFSCL_FULLSCREEN));

  dsc.flags = DSDESC_CAPS;
  dsc.caps  = DSCAPS_PRIMARY ;
/*| DSCAPS_FLIPPING;*/

  DFBCHECK (dfb->CreateSurface( dfb, &dsc, &primary ));
  DFBCHECK (primary->GetSize (primary, &screen_width, &screen_height));
  
  //config.geometry.height = screen_height;
  config.geometry.width = screen_width;

  hdl = fs_create (dfb, primary, &config);
  fs_set_select_cb(hdl, cb);
  
  getchar();

  
  
  {fslist fsl = fs_get_selection(hdl);
   const char * fn;

   fn = fs_get_single_selection(hdl);
   if (fn)
    printf("selected file : %s\n",fn);

    while ( fn = fslist_get_next_file(fsl) ) {
      printf("selected file list : %s\n",fn);
    }
   fslist_release(fsl);
  }

  fs_release(hdl);
//  DFBCHECK (primary->Flip (primary, NULL, 0));

  primary->Release( primary );
  dfb->Release( dfb );
  return 0;
}
