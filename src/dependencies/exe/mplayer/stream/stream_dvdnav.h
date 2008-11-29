#ifndef MPLAYER_STREAM_DVDNAV_H
#define MPLAYER_STREAM_DVDNAV_H

typedef struct {
  int event;             /* event number fromd dvdnav_events.h */
  void * details;        /* event details */
  int len;               /* bytes in details */
} dvdnav_event_t;

typedef struct {
  uint16_t sx, sy;
  uint16_t ex, ey;
} nav_highlight_t;

int dvdnav_number_of_subs(stream_t *stream);
int dvdnav_aid_from_lang(stream_t *stream, unsigned char *language);
int dvdnav_lang_from_aid(stream_t *stream, int id, unsigned char *buf);
int dvdnav_sid_from_lang(stream_t *stream, unsigned char *language);
int dvdnav_lang_from_sid(stream_t *stream, int sid, unsigned char *buf);
int mp_dvdnav_handle_input(stream_t *stream, int cmd, int *button);
void mp_dvdnav_update_mouse_pos(stream_t *stream, int32_t x, int32_t y, int* button);
void mp_dvdnav_get_highlight (stream_t *stream, nav_highlight_t *hl);
unsigned int *mp_dvdnav_get_spu_clut(stream_t *stream);

#endif /* MPLAYER_STREAM_DVDNAV_H */
