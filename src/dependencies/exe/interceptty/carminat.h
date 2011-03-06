#ifndef _CARMINAT_H
#define _CARMINAT_H


/* Begin extract directfb types */

/*
 * DirectFB key types (for advanced mapping)
 */
typedef enum {
     DIKT_UNICODE        = 0x0000,     /* Unicode 3.x character
                                           (compatible to Latin-1) */
     DIKT_SPECIAL        = 0xF000,     /* Special key (e.g. Cursor Up or Menu) */
     DIKT_FUNCTION       = 0xF100,     /* Function key (F1 - Fn) */
     DIKT_MODIFIER       = 0xF200,     /* Modifier key */
     DIKT_LOCK           = 0xF300,     /* Lock key (e.g. CapsLock) */
     DIKT_DEAD           = 0xF400,     /* Dead key (e.g. dead grave) */
     DIKT_CUSTOM         = 0xF500,     /* Custom key (vendor specific) */
     DIKT_IDENTIFIER     = 0xF600      /* DirectFB key identifier */
} DFBInputDeviceKeyType;
#define DFB_KEY(type,index)        ((DIKT_##type) | (index))


/*
 * DirectFB key identifiers (for basic mapping)
 */
typedef enum {
     DIKI_UNKNOWN = DFB_KEY( IDENTIFIER, 0 ),

     DIKI_A,
     DIKI_B,
     DIKI_C,
     DIKI_D,
     DIKI_E,
     DIKI_F,
     DIKI_G,
     DIKI_H,
     DIKI_I,
     DIKI_J,
     DIKI_K,
     DIKI_L,
     DIKI_M,
     DIKI_N,
     DIKI_O,
     DIKI_P,
     DIKI_Q,
     DIKI_R,
     DIKI_S,
     DIKI_T,
     DIKI_U,
     DIKI_V,
     DIKI_W,
     DIKI_X,
     DIKI_Y,
     DIKI_Z,

     DIKI_0,
     DIKI_1,
     DIKI_2,
     DIKI_3,
     DIKI_4,
     DIKI_5,
     DIKI_6,
     DIKI_7,
     DIKI_8,
     DIKI_9,

     DIKI_F1,
     DIKI_F2,
     DIKI_F3,
     DIKI_F4,
     DIKI_F5,
     DIKI_F6,
     DIKI_F7,
     DIKI_F8,
     DIKI_F9,
     DIKI_F10,
     DIKI_F11,
     DIKI_F12,

     DIKI_SHIFT_L,
     DIKI_SHIFT_R,
     DIKI_CONTROL_L,
     DIKI_CONTROL_R,
     DIKI_ALT_L,
     DIKI_ALT_R,
     DIKI_META_L,
     DIKI_META_R,
     DIKI_SUPER_L,
     DIKI_SUPER_R,
     DIKI_HYPER_L,
     DIKI_HYPER_R,

     DIKI_CAPS_LOCK,
     DIKI_NUM_LOCK,
     DIKI_SCROLL_LOCK,

     DIKI_ESCAPE,
     DIKI_LEFT,
     DIKI_RIGHT,
     DIKI_UP,
     DIKI_DOWN,
     DIKI_TAB,
     DIKI_ENTER,
     DIKI_SPACE,
     DIKI_BACKSPACE,
     DIKI_INSERT,
     DIKI_DELETE,
     DIKI_HOME,
     DIKI_END,
     DIKI_PAGE_UP,
     DIKI_PAGE_DOWN,
     DIKI_PRINT,
     DIKI_PAUSE,

     /*  The labels on these keys depend on the type of keyboard.
      *  We've choosen the names from a US keyboard layout. The
      *  comments refer to the ISO 9995 terminology.
      */
     DIKI_QUOTE_LEFT,    /*  TLDE  */
     DIKI_MINUS_SIGN,    /*  AE11  */
     DIKI_EQUALS_SIGN,   /*  AE12  */
     DIKI_BRACKET_LEFT,  /*  AD11  */
     DIKI_BRACKET_RIGHT, /*  AD12  */
     DIKI_BACKSLASH,     /*  BKSL  */
     DIKI_SEMICOLON,     /*  AC10  */
     DIKI_QUOTE_RIGHT,   /*  AC11  */
     DIKI_COMMA,         /*  AB08  */
     DIKI_PERIOD,        /*  AB09  */
     DIKI_SLASH,         /*  AB10  */

     DIKI_LESS_SIGN,     /*  103rd  */

     DIKI_KP_DIV,
     DIKI_KP_MULT,
     DIKI_KP_MINUS,
     DIKI_KP_PLUS,
     DIKI_KP_ENTER,
     DIKI_KP_SPACE,
     DIKI_KP_TAB,
     DIKI_KP_F1,
     DIKI_KP_F2,
     DIKI_KP_F3,
     DIKI_KP_F4,
     DIKI_KP_EQUAL,
     DIKI_KP_SEPARATOR,

     DIKI_KP_DECIMAL,
     DIKI_KP_0,
     DIKI_KP_1,
     DIKI_KP_2,
     DIKI_KP_3,
     DIKI_KP_4,
     DIKI_KP_5,
     DIKI_KP_6,
     DIKI_KP_7,
     DIKI_KP_8,
     DIKI_KP_9,

     DIKI_KEYDEF_END,
     DIKI_NUMBER_OF_KEYS = DIKI_KEYDEF_END - DFB_KEY( IDENTIFIER, 0 )

} DFBInputDeviceKeyIdentifier;

/* End extract directfb types */


#define CARM_CAN_SYNC 0xC0

#define CARM_FRAME_TYPE_KEY 0x8E
#define CARM_FRAME_TYPE_JOYSTICK 0xBE

#define CARM_FRAME_INPUT 0x26
#define CARM_FRAME_INPUT_KEY 0x29
#define CARM_FRAME_INPUT_JOYSTICK 0x2B


enum carminat_keys{
    CARM_KEY_TOP_LEFT = 0,
    CARM_KEY_BACK,
    CARM_KEY_TOP_RIGHT,
    CARM_KEY_INFO,
    CARM_KEY_MENU,
    CARM_KEY_LIGHT,
    CARM_KEY_REPEAT,
    CARM_KEY_DEST,
    CARM_KEY_MAP
};

enum carminat_joy_dir{
    CARM_JOY_DIR_ADVANCED = 0,
    CARM_JOY_DIR_FRONT = 1,
    CARM_JOY_DIR_BACK  = 2,
    CARM_JOY_DIR_LEFT  = 3,
    CARM_JOY_DIR_RIGHT = 4
};

enum carminat_joy_push{
    CARM_JOY_PUSH_UP = 0,
    CARM_JOY_PUSH_DOWN
};

enum carminat_joy_rotate_direction{
    CARM_JOY_ROT_CCW = 1,
    CARM_JOY_ROT_CW = 2
};

#pragma pack(1)
struct carminat_can_event_key {
uint8_t  dtype;         /* 0x26 = input */
uint8_t  src;           /* 0x29 = keys */
uint8_t  length;        /* 0x02 = length*/
uint8_t  key;           /* carminat key */
uint8_t  dummy;         /* unknown, separator? */
};

struct carminat_can_event_joystick {
uint8_t   dtype;                /* 0x26 = input */
uint8_t   src;              /* 0x2b = joystick */
uint8_t   length;           /* 0x05 = length */
uint8_t   down;             /* push down status */
uint8_t   dummy;            // unknown */
uint8_t   direction;        /* joystick directional movement */
uint8_t   rotate;           /* rotate direction */
uint8_t   rotate_steps;     /* number of steps rotated */
};

#endif // header guard
