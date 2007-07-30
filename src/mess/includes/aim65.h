#include "driver.h"

/* R6502 Clock
 *
 * The R6502 on AIM65 operatores at 1 MHz. The frequency reference is a 4 Mhz
 * crystal controllred oscillator. Dual D-type flip-flop Z10 devides the 4 MHz
 * signal by four to drive the R6502 phase 0 (O0) input with a 1 MHz clock.
 */
#define OSC_Y1 4000000

extern DRIVER_INIT( aim65 );

VIDEO_START( aim65 );

/* Printer */
void aim65_printer_data_a(UINT8 data);
void aim65_printer_data_b(UINT8 data);

TIMER_CALLBACK(aim65_printer_timer);
WRITE8_HANDLER( aim65_printer_on );


#define KEY_1 (readinputport(0)&1)
#define KEY_2 (readinputport(0)&2)
#define KEY_3 (readinputport(0)&4)
#define KEY_4 (readinputport(0)&8)
#define KEY_5 (readinputport(0)&0x10)
#define KEY_6 (readinputport(0)&0x20)
#define KEY_7 (readinputport(0)&0x40)
#define KEY_8 (readinputport(0)&0x80)
#define KEY_9 (readinputport(0)&0x100)
#define KEY_0 (readinputport(0)&0x200)
#define KEY_COLON (readinputport(0)&0x400)
#define KEY_F3 (readinputport(0)&0x800)
#define KEY_PRINT (readinputport(0)&0x1000)
#define KEY_ESC (readinputport(1)&1)
#define KEY_Q (readinputport(1)&2)
#define KEY_W (readinputport(1)&4)
#define KEY_E (readinputport(1)&8)
#define KEY_R (readinputport(1)&0x10)
#define KEY_T (readinputport(1)&0x20)
#define KEY_Y (readinputport(1)&0x40)
#define KEY_U (readinputport(1)&0x80)
#define KEY_I (readinputport(1)&0x100)
#define KEY_O (readinputport(1)&0x200)
#define KEY_P (readinputport(1)&0x400)
#define KEY_MINUS (readinputport(1)&0x800)
#define KEY_F2 (readinputport(1)&0x1000)
#define KEY_RETURN (readinputport(1)&0x2000)
#define KEY_CRTL (readinputport(2)&1)
#define KEY_A (readinputport(2)&2)
#define KEY_S (readinputport(2)&4)
#define KEY_D (readinputport(2)&8)
#define KEY_F (readinputport(2)&0x10)
#define KEY_G (readinputport(2)&0x20)
#define KEY_H (readinputport(2)&0x40)
#define KEY_J (readinputport(2)&0x80)
#define KEY_K (readinputport(2)&0x100)
#define KEY_L (readinputport(2)&0x200)
#define KEY_SEMICOLON (readinputport(2)&0x400)
#define KEY_LF (readinputport(2)&0x800)
#define KEY_F1 (readinputport(2)&0x1000)
#define KEY_DEL (readinputport(2)&0x2000)
#define KEY_LEFT_SHIFT (readinputport(3)&1)
#define KEY_Z (readinputport(3)&2)
#define KEY_X (readinputport(3)&4)
#define KEY_C (readinputport(3)&8)
#define KEY_V (readinputport(3)&0x10)
#define KEY_B (readinputport(3)&0x20)
#define KEY_N (readinputport(3)&0x40)
#define KEY_M (readinputport(3)&0x80)
#define KEY_COMMA (readinputport(3)&0x100)
#define KEY_POINT (readinputport(3)&0x200)
#define KEY_SLASH (readinputport(3)&0x400)
#define KEY_RIGHT_SHIFT (readinputport(3)&0x800)
#define KEY_SPACE (readinputport(3)&0x1000)

