
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <directfb.h>
#include <zip.h>
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
                .preview_width_ratio = 35,
  },
  
  .options ={ .preview_box = true,
              .multiple_selection = true,
              .events_thread = true
  },


  .folder = {
    .filter= "^.*\\.(avi|zip)$",
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


static bool unzip_file(const char * arch_name, char * filename_in, char * filename_out){
	struct zip_file * fp_zip_file;
	FILE * fp;
	unsigned char data[1000];
	int len;
        struct zip * fp_zip;

        fp_zip = zip_open(arch_name,0,NULL);
        if (fp_zip == NULL){
          return false;
        }

	fp = fopen( filename_out, "wb" );
	if( fp == NULL ){
	    fprintf( stderr, "Unable to create file <%s>\n" , filename_out );
	    return false;
	}

	fp_zip_file = zip_fopen( fp_zip, filename_in, 0 );

	if( fp_zip_file == NULL ){
	    fprintf( stderr, "Unable to find <%s> in archive\n" , filename_in );
		fclose( fp );
		return false;
	}


	while( (len = zip_fread( fp_zip_file, data, sizeof( data ) ) ) )
		fwrite( data, sizeof( unsigned char ), len, fp );

	zip_fclose( fp_zip_file );
	fclose( fp );

	return true;
}


void cb(IDirectFBSurface * s, const char * c, bool is_select){
  static const char * cmd_mplayer_thumbnail = "DIR=`pwd` && cd /tmp && rm -f 00000001.png && mplayer -ao null -vo png:z=0 -ss 10 -frames 1 \"$DIR/%s\"";
  IDirectFBImageProvider *provider;
  DFBSurfaceDescription idsc;
  DFBSurfaceDescription dsc;
  DFBRectangle rect;

  double iratio;
  double sratio;

  char * cmd ;
  int w,h;
  if (is_select)
    printf("coucou %s\n",c);   
   else 
    printf("bye %s\n",c);   
    
  if (s != NULL){
    if (is_select){
    cmd = malloc(strlen (cmd_mplayer_thumbnail) + strlen(c) + 64);
    s->GetSize (s, &w,&h);  
    sprintf( cmd, cmd_mplayer_thumbnail, c );
    system(cmd) ;
    /*unzip_file( c, "skin.bmp", "00000001.bmp" );*/
    if (dfb->CreateImageProvider (dfb,  "/tmp/00000001.png", &provider) ==DFB_OK ) {
      provider->GetSurfaceDescription (provider, &idsc);
      iratio = idsc.width / idsc.height;
      sratio = w/h;
      if (iratio > sratio) {
        rect.w = w;
        rect.h = rect.w/iratio;
        rect.x = 0;
        rect.y = (h - rect.h) / 2;
      } else {
        rect.h = h;
        rect.w = rect.h*iratio;
        rect.y = 0;
        rect.x =( w - rect.w) /2;
      }
      provider->RenderTo (provider, s, &rect);
      provider->Release (provider);
    }
    free(cmd);
  } 
  } 
}




int main (int argc, char **argv)
{
  int ret_x, ret_y;
  DFBSurfaceDescription dsc;
  fs_handle hdl;  
  IDirectFBDisplayLayer  *layer;  
  DFBGraphicsDeviceDescription  gdesc;
  DFBWindowDescription  desc;  
  DFBDisplayLayerConfig         layer_config;
  IDirectFBWindow * win;
  IDirectFBSurface *  win_surf;
  IDirectFBWindow * win2;
  IDirectFBSurface *  win_surf2;
  DFBWindowDescription  desc2;  
  IDirectFBImageProvider *provider;
  int pos;

  DFBCHECK (DirectFBInit (&argc, &argv));
  DFBCHECK (DirectFBCreate (&dfb));

  DFBCHECK (dfb->SetCooperativeLevel (dfb,/*DFSCL_EXCLUSIVE*/ DFSCL_FULLSCREEN));


  dsc.flags = DSDESC_CAPS;
  dsc.caps  = DSCAPS_PRIMARY ;
/*| DSCAPS_FLIPPING;*/


  DFBCHECK (dfb->CreateSurface( dfb, &dsc, &primary ));
  DFBCHECK (primary->GetSize (primary, &screen_width, &screen_height));
  


  dfb->GetDeviceDescription( dfb, &gdesc );

  desc.flags = ( DWDESC_POSX | DWDESC_POSY |
                 DWDESC_WIDTH | DWDESC_HEIGHT );

  desc.posx   = 10;
  desc.posy   = 10;
  desc.width  = 280 /*screen_width*/ ;
  desc.height = 210 /*screen_height */;




  DFBCHECK(dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer ));
  layer->SetCooperativeLevel( layer,  	 DLSCL_EXCLUSIVE );  
/*  if (!((gdesc.blitting_flags & DSBLIT_BLEND_ALPHACHANNEL) &&
        (gdesc.blitting_flags & DSBLIT_BLEND_COLORALPHA  )))
  {
      layer_config.flags = DLCONF_BUFFERMODE;
      layer_config.buffermode = DLBM_BACKSYSTEM;
      layer->SetConfiguration( layer, &layer_config );
  }*/
  layer->SetBackgroundColor (layer, 0,0xFF,0,0xFF);  	
  layer->SetBackgroundMode( layer,DLBM_COLOR);
  layer->EnableCursor ( layer, 0 );
  layer->CreateWindow (layer, &desc, &win);

  desc2.flags = ( DWDESC_POSX | DWDESC_POSY |
                 DWDESC_WIDTH | DWDESC_HEIGHT );

  desc2.posx   = 0;
  desc2.posy   = 0;
  desc2.width  = 320 /*screen_width*/ ;
  desc2.height = 240 /*screen_height */;
  layer->CreateWindow (layer, &desc2, &win2);

  win->SetOpacity(win,150);
  win2->SetOpacity(win2,255);
  win->RequestFocus(win);
  win->RaiseToTop(win);

  win->GetSurface(win,&win_surf);
  win2->GetSurface(win2,&win_surf2);
/*
  win_surf2->SetColor (win_surf2, 0x00, 0x00, 0xFF, 0xff);  
  win_surf2->FillRectangle (win_surf2, 0,0,250,250);
  	
  win_surf2->SetColor (win_surf2, 0xFF, 0x00, 0xFF, 0xff);  
  win_surf2->DrawLine (win_surf2,0,0,250,220);
*/
  dfb->CreateImageProvider (dfb,  "./res/bg_1.png", &provider);
  provider->RenderTo (provider, win_surf2, NULL);
  win_surf2->Flip (win_surf2, NULL, 0);
  win->GetPosition (win, &ret_x, &ret_y);
  printf("x : %i y %i\n",ret_x,ret_y);
  win_surf->Flip (win_surf, NULL, 0);
/*
  win_surf->SetColor (win_surf, 0x80, 0x80, 0xff, 0xff);  
  win_surf->DrawLine (win_surf,0,0,50,50);
  win_surf->Flip (win_surf, NULL, 0);
*/
  /* == Begin file selector tests  == */

  config.geometry.height =  desc.height/*screen_height-50*/;
  config.geometry.width =  desc.width/*screen_width-20*/;
 
  hdl = fs_create (dfb, win, &config);
  fs_set_select_cb(hdl, cb);
  pos = fs_select_filename(hdl,"file_selector.c" );
  /*fs_select_all(hdl);*/
  if (pos != -1){
    fs_set_first_displayed_item(hdl, pos);
  }
  
  getchar();
 
  {fslist fsl = fs_get_selection(hdl);
   const char * fn;

   fn = fs_get_single_selection(hdl);
   if (fn)
    printf("selected file : %s\n",fn);

    while ( fn = fslist_get_next_file(fsl, true) ) {
      printf("selected file list : %s\n",fn);
    }
   fslist_release(fsl);
  }

  fs_release(hdl);

  /* == End file selector test == */

//  DFBCHECK (primary->Flip (primary, NULL, 0));

  primary->Release( primary ); 
  dfb->Release( dfb );
  return 0;
}
