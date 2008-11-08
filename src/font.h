#ifndef __FONT_H__
#define __FONT_H__

struct font_color{
  unsigned char r;
  unsigned char g;
  unsigned char b;
};


bool font_draw(const struct font_color * ,  const char *, unsigned char ** , int * , int *);
bool font_init(int );
void font_release(void);

#endif
