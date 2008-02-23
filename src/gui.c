/***************************************************************************
 *            gmplayer.c
 *
 *  Sun Jan  6 14:15:55 2008
 *  Copyright  2008  nullpointer
 *  Email
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
 
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include "config.h"
#include "engine.h"
#include "version.h"


#define IDL_DIR    100
#define IDL_FILE   110
#define IDC_PATH   120
#define ID_TIMER   130



static BITMAP video_skin_bmp;
static BITMAP audio_skin_bmp;
static BITMAP loading_bmp;
static BITMAP exiting_bmp;


static DLGTEMPLATE DlgGMplayer =
{
    WS_BORDER | WS_CAPTION,
    WS_EX_NONE,
    8, 7, 304, 225,
    "MPlayer v"VERSION" by nullpointer",
    0, 0,
    5, NULL,
    0
};

static CTRLDATA CtrlGMplayer[] =
{ 
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_SIMPLE, 
        10, 10, 290, 15, 
        IDC_STATIC, 
       "Files:",
        0
    },
    {
        CTRL_LISTBOX,
        WS_VISIBLE | WS_VSCROLL | WS_BORDER | LBS_SORT | LBS_NOTIFY,
        10, 30, 290, 100,
        IDL_FILE,
        "",
        0
    },

    {
        CTRL_STATIC,
        WS_VISIBLE | SS_SIMPLE, 
        10, 150, 290, 15, 
        IDC_PATH, 
       "Path: ",
        0
    },

    {
        "button",
        WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP | WS_GROUP,
        10, 170, 130, 25,
        IDOK, 
        "Play",
        0
    },
    {
        "button",
        WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        150, 170, 130, 25,
        IDCANCEL,
        "Exit",
        0
    },
};

/** Display a rectangular buffer on screen using a specific gui.
*
*
*/
void gui_buffer_rgb(char * buffer,int width, int height, int x, int y){
  
  int x1,y1;   
  unsigned char r,g,b;
  int i =0;

  for( y1 = 0; y1 < height; y1++ ){
    for( x1 = 0; x1 < width ; x1++ ){   
      r=buffer[i++];
      g=buffer[i++];
      b=buffer[i++];
      /* Skip the a value*/
      i++;       
      SetPixelRGB(HDC_SCREEN, x + x1, y +y1, r,g,b);	
    }
  }
  Rectangle(HDC_SCREEN,x,y,x+width, y+height);
}

void blit_video_menu( int fifo, struct skin_config * conf )
{
    char str[100];
    int x,y;
    HDC hdc;
    int r,g,b,a;
    unsigned char * buffer;
    int i = 0;
    
    
    buffer = malloc( video_skin_bmp.bmWidth * video_skin_bmp.bmHeight * 4 );
    
    hdc = CreateCompatibleDC( HDC_SCREEN );
    FillBoxWithBitmap( hdc, 0,0,video_skin_bmp.bmWidth, video_skin_bmp.bmHeight, &video_skin_bmp );
    
    sprintf(str, "RGBA32 %d %d %d %d %d %d\n",video_skin_bmp.bmWidth, video_skin_bmp.bmHeight, 0, 0, 0, 0);
    write(fifo, str, strlen(str));
    for( y = 0; y < video_skin_bmp.bmHeight; y++ ){
        for( x = 0; x < video_skin_bmp.bmWidth; x++ ){
            GetPixelRGB( hdc, x, y, &r,&g,&b);
            if( r == conf->r && g == conf->g && b == conf->b ) a = 0;           
            else a = 255;

            buffer[i++] = (unsigned char )r;
            buffer[i++] = (unsigned char )g;
            buffer[i++] = (unsigned char )b;
            buffer[i++] = (unsigned char )a;
        }
    }
    
    write(fifo, buffer, video_skin_bmp.bmWidth * video_skin_bmp.bmHeight * 4);
    free( buffer );
    DeleteCompatibleDC( hdc );
}


static void fill_boxes (HWND hDlg, const char* path)
{
    struct dirent* dir_ent;
    DIR*   dir;
    struct stat ftype;
    char   fullpath [PATH_MAX + 1];
    //char   extension [PATH_MAX + 1];

    SendDlgItemMessage (hDlg, IDL_FILE, LB_RESETCONTENT, 0, (LPARAM)0);
    SetWindowText (GetDlgItem (hDlg, IDC_PATH), path);
    
    if ((dir = opendir (path)) == NULL)
         return;

    while ( (dir_ent = readdir ( dir )) != NULL ) {
        strncpy (fullpath, path, PATH_MAX);
        strcat (fullpath, "/");
        strcat (fullpath, dir_ent->d_name);
        
        if (stat (fullpath, &ftype) < 0 ) {
           continue;
        }

        if (S_ISREG (ftype.st_mode) ) 
            if( is_video_file( dir_ent->d_name ) || is_audio_file( dir_ent->d_name ) )
                SendDlgItemMessage (hDlg, IDL_FILE, LB_ADDSTRING, 0, (LPARAM)dir_ent->d_name);
    }

    closedir (dir);
}

static void file_notif_proc (HWND hwnd, int id, int nc, DWORD add_data)
{
}

static void play (HWND hDlg)
{
    int i;
    char filename [MAX_NAME + 1];
    RECT rc;
    
    i = SendDlgItemMessage (hDlg, IDL_FILE, LB_GETCURSEL, 0, 0);
    if( i == LB_ERR ){
        MessageBox (hDlg, "No file selected", "GMPlayer", MB_OK | MB_ICONINFORMATION);      
    }
    else{
        SendDlgItemMessage (hDlg, IDL_FILE, LB_GETTEXT, i, (LPARAM)filename) ;
        ShowWindow( hDlg, SW_HIDE );

        if( is_video_file( filename ) ){
            FillBoxWithBitmap( HDC_SCREEN, 0, 0, loading_bmp.bmWidth, loading_bmp.bmHeight, &loading_bmp );
            rc.left = config.video_config.text_x1;
            rc.right = config.video_config.text_x2;
            rc.top = config.video_config.text_y1;
            rc.bottom = config.video_config.text_y2;
        }
        else{
            FillBoxWithBitmap( HDC_SCREEN, 0, 0, audio_skin_bmp.bmWidth, audio_skin_bmp.bmHeight, &audio_skin_bmp );
            rc.left = config.audio_config.text_x1;
            rc.right = config.audio_config.text_x2;
            rc.top = config.audio_config.text_y1;
            rc.bottom = config.audio_config.text_y2;
        }

        DrawText (HDC_SCREEN, filename, -1, &rc, DT_CENTER );

        
        launch_mplayer( filename );

    }
}

static int mouse_hook (void* context, HWND dst_wnd, int msg, WPARAM wParam, LPARAM lParam)
{
   if( msg == MSG_LBUTTONDOWN && ( is_playing_video == TRUE || is_playing_audio == TRUE ) ){
       handle_mouse_event( LOSWORD (lParam), HISWORD (lParam) );
       return HOOK_STOP;
   }
   else
       return HOOK_GOON;
}

static int load_bmp( void ){
    int i;

    i = LoadBitmap( HDC_SCREEN, &video_skin_bmp, config.video_config.image_file );
    if( i != ERR_BMP_OK){
        fprintf( stderr, "Error while loading bitmap file %s <%d>\n",config.video_config.image_file, i);
        return FALSE;
    }

    i = LoadBitmap( HDC_SCREEN, &audio_skin_bmp, config.audio_config.image_file );
    if( i != ERR_BMP_OK){
        fprintf( stderr, "Error while loading bitmap file %s <%d>\n",config.audio_config.image_file, i);
        return FALSE;
    }
    i = LoadBitmap( HDC_SCREEN, &exiting_bmp, config.bmp_exiting_file );
    if( i != ERR_BMP_OK){
        fprintf( stderr, "Error while loading bitmap file %s <%d>\n",config.bmp_exiting_file, i);
        return FALSE;
    }
    
    i = LoadBitmap( HDC_SCREEN, &loading_bmp, config.bmp_loading_file );
    if( i != ERR_BMP_OK){
        fprintf( stderr, "Error while loading bitmap file %s <%d>\n",config.bmp_loading_file, i);
        return FALSE;
    }

    return TRUE;
}

static int GMplayerBoxProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
        case MSG_INITDIALOG:
            if( load_config(&config) == FALSE ){
                fprintf( stderr, "Error while loading config\n" );
                exit(1);
            }
            if( load_bmp() == FALSE ){
                fprintf( stderr, "Error while loading bitmap\n" );
                exit(1);
            }
            SetNotificationCallback (GetDlgItem (hDlg, IDL_FILE), file_notif_proc);
            ShowCursor(FALSE);
            fill_boxes (hDlg, config.folder);
            SetTimer (hDlg, ID_TIMER, 100);
    
            return 1;
        case MSG_COMMAND:
            switch (wParam) {
                case IDOK:
                    play (hDlg);
                    break;
                case IDCANCEL:
                    EndDialog (hDlg, wParam);
                    FillBoxWithBitmap( HDC_SCREEN, 0, 0, exiting_bmp.bmWidth, exiting_bmp.bmHeight, &exiting_bmp );
                    exit(0);
                    break;
            }
            break;
            
        case MSG_TIMER:
            if( ( is_playing_video == TRUE || is_playing_audio == TRUE ) && is_mplayer_finished == TRUE ){
                is_playing_video = FALSE;
                is_playing_audio = FALSE;
                is_mplayer_finished = FALSE;
                ShowWindow( hDlg, SW_SHOW );
                UpdateWindow( hDlg, TRUE );
            }
            break;
    
        case MSG_CLOSE:
            // Destroy the main window
            DestroyMainWindow (hDlg);
            // Post a MSG_QUIT message
            PostQuitMessage(hDlg);
            return 0;
    }
    
    return DefaultDialogProc (hDlg, message, wParam, lParam);
}

int MiniGUIMain (int argc, const char* argv[])
{
    SetWindowBkColor( HWND_DESKTOP, 0 );
    RegisterMouseMsgHook(HWND_DESKTOP, mouse_hook);
    
    DlgGMplayer.controls = CtrlGMplayer;
    
    DialogBoxIndirectParam (&DlgGMplayer, HWND_DESKTOP, GMplayerBoxProc, 0L);

    return 0;
}
