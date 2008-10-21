#ifndef __FILE_SELECTOR_H__

#include <stdint.h>

enum fs_icon_ids{
    FS_ICON_UP,
    FS_ICON_DOWN,
    FS_ICON_CHECK,
    FS_ICON_FOLDER,
    FS_MAX_ICON_ID 
};

struct fs_config{

  struct {
    const char * filename[FS_MAX_ICON_ID]; 
    const char * font;
    DFBColor font_color;
  } graphics;

  struct {
    int x;
    int y;
    int height;
    int width;   
    int preview_width_ratio;
  } geometry;

  struct {
    bool preview_box ;
    bool multiple_selection;
  } options;
  
  struct {
    const char * filter;
    const char * pathname;
  } folder;
} ;

typedef struct fs_data * fs_handle;
typedef void (select_cb)(IDirectFBSurface *, const char *);
typedef struct _fl_handle * fslist;

fs_handle fs_create (IDirectFB  *, IDirectFBSurface *,const struct fs_config *);
bool fs_set_select_cb(fs_handle, select_cb * );
bool fs_release(fs_handle);
bool fs_select(fs_handle, int);
const char * fs_get_single_selection(fs_handle);
fslist   fs_get_selection(fs_handle);

const char * fslist_get_next_file(fslist);
bool fslist_release(fslist);

#endif /* __FILE_SELECTOR_H__ */
