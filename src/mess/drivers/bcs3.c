/***************************************************************************

        BCS 3

        12/05/2009 Skeleton driver.

    Everything in this driver has been worked out by reading the
    ROM and using the debugger, as no information is available.

    Therefore, it is likely to be full of mistakes and omissions.

    Note: The $ key makes a square symbol.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"

class bcs3_state : public driver_device
{
public:
	bcs3_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	const UINT8 *fnt;
	UINT8 *videoram;
};

static READ8_HANDLER( bcs3_videoram_r )
{
	bcs3_state *state = space->machine->driver_data<bcs3_state>();

	return state->videoram[offset];
}

static WRITE8_HANDLER( bcs3_videoram_w )
{
	bcs3_state *state = space->machine->driver_data<bcs3_state>();

	state->videoram[offset] = data;
}

static READ8_HANDLER( bcs3_keyboard_r )
{
	UINT8 data = 0;

	if (~offset & 0x01)
		data |= input_port_read(space->machine, "LINE0");
	if (~offset & 0x02)
		data |= input_port_read(space->machine, "LINE1");
	if (~offset & 0x04)
		data |= input_port_read(space->machine, "LINE2");
	if (~offset & 0x08)
		data |= input_port_read(space->machine, "LINE3");
	if (~offset & 0x10)
		data |= input_port_read(space->machine, "LINE4");
	if (~offset & 0x20)
		data |= input_port_read(space->machine, "LINE5");
	if (~offset & 0x40)
		data |= input_port_read(space->machine, "LINE6");
	if (~offset & 0x80)
		data |= input_port_read(space->machine, "LINE7");
	if (~offset & 0x100)
		data |= input_port_read(space->machine, "LINE8");

	return data;
}

static ADDRESS_MAP_START(bcs3_mem, ADDRESS_SPACE_PROGRAM, 8)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x0fff ) AM_ROM
	AM_RANGE( 0x1000, 0x11ff ) AM_READ_PORT("LINE9")
	AM_RANGE( 0x1200, 0x13ff ) AM_READ(bcs3_keyboard_r)
	AM_RANGE( 0x3c00, 0xffff ) AM_RAM
	AM_RANGE( 0x3c50, 0x3d9f ) AM_READWRITE(bcs3_videoram_r,bcs3_videoram_w) AM_BASE_MEMBER(bcs3_state,videoram)
ADDRESS_MAP_END

static ADDRESS_MAP_START(bcs3a_mem, ADDRESS_SPACE_PROGRAM, 8)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x0fff ) AM_ROM
	AM_RANGE( 0x1000, 0x11ff ) AM_READ_PORT("LINE9")
	AM_RANGE( 0x1200, 0x13ff ) AM_READ(bcs3_keyboard_r)
	AM_RANGE( 0x3c00, 0xefff ) AM_RAM
	AM_RANGE( 0x3c00, 0x5a7f ) AM_READWRITE(bcs3_videoram_r,bcs3_videoram_w) AM_BASE_MEMBER(bcs3_state,videoram)
	AM_RANGE( 0xf000, 0xf3ff ) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(bcs3b_mem, ADDRESS_SPACE_PROGRAM, 8)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x0fff ) AM_ROM
	AM_RANGE( 0x1000, 0x11ff ) AM_READ_PORT("LINE9")
	AM_RANGE( 0x1200, 0x13ff ) AM_READ(bcs3_keyboard_r)
	AM_RANGE( 0x3c00, 0xefff ) AM_RAM
	AM_RANGE( 0x3c00, 0x657f ) AM_READWRITE(bcs3_videoram_r,bcs3_videoram_w) AM_BASE_MEMBER(bcs3_state,videoram)
	AM_RANGE( 0xf000, 0xf3ff ) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(bcs3c_mem, ADDRESS_SPACE_PROGRAM, 8)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x0fff ) AM_ROM
	AM_RANGE( 0x1000, 0x11ff ) AM_READ_PORT("LINE9")
	AM_RANGE( 0x1200, 0x13ff ) AM_READ(bcs3_keyboard_r)
	AM_RANGE( 0x3c00, 0xffff ) AM_RAM
	AM_RANGE( 0x3c00, 0x5ab3 ) AM_READWRITE(bcs3_videoram_r,bcs3_videoram_w) AM_BASE_MEMBER(bcs3_state,videoram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( bcs3_io, ADDRESS_SPACE_IO, 8)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
//  AM_RANGE( 0xf8, 0xfb ) AM_DEVREADWRITE(Z80CTC,"z80ctc",z80ctc_r,z80ctc_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( bcs3 )
	PORT_START("LINE0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('H')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("K :") PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR(':')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A &") PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('&')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('O')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("LINE1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L ;") PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR(';')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B '") PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR(39)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1 !") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("LINE2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M <") PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('<')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C (") PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('(')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2 \"") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR(34)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("LINE3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N =") PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('=')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D )") PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR(')')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3 #") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("LINE4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("O >") PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('>')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E *") PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4 $") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("LINE5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F +") PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('+')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5 %") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("LINE6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(32)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G ,") PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('.')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6 &") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("LINE7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H -") PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7 '") PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("LINE8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("I .") PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8 (") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("LINE9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("J /") PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9 )") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)	// cassette read bit
INPUT_PORTS_END


static MACHINE_RESET(bcs3)
{
}

static VIDEO_START( bcs3 )
{
	bcs3_state *state = machine->driver_data<bcs3_state>();

	state->fnt = machine->region("gfx1")->base();
}

static SCREEN_UPDATE( bcs3 )
{
	bcs3_state *state = screen->machine->driver_data<bcs3_state>();
	UINT8 y,ra,chr,gfx,rat;
	UINT16 sy=0,ma=0,x;

	for (y = 0; y < 12; y++)
	{
		for (ra = 0; ra < 10; ra++)
		{
			UINT16  *p = BITMAP_ADDR16(bitmap, sy++, 0);
			rat = (ra + 1) & 7;

			for (x = ma; x < ma + 28; x++)
			{
				if (ra < 8)
				{
					chr = state->videoram[x] & 0x7f;

					/* get pattern of pixels for that character scanline */
					gfx = state->fnt[(chr<<3) | rat ];
				}
				else
					gfx = 0;

				/* Display a scanline of a character (8 pixels) */
				*p++ = ( gfx & 0x80 ) ? 0 : 1;
				*p++ = ( gfx & 0x40 ) ? 0 : 1;
				*p++ = ( gfx & 0x20 ) ? 0 : 1;
				*p++ = ( gfx & 0x10 ) ? 0 : 1;
				*p++ = ( gfx & 0x08 ) ? 0 : 1;
				*p++ = ( gfx & 0x04 ) ? 0 : 1;
				*p++ = ( gfx & 0x02 ) ? 0 : 1;
				*p++ = ( gfx & 0x01 ) ? 0 : 1;
			}
		}
		ma+=28;
	}
	return 0;
}

/* This has 100 lines of screen data. I'm assuming that it only shows a portion of this,
    with the cursor always in sight. */
static SCREEN_UPDATE( bcs3a )
{
	bcs3_state *state = screen->machine->driver_data<bcs3_state>();
	UINT8 y,ra,chr,gfx,rat;
	UINT16 sy=0,ma=128,x;
	UINT16 cursor = (state->videoram[0x7a] | (state->videoram[0x7b] << 8)) - 0x3c80;	// get cursor relative position
	rat = cursor / 30;
	if (rat > 11) ma = (rat-11) * 30 + 128;

	for (y = 0; y < 12; y++)
	{
		for (ra = 0; ra < 10; ra++)
		{
			UINT16  *p = BITMAP_ADDR16(bitmap, sy++, 0);
			rat = (ra + 1) & 7;

			for (x = ma; x < ma + 29; x++)
			{
				if (ra < 8)
				{
					chr = state->videoram[x] & 0x7f;

					/* get pattern of pixels for that character scanline */
					gfx = state->fnt[(chr<<3) | rat ];
				}
				else
					gfx = 0;

				/* Display a scanline of a character (8 pixels) */
				*p++ = ( gfx & 0x80 ) ? 0 : 1;
				*p++ = ( gfx & 0x40 ) ? 0 : 1;
				*p++ = ( gfx & 0x20 ) ? 0 : 1;
				*p++ = ( gfx & 0x10 ) ? 0 : 1;
				*p++ = ( gfx & 0x08 ) ? 0 : 1;
				*p++ = ( gfx & 0x04 ) ? 0 : 1;
				*p++ = ( gfx & 0x02 ) ? 0 : 1;
				*p++ = ( gfx & 0x01 ) ? 0 : 1;
			}
		}
		ma+=30;
	}
	return 0;
}

static SCREEN_UPDATE( bcs3b )
{
	bcs3_state *state = screen->machine->driver_data<bcs3_state>();
	UINT8 y,ra,chr,gfx,rat;
	UINT16 sy=0,ma=128,x;
	UINT16 cursor = (state->videoram[0x7a] | (state->videoram[0x7b] << 8)) - 0x3c80;	// get cursor relative position
	rat = cursor / 41;
	if (rat > 23) ma = (rat-23) * 41 + 128;

	for (y = 0; y < 24; y++)
	{
		for (ra = 0; ra < 10; ra++)
		{
			UINT16  *p = BITMAP_ADDR16(bitmap, sy++, 0);
			rat = (ra + 1) & 7;

			for (x = ma; x < ma + 40; x++)
			{
				if (ra < 8)
				{
					chr = state->videoram[x] & 0x7f;

					/* get pattern of pixels for that character scanline */
					gfx = state->fnt[(chr<<3) | rat ];
				}
				else
					gfx = 0;

				/* Display a scanline of a character (8 pixels) */
				*p++ = ( gfx & 0x80 ) ? 0 : 1;
				*p++ = ( gfx & 0x40 ) ? 0 : 1;
				*p++ = ( gfx & 0x20 ) ? 0 : 1;
				*p++ = ( gfx & 0x10 ) ? 0 : 1;
				*p++ = ( gfx & 0x08 ) ? 0 : 1;
				*p++ = ( gfx & 0x04 ) ? 0 : 1;
				*p++ = ( gfx & 0x02 ) ? 0 : 1;
				*p++ = ( gfx & 0x01 ) ? 0 : 1;
			}
		}
		ma+=41;
	}
	return 0;
}

static SCREEN_UPDATE( bcs3c )
{
	bcs3_state *state = screen->machine->driver_data<bcs3_state>();
	UINT8 y,ra,chr,gfx,rat;
	UINT16 sy=0,ma=0xb4,x;
	UINT16 cursor = (state->videoram[0x08] | (state->videoram[0x09] << 8)) - 0x3c80;	// get cursor relative position
	rat = cursor / 30;
	if (rat > 11) ma = (rat-11) * 30 + 0xb4;

	for (y = 0; y < 12; y++)
	{
		for (ra = 0; ra < 10; ra++)
		{
			UINT16  *p = BITMAP_ADDR16(bitmap, sy++, 0);
			rat = (ra + 1) & 7;

			for (x = ma; x < ma + 29; x++)
			{
				if (ra < 8)
				{
					chr = state->videoram[x] & 0x7f;

					/* get pattern of pixels for that character scanline */
					gfx = state->fnt[(chr<<3) | rat ];
				}
				else
					gfx = 0;

				/* Display a scanline of a character (8 pixels) */
				*p++ = ( gfx & 0x80 ) ? 0 : 1;
				*p++ = ( gfx & 0x40 ) ? 0 : 1;
				*p++ = ( gfx & 0x20 ) ? 0 : 1;
				*p++ = ( gfx & 0x10 ) ? 0 : 1;
				*p++ = ( gfx & 0x08 ) ? 0 : 1;
				*p++ = ( gfx & 0x04 ) ? 0 : 1;
				*p++ = ( gfx & 0x02 ) ? 0 : 1;
				*p++ = ( gfx & 0x01 ) ? 0 : 1;
			}
		}
		ma+=30;
	}
	return 0;
}

/* F4 Character Displayer */
static const gfx_layout bcs3_charlayout =
{
	8, 8,					/* 8 x 8 characters */
	128,					/* 128 characters */
	1,					/* 1 bits per pixel */
	{ 0 },					/* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 0*8 },
	8*8					/* every char takes 8 bytes */
};

static GFXDECODE_START( bcs3 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, bcs3_charlayout, 0, 1 )
GFXDECODE_END


static MACHINE_CONFIG_START( bcs3, bcs3_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_5MHz /2)
	MCFG_CPU_PROGRAM_MAP(bcs3_mem)
	MCFG_CPU_IO_MAP(bcs3_io)

	MCFG_MACHINE_RESET(bcs3)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(28*8, 12*10)
	MCFG_SCREEN_VISIBLE_AREA(0,28*8-1,0,12*10-1)
	MCFG_SCREEN_UPDATE(bcs3)

	MCFG_GFXDECODE(bcs3)
	MCFG_PALETTE_LENGTH(2)
	MCFG_PALETTE_INIT(black_and_white)

	MCFG_VIDEO_START(bcs3)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( bcs3a, bcs3 )
	/* basic machine hardware */
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP(bcs3a_mem)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_SIZE(29*8, 12*10)
	MCFG_SCREEN_VISIBLE_AREA(0,29*8-1,0,12*10-1)
	MCFG_SCREEN_UPDATE(bcs3a)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( bcs3b, bcs3 )
	/* basic machine hardware */
	MCFG_CPU_REPLACE( "maincpu", Z80, XTAL_7MHz / 2)
	MCFG_CPU_PROGRAM_MAP(bcs3b_mem)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_SIZE(40*8, 24*10)
	MCFG_SCREEN_VISIBLE_AREA(0,40*8-1,0,24*10-1)
	MCFG_SCREEN_UPDATE(bcs3b)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( bcs3c, bcs3 )
	/* basic machine hardware */
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP(bcs3c_mem)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_SIZE(29*8, 12*10)
	MCFG_SCREEN_VISIBLE_AREA(0,29*8-1,0,12*10-1)
	MCFG_SCREEN_UPDATE(bcs3c)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( bcs3 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "se24.bin", 0x0000, 0x0800, CRC(268de5ee) SHA1(78784945956c1b0282a4e82ad55e7c3a77389e50))

	ROM_REGION( 0x0400, "gfx1", 0 )
	ROM_LOAD( "se24font.bin", 0x0000, 0x0400, CRC(eaed9d84) SHA1(7023a6187cd6bd0c6489d76ff662453f14e5b636))
ROM_END

ROM_START( bcs3a )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "se31_29.bin", 0x0000, 0x1000, CRC(e9b55544) SHA1(82bae68c4bcaecf66632f5b43913b50a1acba316))
	ROM_LOAD( "se31mceditor.bin", 0xf000, 0x0400, CRC(8eac92ec) SHA1(8950a3ef05d02abf34269bfce002c46d273ce113))

	ROM_REGION( 0x0400, "gfx1", 0 )
	ROM_LOAD( "se31font.bin", 0x0000, 0x0400, CRC(a20c93c9) SHA1(b2be1c0d98b7ac05713349b099b392975968be1d))
ROM_END

ROM_START( bcs3b )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "se31_40.bin", 0x0000, 0x1000, CRC(4e993152) SHA1(6bb01ff5779627fa2eb2df432fffcfccc1e33231))
	ROM_LOAD( "se31mceditor.bin", 0xf000, 0x0400, CRC(8eac92ec) SHA1(8950a3ef05d02abf34269bfce002c46d273ce113))

	ROM_REGION( 0x0400, "gfx1", 0 )
	ROM_LOAD( "se31font.bin", 0x0000, 0x0400, CRC(a20c93c9) SHA1(b2be1c0d98b7ac05713349b099b392975968be1d))
ROM_END

ROM_START( bcs3c )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "sp33_29.bin", 0x0000, 0x1000, CRC(1c851eb2) SHA1(4f8bb5274ea1861a35a840e8f3482bdc693047c4))

	ROM_REGION( 0x0400, "gfx1", 0 )
	ROM_LOAD( "sp33font.bin", 0x0000, 0x0400, CRC(b27f1c07) SHA1(61c80c585f198370ba5e856839c12b15acdc58ee))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT     COMPANY   FULLNAME       FLAGS */
COMP( 1984, bcs3,   0,       0,      bcs3,	bcs3,	 0, 	 "Eckhard Schiller",   "BCS 3 rev 2.4", GAME_NOT_WORKING | GAME_NO_SOUND)
COMP( 1986, bcs3a,  bcs3,    0,      bcs3a, 	bcs3,	 0, 	 "Eckhard Schiller",   "BCS 3 rev 3.1 29-column", GAME_NOT_WORKING | GAME_NO_SOUND)
COMP( 1986, bcs3b,  bcs3,    0,      bcs3b, 	bcs3,	 0, 	 "Eckhard Schiller",   "BCS 3 rev 3.1 40-column", GAME_NOT_WORKING | GAME_NO_SOUND)
COMP( 198?, bcs3c,  bcs3,    0,      bcs3c, 	bcs3,	 0, 	 "Eckhard Schiller",   "BCS 3 rev 3.3", GAME_NOT_WORKING | GAME_NO_SOUND)
