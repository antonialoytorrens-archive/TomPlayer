/***************************************************************************
 *           GUI based on minigui for Tomplayer
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
#include "resume.h"
#include "pwm.h"
#include "power.h"
#include "playlist.h"
#include "widescreen.h"
#include "zip_skin.h"


#define DEFAULT_PLAYLIST        "/tmp/tomplayer.m3u"
#define ICON_FOLDER_FILENAME    "./res/icon/folder.ico"
#define ICON_FILE_FILENAME      "./res/icon/file.ico"

#define VIDEO_SKIN_FOLDER   "./skins/video"
#define AUDIO_SKIN_FOLDER   "./skins/audio"

#define ID_TIMER            100
#define IDC_PROPSHEET       110

#define IDL_FILE_VIDEO      210
#define IDC_PATH_VIDEO      220
#define IDB_PLAY_VIDEO      230
#define IDB_EXIT_VIDEO      240
#define IDB_RESUME_VIDEO    250
#define IDC_STATIC_VIDEO    260

#define IDL_FILE_AUDIO      310
#define IDC_PATH_AUDIO      320
#define IDB_PLAY_AUDIO      330
#define IDB_EXIT_AUDIO      340
#define IDB_RANDOM_AUDIO    350
#define IDC_STATIC_AUDIO    360

#define IDL_FILE_AUDIO_SKIN     410
#define IDC_PATH_AUDIO_SKIN     420
#define IDB_SELECT_AUDIO_SKIN   430
#define IDC_STATIC_AUDIO_SKIN   440

#define IDL_FILE_VIDEO_SKIN     410
#define IDC_PATH_VIDEO_SKIN     420
#define IDB_SELECT_VIDEO_SKIN   430
#define IDC_STATIC_VIDEO_SKIN   440

static HICON hicon_folder;
static HICON hicon_file;
static HWND hlb_video;
static HWND hlb_audio;


static DLGTEMPLATE DlgTomPlayerPropSheet =
{
    WS_BORDER | WS_CAPTION,
    WS_EX_NONE,
    1, 1, 318, 238,
    "TomPlayer v"VERSION,
    0, 0,
    1, NULL,
    0
};

static CTRLDATA CtrlTomPlayerPropSheet[] =
{ 
    {
        CTRL_PROPSHEET,
        WS_VISIBLE | PSS_COMPACTTAB, 
        5, 5, 310, 220,
        IDC_PROPSHEET,
        "",
        0
    },
};


static DLGTEMPLATE DlgTomPlayerAudio =
{
    WS_BORDER | WS_CAPTION,
    WS_EX_NONE,
    0, 0, 0, 0,
    "Audio",
    0, 0,
    6, NULL,
    0
};

static CTRLDATA CtrlTomPlayerAudio[] =
{ 
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_SIMPLE, 
        10, 10, 260, 15, 
        IDC_STATIC_AUDIO, 
       "Files:",
        0
    },
    {
        CTRL_LISTBOX,
        WS_VISIBLE | WS_VSCROLL | WS_BORDER | LBS_SORT | LBS_NOTIFY | LBS_USEICON,
        10, 30, 260, 100,
        IDL_FILE_AUDIO,
        "",
        0
    },

    {
        CTRL_STATIC,
        WS_VISIBLE | SS_SIMPLE, 
        10, 130, 260, 15, 
        IDC_PATH_AUDIO, 
       "Path: ",
        0
    },

    {
        CTRL_BUTTON,
        WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP | WS_GROUP,
        8, 150, 92, 25,
        IDB_PLAY_AUDIO, 
        "Play",
        0
    },
    {
        CTRL_BUTTON,
        WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        208, 150, 92, 25,
        IDB_EXIT_AUDIO,
        "Exit",
        0
    },
    {
        CTRL_BUTTON,
        WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        108, 150, 92, 25,
        IDB_RANDOM_AUDIO,
        "Play dir.",
        0
    },
};

static DLGTEMPLATE DlgTomPlayerVideo =
{
    WS_BORDER | WS_CAPTION,
    WS_EX_NONE,
    0, 0, 0, 0,
    "Video",
    0, 0,
    6, NULL,
    0
};

static CTRLDATA CtrlTomPlayerVideo[] =
{ 
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_SIMPLE, 
        10, 10, 260, 15, 
        IDC_STATIC_VIDEO, 
       "Files:",
        0
    },
    {
        CTRL_LISTBOX,
        WS_VISIBLE | WS_VSCROLL | WS_BORDER | LBS_SORT | LBS_NOTIFY | LBS_USEICON,
        10, 30, 260, 100,
        IDL_FILE_VIDEO,
        "",
        0
    },

    {
        CTRL_STATIC,
        WS_VISIBLE | SS_SIMPLE, 
        10, 130, 260, 15, 
        IDC_PATH_VIDEO, 
       "Path: ",
        0
    },

    {
        CTRL_BUTTON,
        WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP | WS_GROUP,
        8, 150, 92, 25,
        IDB_PLAY_VIDEO, 
        "Play",
        0
    },
    {
        CTRL_BUTTON,
        WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        208, 150, 92, 25,
        IDB_EXIT_VIDEO,
        "Exit",
        0
    },
    {
        CTRL_BUTTON,
        WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        108, 150, 92, 25,
        IDB_RESUME_VIDEO,
        "Resume",
        0
    },
};



static DLGTEMPLATE DlgTomPlayerAudioSkin =
{
    WS_BORDER | WS_CAPTION,
    WS_EX_NONE,
    0, 0, 0, 0,
    "Audio skin",
    0, 0,
    4, NULL,
    0
};

static CTRLDATA CtrlTomPlayerAudioSkin[] =
{ 
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_SIMPLE, 
        10, 10, 260, 15, 
        IDC_STATIC_AUDIO_SKIN, 
       "Files:",
        0
    },
    {
        CTRL_LISTBOX,
        WS_VISIBLE | WS_VSCROLL | WS_BORDER | LBS_SORT | LBS_NOTIFY | LBS_USEICON,
        10, 30, 260, 100,
        IDL_FILE_AUDIO_SKIN,
        "",
        0
    },

    {
        CTRL_STATIC,
        WS_VISIBLE | SS_SIMPLE, 
        10, 130, 260, 15, 
        IDC_PATH_AUDIO_SKIN, 
       "Path: ",
        0
    },
    {
        CTRL_BUTTON,
        WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        108, 150, 92, 25,
        IDB_SELECT_AUDIO_SKIN,
        "Select skin",
        0
    },
};



static DLGTEMPLATE DlgTomPlayerVideoSkin =
{
    WS_BORDER | WS_CAPTION,
    WS_EX_NONE,
    0, 0, 0, 0,
    "Video skin",
    0, 0,
    4, NULL,
    0
};

static CTRLDATA CtrlTomPlayerVideoSkin[] =
{ 
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_SIMPLE, 
        10, 10, 260, 15, 
        IDC_STATIC_VIDEO_SKIN, 
       "Files:",
        0
    },
    {
        CTRL_LISTBOX,
        WS_VISIBLE | WS_VSCROLL | WS_BORDER | LBS_SORT | LBS_NOTIFY | LBS_USEICON,
        10, 30, 260, 100,
        IDL_FILE_VIDEO_SKIN,
        "",
        0
    },

    {
        CTRL_STATIC,
        WS_VISIBLE | SS_SIMPLE, 
        10, 130, 260, 15, 
        IDC_PATH_VIDEO_SKIN, 
       "Path: ",
        0
    },
    {
        CTRL_BUTTON,
        WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        108, 150, 92, 25,
        IDB_SELECT_VIDEO_SKIN,
        "Select skin",
        0
    },
};


void ExtendDialogBoxToScreen( DLGTEMPLATE * dlg ){
    int i;

    EXTEND_X( dlg->x );
    EXTEND_Y( dlg->y );    
    
    EXTEND_X( dlg->w );
    EXTEND_Y( dlg->h );
    
    for( i = 0; i < dlg->controlnr; i++ ){
        EXTEND_X( dlg->controls[i].x );
        EXTEND_Y( dlg->controls[i].y );    
        
        EXTEND_X( dlg->controls[i].w );
        EXTEND_Y( dlg->controls[i].h );
    }
}


/** Display a rectangular buffer on screen using a specific gui.
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
  Rectangle(HDC_SCREEN,0,y,GetGDCapability( HDC_SCREEN, GDCAP_HPIXEL),GetGDCapability( HDC_SCREEN, GDCAP_VPIXEL));  
}





static void fill_boxes (HWND hDlg, int list_box_id, int text_path_id, char * extension, const char* path)
{

    struct dirent* dir_ent;
    DIR*   dir;
    struct stat ftype;
    char   fullpath [PATH_MAX + 1];
    LISTBOXITEMINFO lbii;
    
    if ((dir = opendir (path)) == NULL)
         return;

    SendDlgItemMessage (hDlg, list_box_id, LB_RESETCONTENT, 0, (LPARAM)0);
    SetWindowText (GetDlgItem (hDlg, text_path_id), path);

    
    while ( (dir_ent = readdir ( dir )) != NULL ) {
        strncpy (fullpath, path, PATH_MAX);
        strcat (fullpath, "/");
        strcat (fullpath, dir_ent->d_name);
        
        if (stat (fullpath, &ftype) < 0 ) {
           continue;
        }
        
        lbii.string = dir_ent->d_name;
        if (S_ISDIR (ftype.st_mode) ){
            lbii.hIcon = hicon_folder; 
            SendDlgItemMessage (hDlg, list_box_id, LB_ADDSTRING, 0, (LPARAM)&lbii);
        }
        else if (S_ISREG (ftype.st_mode) ) 
            if( has_extension(dir_ent->d_name, extension ) ){
                lbii.hIcon = hicon_file;
                SendDlgItemMessage (hDlg, list_box_id, LB_ADDSTRING, 0, (LPARAM)&lbii);
            }
    }

    closedir (dir);
}

static void dir_notif_proc (HWND hwnd, int id, int nc, int text_path_id, char * extensions)
{
   /* When the user double clicked the directory name or
    * pressed the ENTER key, he will enter the corresponding directory */
   if (nc == LBN_CLICKED || nc == LBN_ENTER) {
       int cur_sel = SendMessage (hwnd, LB_GETCURSEL, 0, 0L);
       if (cur_sel >= 0) {
           char cwd [MAX_PATH + 1];
           char dir [MAX_NAME + 1];
           GetWindowText (GetDlgItem (GetParent (hwnd), text_path_id), cwd, MAX_PATH);
           SendMessage (hwnd, LB_GETTEXT, cur_sel, (LPARAM)dir);
           if (strcmp (dir, ".") == 0)
               return;
           
           if( strcmp( dir, ".." ) == 0 ){
               int i;
               i = strlen( cwd );
               while( --i > 0 )
                   if( cwd[i] == '/' ){
                       cwd[i] = 0;
                       break;
                   }
           }
           else{
               strcat (cwd, "/");
               strcat (cwd, dir);
           }

           fill_boxes (GetParent (hwnd), id, text_path_id, extensions, cwd);
       }
   }
}


static void video_file_notif_proc (HWND hwnd, int id, int nc, DWORD add_data)
{
    dir_notif_proc (hwnd, IDL_FILE_VIDEO, nc, IDC_PATH_VIDEO, config.filter_video_ext );
}

static void audio_file_notif_proc (HWND hwnd, int id, int nc, DWORD add_data)
{
    char extensions[PATH_MAX+1];
    strcpy( extensions, config.filter_audio_ext );
    strcat( extensions, " .m3u" );
    dir_notif_proc (hwnd, IDL_FILE_AUDIO, nc, IDC_PATH_AUDIO, extensions );
}


static void play (HWND hDlg, char * folder, char * filename, BOOL resume)
{
    int pos = 0;
    

    if (resume == TRUE) {
        if (resume_get_file_infos(filename, PATH_MAX, &pos) != 0){
            MessageBox (hDlg, "Unable to retrieve resume informations", "TomPlayer", MB_OK | MB_ICONINFORMATION);      
            return;
        }
    }
    
    ShowWindow( hDlg, SW_HIDE );

    if( is_video_file( filename ) ){
        display_image_to_fb( config.bitmap_loading );
        display_current_file( filename, &config.video_config );
    }
    else{
        display_image_to_fb( config.audio_config.bitmap );
        display_current_file( filename, &config.audio_config );
    }
    launch_mplayer( folder, filename, pos );   
}

void display_current_file( char * filename, struct skin_config * skin_conf )
{
    RECT rc;
    
    display_image_to_fb( skin_conf->bitmap );
    
    rc.left = skin_conf->text_x1;
    rc.right = skin_conf->text_x2;
    rc.top = skin_conf->text_y1;
    rc.bottom = skin_conf->text_y2;
    SetBkMode(HDC_SCREEN, BM_TRANSPARENT);
    TextOut( HDC_SCREEN, rc.left, rc.top, filename );
    //DrawText (HDC_SCREEN, filename, -1, &rc, DT_CENTER );  
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

static int load_icons( void ){
    hicon_folder = LoadIconFromFile( HDC_SCREEN, ICON_FOLDER_FILENAME, 0 );
	if( !hicon_folder ){
	    fprintf( stderr, "Erreur chargement folder.ico\n" );
	    return FALSE;
	}
	
    hicon_file = LoadIconFromFile( HDC_SCREEN, ICON_FILE_FILENAME, 0 );
    if( !hicon_file ){
        fprintf( stderr, "Erreur chargement file.ico\n" );
        return FALSE;
    }

    return TRUE;
}

static int listbox_cmp( char * path, char * v1, char * v2 ){
    struct stat ftype1,ftype2;
    char f_v1[MAX_PATH+1];
    char f_v2[MAX_PATH+1];

    sprintf( f_v1, "%s/%s", path, v1 );
    sprintf( f_v2, "%s/%s", path, v2 );
    stat (f_v1, &ftype1);
    stat (f_v2, &ftype2);
    
    if( S_ISDIR (ftype1.st_mode) && S_ISDIR (ftype2.st_mode) ) return strcasecmp( v1, v2 );
    else if( S_ISDIR (ftype1.st_mode) ) return -1;
    else if( S_ISDIR (ftype2.st_mode) ) return +1;
    else return strcasecmp( v1, v2 );
}

static int listbox_video_cmp( char * v1, char * v2, size_t n ){
    char path[MAX_PATH+1];

    GetWindowText (GetDlgItem (GetParent (hlb_video), IDC_PATH_VIDEO), path, MAX_PATH);
    return listbox_cmp( path, v1, v2 );
}

static int listbox_audio_cmp( char * v1, char * v2, size_t n ){
    char path[MAX_PATH+1];

    GetWindowText (GetDlgItem (GetParent (hlb_audio), IDC_PATH_AUDIO), path, MAX_PATH);
    return listbox_cmp( path, v1, v2 );
}


static int TomPlayerVideoProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
    char folder[MAX_PATH+1];
    int i;
    char filename[MAX_NAME+1];
    
    switch (message) {
        case MSG_INITPAGE:
            hlb_video = GetDlgItem (hDlg, IDL_FILE_VIDEO);
            SendMessage (hlb_video, LB_SETSTRCMPFUNC, 0, (LPARAM)listbox_video_cmp);
            fill_boxes (hDlg, IDL_FILE_VIDEO, IDC_PATH_VIDEO, config.filter_video_ext, config.video_folder);
            SetNotificationCallback (GetDlgItem (hDlg, IDL_FILE_VIDEO), video_file_notif_proc);
            break;
    
        case MSG_SHOWPAGE:
            return 1;
    
        case MSG_COMMAND:
            switch (wParam) {
                case IDB_PLAY_VIDEO:
                    GetWindowText (GetDlgItem (hDlg, IDC_PATH_VIDEO), folder, MAX_PATH);
                    strcat( folder, "/" );
                    i = SendDlgItemMessage (hDlg, IDL_FILE_VIDEO, LB_GETCURSEL, 0, 0);
                    if( i == LB_ERR ){
                        MessageBox (hDlg, "No file selected", "TomPlayer", MB_OK | MB_ICONINFORMATION);      
                        break;
                    } else {
                        SendDlgItemMessage (hDlg, IDL_FILE_VIDEO, LB_GETTEXT, i, (LPARAM)filename) ;
                    }
                    play (GetParent(hDlg), folder, filename, FALSE);
                    break;
                case IDB_EXIT_VIDEO:
                    EndDialog (GetParent(hDlg), wParam);
                    display_image_to_fb( config.bitmap_exiting );
                    exit(0);
                    break;
                case IDB_RESUME_VIDEO:
                    play (GetParent(hDlg), "",filename,TRUE);
                    break;
            }
            break;

    }
    
    return DefaultPageProc (hDlg, message, wParam, lParam);
}


static int TomPlayerAudioProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
    char folder[MAX_PATH+1];
    int i;
    char filename[MAX_NAME+1];
    
    switch (message) {
        case MSG_INITPAGE:
            hlb_audio = GetDlgItem (hDlg, IDL_FILE_AUDIO);
            SendMessage (hlb_audio, LB_SETSTRCMPFUNC, 0, (LPARAM)listbox_audio_cmp);
            fill_boxes (hDlg, IDL_FILE_AUDIO, IDC_PATH_AUDIO, config.filter_audio_ext, config.audio_folder);
            SetNotificationCallback (GetDlgItem (hDlg, IDL_FILE_AUDIO), audio_file_notif_proc);        
            break;
    
        case MSG_SHOWPAGE:
            return 1;
    
        case MSG_COMMAND:
            switch (wParam) {
                case IDB_PLAY_AUDIO:
                    GetWindowText (GetDlgItem (hDlg, IDC_PATH_AUDIO), folder, MAX_PATH);
                    strcat( folder, "/" );
                    i = SendDlgItemMessage (hDlg, IDL_FILE_AUDIO, LB_GETCURSEL, 0, 0);
                    if( i == LB_ERR ){
                        MessageBox (hDlg, "No file selected", "TomPlayer", MB_OK | MB_ICONINFORMATION);      
                        break;
                    } else {
                        SendDlgItemMessage (hDlg, IDL_FILE_AUDIO, LB_GETTEXT, i, (LPARAM)filename) ;
                    }
                    play (GetParent(hDlg), folder, filename, FALSE);
                    break;
                case IDB_EXIT_AUDIO:
                    EndDialog (GetParent(hDlg), wParam);
                    display_image_to_fb( config.bitmap_exiting );
                    exit(0);
                    break;
                case IDB_RANDOM_AUDIO:
                    GetWindowText (GetDlgItem (hDlg, IDC_PATH_AUDIO), folder, MAX_PATH);
                    if( generate_random_playlist( folder, DEFAULT_PLAYLIST ) == TRUE )
                        play (GetParent(hDlg), "",DEFAULT_PLAYLIST, FALSE);
                    else MessageBox (hDlg, "Unable to create playlist", "TomPlayer", MB_OK | MB_ICONINFORMATION);
                    break;
            }
    }
    
    return DefaultPageProc (hDlg, message, wParam, lParam);
}


static int TomPlayerAudioSkinProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
    char zip_file[MAX_PATH+1];
    int i;
    char filename[MAX_NAME+1];

    switch (message) {
        case MSG_INITPAGE:
            fill_boxes (hDlg, IDL_FILE_AUDIO_SKIN, IDC_PATH_AUDIO_SKIN, ".zip", AUDIO_SKIN_FOLDER);
            break;
    
        case MSG_SHOWPAGE:
            return 1;
    
        case MSG_COMMAND:
            switch (wParam) {
                case IDB_SELECT_AUDIO_SKIN:
                    GetWindowText (GetDlgItem (hDlg, IDC_PATH_AUDIO_SKIN), zip_file, MAX_PATH);
                    strcat( zip_file, "/" );
                    i = SendDlgItemMessage (hDlg, IDL_FILE_AUDIO_SKIN, LB_GETCURSEL, 0, 0);
                    if( i == LB_ERR ){
                        MessageBox (hDlg, "No file selected", "TomPlayer", MB_OK | MB_ICONINFORMATION);      
                        break;
                    } else {
                        SendDlgItemMessage (hDlg, IDL_FILE_AUDIO_SKIN, LB_GETTEXT, i, (LPARAM)filename) ;
                    }
                    strcat( zip_file, filename );
                    unload_skin( &config.audio_config );
                    
                    if( load_skin_from_zip( zip_file, &config.audio_config ) == FALSE ){
                        MessageBox (hDlg, "Unable to load this skin", "Error", MB_OK | MB_ICONINFORMATION);
                        load_skin_from_zip( config.audio_skin_filename, &config.audio_config );
                    }
                    else{
                        strcpy( config.audio_skin_filename, zip_file );
                        SetValueToEtcFile (CONFIG_FILE, SECTION_AUDIO_SKIN, KEY_SKIN_FILENAME, zip_file);
                    }
                    break;
            }
    }
    
    return DefaultPageProc (hDlg, message, wParam, lParam);
}

static int TomPlayerVideoSkinProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
    char zip_file[MAX_PATH+1];
    int i;
    char filename[MAX_NAME+1];

    switch (message) {
        case MSG_INITPAGE:
            fill_boxes (hDlg, IDL_FILE_VIDEO_SKIN, IDC_PATH_VIDEO_SKIN, ".zip", VIDEO_SKIN_FOLDER);
            break;
    
        case MSG_SHOWPAGE:
            return 1;
    
        case MSG_COMMAND:
            switch (wParam) {
                case IDB_SELECT_VIDEO_SKIN:
                    GetWindowText (GetDlgItem (hDlg, IDC_PATH_VIDEO_SKIN), zip_file, MAX_PATH);
                    strcat( zip_file, "/" );
                    i = SendDlgItemMessage (hDlg, IDL_FILE_VIDEO_SKIN, LB_GETCURSEL, 0, 0);
                    if( i == LB_ERR ){
                        MessageBox (hDlg, "No file selected", "TomPlayer", MB_OK | MB_ICONINFORMATION);      
                        break;
                    } else {
                        SendDlgItemMessage (hDlg, IDL_FILE_VIDEO_SKIN, LB_GETTEXT, i, (LPARAM)filename) ;
                    }
                    strcat( zip_file, filename );
                    unload_skin( &config.video_config );
                    load_skin_from_zip( zip_file, &config.video_config );
                    
                    if( load_skin_from_zip( zip_file, &config.video_config ) == FALSE ){
                        MessageBox (hDlg, "Unable to load this skin", "Error", MB_OK | MB_ICONINFORMATION);
                        load_skin_from_zip( config.video_skin_filename, &config.video_config );
                    }
                    else{
                        strcpy( config.video_skin_filename, zip_file );
                        SetValueToEtcFile (CONFIG_FILE, SECTION_VIDEO_SKIN, KEY_SKIN_FILENAME, zip_file);
                    }
                    break;
            }
    }
    
    return DefaultPageProc (hDlg, message, wParam, lParam);
}

static int TomPlayerPropSheetProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{

    switch (message) {
        case MSG_INITDIALOG:
            if( load_config(&config) == FALSE ){
                fprintf( stderr, "Error while loading config\n" );
                exit(1);
            }
            if( load_icons() == FALSE ){
                fprintf( stderr, "Error while loading icons\n" );
                exit(1);
            }
            
            
            HWND pshwnd = GetDlgItem (hDlg, IDC_PROPSHEET);

            DlgTomPlayerVideo.controls = CtrlTomPlayerVideo;
            ExtendDialogBoxToScreen( &DlgTomPlayerVideo );
            SendMessage (pshwnd, PSM_ADDPAGE, (WPARAM)&DlgTomPlayerVideo, (LPARAM)TomPlayerVideoProc);

            DlgTomPlayerAudio.controls = CtrlTomPlayerAudio;
            ExtendDialogBoxToScreen( &DlgTomPlayerAudio );
            SendMessage (pshwnd, PSM_ADDPAGE, (WPARAM)&DlgTomPlayerAudio, (LPARAM)TomPlayerAudioProc);
            
            DlgTomPlayerAudioSkin.controls = CtrlTomPlayerAudioSkin;
            ExtendDialogBoxToScreen( &DlgTomPlayerAudioSkin );
            SendMessage (pshwnd, PSM_ADDPAGE, (WPARAM)&DlgTomPlayerAudioSkin, (LPARAM)TomPlayerAudioSkinProc);

            DlgTomPlayerVideoSkin.controls = CtrlTomPlayerVideoSkin;
            ExtendDialogBoxToScreen( &DlgTomPlayerVideoSkin );
            SendMessage (pshwnd, PSM_ADDPAGE, (WPARAM)&DlgTomPlayerVideoSkin, (LPARAM)TomPlayerVideoSkinProc);
            
            SendMessage (pshwnd, PSM_SETACTIVEINDEX, (WPARAM)0, (LPARAM)0);
            
            ShowCursor(FALSE);
            SetTimer (hDlg, ID_TIMER, 100);
    
            return 1;
            
        case MSG_TIMER:
            if( ( is_playing_video == TRUE || is_playing_audio == TRUE ) && is_mplayer_finished == TRUE ){
                is_playing_video = FALSE;
                is_playing_audio = FALSE;
                /* wolf : will be reinit on mplayer launch => Harmfull for udpate thread exit for now...
                 * is_mplayer_finished = FALSE;*/
                ShowWindow( hDlg, SW_SHOW );
                ShowWindow( GetDlgItem (hDlg, IDC_PROPSHEET), SW_SHOW );
                UpdateWindow( hDlg, TRUE );
                UpdateWindow( GetDlgItem (hDlg, IDC_PROPSHEET), TRUE );

                /* Turn ON screen if it is not */
                pwm_resume();                               
            }
            /* Test OFF butto, */
            if (power_is_off_button_pushed()){
            	EndDialog (hDlg, wParam);
            	display_image_to_fb( config.bitmap_exiting );
            	exit(0);
            }
            break;

        case MSG_CLOSE:
            EndDialog (hDlg, wParam);
            display_image_to_fb( config.bitmap_exiting );
            exit(0);
    }
    
    return DefaultDialogProc (hDlg, message, wParam, lParam);
}


int MiniGUIMain (int argc, const char* argv[])
{

    init_engine();
	
    SetWindowBkColor( HWND_DESKTOP, 0 );
    RegisterMouseMsgHook(HWND_DESKTOP, mouse_hook);
    
    DlgTomPlayerPropSheet.controls = CtrlTomPlayerPropSheet;
    
    ExtendDialogBoxToScreen( &DlgTomPlayerPropSheet );
    
    DialogBoxIndirectParam (&DlgTomPlayerPropSheet, HWND_DESKTOP, TomPlayerPropSheetProc, 0L);

    release_engine();
    
    return 0;
}
