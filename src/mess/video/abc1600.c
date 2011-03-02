#include "includes/abc1600.h"



//**************************************************************************
//  CONSTANTS / MACROS
//**************************************************************************

// IOWR0 registers
enum
{
	LDSX_HB = 0,
	LSDX_LB,
	LDSY_HB,
	LDSY_LB,
	LDTX_HB,
	LDTX_LB,
	LDTY_HB,
	LDTY_LB
};


// IOWR1 registers
enum
{
	LDFX_HB = 0,
	LDFX_LB,
	LDFY_HB,
	LDFY_LB,
	WRML = 5,
	WRDL = 7
};


// IOWR2 registers
enum
{
	WRMASK_STROBE_HB = 0,
	WRMASK_STROBE_LB,
	ENABLE_CLOCKS,
	FLAG_STROBE,
	ENDISP
};



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

//-------------------------------------------------
//  iord0_r -
//-------------------------------------------------

READ8_MEMBER( abc1600_state::iord0_r )
{
	/*

		bit		description

		0		0
		1		SCREENPOS
		2		
		3		
		4		
		5		
		6		VSYNC
		7		BUSY

	*/

	UINT8 data = 0;

	// vertical sync
	data |= m_vs << 6;

	return data;
}


//-------------------------------------------------
//  iowr0_w -
//-------------------------------------------------

WRITE8_MEMBER( abc1600_state::iowr0_w )
{
}


//-------------------------------------------------
//  iowr1_w -
//-------------------------------------------------

WRITE8_MEMBER( abc1600_state::iowr1_w )
{
}


//-------------------------------------------------
//  iowr2_w -
//-------------------------------------------------

WRITE8_MEMBER( abc1600_state::iowr2_w )
{
}



//**************************************************************************
//  VIDEO
//**************************************************************************

//-------------------------------------------------
//  mc6845_interface crtc_intf
//-------------------------------------------------

static MC6845_UPDATE_ROW( abc1600_update_row )
{
}

WRITE_LINE_MEMBER( abc1600_state::vs_w )
{
	m_vs = state;
}

static MC6845_ON_UPDATE_ADDR_CHANGED( crtc_update )
{
}

static const mc6845_interface crtc_intf =
{
	SCREEN_TAG,
	8,
	NULL,
	abc1600_update_row,
	NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_LINE_MEMBER(abc1600_state, vs_w),
	crtc_update
};


//-------------------------------------------------
//  VIDEO_START( abc1600 )
//-------------------------------------------------

void abc1600_state::video_start()
{
	// allocate video RAM
	m_video_ram = auto_alloc_array(machine, UINT8, 512*1024);
}


//-------------------------------------------------
//  SCREEN_UPDATE( abc1600 )
//-------------------------------------------------

bool abc1600_state::screen_update(screen_device &screen, bitmap_t &bitmap, const rectangle &cliprect)
{
	mc6845_update(m_crtc, &bitmap, &cliprect);
	
	for (int y = 0; y < 1024; y++)
	{
		UINT32 addr = y * 64;

		for (int sx = 0; sx < 48; sx++)
		{
			UINT16 data = m_video_ram[addr] << 8 | m_video_ram[addr + 1];

			for (int x = 0; x < 16; x++)
			{
				*BITMAP_ADDR16(&bitmap, y, (sx * 16) + x) = BIT(data, x);
			}

			addr += 2;
		}
	}

	return 0;
}


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( abc1600_video )
//-------------------------------------------------

MACHINE_CONFIG_FRAGMENT( abc1600_video )
    MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
    MCFG_SCREEN_REFRESH_RATE(60)
    MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) // not accurate
    MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
    MCFG_SCREEN_SIZE(768, 1024)
    MCFG_SCREEN_VISIBLE_AREA(0, 768-1, 0, 1024-1)
    MCFG_PALETTE_LENGTH(2)
    MCFG_PALETTE_INIT(black_and_white)
	MCFG_MC6845_ADD(SY6845E_TAG, SY6845E, XTAL_64MHz/32, crtc_intf)
MACHINE_CONFIG_END
