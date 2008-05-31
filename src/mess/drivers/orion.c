/***************************************************************************

        Orion driver by Miodrag Milanovic

        22/04/2008 Orion Pro added
        02/04/2008 Preliminary driver.

****************************************************************************/


#include "driver.h"
#include "cpu/i8085/i8085.h"
#include "machine/8255ppi.h"
#include "includes/orion.h"
#include "machine/mc146818.h"
#include "devices/basicdsk.h"
#include "devices/cassette.h"
#include "devices/cartslot.h"
#include "formats/rk_cas.h"
#include "sound/ay8910.h"
#include "sound/speaker.h"

/* Address maps */

/* Orion 128 */
static ADDRESS_MAP_START(orion128_mem, ADDRESS_SPACE_PROGRAM, 8)
	AM_RANGE( 0x0000, 0xefff ) AM_RAMBANK(1)
	AM_RANGE( 0xf000, 0xf3ff ) AM_RAMBANK(2)
    AM_RANGE( 0xf400, 0xf4ff ) AM_READWRITE(orion128_system_r,orion128_system_w)  // Keyboard and cassette
    AM_RANGE( 0xf500, 0xf5ff ) AM_READWRITE(orion128_romdisk_r,orion128_romdisk_w)
    AM_RANGE( 0xf700, 0xf7ff ) AM_READWRITE(orion128_floppy_r,orion128_floppy_w)
    AM_RANGE( 0xf800, 0xffff ) AM_ROM
    AM_RANGE( 0xf800, 0xf8ff ) AM_WRITE (orion128_video_mode_w)
    AM_RANGE( 0xf900, 0xf9ff ) AM_WRITE (orion128_memory_page_w)
    AM_RANGE( 0xfa00, 0xfaff ) AM_WRITE (orion128_video_page_w)
ADDRESS_MAP_END

/* Orion Z80 Card II */
static ADDRESS_MAP_START( orion128_io , ADDRESS_SPACE_IO, 8)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0xf8, 0xf8) AM_WRITE ( orion128_video_mode_w )
	AM_RANGE( 0xf9, 0xf9) AM_WRITE ( orion128_memory_page_w )
	AM_RANGE( 0xfa, 0xfa) AM_WRITE ( orion128_video_page_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START(orionz80_mem, ADDRESS_SPACE_PROGRAM, 8)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x3fff ) AM_RAMBANK(1)
	AM_RANGE( 0x4000, 0xefff ) AM_RAMBANK(2)
	AM_RANGE( 0xf000, 0xf3ff ) AM_RAMBANK(3)
    AM_RANGE( 0xf400, 0xf7ff ) AM_RAMBANK(4)
    AM_RANGE( 0xf800, 0xffff ) AM_RAMBANK(5)
ADDRESS_MAP_END

/* Orion Pro */
static ADDRESS_MAP_START( orionz80_io , ADDRESS_SPACE_IO, 8)
    AM_RANGE( 0x0000, 0xffff) AM_READWRITE ( orionz80_io_r, orionz80_io_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START(orionpro_mem, ADDRESS_SPACE_PROGRAM, 8)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x1fff ) AM_RAMBANK(1)
	AM_RANGE( 0x2000, 0x3fff ) AM_RAMBANK(2)
	AM_RANGE( 0x4000, 0x7fff ) AM_RAMBANK(3)
	AM_RANGE( 0x8000, 0xbfff ) AM_RAMBANK(4)
	AM_RANGE( 0xc000, 0xefff ) AM_RAMBANK(5)
	AM_RANGE( 0xf000, 0xf3ff ) AM_RAMBANK(6)
    AM_RANGE( 0xf400, 0xf7ff ) AM_RAMBANK(7)
    AM_RANGE( 0xf800, 0xffff ) AM_RAMBANK(8)
ADDRESS_MAP_END

static ADDRESS_MAP_START( orionpro_io , ADDRESS_SPACE_IO, 8)
    AM_RANGE( 0x0000, 0xffff) AM_READWRITE ( orionpro_io_r, orionpro_io_w )
ADDRESS_MAP_END

/* Input ports */
INPUT_PORTS_START( orion128 )
	PORT_START_TAG("LINE0")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Home") PORT_CODE(KEYCODE_HOME)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ins") PORT_CODE(KEYCODE_INSERT)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5)
	PORT_START_TAG("LINE1")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("End") PORT_CODE(KEYCODE_END)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Back") PORT_CODE(KEYCODE_BACKSPACE)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)
	PORT_START_TAG("LINE2")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)
	PORT_START_TAG("LINE3")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(":") PORT_CODE(KEYCODE_EQUALS)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("+") PORT_CODE(KEYCODE_COLON)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("<") PORT_CODE(KEYCODE_COMMA)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(">") PORT_CODE(KEYCODE_STOP)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_SLASH)
	PORT_START_TAG("LINE4")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("~") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)
	PORT_START_TAG("LINE5")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)
	PORT_START_TAG("LINE6")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)
	PORT_START_TAG("LINE7")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_OPENBRACE)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\") PORT_CODE(KEYCODE_BACKSLASH)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("]") PORT_CODE(KEYCODE_CLOSEBRACE)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("'") PORT_CODE(KEYCODE_QUOTE)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)
	PORT_START_TAG("LINE8")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Rus/Lat") PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT)
INPUT_PORTS_END
INPUT_PORTS_START( ms7007 )
	PORT_START_TAG("LINE0")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num 6") PORT_CODE(KEYCODE_6_PAD)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ins") PORT_CODE(KEYCODE_INSERT)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("End") PORT_CODE(KEYCODE_END)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_SLASH)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(":") PORT_CODE(KEYCODE_EQUALS)
	PORT_START_TAG("LINE1")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
	PORT_START_TAG("LINE2")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_OPENBRACE)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("]") PORT_CODE(KEYCODE_CLOSEBRACE)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)
	PORT_START_TAG("LINE3")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)
	PORT_START_TAG("LINE4")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)
	PORT_START_TAG("LINE5")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("<") PORT_CODE(KEYCODE_COMMA)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("'") PORT_CODE(KEYCODE_QUOTE)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)
	PORT_START_TAG("LINE6")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\") PORT_CODE(KEYCODE_BACKSLASH)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num 4") PORT_CODE(KEYCODE_4_PAD)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num +") PORT_CODE(KEYCODE_PLUS_PAD)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Back") PORT_CODE(KEYCODE_BACKSPACE)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(">") PORT_CODE(KEYCODE_STOP)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_START_TAG("LINE7")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num 5") PORT_CODE(KEYCODE_5_PAD)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F6") PORT_CODE(KEYCODE_F6)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Home") PORT_CODE(KEYCODE_HOME)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num /") PORT_CODE(KEYCODE_SLASH_PAD)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER)
	PORT_START_TAG("LINE8")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Rus/Lat") PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT)
INPUT_PORTS_END

/* Machine driver */
static MACHINE_DRIVER_START( orion128 )
    MDRV_CPU_ADD(8080, 2000000)
    MDRV_CPU_PROGRAM_MAP(orion128_mem, 0)
    MDRV_CPU_IO_MAP(orion128_io, 0)

    MDRV_MACHINE_START( orion128 )
    MDRV_MACHINE_RESET( orion128 )

	MDRV_DEVICE_ADD( "ppi8255_1", PPI8255 )
	MDRV_DEVICE_CONFIG( orion128_ppi8255_interface_1 )

	MDRV_DEVICE_ADD( "ppi8255_2", PPI8255 )
	MDRV_DEVICE_CONFIG( orion128_ppi8255_interface_2 )

    /* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(50)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(384, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 384-1, 0, 256-1)

	MDRV_PALETTE_LENGTH(18)
	MDRV_PALETTE_INIT( orion128 )


	MDRV_VIDEO_START(orion128)
    MDRV_VIDEO_UPDATE(orion128)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD(WAVE, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_DRIVER_END

static const struct AY8910interface orionz80_ay_interface =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	NULL
};

static MACHINE_DRIVER_START( orionz80 )
    MDRV_CPU_ADD(Z80, 2500000)
    MDRV_CPU_PROGRAM_MAP(orionz80_mem, 0)
    MDRV_CPU_IO_MAP(orionz80_io, 0)
    MDRV_CPU_VBLANK_INT("main",orionz80_interrupt)

    MDRV_MACHINE_START( orionz80 )
    MDRV_MACHINE_RESET( orionz80 )

	MDRV_DEVICE_ADD( "ppi8255_1", PPI8255 )
	MDRV_DEVICE_CONFIG( orion128_ppi8255_interface_1 )

	MDRV_DEVICE_ADD( "ppi8255_2", PPI8255 )
	MDRV_DEVICE_CONFIG( orion128_ppi8255_interface_2 )

    /* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(50)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(384, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 384-1, 0, 256-1)

	MDRV_PALETTE_LENGTH(18)
	MDRV_PALETTE_INIT( orion128 )

	MDRV_VIDEO_START(orion128)
	MDRV_VIDEO_UPDATE(orion128)

	MDRV_NVRAM_HANDLER( mc146818 )

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD(SPEAKER, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MDRV_SOUND_ADD(WAVE, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
	MDRV_SOUND_ADD(AY8912, 1773400)
	MDRV_SOUND_CONFIG(orionz80_ay_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( orionpro )
    MDRV_CPU_ADD(Z80, 5000000)
    MDRV_CPU_PROGRAM_MAP(orionpro_mem, 0)
    MDRV_CPU_IO_MAP(orionpro_io, 0)

    MDRV_MACHINE_START( orionpro )
    MDRV_MACHINE_RESET( orionpro )

	MDRV_DEVICE_ADD( "ppi8255_1", PPI8255 )
	MDRV_DEVICE_CONFIG( orion128_ppi8255_interface_1 )

	MDRV_DEVICE_ADD( "ppi8255_2", PPI8255 )
	MDRV_DEVICE_CONFIG( orion128_ppi8255_interface_2 )

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(50)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(384, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 384-1, 0, 256-1)

	MDRV_PALETTE_LENGTH(18)
	MDRV_PALETTE_INIT( orion128 )

	MDRV_VIDEO_START(orion128)
    MDRV_VIDEO_UPDATE(orion128)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD(SPEAKER, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MDRV_SOUND_ADD(WAVE, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
	MDRV_SOUND_ADD(AY8912, 1773400)
	MDRV_SOUND_CONFIG(orionz80_ay_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_DRIVER_END

static void orion_cassette_getinfo(const mess_device_class *devclass, UINT32 state, union devinfo *info)
{
	/* cassette */
	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case MESS_DEVINFO_INT_COUNT:				info->i = 1; break;
		case MESS_DEVINFO_INT_CASSETTE_DEFAULT_STATE:	info->i = CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED; break;
		case MESS_DEVINFO_PTR_CASSETTE_FORMATS:		info->p = (void *)rko_cassette_formats; break;

		default:					cassette_device_getinfo(devclass, state, info); break;
	}
}

static void orion_floppy_getinfo(const mess_device_class *devclass, UINT32 state, union devinfo *info)
{
	/* floppy */
	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case MESS_DEVINFO_INT_COUNT:							info->i = 4; break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case MESS_DEVINFO_PTR_LOAD:							info->load = DEVICE_IMAGE_LOAD_NAME(orion_floppy); break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case MESS_DEVINFO_STR_FILE_EXTENSIONS:				strcpy(info->s = device_temp_str(), "odi,img"); break;

		default:										legacybasicdsk_device_getinfo(devclass, state, info); break;
	}
}

SYSTEM_CONFIG_START(orion128)
	CONFIG_RAM_DEFAULT(256 * 1024)
	CONFIG_DEVICE(cartslot_device_getinfo)
	CONFIG_DEVICE(orion_cassette_getinfo);
	CONFIG_DEVICE(orion_floppy_getinfo);
SYSTEM_CONFIG_END

SYSTEM_CONFIG_START(orionz80)
	CONFIG_RAM_DEFAULT(512 * 1024)
	CONFIG_DEVICE(cartslot_device_getinfo)
	CONFIG_DEVICE(orion_cassette_getinfo);
	CONFIG_DEVICE(orion_floppy_getinfo);
SYSTEM_CONFIG_END

SYSTEM_CONFIG_START(orionpro)
	CONFIG_RAM_DEFAULT(512 * 1024)
	CONFIG_DEVICE(cartslot_device_getinfo)
	CONFIG_DEVICE(orion_cassette_getinfo);
	CONFIG_DEVICE(orion_floppy_getinfo);
SYSTEM_CONFIG_END

/* ROM definition */

ROM_START( orion128 )
    ROM_REGION( 0x30000, REGION_CPU1, ROMREGION_ERASEFF )
    ROM_SYSTEM_BIOS( 0, "m2rk", "Version 3.2 rk" )
    ROMX_LOAD( "m2rk.bin",    0x0f800, 0x0800, CRC(2025c234) SHA1(caf86918629be951fe698cddcdf4589f07e2fb96), ROM_BIOS(1) )
    ROM_SYSTEM_BIOS( 1, "m2_2rk", "Version 3.2.2 rk" )
    ROMX_LOAD( "m2_2rk.bin",  0x0f800, 0x0800, CRC(fc662351) SHA1(7c6de67127fae5869281449de1c503597c0c058e), ROM_BIOS(2) )
    ROM_CART_LOAD(0, "bin", 0x10000, 0x10000, ROM_FILL_FF | ROM_OPTIONAL)
ROM_END

ROM_START( orionms )
    ROM_REGION( 0x30000, REGION_CPU1, ROMREGION_ERASEFF )
    ROM_LOAD( "ms7007.bin",   0x0f800, 0x0800, CRC(c6174ba3) SHA1(8f9a42c3e09684718fe4121a8408e7860129d26f) )
    ROM_CART_LOAD(0, "bin", 0x10000, 0x10000, ROM_FILL_FF | ROM_OPTIONAL)
ROM_END

ROM_START( orionz80 )
    ROM_REGION( 0x30000, REGION_CPU1, ROMREGION_ERASEFF )
    ROM_SYSTEM_BIOS( 0, "m31", "Version 3.1" )
    ROMX_LOAD( "m31.bin",     0x0f800, 0x0800, CRC(007c6dc6) SHA1(338ff95497c820338f7f79c75f65bc540a5541c4), ROM_BIOS(1) )
    ROM_SYSTEM_BIOS( 1, "m32zrk", "Version 3.2 zrk" )
    ROMX_LOAD( "m32zrk.bin",  0x0f800, 0x0800, CRC(4ec3f012) SHA1(6b0b2bfc515a80e7caf72c3c33cf2dcf192d4711), ROM_BIOS(2) )
    ROM_SYSTEM_BIOS( 2, "m33zrkd", "Version 3.3 zrkd" )
    ROMX_LOAD( "m33zrkd.bin", 0x0f800, 0x0800, CRC(f404032d) SHA1(088cd9ed05f0dda4fa0a005c609208d9f57ad3d9), ROM_BIOS(3) )
    ROM_SYSTEM_BIOS( 3, "m34zrk", "Version 3.4 zrk" )
    ROMX_LOAD( "m34zrk.bin",  0x0f800, 0x0800, CRC(787c3903) SHA1(476c1c0b88e5efb582292eebec15e24d054c8851), ROM_BIOS(4) )
    ROM_SYSTEM_BIOS( 4, "m35zrkd", "Version 3.5 zrkd" )
    ROMX_LOAD( "m35zrkd.bin", 0x0f800, 0x0800, CRC(9368b38f) SHA1(64a77f22119d40c9b18b64d78ad12acc6fff9efb), ROM_BIOS(5) )
    ROM_CART_LOAD(0, "bin", 0x10000, 0x10000, ROM_FILL_FF | ROM_OPTIONAL)
ROM_END

ROM_START( orionzms )
    ROM_REGION( 0x30000, REGION_CPU1, ROMREGION_ERASEFF )
    ROM_SYSTEM_BIOS( 0, "m32zms", "Version 3.2 zms" )
    ROMX_LOAD( "m32zms.bin",  0x0f800, 0x0800, CRC(44cfd2ae) SHA1(84d53fbc249938c56be76ee4e6ab297f0461835b), ROM_BIOS(1) )
    ROM_SYSTEM_BIOS( 1, "m34zms", "Version 3.4 zms" )
    ROMX_LOAD( "m34zms.bin",  0x0f800, 0x0800, CRC(0f87a80b) SHA1(ab1121092e61268d8162ed8a7d4fd081016a409a), ROM_BIOS(2) )
    ROM_SYSTEM_BIOS( 2, "m35zmsd", "Version 3.5 zmsd" )
    ROMX_LOAD( "m35zmsd.bin", 0x0f800, 0x0800, CRC(f714ff37) SHA1(fbe9514adb3384aff146cbedd4fede37ce9591e1), ROM_BIOS(3) )
    ROM_CART_LOAD(0, "bin", 0x10000, 0x10000, ROM_FILL_FF | ROM_OPTIONAL)
ROM_END

ROM_START( orionpro )
	ROM_REGION( 0x52000, REGION_CPU1, ROMREGION_ERASEFF )
    ROM_CART_LOAD(0, "bin",   0x10000, 0x10000, ROM_FILL_FF | ROM_OPTIONAL)
    ROM_LOAD( "rom1-210.bin", 0x20000, 0x2000,  CRC(8e1a0c78) SHA1(61c8a5ed596ce7e3fd32da920dcc80dc5375b421) )
	ROM_LOAD( "rom2-210.bin", 0x22000, 0x10000, CRC(7cb7a49b) SHA1(601f3dd61db323407c4874fd7f23c10dccac0209) )
ROM_END

/* Driver */

/*    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT           INIT    CONFIG  COMPANY              FULLNAME   FLAGS */
COMP( 1990, orion128, 	 0,  		0,		orion128, 	orion128, 	orion128, orion128,  "", 					 "Orion 128",	 0)
COMP( 1990, orionms, 	 orion128, 	0,		orion128, 	ms7007, 	orion128, orion128,  "", 					 "Orion 128 (MS7007)",	 0)
COMP( 1990, orionz80, 	 orion128, 	0,		orionz80, 	orion128,	orion128, orionz80,  "", 					 "Orion 128 + Z80 Card II",	 0)
COMP( 1990, orionzms, 	 orion128, 	0,		orionz80, 	ms7007, 	orion128, orionz80,  "", 					 "Orion 128 + Z80 Card II (MS7007)",	 0)
COMP( 1994, orionpro, 	 orion128, 	0,		orionpro, 	orion128, 	orion128, orionpro,  "", 					 "Orion Pro",	 0)
