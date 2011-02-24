/***************************************************************************
 *  Sharp MZ700
 *
 *  video hardware
 *
 *  Juergen Buchmueller <pullmoll@t-online.de>, Jul 2000
 *
 *  Reference: http://sharpmz.computingmuseum.com
 *
 ***************************************************************************/

#include "emu.h"
#include "machine/pit8253.h"
#include "includes/mz700.h"


#ifndef VERBOSE
#define VERBOSE 1
#endif

#define LOG(N,M,A)	\
	do { \
		if(VERBOSE>=N) \
		{ \
			if( M ) \
				logerror("%11.6f: %-24s",machine->time().as_double(),(char*)M ); \
			logerror A; \
		} \
	} while (0)


PALETTE_INIT( mz700 )
{
	int i;

	machine->colortable = colortable_alloc(machine, 8);

	for (i = 0; i < 8; i++)
		colortable_palette_set_color(machine->colortable, i, MAKE_RGB((i & 2) ? 0xff : 0x00, (i & 4) ? 0xff : 0x00, (i & 1) ? 0xff : 0x00));

	for (i = 0; i < 256; i++)
	{
		colortable_entry_set_value(machine->colortable, i*2, i & 7);
        	colortable_entry_set_value(machine->colortable, i*2+1, (i >> 4) & 7);
	}
}


SCREEN_UPDATE( mz700 )
{
	mz_state *state = screen->machine->driver_data<mz_state>();
	UINT8 *videoram = state->videoram;
	int offs;
	mz_state *mz = screen->machine->driver_data<mz_state>();

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	for(offs = 0; offs < 40*25; offs++)
	{
		int sx, sy, code, color;

		sy = (offs / 40) * 8;
		sx = (offs % 40) * 8;

		color = mz->colorram[offs];
		code = videoram[offs] | (color & 0x80) << 1;

		drawgfx_opaque(bitmap, cliprect, screen->machine->gfx[0], code, color, 0, 0, sx, sy);
	}

	return 0;
}


/***************************************************************************
    MZ-800
***************************************************************************/

VIDEO_START( mz800 )
{
	mz_state *mz = machine->driver_data<mz_state>();
	gfx_element_set_source(machine->gfx[0], mz->cgram);
}

SCREEN_UPDATE( mz800 )
{
	mz_state *state = screen->machine->driver_data<mz_state>();
	UINT8 *videoram = state->videoram;
	mz_state *mz = screen->machine->driver_data<mz_state>();

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	if (mz->mz700_mode)
		return SCREEN_UPDATE_CALL(mz700);
	else
	{
		if (mz->hires_mode)
		{

		}
		else
		{
			int x, y;
			UINT8 *start_addr = videoram;

			for (x = 0; x < 40; x++)
			{
				for (y = 0; y < 200; y++)
				{
					*BITMAP_ADDR16(bitmap, y, x * 8 + 0) = BIT(start_addr[x * 8 + y], 0);
					*BITMAP_ADDR16(bitmap, y, x * 8 + 1) = BIT(start_addr[x * 8 + y], 1);
					*BITMAP_ADDR16(bitmap, y, x * 8 + 2) = BIT(start_addr[x * 8 + y], 2);
					*BITMAP_ADDR16(bitmap, y, x * 8 + 3) = BIT(start_addr[x * 8 + y], 3);
					*BITMAP_ADDR16(bitmap, y, x * 8 + 4) = BIT(start_addr[x * 8 + y], 4);
					*BITMAP_ADDR16(bitmap, y, x * 8 + 5) = BIT(start_addr[x * 8 + y], 5);
					*BITMAP_ADDR16(bitmap, y, x * 8 + 6) = BIT(start_addr[x * 8 + y], 6);
					*BITMAP_ADDR16(bitmap, y, x * 8 + 7) = BIT(start_addr[x * 8 + y], 7);
				}
			}
		}

		return 0;
	}
}

/***************************************************************************
    CGRAM
***************************************************************************/

WRITE8_HANDLER( mz800_cgram_w )
{
	mz_state *mz = space->machine->driver_data<mz_state>();
	mz->cgram[offset] = data;

	gfx_element_mark_dirty(space->machine->gfx[0], offset/8);
}
