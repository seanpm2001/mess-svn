/***************************************************************************

    input.h

    Handle input from the user.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __INPUT_H__
#define __INPUT_H__

#include "mamecore.h"
#include "astring.h"


/***************************************************************************
    MACROS
***************************************************************************/

/* make sure RELATIVE and ABSOLUTE aren't defined elsewhere; otherwise it fouls up our macros */
#undef RELATIVE
#undef ABSOLUTE

/* relative devices return ~512 units per onscreen pixel */
#define INPUT_RELATIVE_PER_PIXEL		512

/* absolute devices return values between -65536 and +65536 */
#define INPUT_ABSOLUTE_MIN				-65536
#define INPUT_ABSOLUTE_MAX				65536

/* flags in the top 4 bits of the input code */
#define INPUT_CODE_INTERNAL				(1 << 31)

/* extract components of the input code */
#define INPUT_CODE_IS_INTERNAL(c)		(((c) & INPUT_CODE_INTERNAL) != 0)
#define INPUT_CODE_DEVCLASS(c)			((input_device_class)(((c) >> 24) & 0x0f))
#define INPUT_CODE_DEVINDEX(c)			((UINT8)(((c) >> 20) & 0x0f))
#define INPUT_CODE_ITEMCLASS(c)			((input_item_class)(((c) >> 16) & 0x0f))
#define INPUT_CODE_MODIFIER(c)			((input_item_modifier)(((c) >> 12) & 0x0f))
#define INPUT_CODE_ITEMID(c)			((input_item_id)(int)((c) & 0xfff))

/* build or modify input codes */
#define INPUT_CODE(d,x,i,m,o)			((((d) & 0x0f) << 24) | (((x) & 0x0f) << 20) | (((i) & 0x0f) << 16) | (((m) & 0x0f) << 12) | ((o) & 0xfff))
#define INPUT_CODE_SET_DEVINDEX(c,x) 	(((c) & ~(0xf << 20)) | (((x) & 0x0f) << 20))
#define INPUT_CODE_SET_ITEMCLASS(c,i) 	(((c) & ~(0xf << 16)) | (((i) & 0x0f) << 16))
#define INPUT_CODE_SET_MODIFIER(c,m) 	(((c) & ~(0xf << 12)) | (((m) & 0x0f) << 12))
#define INPUT_CODE_SET_ITEMID(c,d) 		(((c) & ~0xfff) | ((d) & 0xfff))

/* standard code building */
#define STANDARD_CODE(d,x,i,m,o)		(INPUT_CODE(DEVICE_CLASS_##d, x, ITEM_CLASS_##i, ITEM_MODIFIER_##m, ITEM_ID_##o))
#define INTERNAL_CODE(x)				(INPUT_CODE_INTERNAL | ((x) & 0xfff))
#define INPUT_CODE_INVALID				STANDARD_CODE(INVALID, 0, INVALID, NONE, INVALID)

/* maximum number of axis/buttons/hats with ITEM_IDs for use by osd layer */
#define INPUT_MAX_AXIS					(8)
#define INPUT_MAX_BUTTONS				(16)
#define INPUT_MAX_HATS					(4)
#define INPUT_MAX_ADD_SWITCH			(16)
#define INPUT_MAX_ADD_ABSOLUTE			(16)
#define INPUT_MAX_ADD_RELATIVE			(16)



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* input device classes */
enum _input_device_class
{
	DEVICE_CLASS_INVALID,
	DEVICE_CLASS_FIRST_VALID,
	DEVICE_CLASS_KEYBOARD = DEVICE_CLASS_FIRST_VALID,
	DEVICE_CLASS_MOUSE,
	DEVICE_CLASS_LIGHTGUN,
	DEVICE_CLASS_JOYSTICK,
	DEVICE_CLASS_MAXIMUM
};
typedef enum _input_device_class input_device_class;
DECLARE_ENUM_OPERATORS(input_device_class)


/* input item classes */
enum _input_item_class
{
	ITEM_CLASS_INVALID,
	ITEM_CLASS_SWITCH,
	ITEM_CLASS_ABSOLUTE,
	ITEM_CLASS_RELATIVE,
	ITEM_CLASS_MAXIMUM
};
typedef enum _input_item_class input_item_class;


/* input item modifiers */
enum _input_item_modifier
{
	ITEM_MODIFIER_NONE,
	ITEM_MODIFIER_POS,
	ITEM_MODIFIER_NEG,
	ITEM_MODIFIER_LEFT,
	ITEM_MODIFIER_RIGHT,
	ITEM_MODIFIER_UP,
	ITEM_MODIFIER_DOWN,
	ITEM_MODIFIER_MAXIMUM
};
typedef enum _input_item_modifier input_item_modifier;


/* standard item IDs */
enum _input_item_id
{
	ITEM_ID_INVALID,
	ITEM_ID_FIRST_VALID,

	/* standard keyboard IDs */
	ITEM_ID_A = ITEM_ID_FIRST_VALID,
	ITEM_ID_B,
	ITEM_ID_C,
	ITEM_ID_D,
	ITEM_ID_E,
	ITEM_ID_F,
	ITEM_ID_G,
	ITEM_ID_H,
	ITEM_ID_I,
	ITEM_ID_J,
	ITEM_ID_K,
	ITEM_ID_L,
	ITEM_ID_M,
	ITEM_ID_N,
	ITEM_ID_O,
	ITEM_ID_P,
	ITEM_ID_Q,
	ITEM_ID_R,
	ITEM_ID_S,
	ITEM_ID_T,
	ITEM_ID_U,
	ITEM_ID_V,
	ITEM_ID_W,
	ITEM_ID_X,
	ITEM_ID_Y,
	ITEM_ID_Z,
	ITEM_ID_0,
	ITEM_ID_1,
	ITEM_ID_2,
	ITEM_ID_3,
	ITEM_ID_4,
	ITEM_ID_5,
	ITEM_ID_6,
	ITEM_ID_7,
	ITEM_ID_8,
	ITEM_ID_9,
	ITEM_ID_F1,
	ITEM_ID_F2,
	ITEM_ID_F3,
	ITEM_ID_F4,
	ITEM_ID_F5,
	ITEM_ID_F6,
	ITEM_ID_F7,
	ITEM_ID_F8,
	ITEM_ID_F9,
	ITEM_ID_F10,
	ITEM_ID_F11,
	ITEM_ID_F12,
	ITEM_ID_F13,
	ITEM_ID_F14,
	ITEM_ID_F15,
	ITEM_ID_ESC,
	ITEM_ID_TILDE,
	ITEM_ID_MINUS,
	ITEM_ID_EQUALS,
	ITEM_ID_BACKSPACE,
	ITEM_ID_TAB,
	ITEM_ID_OPENBRACE,
	ITEM_ID_CLOSEBRACE,
	ITEM_ID_ENTER,
	ITEM_ID_COLON,
	ITEM_ID_QUOTE,
	ITEM_ID_BACKSLASH,
	ITEM_ID_BACKSLASH2,
	ITEM_ID_COMMA,
	ITEM_ID_STOP,
	ITEM_ID_SLASH,
	ITEM_ID_SPACE,
	ITEM_ID_INSERT,
	ITEM_ID_DEL,
	ITEM_ID_HOME,
	ITEM_ID_END,
	ITEM_ID_PGUP,
	ITEM_ID_PGDN,
	ITEM_ID_LEFT,
	ITEM_ID_RIGHT,
	ITEM_ID_UP,
	ITEM_ID_DOWN,
	ITEM_ID_0_PAD,
	ITEM_ID_1_PAD,
	ITEM_ID_2_PAD,
	ITEM_ID_3_PAD,
	ITEM_ID_4_PAD,
	ITEM_ID_5_PAD,
	ITEM_ID_6_PAD,
	ITEM_ID_7_PAD,
	ITEM_ID_8_PAD,
	ITEM_ID_9_PAD,
	ITEM_ID_SLASH_PAD,
	ITEM_ID_ASTERISK,
	ITEM_ID_MINUS_PAD,
	ITEM_ID_PLUS_PAD,
	ITEM_ID_DEL_PAD,
	ITEM_ID_ENTER_PAD,
	ITEM_ID_PRTSCR,
	ITEM_ID_PAUSE,
	ITEM_ID_LSHIFT,
	ITEM_ID_RSHIFT,
	ITEM_ID_LCONTROL,
	ITEM_ID_RCONTROL,
	ITEM_ID_LALT,
	ITEM_ID_RALT,
	ITEM_ID_SCRLOCK,
	ITEM_ID_NUMLOCK,
	ITEM_ID_CAPSLOCK,
	ITEM_ID_LWIN,
	ITEM_ID_RWIN,
	ITEM_ID_MENU,
	ITEM_ID_CANCEL,

	/* standard mouse/joystick/gun IDs */
	ITEM_ID_XAXIS,
	ITEM_ID_YAXIS,
	ITEM_ID_ZAXIS,
	ITEM_ID_RXAXIS,
	ITEM_ID_RYAXIS,
	ITEM_ID_RZAXIS,
	ITEM_ID_SLIDER1,
	ITEM_ID_SLIDER2,
	ITEM_ID_BUTTON1,
	ITEM_ID_BUTTON2,
	ITEM_ID_BUTTON3,
	ITEM_ID_BUTTON4,
	ITEM_ID_BUTTON5,
	ITEM_ID_BUTTON6,
	ITEM_ID_BUTTON7,
	ITEM_ID_BUTTON8,
	ITEM_ID_BUTTON9,
	ITEM_ID_BUTTON10,
	ITEM_ID_BUTTON11,
	ITEM_ID_BUTTON12,
	ITEM_ID_BUTTON13,
	ITEM_ID_BUTTON14,
	ITEM_ID_BUTTON15,
	ITEM_ID_BUTTON16,
	ITEM_ID_START,
	ITEM_ID_SELECT,

	/* Hats */
	ITEM_ID_HAT1UP,
	ITEM_ID_HAT1DOWN,
	ITEM_ID_HAT1LEFT,
	ITEM_ID_HAT1RIGHT,
	ITEM_ID_HAT2UP,
	ITEM_ID_HAT2DOWN,
	ITEM_ID_HAT2LEFT,
	ITEM_ID_HAT2RIGHT,
	ITEM_ID_HAT3UP,
	ITEM_ID_HAT3DOWN,
	ITEM_ID_HAT3LEFT,
	ITEM_ID_HAT3RIGHT,
	ITEM_ID_HAT4UP,
	ITEM_ID_HAT4DOWN,
	ITEM_ID_HAT4LEFT,
	ITEM_ID_HAT4RIGHT,

	/* Additional IDs */
	ITEM_ID_ADD_SWITCH1,
	ITEM_ID_ADD_SWITCH2,
	ITEM_ID_ADD_SWITCH3,
	ITEM_ID_ADD_SWITCH4,
	ITEM_ID_ADD_SWITCH5,
	ITEM_ID_ADD_SWITCH6,
	ITEM_ID_ADD_SWITCH7,
	ITEM_ID_ADD_SWITCH8,
	ITEM_ID_ADD_SWITCH9,
	ITEM_ID_ADD_SWITCH10,
	ITEM_ID_ADD_SWITCH11,
	ITEM_ID_ADD_SWITCH12,
	ITEM_ID_ADD_SWITCH13,
	ITEM_ID_ADD_SWITCH14,
	ITEM_ID_ADD_SWITCH15,
	ITEM_ID_ADD_SWITCH16,

	ITEM_ID_ADD_ABSOLUTE1,
	ITEM_ID_ADD_ABSOLUTE2,
	ITEM_ID_ADD_ABSOLUTE3,
	ITEM_ID_ADD_ABSOLUTE4,
	ITEM_ID_ADD_ABSOLUTE5,
	ITEM_ID_ADD_ABSOLUTE6,
	ITEM_ID_ADD_ABSOLUTE7,
	ITEM_ID_ADD_ABSOLUTE8,
	ITEM_ID_ADD_ABSOLUTE9,
	ITEM_ID_ADD_ABSOLUTE10,
	ITEM_ID_ADD_ABSOLUTE11,
	ITEM_ID_ADD_ABSOLUTE12,
	ITEM_ID_ADD_ABSOLUTE13,
	ITEM_ID_ADD_ABSOLUTE14,
	ITEM_ID_ADD_ABSOLUTE15,
	ITEM_ID_ADD_ABSOLUTE16,

	ITEM_ID_ADD_RELATIVE1,
	ITEM_ID_ADD_RELATIVE2,
	ITEM_ID_ADD_RELATIVE3,
	ITEM_ID_ADD_RELATIVE4,
	ITEM_ID_ADD_RELATIVE5,
	ITEM_ID_ADD_RELATIVE6,
	ITEM_ID_ADD_RELATIVE7,
	ITEM_ID_ADD_RELATIVE8,
	ITEM_ID_ADD_RELATIVE9,
	ITEM_ID_ADD_RELATIVE10,
	ITEM_ID_ADD_RELATIVE11,
	ITEM_ID_ADD_RELATIVE12,
	ITEM_ID_ADD_RELATIVE13,
	ITEM_ID_ADD_RELATIVE14,
	ITEM_ID_ADD_RELATIVE15,
	ITEM_ID_ADD_RELATIVE16,

	/* generic other IDs */
	ITEM_ID_OTHER_SWITCH,
	ITEM_ID_OTHER_AXIS_ABSOLUTE,
	ITEM_ID_OTHER_AXIS_RELATIVE,
	ITEM_ID_MAXIMUM,

	/* absolute maximum ID */
	ITEM_ID_ABSOLUTE_MAXIMUM = 0xfff
};
typedef enum _input_item_id input_item_id;
DECLARE_ENUM_OPERATORS(input_item_id)


/* expanded codes referencing specific devices for input definitions */
/* note that these all implcitly refer to device 0; to reference additional */
/* devices, wrap the code in INPUT_CODE_SET_DEVINDEX() */
enum
{
	/* keyboard codes */
	KEYCODE_A = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, A),
	KEYCODE_B = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, B),
	KEYCODE_C = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, C),
	KEYCODE_D = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, D),
	KEYCODE_E = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, E),
	KEYCODE_F = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, F),
	KEYCODE_G = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, G),
	KEYCODE_H = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, H),
	KEYCODE_I = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, I),
	KEYCODE_J = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, J),
	KEYCODE_K = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, K),
	KEYCODE_L = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, L),
	KEYCODE_M = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, M),
	KEYCODE_N = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, N),
	KEYCODE_O = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, O),
	KEYCODE_P = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, P),
	KEYCODE_Q = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, Q),
	KEYCODE_R = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, R),
	KEYCODE_S = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, S),
	KEYCODE_T = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, T),
	KEYCODE_U = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, U),
	KEYCODE_V = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, V),
	KEYCODE_W = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, W),
	KEYCODE_X = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, X),
	KEYCODE_Y = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, Y),
	KEYCODE_Z = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, Z),
	KEYCODE_0 = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, 0),
	KEYCODE_1 = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, 1),
	KEYCODE_2 = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, 2),
	KEYCODE_3 = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, 3),
	KEYCODE_4 = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, 4),
	KEYCODE_5 = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, 5),
	KEYCODE_6 = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, 6),
	KEYCODE_7 = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, 7),
	KEYCODE_8 = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, 8),
	KEYCODE_9 = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, 9),
	KEYCODE_F1 = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, F1),
	KEYCODE_F2 = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, F2),
	KEYCODE_F3 = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, F3),
	KEYCODE_F4 = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, F4),
	KEYCODE_F5 = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, F5),
	KEYCODE_F6 = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, F6),
	KEYCODE_F7 = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, F7),
	KEYCODE_F8 = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, F8),
	KEYCODE_F9 = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, F9),
	KEYCODE_F10 = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, F10),
	KEYCODE_F11 = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, F11),
	KEYCODE_F12 = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, F12),
	KEYCODE_F13 = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, F13),
	KEYCODE_F14 = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, F14),
	KEYCODE_F15 = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, F15),
	KEYCODE_ESC = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, ESC),
	KEYCODE_TILDE = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, TILDE),
	KEYCODE_MINUS = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, MINUS),
	KEYCODE_EQUALS = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, EQUALS),
	KEYCODE_BACKSPACE = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, BACKSPACE),
	KEYCODE_TAB = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, TAB),
	KEYCODE_OPENBRACE = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, OPENBRACE),
	KEYCODE_CLOSEBRACE = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, CLOSEBRACE),
	KEYCODE_ENTER = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, ENTER),
	KEYCODE_COLON = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, COLON),
	KEYCODE_QUOTE = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, QUOTE),
	KEYCODE_BACKSLASH = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, BACKSLASH),
	KEYCODE_BACKSLASH2 = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, BACKSLASH2),
	KEYCODE_COMMA = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, COMMA),
	KEYCODE_STOP = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, STOP),
	KEYCODE_SLASH = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, SLASH),
	KEYCODE_SPACE = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, SPACE),
	KEYCODE_INSERT = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, INSERT),
	KEYCODE_DEL = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, DEL),
	KEYCODE_HOME = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, HOME),
	KEYCODE_END = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, END),
	KEYCODE_PGUP = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, PGUP),
	KEYCODE_PGDN = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, PGDN),
	KEYCODE_LEFT = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, LEFT),
	KEYCODE_RIGHT = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, RIGHT),
	KEYCODE_UP = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, UP),
	KEYCODE_DOWN = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, DOWN),
	KEYCODE_0_PAD = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, 0_PAD),
	KEYCODE_1_PAD = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, 1_PAD),
	KEYCODE_2_PAD = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, 2_PAD),
	KEYCODE_3_PAD = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, 3_PAD),
	KEYCODE_4_PAD = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, 4_PAD),
	KEYCODE_5_PAD = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, 5_PAD),
	KEYCODE_6_PAD = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, 6_PAD),
	KEYCODE_7_PAD = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, 7_PAD),
	KEYCODE_8_PAD = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, 8_PAD),
	KEYCODE_9_PAD = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, 9_PAD),
	KEYCODE_SLASH_PAD = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, SLASH_PAD),
	KEYCODE_ASTERISK = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, ASTERISK),
	KEYCODE_MINUS_PAD = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, MINUS_PAD),
	KEYCODE_PLUS_PAD = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, PLUS_PAD),
	KEYCODE_DEL_PAD = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, DEL_PAD),
	KEYCODE_ENTER_PAD = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, ENTER_PAD),
	KEYCODE_PRTSCR = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, PRTSCR),
	KEYCODE_PAUSE = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, PAUSE),
	KEYCODE_LSHIFT = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, LSHIFT),
	KEYCODE_RSHIFT = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, RSHIFT),
	KEYCODE_LCONTROL = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, LCONTROL),
	KEYCODE_RCONTROL = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, RCONTROL),
	KEYCODE_LALT = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, LALT),
	KEYCODE_RALT = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, RALT),
	KEYCODE_SCRLOCK = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, SCRLOCK),
	KEYCODE_NUMLOCK = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, NUMLOCK),
	KEYCODE_CAPSLOCK = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, CAPSLOCK),
	KEYCODE_LWIN = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, LWIN),
	KEYCODE_RWIN = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, RWIN),
	KEYCODE_MENU = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, MENU),
	KEYCODE_CANCEL = STANDARD_CODE(KEYBOARD, 0, SWITCH, NONE, CANCEL),

	/* mouse axes as relative devices */
	MOUSECODE_X = STANDARD_CODE(MOUSE, 0, RELATIVE, NONE, XAXIS),
	MOUSECODE_Y = STANDARD_CODE(MOUSE, 0, RELATIVE, NONE, YAXIS),
	MOUSECODE_Z = STANDARD_CODE(MOUSE, 0, RELATIVE, NONE, ZAXIS),

	/* mouse axes as switches in +/- direction */
	MOUSECODE_X_POS_SWITCH = STANDARD_CODE(MOUSE, 0, SWITCH, POS, XAXIS),
	MOUSECODE_X_NEG_SWITCH = STANDARD_CODE(MOUSE, 0, SWITCH, NEG, XAXIS),
	MOUSECODE_Y_POS_SWITCH = STANDARD_CODE(MOUSE, 0, SWITCH, POS, YAXIS),
	MOUSECODE_Y_NEG_SWITCH = STANDARD_CODE(MOUSE, 0, SWITCH, NEG, YAXIS),
	MOUSECODE_Z_POS_SWITCH = STANDARD_CODE(MOUSE, 0, SWITCH, POS, ZAXIS),
	MOUSECODE_Z_NEG_SWITCH = STANDARD_CODE(MOUSE, 0, SWITCH, NEG, ZAXIS),

	/* mouse buttons */
	MOUSECODE_BUTTON1 = STANDARD_CODE(MOUSE, 0, SWITCH, NONE, BUTTON1),
	MOUSECODE_BUTTON2 = STANDARD_CODE(MOUSE, 0, SWITCH, NONE, BUTTON2),
	MOUSECODE_BUTTON3 = STANDARD_CODE(MOUSE, 0, SWITCH, NONE, BUTTON3),
	MOUSECODE_BUTTON4 = STANDARD_CODE(MOUSE, 0, SWITCH, NONE, BUTTON4),
	MOUSECODE_BUTTON5 = STANDARD_CODE(MOUSE, 0, SWITCH, NONE, BUTTON5),
	MOUSECODE_BUTTON6 = STANDARD_CODE(MOUSE, 0, SWITCH, NONE, BUTTON6),
	MOUSECODE_BUTTON7 = STANDARD_CODE(MOUSE, 0, SWITCH, NONE, BUTTON7),
	MOUSECODE_BUTTON8 = STANDARD_CODE(MOUSE, 0, SWITCH, NONE, BUTTON8),

	/* gun axes as absolute devices */
	GUNCODE_X = STANDARD_CODE(LIGHTGUN, 0, ABSOLUTE, NONE, XAXIS),
	GUNCODE_Y = STANDARD_CODE(LIGHTGUN, 0, ABSOLUTE, NONE, YAXIS),

	/* gun buttons */
	GUNCODE_BUTTON1 = STANDARD_CODE(LIGHTGUN, 0, SWITCH, NONE, BUTTON1),
	GUNCODE_BUTTON2 = STANDARD_CODE(LIGHTGUN, 0, SWITCH, NONE, BUTTON2),
	GUNCODE_BUTTON3 = STANDARD_CODE(LIGHTGUN, 0, SWITCH, NONE, BUTTON3),
	GUNCODE_BUTTON4 = STANDARD_CODE(LIGHTGUN, 0, SWITCH, NONE, BUTTON4),
	GUNCODE_BUTTON5 = STANDARD_CODE(LIGHTGUN, 0, SWITCH, NONE, BUTTON5),
	GUNCODE_BUTTON6 = STANDARD_CODE(LIGHTGUN, 0, SWITCH, NONE, BUTTON6),
	GUNCODE_BUTTON7 = STANDARD_CODE(LIGHTGUN, 0, SWITCH, NONE, BUTTON7),
	GUNCODE_BUTTON8 = STANDARD_CODE(LIGHTGUN, 0, SWITCH, NONE, BUTTON8),

	/* joystick axes as absolute devices */
	JOYCODE_X = STANDARD_CODE(JOYSTICK, 0, ABSOLUTE, NONE, XAXIS),
	JOYCODE_Y = STANDARD_CODE(JOYSTICK, 0, ABSOLUTE, NONE, YAXIS),
	JOYCODE_Z = STANDARD_CODE(JOYSTICK, 0, ABSOLUTE, NONE, ZAXIS),
	JOYCODE_U = STANDARD_CODE(JOYSTICK, 0, ABSOLUTE, NONE, RXAXIS),
	JOYCODE_V = STANDARD_CODE(JOYSTICK, 0, ABSOLUTE, NONE, RYAXIS),

	/* joystick axes as absolute half-axes */
	JOYCODE_X_POS_ABSOLUTE = STANDARD_CODE(JOYSTICK, 0, ABSOLUTE, POS, XAXIS),
	JOYCODE_X_NEG_ABSOLUTE = STANDARD_CODE(JOYSTICK, 0, ABSOLUTE, NEG, XAXIS),
	JOYCODE_Y_POS_ABSOLUTE = STANDARD_CODE(JOYSTICK, 0, ABSOLUTE, POS, YAXIS),
	JOYCODE_Y_NEG_ABSOLUTE = STANDARD_CODE(JOYSTICK, 0, ABSOLUTE, NEG, YAXIS),
	JOYCODE_Z_POS_ABSOLUTE = STANDARD_CODE(JOYSTICK, 0, ABSOLUTE, POS, ZAXIS),
	JOYCODE_Z_NEG_ABSOLUTE = STANDARD_CODE(JOYSTICK, 0, ABSOLUTE, NEG, ZAXIS),
	JOYCODE_U_POS_ABSOLUTE = STANDARD_CODE(JOYSTICK, 0, ABSOLUTE, POS, RXAXIS),
	JOYCODE_U_NEG_ABSOLUTE = STANDARD_CODE(JOYSTICK, 0, ABSOLUTE, NEG, RXAXIS),
	JOYCODE_V_POS_ABSOLUTE = STANDARD_CODE(JOYSTICK, 0, ABSOLUTE, POS, RYAXIS),
	JOYCODE_V_NEG_ABSOLUTE = STANDARD_CODE(JOYSTICK, 0, ABSOLUTE, NEG, RYAXIS),

	/* joystick axes as switches; X/Y are specially handled for left/right/up/down mapping */
	JOYCODE_X_LEFT_SWITCH = STANDARD_CODE(JOYSTICK, 0, SWITCH, LEFT, XAXIS),
	JOYCODE_X_RIGHT_SWITCH = STANDARD_CODE(JOYSTICK, 0, SWITCH, RIGHT, XAXIS),
	JOYCODE_Y_UP_SWITCH = STANDARD_CODE(JOYSTICK, 0, SWITCH, UP, YAXIS),
	JOYCODE_Y_DOWN_SWITCH = STANDARD_CODE(JOYSTICK, 0, SWITCH, DOWN, YAXIS),
	JOYCODE_Z_POS_SWITCH = STANDARD_CODE(JOYSTICK, 0, SWITCH, POS, ZAXIS),
	JOYCODE_Z_NEG_SWITCH = STANDARD_CODE(JOYSTICK, 0, SWITCH, NEG, ZAXIS),
	JOYCODE_U_POS_SWITCH = STANDARD_CODE(JOYSTICK, 0, SWITCH, POS, RXAXIS),
	JOYCODE_U_NEG_SWITCH = STANDARD_CODE(JOYSTICK, 0, SWITCH, NEG, RXAXIS),
	JOYCODE_V_POS_SWITCH = STANDARD_CODE(JOYSTICK, 0, SWITCH, POS, RYAXIS),
	JOYCODE_V_NEG_SWITCH = STANDARD_CODE(JOYSTICK, 0, SWITCH, NEG, RYAXIS),

	/* joystick buttons */
	JOYCODE_BUTTON1 = STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, BUTTON1),
	JOYCODE_BUTTON2 = STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, BUTTON2),
	JOYCODE_BUTTON3 = STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, BUTTON3),
	JOYCODE_BUTTON4 = STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, BUTTON4),
	JOYCODE_BUTTON5 = STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, BUTTON5),
	JOYCODE_BUTTON6 = STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, BUTTON6),
	JOYCODE_BUTTON7 = STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, BUTTON7),
	JOYCODE_BUTTON8 = STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, BUTTON8),
	JOYCODE_BUTTON9 = STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, BUTTON9),
	JOYCODE_BUTTON10 = STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, BUTTON10),
	JOYCODE_BUTTON11 = STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, BUTTON11),
	JOYCODE_BUTTON12 = STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, BUTTON12),
	JOYCODE_BUTTON13 = STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, BUTTON13),
	JOYCODE_BUTTON14 = STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, BUTTON14),
	JOYCODE_BUTTON15 = STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, BUTTON15),
	JOYCODE_BUTTON16 = STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, BUTTON16),
	JOYCODE_START = STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, START),
	JOYCODE_SELECT = STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, SELECT)
};



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* input codes are UINT32 */
typedef UINT32 input_code;

/* callback for getting the value of an item on a device */
typedef INT32 (*item_get_state_func)(void *device_internal, void *item_internal);

/* (opaque) input device object */
typedef struct _input_device input_device;



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

/* joystick maps */
extern const char joystick_map_8way[];
extern const char joystick_map_4way_sticky[];
extern const char joystick_map_4way_diagonal[];



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- initialization and configuration ----- */

/* core initialization, prior to calling osd_init() */
void input_init(running_machine *machine);

/* enable or disable a device class */
void input_device_class_enable(running_machine *machine, input_device_class devclass, UINT8 enable);

/* is a device class enabled? */
UINT8 input_device_class_enabled(running_machine *machine, input_device_class devclass);

/* configure default joystick maps */
int input_device_set_joystick_map(running_machine *machine, int devindex, const char *mapstring);


/* ----- OSD configuration and access ----- */

/* add a new input device */
input_device *input_device_add(running_machine *machine, input_device_class devclass, const char *name, void *internal);

/* add a new item to an input device */
void input_device_item_add(input_device *device, const char *name, void *internal, input_item_id itemid, item_get_state_func getstate);



/* ----- state queries ----- */

/* return the value of a particular input code */
INT32 input_code_value(running_machine *machine, input_code code);

/* return TRUE if the given input code has been pressed */
INT32 input_code_pressed(running_machine *machine, input_code code);

/* same as above, but returns TRUE only on the first call after an off->on transition */
INT32 input_code_pressed_once(running_machine *machine, input_code code);

/* translates an input_item_id to an input_code */
input_code input_code_from_input_item_id(running_machine *machine, input_item_id itemid);

/* poll for any switch input, optionally resetting internal memory */
input_code input_code_poll_switches(running_machine *machine, int reset);

/* poll for any keyboard switch input, optionally resetting internal memory */
input_code input_code_poll_keyboard_switches(running_machine *machine, int reset);

/* poll for any axis input, optionally resetting internal memory */
input_code input_code_poll_axes(running_machine *machine, int reset);



/* ----- strings and tokenization ----- */

/* generate the friendly name of an input code, returning the length (buffer can be NULL) */
astring *input_code_name(running_machine *machine, astring *buffer, input_code code);

/* convert an input code to a token, returning the length (buffer can be NULL) */
astring *input_code_to_token(running_machine *machine, astring *buffer, input_code code);

/* convert a token back to an input code */
input_code input_code_from_token(running_machine *machine, const char *_token);



/* ----- debugging utilities ----- */

/* return TRUE if the given input code has been pressed */
INT32 debug_global_input_code_pressed(input_code code);


#endif	/* __INPUT_H__ */
