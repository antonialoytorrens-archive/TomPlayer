/*
   (c) Copyright 2001-2008  The world wide DirectFB Open Source Community (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrjälä <syrjala@sci.fi> and
              Claudio Ciccani <klan@users.sf.net>.

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

#include <config.h>


#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
typedef unsigned long kernel_ulong_t;
#define BITS_PER_LONG    (sizeof(long)*8)
#endif

/* compat defines for older kernel like 2.4.x */
#ifndef EV_SYN
#define EV_SYN			0x00
#define SYN_REPORT              0
#define SYN_CONFIG              1
#define ABS_TOOL_WIDTH		0x1c
#define BTN_TOOL_DOUBLETAP	0x14d
#define BTN_TOOL_TRIPLETAP	0x14e
#endif

#include <linux/input.h>
#ifndef KEY_OK
/* Linux kernel 2.5.42+ defines additional keys in linux/input.h */
#include "input_fake.h"
#endif

#ifndef EVIOCGLED
#define EVIOCGLED(len) _IOC(_IOC_READ, 'E', 0x19, len)
#endif

#ifndef EVIOCGRAB
#define EVIOCGRAB _IOW('E', 0x90, int)
#endif

#include <linux/keyboard.h>


#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <dfb_types.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/kd.h>
#include <stdlib.h>


#include <directfb.h>
#include <directfb_keyboard.h>

#include <core/coredefs.h>
#include <core/coretypes.h>
#include <core/input.h>
#include <core/system.h>

#include <direct/debug.h>
#include <direct/mem.h>
#include <direct/messages.h>
#include <direct/thread.h>
#include <direct/util.h>

#include <misc/conf.h>
#include <misc/util.h>

#ifdef LINUX_INPUT_USE_FBDEV
#include <fbdev/fbdev.h>
#endif

#include <core/input_driver.h>


DFB_INPUT_DRIVER( linux_input )

#ifndef BITS_PER_LONG
#define BITS_PER_LONG        (sizeof(long) * 8)
#endif
#define NBITS(x)             ((((x)-1)/BITS_PER_LONG)+1)
#define OFF(x)               ((x)%BITS_PER_LONG)
#define BIT(x)               (1UL<<OFF(x))
#define LONG(x)              ((x)/BITS_PER_LONG)
#undef test_bit
#define test_bit(bit, array) ((array[LONG(bit)] >> OFF(bit)) & 1)

/* compat for 2.4.x kernel - just a compile fix */
#ifndef HAVE_INPUT_ABSINFO
struct input_absinfo {
        s32 value;
        s32 minimum;
        s32 maximum;
        s32 fuzz;
        s32 flat;
};
#endif


/*
 * declaration of private data
 */
typedef struct {
     CoreInputDevice         *device;
     DirectThread            *thread;

     int                      fd;

     bool                     has_leds;
     unsigned long            led_state[NBITS(LED_MAX)];
     DFBInputDeviceLockState  locks;

#ifdef LINUX_INPUT_USE_FBDEV
     VirtualTerminal         *vt;
#endif

     int                      dx;
     int                      dy;

     bool                     touchpad;
} LinuxInputData;


#define MAX_LINUX_INPUT_DEVICES 16

static int num_devices = 0;
static char *device_names[MAX_LINUX_INPUT_DEVICES];


static const
int basic_keycodes [] = {
     DIKI_UNKNOWN, DIKI_ESCAPE, DIKI_1, DIKI_2, DIKI_3, DIKI_4, DIKI_5,
     DIKI_6, DIKI_7, DIKI_8, DIKI_9, DIKI_0, DIKI_MINUS_SIGN,
     DIKI_EQUALS_SIGN, DIKI_BACKSPACE,

     DIKI_TAB, DIKI_Q, DIKI_W, DIKI_E, DIKI_R, DIKI_T, DIKI_Y, DIKI_U,
     DIKI_I, DIKI_O, DIKI_P, DIKI_BRACKET_LEFT, DIKI_BRACKET_RIGHT,
     DIKI_ENTER,

     DIKI_CONTROL_L, DIKI_A, DIKI_S, DIKI_D, DIKI_F, DIKI_G, DIKI_H, DIKI_J,
     DIKI_K, DIKI_L, DIKI_SEMICOLON, DIKI_QUOTE_RIGHT, DIKI_QUOTE_LEFT,

     DIKI_SHIFT_L, DIKI_BACKSLASH, DIKI_Z, DIKI_X, DIKI_C, DIKI_V, DIKI_B,
     DIKI_N, DIKI_M, DIKI_COMMA, DIKI_PERIOD, DIKI_SLASH, DIKI_SHIFT_R,
     DIKI_KP_MULT, DIKI_ALT_L, DIKI_SPACE, DIKI_CAPS_LOCK,

     DIKI_F1, DIKI_F2, DIKI_F3, DIKI_F4, DIKI_F5, DIKI_F6, DIKI_F7, DIKI_F8,
     DIKI_F9, DIKI_F10, DIKI_NUM_LOCK, DIKI_SCROLL_LOCK,

     DIKI_KP_7, DIKI_KP_8, DIKI_KP_9, DIKI_KP_MINUS,
     DIKI_KP_4, DIKI_KP_5, DIKI_KP_6, DIKI_KP_PLUS,
     DIKI_KP_1, DIKI_KP_2, DIKI_KP_3, DIKI_KP_0, DIKI_KP_DECIMAL,

     /*KEY_103RD,*/ DIKI_BACKSLASH,
     /*KEY_F13,*/ DFB_FUNCTION_KEY(13),
     /*KEY_102ND*/ DIKI_LESS_SIGN,

     DIKI_F11, DIKI_F12, DFB_FUNCTION_KEY(14), DFB_FUNCTION_KEY(15),
     DFB_FUNCTION_KEY(16), DFB_FUNCTION_KEY(17), DFB_FUNCTION_KEY(18),
     DFB_FUNCTION_KEY(19), DFB_FUNCTION_KEY(20),

     DIKI_KP_ENTER, DIKI_CONTROL_R, DIKI_KP_DIV, DIKI_PRINT, DIKS_ALTGR,

     /*KEY_LINEFEED*/ DIKI_UNKNOWN,

     DIKI_HOME, DIKI_UP, DIKI_PAGE_UP, DIKI_LEFT, DIKI_RIGHT,
     DIKI_END, DIKI_DOWN, DIKI_PAGE_DOWN, DIKI_INSERT, DIKI_DELETE,

     /*KEY_MACRO,*/ DIKI_UNKNOWN,

     DIKS_MUTE, DIKS_VOLUME_DOWN, DIKS_VOLUME_UP, DIKS_POWER, DIKI_KP_EQUAL,

     /*KEY_KPPLUSMINUS,*/ DIKI_UNKNOWN,

     DIKS_PAUSE, DFB_FUNCTION_KEY(21), DFB_FUNCTION_KEY(22),
     DFB_FUNCTION_KEY(23), DFB_FUNCTION_KEY(24),

     DIKI_KP_SEPARATOR, DIKI_META_L, DIKI_META_R, DIKI_SUPER_L,

     DIKS_STOP,

     /*DIKS_AGAIN, DIKS_PROPS, DIKS_UNDO, DIKS_FRONT, DIKS_COPY,
     DIKS_OPEN, DIKS_PASTE, DIKS_FIND, DIKS_CUT,*/
     DIKI_UNKNOWN, DIKI_UNKNOWN, DIKI_UNKNOWN, DIKI_UNKNOWN, DIKI_UNKNOWN,
     DIKI_UNKNOWN, DIKI_UNKNOWN, DIKI_UNKNOWN, DIKI_UNKNOWN,

     DIKS_HELP, DIKS_MENU, DIKS_CALCULATOR, DIKS_SETUP,

     /*KEY_SLEEP, KEY_WAKEUP, KEY_FILE, KEY_SENDFILE, KEY_DELETEFILE,
     KEY_XFER,*/
     DIKI_UNKNOWN, DIKI_UNKNOWN, DIKI_UNKNOWN, DIKI_UNKNOWN, DIKI_UNKNOWN,
     DIKI_UNKNOWN,

     /*KEY_PROG1, KEY_PROG2,*/
     DIKS_CUSTOM1, DIKS_CUSTOM2,

     DIKS_INTERNET,

     /*KEY_MSDOS, KEY_COFFEE, KEY_DIRECTION, KEY_CYCLEWINDOWS,*/
     DIKI_UNKNOWN, DIKI_UNKNOWN, DIKI_UNKNOWN, DIKI_UNKNOWN,

     DIKS_MAIL,

     /*KEY_BOOKMARKS, KEY_COMPUTER, */
     DIKI_UNKNOWN, DIKI_UNKNOWN,

     DIKS_BACK, DIKS_FORWARD,

     /*KEY_CLOSECD, KEY_EJECTCD, KEY_EJECTCLOSECD,*/
     DIKS_EJECT, DIKS_EJECT, DIKS_EJECT,

     DIKS_NEXT, DIKS_PLAYPAUSE, DIKS_PREVIOUS, DIKS_STOP, DIKS_RECORD,
     DIKS_REWIND, DIKS_PHONE,

     /*KEY_ISO,*/ DIKI_UNKNOWN,
     /*KEY_CONFIG,*/ DIKS_SETUP,
     /*KEY_HOMEPAGE, KEY_REFRESH,*/ DIKI_UNKNOWN, DIKS_SHUFFLE,

     DIKS_EXIT, /*KEY_MOVE,*/ DIKI_UNKNOWN, DIKS_EDITOR,

     /*KEY_SCROLLUP,*/ DIKS_PAGE_UP,
     /*KEY_SCROLLDOWN,*/ DIKS_PAGE_DOWN,
     /*KEY_KPLEFTPAREN,*/ DIKI_UNKNOWN,
     /*KEY_KPRIGHTPAREN,*/ DIKI_UNKNOWN,

     /* unused codes 181-182: */
     DIKI_UNKNOWN, DIKI_UNKNOWN,

     DFB_FUNCTION_KEY(13), DFB_FUNCTION_KEY(14), DFB_FUNCTION_KEY(15),
     DFB_FUNCTION_KEY(16), DFB_FUNCTION_KEY(17), DFB_FUNCTION_KEY(18),
     DFB_FUNCTION_KEY(19), DFB_FUNCTION_KEY(20), DFB_FUNCTION_KEY(21),
     DFB_FUNCTION_KEY(22), DFB_FUNCTION_KEY(23), DFB_FUNCTION_KEY(24),

     /* unused codes 195-199: */
     DIKI_UNKNOWN, DIKI_UNKNOWN, DIKI_UNKNOWN, DIKI_UNKNOWN, DIKI_UNKNOWN,

     /* KEY_PLAYCD, KEY_PAUSECD */
     DIKS_PLAY, DIKS_PAUSE,

     /*KEY_PROG3, KEY_PROG4,*/
     DIKS_CUSTOM3, DIKS_CUSTOM4,

     DIKI_UNKNOWN,

     /*KEY_SUSPEND, KEY_CLOSE*/
     DIKI_UNKNOWN, DIKI_UNKNOWN,

     /* KEY_PLAY */
     DIKS_PLAY,

     /* KEY_FASTFORWARD */
     DIKS_FASTFORWARD,

     /* KEY_BASSBOOST */
     DIKI_UNKNOWN,

     /* KEY_PRINT */
     DIKS_PRINT,

     /* KEY_HP             */  DIKI_UNKNOWN,
     /* KEY_CAMERA         */  DIKI_UNKNOWN,
     /* KEY_SOUND          */  DIKS_AUDIO,
     /* KEY_QUESTION       */  DIKS_HELP,
     /* KEY_EMAIL          */  DIKS_MAIL,
     /* KEY_CHAT           */  DIKI_UNKNOWN,
     /* KEY_SEARCH         */  DIKI_UNKNOWN,
     /* KEY_CONNECT        */  DIKI_UNKNOWN,
     /* KEY_FINANCE        */  DIKI_UNKNOWN,
     /* KEY_SPORT          */  DIKI_UNKNOWN,
     /* KEY_SHOP           */  DIKI_UNKNOWN,
     /* KEY_ALTERASE       */  DIKI_UNKNOWN,
     /* KEY_CANCEL         */  DIKS_CANCEL,
     /* KEY_BRIGHTNESSDOWN */  DIKI_UNKNOWN,
     /* KEY_BRIGHTNESSUP   */  DIKI_UNKNOWN,
     /* KEY_MEDIA          */  DIKI_UNKNOWN,
};

static const
int ext_keycodes [] = {
     DIKS_OK, DIKS_SELECT, DIKS_GOTO, DIKS_CLEAR, DIKS_POWER2, DIKS_OPTION,
     DIKS_INFO, DIKS_TIME, DIKS_VENDOR, DIKS_ARCHIVE, DIKS_PROGRAM,
     DIKS_CHANNEL, DIKS_FAVORITES, DIKS_EPG, DIKS_PVR, DIKS_MHP,
     DIKS_LANGUAGE, DIKS_TITLE, DIKS_SUBTITLE, DIKS_ANGLE, DIKS_ZOOM,
     DIKS_MODE, DIKS_KEYBOARD, DIKS_SCREEN, DIKS_PC, DIKS_TV, DIKS_TV2,
     DIKS_VCR, DIKS_VCR2, DIKS_SAT, DIKS_SAT2, DIKS_CD, DIKS_TAPE,
     DIKS_RADIO, DIKS_TUNER, DIKS_PLAYER, DIKS_TEXT, DIKS_DVD, DIKS_AUX,
     DIKS_MP3, DIKS_AUDIO, DIKS_VIDEO, DIKS_DIRECTORY, DIKS_LIST, DIKS_MEMO,
     DIKS_CALENDAR, DIKS_RED, DIKS_GREEN, DIKS_YELLOW, DIKS_BLUE,
     DIKS_CHANNEL_UP, DIKS_CHANNEL_DOWN, DIKS_FIRST, DIKS_LAST, DIKS_AB,
     DIKS_NEXT, DIKS_RESTART, DIKS_SLOW, DIKS_SHUFFLE, DIKS_FASTFORWARD,
     DIKS_PREVIOUS, DIKS_NEXT, DIKS_DIGITS, DIKS_TEEN, DIKS_TWEN, DIKS_BREAK
};

/*
 * Touchpads related stuff
 */
enum {
     TOUCHPAD_FSM_START,
     TOUCHPAD_FSM_MAIN,
     TOUCHPAD_FSM_DRAG_START,
     TOUCHPAD_FSM_DRAG_MAIN,
};
struct touchpad_axis {
     int old, min, max;
};
struct touchpad_fsm_state {
     int fsm_state;
     struct touchpad_axis x;
     struct touchpad_axis y;
     struct timeval timeout;
};

static void
touchpad_fsm_init( struct touchpad_fsm_state *state );
static int
touchpad_fsm( struct touchpad_fsm_state *state,
              const struct input_event  *levt,
              DFBInputEvent             *devt );

static bool
timeout_passed( const struct timeval *timeout, const struct timeval *current );
static bool
timeout_is_set( const struct timeval *timeout );
static void
timeout_sub( struct timeval *timeout, const struct timeval *sub );

/*
 * Translates a Linux input keycode into a DirectFB keycode.
 */
static int
translate_key( unsigned short code )
{
     if (code < D_ARRAY_SIZE( basic_keycodes ))
          return basic_keycodes[code];

     if (code >= KEY_OK)
          if (code - KEY_OK < D_ARRAY_SIZE( ext_keycodes ))
               return ext_keycodes[code-KEY_OK];

     return DIKI_UNKNOWN;
}

#ifdef LINUX_INPUT_USE_FBDEV
static DFBInputDeviceKeySymbol
keyboard_get_symbol( int                             code,
                     unsigned short                  value,
                     DFBInputDeviceKeymapSymbolIndex level )
{
     unsigned char type  = KTYP(value);
     unsigned char index = KVAL(value);
     int           base  = (level == DIKSI_BASE);

     switch (type) {
          case KT_FN:
               if (index < 20)
                    return DFB_FUNCTION_KEY( index + 1 );
               break;
          case KT_LETTER:
          case KT_LATIN:
               switch (index) {
                    case 0x1c:
                         return DIKS_PRINT;
                    case 0x7f:
                         return DIKS_BACKSPACE;
                    case 0xa4:
                         return 0x20ac; /* euro currency sign */
                    default:
                         return index;
               }
               break;
          case KT_DEAD:
               switch (value) {
                    case K_DGRAVE:
                         return DIKS_DEAD_GRAVE;

                    case K_DACUTE:
                         return DIKS_DEAD_ACUTE;

                    case K_DCIRCM:
                         return DIKS_DEAD_CIRCUMFLEX;

                    case K_DTILDE:
                         return DIKS_DEAD_TILDE;

                    case K_DDIERE:
                         return DIKS_DEAD_DIAERESIS;

                    case K_DCEDIL:
                         return DIKS_DEAD_CEDILLA;

                    default:
                         break;
               }
               break;
          case KT_PAD:
               if (index <= 9 && level != DIKSI_BASE)
                    return DIKS_0 + index;
               break;
          case 0xe: /* special IPAQ H3600 case - AH */
               switch (index) {
                    case 0x20:     return DIKS_CALENDAR;
                    case 0x1a:     return DIKS_BACK;
                    case 0x1c:     return DIKS_MEMO;
                    case 0x21:     return DIKS_POWER;
               }
               break;
          case 0xd: /* another special IPAQ H3600 case - AH */
               switch (index) {
                    case 0x2:     return DIKS_DIRECTORY;
                    case 0x1:     return DIKS_MAIL;  /* Q on older iPaqs */
               }
               break;
     }

     switch (value) {
          case K_LEFT:    return DIKS_CURSOR_LEFT;
          case K_RIGHT:   return DIKS_CURSOR_RIGHT;
          case K_UP:      return DIKS_CURSOR_UP;
          case K_DOWN:    return DIKS_CURSOR_DOWN;
          case K_ENTER:   return DIKS_ENTER;
          case K_CTRL:    return DIKS_CONTROL;
          case K_SHIFT:   return DIKS_SHIFT;
          case K_ALT:     return DIKS_ALT;
          case K_ALTGR:   return DIKS_ALTGR;
          case K_INSERT:  return DIKS_INSERT;
          case K_REMOVE:  return DIKS_DELETE;
          case K_FIND:    return DIKS_HOME;
          case K_SELECT:  return DIKS_END;
          case K_PGUP:    return DIKS_PAGE_UP;
          case K_PGDN:    return DIKS_PAGE_DOWN;
          case K_NUM:     return DIKS_NUM_LOCK;
          case K_HOLD:    return DIKS_SCROLL_LOCK;
          case K_PAUSE:   return DIKS_PAUSE;
          case K_BREAK:   return DIKS_BREAK;
          case K_CAPS:    return DIKS_CAPS_LOCK;

          case K_P0:      return DIKS_INSERT;
          case K_P1:      return DIKS_END;
          case K_P2:      return DIKS_CURSOR_DOWN;
          case K_P3:      return DIKS_PAGE_DOWN;
          case K_P4:      return DIKS_CURSOR_LEFT;
          case K_P5:      return DIKS_BEGIN;
          case K_P6:      return DIKS_CURSOR_RIGHT;
          case K_P7:      return DIKS_HOME;
          case K_P8:      return DIKS_CURSOR_UP;
          case K_P9:      return DIKS_PAGE_UP;
          case K_PPLUS:   return DIKS_PLUS_SIGN;
          case K_PMINUS:  return DIKS_MINUS_SIGN;
          case K_PSTAR:   return DIKS_ASTERISK;
          case K_PSLASH:  return DIKS_SLASH;
          case K_PENTER:  return DIKS_ENTER;
          case K_PCOMMA:  return base ? DIKS_DELETE : DIKS_COMMA;
          case K_PDOT:    return base ? DIKS_DELETE : DIKS_PERIOD;
          case K_PPARENL: return DIKS_PARENTHESIS_LEFT;
          case K_PPARENR: return DIKS_PARENTHESIS_RIGHT;
     }

     /* special keys not in the map, hack? */
     if (code == 99)
          return DIKS_PRINT;

     if (code == 124)         /* keypad equal key */
          return DIKS_EQUALS_SIGN;

     if (code == 125)         /* left windows key */
          return DIKS_META;

     if (code == 126)         /* right windows key */
          return DIKS_META;

     if (code == 127)         /* context menu key */
          return DIKS_SUPER;

     return DIKS_NULL;
}

static DFBInputDeviceKeyIdentifier
keyboard_get_identifier( int code, unsigned short value )
{
     unsigned char type  = KTYP(value);
     unsigned char index = KVAL(value);

     if (type == KT_PAD) {
          if (index <= 9)
               return DIKI_KP_0 + index;

          switch (value) {
               case K_PSLASH: return DIKI_KP_DIV;
               case K_PSTAR:  return DIKI_KP_MULT;
               case K_PMINUS: return DIKI_KP_MINUS;
               case K_PPLUS:  return DIKI_KP_PLUS;
               case K_PENTER: return DIKI_KP_ENTER;
               case K_PCOMMA:
               case K_PDOT:   return DIKI_KP_DECIMAL;
          }
     }

     /* Looks like a hack, but don't know a better way yet. */
     switch (code) {
          case 12: return DIKI_MINUS_SIGN;
          case 13: return DIKI_EQUALS_SIGN;
          case 26: return DIKI_BRACKET_LEFT;
          case 27: return DIKI_BRACKET_RIGHT;
          case 39: return DIKI_SEMICOLON;
          case 40: return DIKI_QUOTE_RIGHT;
          case 41: return DIKI_QUOTE_LEFT;
          case 43: return DIKI_BACKSLASH;
          case 51: return DIKI_COMMA;
          case 52: return DIKI_PERIOD;
          case 53: return DIKI_SLASH;
          case 54: return DIKI_SHIFT_R;
          case 97: return DIKI_CONTROL_R;
          case 100: return DIKI_ALT_R;
          default:
               ;
     }

     /* special keys not in the map, hack? */
     if (code == 124)         /* keypad equal key */
          return DIKI_KP_EQUAL;

     if (code == 125)         /* left windows key */
          return DIKI_META_L;

     if (code == 126)         /* right windows key */
          return DIKI_META_R;

     if (code == 127)         /* context menu key */
          return DIKI_SUPER_R;

     return DIKI_UNKNOWN;
}

static unsigned short
keyboard_read_value( const LinuxInputData *data,
                     unsigned char table, unsigned char index )
{
     struct kbentry entry;

     entry.kb_table = table;
     entry.kb_index = index;
     entry.kb_value = 0;

     if (ioctl( data->vt->fd, KDGKBENT, &entry )) {
          D_PERROR("DirectFB/keyboard: KDGKBENT (table: %d, index: %d) "
                    "failed!\n", table, index);
          return 0;
     }

     return entry.kb_value;
}
#endif /* LINUX_INPUT_USE_FBDEV */

/*
 * Translates key and button events.
 */
static bool
key_event( const struct input_event *levt,
           DFBInputEvent            *devt )
{
     int code = levt->code;

     /* map touchscreen and smartpad events to button mouse */
     if (code == BTN_TOUCH || code == BTN_TOOL_FINGER)
          code = BTN_MOUSE;

     if ((code >= BTN_MOUSE && code < BTN_JOYSTICK) || code == BTN_TOUCH) {
          /* ignore repeat events for buttons */
          if (levt->value == 2)
               return false;

          devt->type   = levt->value ? DIET_BUTTONPRESS : DIET_BUTTONRELEASE;
          /* don't set DIEF_BUTTONS, it will be set by the input core */
          devt->button = DIBI_FIRST + code - BTN_MOUSE;
     }
     else {
          int key = translate_key( code );

          if (key == DIKI_UNKNOWN)
               return false;

          devt->type = levt->value ? DIET_KEYPRESS : DIET_KEYRELEASE;

          if (DFB_KEY_TYPE(key) == DIKT_IDENTIFIER) {
               devt->key_id = key;
               devt->flags |= DIEF_KEYID;
          }
          else {
               devt->key_symbol = key;
               devt->flags |= DIEF_KEYSYMBOL;
          }

          devt->flags |= DIEF_KEYCODE;
          devt->key_code = code;
     }

     if (levt->value == 2)
          devt->flags |= DIEF_REPEAT;

     return true;
}

/*
 * Translates relative axis events.
 */
static bool
rel_event( const struct input_event *levt,
           DFBInputEvent            *devt )
{
     switch (levt->code) {
          case REL_X:
               devt->axis = DIAI_X;
               devt->axisrel = levt->value;
               break;

          case REL_Y:
               devt->axis = DIAI_Y;
               devt->axisrel = levt->value;
               break;

          case REL_Z:
          case REL_WHEEL:
               devt->axis = DIAI_Z;
               devt->axisrel = -levt->value;
               break;

          default:
               if (levt->code > REL_MAX || levt->code > DIAI_LAST)
                    return false;
               devt->axis = levt->code;
               devt->axisrel = levt->value;
     }

     devt->type    = DIET_AXISMOTION;
     devt->flags  |= DIEF_AXISREL;

     return true;
}

/*
 * Translates absolute axis events.
 */
static bool
abs_event( const struct input_event *levt,
           DFBInputEvent            *devt )
{
     switch (levt->code) {
          case ABS_X:
               devt->axis = DIAI_X;
               break;

          case ABS_Y:
               devt->axis = DIAI_Y;
               break;

          case ABS_Z:
          case ABS_WHEEL:
               devt->axis = DIAI_Z;
               break;

          default:
               if (levt->code >= ABS_PRESSURE || levt->code > DIAI_LAST)
                    return false;
               devt->axis = levt->code;
     }

     devt->type    = DIET_AXISMOTION;
     devt->flags  |= DIEF_AXISABS;
     devt->axisabs = levt->value;

     return true;
}

/*
 * Translates a Linux input event into a DirectFB input event.
 */
static bool
translate_event( const struct input_event *levt,
                 DFBInputEvent            *devt )
{
     devt->flags     = DIEF_TIMESTAMP;
     devt->timestamp = levt->time;

     switch (levt->type) {
          case EV_KEY:
               return key_event( levt, devt );

          case EV_REL:
               return rel_event( levt, devt );

          case EV_ABS:
               return abs_event( levt, devt );

          default:
               ;
     }

     return false;
}

static void
set_led( const LinuxInputData *data, int led, int state )
{
     struct input_event levt;

     levt.type = EV_LED;
     levt.code = led;
     levt.value = !!state;

     write( data->fd, &levt, sizeof(levt) );
}

static void
flush_xy( LinuxInputData *data, bool last )
{
     DFBInputEvent evt;

     if (data->dx) {
          evt.type    = DIET_AXISMOTION;
          evt.flags   = DIEF_AXISREL;
          evt.axis    = DIAI_X;
          evt.axisrel = data->dx;

          /* Signal immediately following event. */
          if (!last || data->dy)
               evt.flags |= DIEF_FOLLOW;

          dfb_input_dispatch( data->device, &evt );

          data->dx = 0;
     }

     if (data->dy) {
          evt.type    = DIET_AXISMOTION;
          evt.flags   = DIEF_AXISREL;
          evt.axis    = DIAI_Y;
          evt.axisrel = data->dy;

          /* Signal immediately following event. */
          if (!last)
               evt.flags |= DIEF_FOLLOW;

          dfb_input_dispatch( data->device, &evt );

          data->dy = 0;
     }
}

/*
 * Input thread reading from device.
 * Generates events on incoming data.
 */
static void*
linux_input_EventThread( DirectThread *thread, void *driver_data )
{
     LinuxInputData    *data = (LinuxInputData*) driver_data;
     int                readlen, status;
     unsigned int       i;
     struct input_event levt[64];
     fd_set             set;
     struct touchpad_fsm_state fsm_state;

     /* Query min/max coordinates. */
     if (data->touchpad) {
          touchpad_fsm_init( &fsm_state );

          struct input_absinfo absinfo;

          ioctl( data->fd, EVIOCGABS(ABS_X), &absinfo );
          fsm_state.x.min = absinfo.minimum;
          fsm_state.x.max = absinfo.maximum;

          ioctl( data->fd, EVIOCGABS(ABS_Y), &absinfo );
          fsm_state.y.min = absinfo.minimum;
          fsm_state.y.max = absinfo.maximum;
     }

     while (1) {
          DFBInputEvent devt = { type: DIET_UNKNOWN, flags: DIEF_NONE };
          
          FD_ZERO( &set );
          FD_SET( data->fd, &set );

          if (data->touchpad && timeout_is_set( &fsm_state.timeout )) {
               struct timeval time;
               gettimeofday( &time, NULL );

               if (!timeout_passed( &fsm_state.timeout, &time )) {
                    struct timeval timeout = fsm_state.timeout;
                    timeout_sub( &timeout, &time );
                    status = select( data->fd + 1, &set, NULL, NULL, &timeout );
               } else {
                    status = 0;
               }
          }
          else {
               status = select( data->fd + 1, &set, NULL, NULL, NULL );
          }

          if (status < 0 && errno != EINTR)
               break;

          direct_thread_testcancel( thread );

          if (status < 0)
               continue;

          /* timeout? */
          if (status == 0) {
               DFBInputEvent devt;

               if (data->touchpad && touchpad_fsm( &fsm_state, NULL, &devt ) > 0)
                    dfb_input_dispatch( data->device, &devt );

               continue;
          }

          readlen = read( data->fd, levt, sizeof(levt) );

          if (readlen < 0 && errno != EINTR)
               break;

          direct_thread_testcancel( thread );

          if (readlen <= 0)
               continue;

          for (i=0; i<readlen / sizeof(levt[0]); i++) {
               DFBInputEvent temp;
               
               if (data->touchpad) {
                    status = touchpad_fsm( &fsm_state, &levt[i], &temp );
                    if (status < 0) {
                         /* Not handled. Try the direct approach. */
                         if (!translate_event( &levt[i], &temp ))
                              continue;
                    }
                    else if (status == 0) {
                         /* Handled but no further processing is necessary. */
                         continue;
                    }
               }
               else {
                    if (!translate_event( &levt[i], &temp ))
                         continue;
               }

               /* Flush previous event with DIEF_FOLLOW? */
               if (devt.type != DIET_UNKNOWN) {
                    flush_xy( data, false );

                    /* Signal immediately following event. */
                    devt.flags |= DIEF_FOLLOW;

                    dfb_input_dispatch( data->device, &devt );

                    devt.type  = DIET_UNKNOWN;
                    devt.flags = DIEF_NONE;
               }
               
               devt = temp;

               if (D_FLAGS_IS_SET( devt.flags, DIEF_AXISREL ) && devt.type == DIET_AXISMOTION &&
                   dfb_config->mouse_motion_compression)
               {
                    switch (devt.axis) {
                         case DIAI_X:
                              data->dx += devt.axisrel;
                              continue;

                         case DIAI_Y:
                              data->dy += devt.axisrel;
                              continue;

                         default:
                              break;
                    }
               }
               
               /* Event is dispatched in next round of loop. */

               if (data->has_leds && (devt.locks != data->locks)) {
                    set_led( data, LED_SCROLLL, devt.locks & DILS_SCROLL );
                    set_led( data, LED_NUML, devt.locks & DILS_NUM );
                    set_led( data, LED_CAPSL, devt.locks & DILS_CAPS );
                    data->locks = devt.locks;
               }
          }

          /* Flush last event without DIEF_FOLLOW. */
          if (devt.type != DIET_UNKNOWN) {
               flush_xy( data, false );

               dfb_input_dispatch( data->device, &devt );
          }
          else
               flush_xy( data, true );
     }

     if (status <= 0)
          D_PERROR ("linux_input thread died\n");

     return NULL;
}

/*
 * Fill device information.
 * Queries the input device and tries to classify it.
 */
static void
get_device_info( int              fd,
                 InputDeviceInfo *info,
                 bool            *touchpad )
{
     unsigned int  num_keys     = 0;
     unsigned int  num_ext_keys = 0;
     unsigned int  num_buttons  = 0;
     unsigned int  num_rels     = 0;
     unsigned int  num_abs      = 0;

     unsigned long evbit[NBITS(EV_MAX)];
     unsigned long keybit[NBITS(KEY_MAX)];
     unsigned long relbit[NBITS(REL_MAX)];
     unsigned long absbit[NBITS(ABS_MAX)];

     /* get device name */
     ioctl( fd, EVIOCGNAME(DFB_INPUT_DEVICE_DESC_NAME_LENGTH - 1), info->desc.name );

     /* set device vendor */
     snprintf( info->desc.vendor,
               DFB_INPUT_DEVICE_DESC_VENDOR_LENGTH, "Linux" );

     /* get event type bits */
     ioctl( fd, EVIOCGBIT(0, sizeof(evbit)), evbit );

     if (test_bit( EV_KEY, evbit )) {
          int i;

          /* get keyboard bits */
          ioctl( fd, EVIOCGBIT(EV_KEY, sizeof(keybit)), keybit );

	  /**  count typical keyboard keys only */
          for (i=KEY_Q; i<KEY_M; i++)
               if (test_bit( i, keybit ))
                    num_keys++;

          for (i=KEY_OK; i<KEY_MAX; i++)
               if (test_bit( i, keybit ))
                    num_ext_keys++;

          for (i=BTN_MOUSE; i<BTN_JOYSTICK; i++)
               if (test_bit( i, keybit ))
                    num_buttons++;
     }

     if (test_bit( EV_REL, evbit )) {
          int i;

          /* get bits for relative axes */
          ioctl( fd, EVIOCGBIT(EV_REL, sizeof(relbit)), relbit );

          for (i=0; i<REL_MAX; i++)
               if (test_bit( i, relbit ))
                    num_rels++;
     }

     if (test_bit( EV_ABS, evbit )) {
          int i;

          /* get bits for absolute axes */
          ioctl( fd, EVIOCGBIT(EV_ABS, sizeof(absbit)), absbit );

          for (i=0; i<ABS_PRESSURE; i++)
               if (test_bit( i, absbit ))
                    num_abs++;
     }

     /* Touchpad? */
     /* FIXME: can we rely on BTN_TOUCH? xorg synaptics driver doesn't use it. */
     if (test_bit( EV_KEY, evbit ) &&
         test_bit( BTN_TOUCH, keybit ) &&
         test_bit( BTN_TOOL_FINGER, keybit) &&
         test_bit( EV_ABS, evbit ) &&
         test_bit( ABS_X, absbit ) &&
         test_bit( ABS_Y, absbit ) &&
         test_bit( ABS_PRESSURE, absbit ))
          *touchpad = true;
     else
          *touchpad = false;

     /* Mouse, Touchscreen or Smartpad ? */
     if ((test_bit( EV_KEY, evbit ) &&
          (test_bit( BTN_TOUCH, keybit ) || test_bit( BTN_TOOL_FINGER, keybit ))) ||
          ((num_rels >= 2 && num_buttons)  ||  (num_abs == 2 && (num_buttons == 1))))
          info->desc.type |= DIDTF_MOUSE;
     else if (num_abs && num_buttons) /* Or a Joystick? */
          info->desc.type |= DIDTF_JOYSTICK;

     /* A Keyboard, do we have at least some letters? */
     if (num_keys > 20) {
          info->desc.type |= DIDTF_KEYBOARD;
          info->desc.caps |= DICAPS_KEYS;

          info->desc.min_keycode = 0;
          info->desc.max_keycode = 127;
     }

     /* A Remote Control? */
     if (num_ext_keys) {
          info->desc.type |= DIDTF_REMOTE;
          info->desc.caps |= DICAPS_KEYS;
     }

     /* Buttons */
     if (num_buttons) {
          info->desc.caps       |= DICAPS_BUTTONS;
          info->desc.max_button  = DIBI_FIRST + num_buttons - 1;
     }

     /* Axes */
     if (num_rels || num_abs) {
          info->desc.caps       |= DICAPS_AXES;
          info->desc.max_axis    = DIAI_FIRST + MAX(num_rels, num_abs) - 1;
     }

     /* Decide which primary input device to be. */
     if (info->desc.type & DIDTF_KEYBOARD)
          info->prefered_id = DIDID_KEYBOARD;
     else if (info->desc.type & DIDTF_REMOTE)
          info->prefered_id = DIDID_REMOTE;
     else if (info->desc.type & DIDTF_JOYSTICK)
          info->prefered_id = DIDID_JOYSTICK;
     else if (info->desc.type & DIDTF_MOUSE)
          info->prefered_id = DIDID_MOUSE;
     else
          info->prefered_id = DIDID_ANY;
}

static bool
check_device( const char *device )
{
     int  fd;

     /* Check if we are able to open the device */
     fd = open( device, O_RDWR );
     if (fd < 0) {
          return false;
     }
     else {
          InputDeviceInfo info;
          bool touchpad;

          /* try to grab the device */
          if (dfb_config->linux_input_grab) {
               /* 2.4 kernels don't have EVIOCGRAB so ignore EINVAL */
               if (ioctl( fd, EVIOCGRAB, 1 ) && errno != EINVAL) {
                    close( fd );
                    return false;
               }
          }

          memset( &info, 0, sizeof(InputDeviceInfo) );

          get_device_info( fd, &info, &touchpad );

          if (dfb_config->linux_input_grab)
               ioctl( fd, EVIOCGRAB, 0 );
          close( fd );

          if (!dfb_config->linux_input_ir_only ||
              (info.desc.type & DIDTF_REMOTE))
               return true;
     }

     return false;
}

/* exported symbols */

/*
 * Return the number of available devices.
 * Called once during initialization of DirectFB.
 */
static int
driver_get_available()
{
     int   i;
     char *tsdev;

#ifdef LINUX_INPUT_USE_FBDEV
     if (dfb_system_type() != CORE_FBDEV)
          return 0;
#endif

     /* Use the devices specified in the configuration. */
     if (fusion_vector_has_elements( &dfb_config->linux_input_devices )) {
          const char *device;

          fusion_vector_foreach (device, i, dfb_config->linux_input_devices) {
               if (check_device( device ))
                    device_names[num_devices++] = D_STRDUP( device );
          }

          return num_devices;
     }

     /* Check for tslib device being used. */
     tsdev = getenv( "TSLIB_TSDEVICE" );

     /* No devices specified. Try to guess some. */
     for (i=0; i<MAX_LINUX_INPUT_DEVICES; i++) {
          char buf[32];

          snprintf( buf, 32, "/dev/input/event%d", i );

          /* Let tslib driver handle its device. */
          if (tsdev && !strcmp( tsdev, buf ))
               continue;

          if (check_device( buf ))
               device_names[num_devices++] = D_STRDUP( buf );
     }

     return num_devices;
}

/*
 * Fill out general information about this driver.
 * Called once during initialization of DirectFB.
 */
static void
driver_get_info( InputDriverInfo *info )
{
     /* fill driver info structure */
     snprintf ( info->name,
                DFB_INPUT_DRIVER_INFO_NAME_LENGTH, "Linux Input Driver" );
     snprintf ( info->vendor,
                DFB_INPUT_DRIVER_INFO_VENDOR_LENGTH, "directfb.org" );

     info->version.major = 0;
     info->version.minor = 1;
}

/*
 * Open the device, fill out information about it,
 * allocate and fill private data, start input thread.
 * Called during initialization, resuming or taking over mastership.
 */
static DFBResult
driver_open_device( CoreInputDevice  *device,
                    unsigned int      number,
                    InputDeviceInfo  *info,
                    void            **driver_data )
{
     int              fd, ret;
     unsigned long    ledbit[NBITS(LED_MAX)];
     LinuxInputData  *data;
#ifdef LINUX_INPUT_USE_FBDEV
     FBDev           *dfb_fbdev = dfb_system_data();
#endif
     bool             touchpad;

     /* open device */
     fd = open( device_names[number], O_RDWR );
     if (fd < 0) {
          D_PERROR( "DirectFB/linux_input: could not open device" );
          return DFB_INIT;
     }

     /* grab device */
     if (dfb_config->linux_input_grab) {
          ret = ioctl( fd, EVIOCGRAB, 1 );
          /* 2.4 kernels don't have EVIOCGRAB so ignore EINVAL */
          if (ret && errno != EINVAL) {
               D_PERROR( "Direc      tFB/linux_input: could not grab device" );
               close( fd );
               return DFB_INIT;
          }
     }

     /* fill device info structure */
     get_device_info( fd, info, &touchpad );

     /* allocate and fill private data */
     data = D_CALLOC( 1, sizeof(LinuxInputData) );

     data->fd     = fd;
     data->device = device;
#ifdef LINUX_INPUT_USE_FBDEV
     data->vt     = dfb_fbdev->vt;
#endif
     data->touchpad = touchpad;

     /* check if the device has LEDs */
     ret = ioctl( fd, EVIOCGBIT(EV_LED, sizeof(ledbit)), ledbit );
     if (ret < 0)
          D_PERROR( "DirectFB/linux_input: could not get LED bits" );
     else
          data->has_leds = test_bit( LED_SCROLLL, ledbit ) ||
                           test_bit( LED_NUML, ledbit ) ||
                           test_bit( LED_CAPSL, ledbit );

     if (data->has_leds) {
          /* get LED state */
          ret = ioctl( fd, EVIOCGLED(sizeof(data->led_state)), data->led_state );
          if (ret < 0) {
               D_PERROR( "DirectFB/linux_input: could not get LED state" );
               if (dfb_config->linux_input_grab)
                    ioctl( fd, EVIOCGRAB, 0 );
               close( fd );
               D_FREE( data );
               return DFB_INIT;
          }

          /* turn off LEDs */
          set_led( data, LED_SCROLLL, 0 );
          set_led( data, LED_NUML, 0 );
          set_led( data, LED_CAPSL, 0 );
     }

     /* start input thread */
     data->thread = direct_thread_create( DTT_INPUT, linux_input_EventThread, data, "Linux Input" );

     /* set private data pointer */
     *driver_data = data;

     return DFB_OK;
}


/*
 * Fetch one entry from the kernel keymap.
 */
static DFBResult
driver_get_keymap_entry( CoreInputDevice           *device,
                         void                      *driver_data,
                         DFBInputDeviceKeymapEntry *entry )
{
#ifdef LINUX_INPUT_USE_FBDEV
     LinuxInputData             *data = (LinuxInputData*) driver_data;
     int                         code = entry->code;
     unsigned short              value;
     DFBInputDeviceKeyIdentifier identifier;

     if (!data->vt)
          return DFB_UNSUPPORTED;

     /* fetch the base level */
     value = keyboard_read_value( driver_data, K_NORMTAB, code );

     /* get the identifier for basic mapping */
     identifier = keyboard_get_identifier( code, value );

     /* is CapsLock effective? */
     if (KTYP(value) == KT_LETTER)
          entry->locks |= DILS_CAPS;

     /* is NumLock effective? */
     if (identifier >= DIKI_KP_DECIMAL && identifier <= DIKI_KP_9)
          entry->locks |= DILS_NUM;

     /* write identifier to entry */
     entry->identifier = identifier;

     /* write base level symbol to entry */
     entry->symbols[DIKSI_BASE] = keyboard_get_symbol( code, value, DIKSI_BASE );


     /* fetch the shifted base level */
     value = keyboard_read_value( driver_data, K_SHIFTTAB, entry->code );

     /* write shifted base level symbol to entry */
     entry->symbols[DIKSI_BASE_SHIFT] = keyboard_get_symbol( code, value,
                                                             DIKSI_BASE_SHIFT );


     /* fetch the alternative level */
     value = keyboard_read_value( driver_data, K_ALTTAB, entry->code );

     /* write alternative level symbol to entry */
     entry->symbols[DIKSI_ALT] = keyboard_get_symbol( code, value, DIKSI_ALT );


     /* fetch the shifted alternative level */
     value = keyboard_read_value( driver_data, K_ALTSHIFTTAB, entry->code );

     /* write shifted alternative level symbol to entry */
     entry->symbols[DIKSI_ALT_SHIFT] = keyboard_get_symbol( code, value,
                                                            DIKSI_ALT_SHIFT );

     return DFB_OK;
#else
     return DFB_UNSUPPORTED;
#endif
}

/*
 * End thread, close device and free private data.
 */
static void
driver_close_device( void *driver_data )
{
     LinuxInputData *data = (LinuxInputData*) driver_data;

     /* stop input thread */
     direct_thread_cancel( data->thread );
     direct_thread_join( data->thread );
     direct_thread_destroy( data->thread );

     if (data->has_leds) {
          /* restore LED state */
          set_led( data, LED_SCROLLL, test_bit( LED_SCROLLL, data->led_state ) );
          set_led( data, LED_NUML, test_bit( LED_NUML, data->led_state ) );
          set_led( data, LED_CAPSL, test_bit( LED_CAPSL, data->led_state ) );
     }

     /* release device */
     if (dfb_config->linux_input_grab)
          ioctl( data->fd, EVIOCGRAB, 0 );

     /* close file */
     close( data->fd );

     /* free private data */
     D_FREE( data );
}

static bool
timeout_is_set( const struct timeval *timeout )
{
     return timeout->tv_sec || timeout->tv_usec;
}

static bool
timeout_passed( const struct timeval *timeout, const struct timeval *current )
{
     return !timeout_is_set( timeout ) ||
          current->tv_sec > timeout->tv_sec ||
          (current->tv_sec == timeout->tv_sec && current->tv_usec > timeout->tv_usec);
}

static void
timeout_clear( struct timeval *timeout )
{
     timeout->tv_sec  = 0;
     timeout->tv_usec = 0;
}

static void
timeout_add( struct timeval *timeout, const struct timeval *add )
{
     timeout->tv_sec += add->tv_sec;
     timeout->tv_usec += add->tv_usec;
     while (timeout->tv_usec >= 1000000) {
          timeout->tv_sec++;
          timeout->tv_usec -= 1000000;
     }
}

static void
timeout_sub( struct timeval *timeout, const struct timeval *sub )
{
     timeout->tv_sec -= sub->tv_sec;
     timeout->tv_usec -= sub->tv_usec;
     while (timeout->tv_usec < 0) {
          timeout->tv_sec--;
          timeout->tv_usec += 1000000;
     }
}

static void
touchpad_fsm_init( struct touchpad_fsm_state *state )
{
     state->x.old = -1;
     state->y.old = -1;
     state->fsm_state = TOUCHPAD_FSM_START;
     timeout_clear( &state->timeout );
}

static int
touchpad_normalize( const struct touchpad_axis *axis, int value )
{
     return ((value - axis->min) << 9) / (axis->max - axis->min);
}

static int
touchpad_translate( struct touchpad_fsm_state *state,
                    const struct input_event  *levt,
                    DFBInputEvent             *devt )
{
     struct touchpad_axis *axis = NULL;
     int abs, rel;

     devt->flags     = DIEF_TIMESTAMP | DIEF_AXISREL;
     devt->timestamp = levt->time;
     devt->type      = DIET_AXISMOTION;

     switch (levt->code) {
     case ABS_X:
          axis       = &state->x;
          devt->axis = DIAI_X;
          break;
     case ABS_Y:
          axis       = &state->y;
          devt->axis = DIAI_Y;
          break;
     default:
          return 0;
     }

     abs = touchpad_normalize( axis, levt->value );
     if (axis->old == -1)
          axis->old = abs;
     rel = abs - axis->old;

#define ACCEL_THRESHOLD 25
#define ACCEL_NUM       3
#define ACCEL_DENOM     1

     if (rel > ACCEL_THRESHOLD)
          rel += (rel - ACCEL_THRESHOLD) * ACCEL_NUM / ACCEL_DENOM;
     else if (rel < -ACCEL_THRESHOLD)
          rel += (rel + ACCEL_THRESHOLD) * ACCEL_NUM / ACCEL_DENOM;

     axis->old     = abs;
     devt->axisrel = rel;

     return 1;
}

static bool
touchpad_finger_landing( const struct input_event *levt )
{
     return levt->type == EV_KEY && levt->code == BTN_TOUCH && levt->value == 1;
}

static bool
touchpad_finger_leaving( const struct input_event *levt )
{
     return levt->type == EV_KEY && levt->code == BTN_TOUCH && levt->value == 0;
}

static bool
touchpad_finger_moving( const struct input_event *levt )
{
     return levt->type == EV_ABS && (levt->code == ABS_X || levt->code == ABS_Y);
}

/*
 * This FSM takes into accout finger landing on touchpad and leaving and
 * translates absolute DFBInputEvent into a relative one
 */
static int
touchpad_fsm( struct touchpad_fsm_state *state,
              const struct input_event  *levt,
              DFBInputEvent             *devt )
{
     struct timeval timeout = { 0, 125000 };

     /* select() timeout? */
     if (!levt) {
          /* Check if button release is due. */
          if (state->fsm_state == TOUCHPAD_FSM_DRAG_START) {
               devt->flags     = DIEF_TIMESTAMP;
               devt->timestamp = state->timeout; /* timeout of current time? */
               devt->type      = DIET_BUTTONRELEASE;
               devt->button    = DIBI_FIRST;

               touchpad_fsm_init( state );
               return 1;
          }

          /* Already passed, clear it so select() won't return until there is something to do. */
          timeout_clear( &state->timeout );
          return 0;
     }

     /* More or less ignore these events for now */
     if ((levt->type == EV_SYN && levt->code == SYN_REPORT) ||
         (levt->type == EV_ABS && levt->code == ABS_PRESSURE) ||
         (levt->type == EV_ABS && levt->code == ABS_TOOL_WIDTH) ||
         (levt->type == EV_KEY && levt->code == BTN_TOOL_FINGER) ||
         (levt->type == EV_KEY && levt->code == BTN_TOOL_DOUBLETAP) ||
         (levt->type == EV_KEY && levt->code == BTN_TOOL_TRIPLETAP)) {

          /* Check if button release is due. */
          if (state->fsm_state == TOUCHPAD_FSM_DRAG_START &&
              timeout_passed( &state->timeout, &levt->time )) {
               devt->flags     = DIEF_TIMESTAMP;
               devt->timestamp = state->timeout; /* timeout of levt->time? */
               devt->type      = DIET_BUTTONRELEASE;
               devt->button    = DIBI_FIRST;

               touchpad_fsm_init( state );
               return 1;
          }

          return 0;
     }

     /* Use translate_event() for other events. */
     if (!(levt->type == EV_KEY && levt->code == BTN_TOUCH) &&
         !(levt->type == EV_ABS && (levt->code == ABS_X || levt->code == ABS_Y)))
          return -1;

     switch (state->fsm_state) {
     case TOUCHPAD_FSM_START:
          if (touchpad_finger_landing( levt )) {
               state->fsm_state = TOUCHPAD_FSM_MAIN;
               state->timeout = levt->time;
               timeout_add( &state->timeout, &timeout );
          }
          return 0;

     case TOUCHPAD_FSM_MAIN:
          if (touchpad_finger_moving( levt )) {
               if (1){//timeout_passed( &state->timeout, &levt->time )) {
                    //timeout_clear( &state->timeout );
                    return touchpad_translate( state, levt, devt );
               }
          }
          else if (touchpad_finger_leaving( levt )) {
               if (!timeout_passed( &state->timeout, &levt->time )) {
                    devt->flags     = DIEF_TIMESTAMP;
                    devt->timestamp = levt->time;
                    devt->type      = DIET_BUTTONPRESS;
                    devt->button    = DIBI_FIRST;

                    touchpad_fsm_init( state );
                    state->fsm_state = TOUCHPAD_FSM_DRAG_START;
                    state->timeout = levt->time;
                    timeout_add( &state->timeout, &timeout );
                    return 1;
               }
               else {
                    touchpad_fsm_init( state );
               }
          }
          return 0;

     case TOUCHPAD_FSM_DRAG_START:
          if (timeout_passed( &state->timeout, &levt->time )){
               devt->flags     = DIEF_TIMESTAMP;
               devt->timestamp = state->timeout; /* timeout of levt->time? */
               devt->type      = DIET_BUTTONRELEASE;
               devt->button    = DIBI_FIRST;

               touchpad_fsm_init(state);
               return 1;
          }
          else {
               if (touchpad_finger_landing( levt )) {
                    state->fsm_state = TOUCHPAD_FSM_DRAG_MAIN;
                    state->timeout = levt->time;
                    timeout_add( &state->timeout, &timeout );
               }
          }
          return 0;

     case TOUCHPAD_FSM_DRAG_MAIN:
          if (touchpad_finger_moving( levt )) {
               if (1){//timeout_passed( &state->timeout, &levt->time )) {
                   //timeout_clear( &state->timeout );
                    return touchpad_translate( state, levt, devt );
               }
          }
          else if (touchpad_finger_leaving( levt )) {
               devt->flags     = DIEF_TIMESTAMP;
               devt->timestamp = levt->time;
               devt->type      = DIET_BUTTONRELEASE;
               devt->button    = DIBI_FIRST;

               touchpad_fsm_init( state );
               return 1;
          }
          return 0;

     default:
          return 0;
     }

     return 0;
}
