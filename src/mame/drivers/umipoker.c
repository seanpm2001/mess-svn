/***************************************************************************

    Umi de Poker (c) 1997 World Station Co.,LTD
    Slot Poker Saiyuki (c) 1998 World Station Co.,LTD

    driver by Angelo Salese

    TODO:
    - fix clocks;
    - inputs are bare-bones;

    TMP68HC000-16 + z80 + YM3812 + OKI6295

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/3812intf.h"
#include "sound/okim6295.h"
#include "machine/nvram.h"


class umipoker_state : public driver_device
{
public:
	umipoker_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_vram_0;
	UINT16 *m_vram_1;
	UINT16 *m_vram_2;
	UINT16 *m_vram_3;
	tilemap_t *m_tilemap_0;
	tilemap_t *m_tilemap_1;
	tilemap_t *m_tilemap_2;
	tilemap_t *m_tilemap_3;
	UINT8 *m_z80_wram;
	int m_umipoker_scrolly[4];
};

static TILE_GET_INFO( get_tile_info_0 )
{
	umipoker_state *state = machine.driver_data<umipoker_state>();
	int tile = state->m_vram_0[tile_index*2+0];
	int color = state->m_vram_0[tile_index*2+1] & 0x3f;

	SET_TILE_INFO(
			0,
			tile,
			color,
			0);
}

static TILE_GET_INFO( get_tile_info_1 )
{
	umipoker_state *state = machine.driver_data<umipoker_state>();
	int tile = state->m_vram_1[tile_index*2+0];
	int color = state->m_vram_1[tile_index*2+1] & 0x3f;

	SET_TILE_INFO(
			0,
			tile,
			color,
			0);
}

static TILE_GET_INFO( get_tile_info_2 )
{
	umipoker_state *state = machine.driver_data<umipoker_state>();
	int tile = state->m_vram_2[tile_index*2+0];
	int color = state->m_vram_2[tile_index*2+1] & 0x3f;

	SET_TILE_INFO(
			0,
			tile,
			color,
			0);
}

static TILE_GET_INFO( get_tile_info_3 )
{
	umipoker_state *state = machine.driver_data<umipoker_state>();
	int tile = state->m_vram_3[tile_index*2+0];
	int color = state->m_vram_3[tile_index*2+1] & 0x3f;

	SET_TILE_INFO(
			0,
			tile,
			color,
			0);
}

static VIDEO_START( umipoker )
{
	umipoker_state *state = machine.driver_data<umipoker_state>();

	state->m_tilemap_0 = tilemap_create(machine, get_tile_info_0,tilemap_scan_rows,8,8,64,32);
	state->m_tilemap_1 = tilemap_create(machine, get_tile_info_1,tilemap_scan_rows,8,8,64,32);
	state->m_tilemap_2 = tilemap_create(machine, get_tile_info_2,tilemap_scan_rows,8,8,64,32);
	state->m_tilemap_3 = tilemap_create(machine, get_tile_info_3,tilemap_scan_rows,8,8,64,32);

	tilemap_set_transparent_pen(state->m_tilemap_0,0);
	tilemap_set_transparent_pen(state->m_tilemap_1,0);
	tilemap_set_transparent_pen(state->m_tilemap_2,0);
	tilemap_set_transparent_pen(state->m_tilemap_3,0);

}

static SCREEN_UPDATE( umipoker )
{
	umipoker_state *state = screen->machine().driver_data<umipoker_state>();

	tilemap_set_scrolly(state->m_tilemap_0, 0, state->m_umipoker_scrolly[0]);
	tilemap_set_scrolly(state->m_tilemap_1, 0, state->m_umipoker_scrolly[1]);
	tilemap_set_scrolly(state->m_tilemap_2, 0, state->m_umipoker_scrolly[2]);
	tilemap_set_scrolly(state->m_tilemap_3, 0, state->m_umipoker_scrolly[3]);

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine()));

	tilemap_draw(bitmap,cliprect,state->m_tilemap_0,0,0);
	tilemap_draw(bitmap,cliprect,state->m_tilemap_1,0,0);
	tilemap_draw(bitmap,cliprect,state->m_tilemap_2,0,0);
	tilemap_draw(bitmap,cliprect,state->m_tilemap_3,0,0);

	return 0;
}

static READ8_HANDLER( z80_rom_readback_r )
{
	UINT8 *ROM = space->machine().region("audiocpu")->base();

	return ROM[offset];
}

static READ8_HANDLER( z80_shared_ram_r )
{
	umipoker_state *state = space->machine().driver_data<umipoker_state>();

	space->machine().scheduler().synchronize(); // force resync

	return state->m_z80_wram[offset];
}

static WRITE8_HANDLER( z80_shared_ram_w )
{
	umipoker_state *state = space->machine().driver_data<umipoker_state>();

	space->machine().scheduler().synchronize(); // force resync

	state->m_z80_wram[offset] = data;
}

static WRITE16_HANDLER( umipoker_irq_ack_w )
{
	cputag_set_input_line(space->machine(), "maincpu", 6, CLEAR_LINE);

	/* shouldn't happen */
	if(data)
		popmessage("%04x IRQ ACK, contact MAMEdev",data);
}

static WRITE16_HANDLER( umipoker_scrolly_0_w ) { umipoker_state *state = space->machine().driver_data<umipoker_state>(); COMBINE_DATA(&state->m_umipoker_scrolly[0]); }
static WRITE16_HANDLER( umipoker_scrolly_1_w ) { umipoker_state *state = space->machine().driver_data<umipoker_state>(); COMBINE_DATA(&state->m_umipoker_scrolly[1]); }
static WRITE16_HANDLER( umipoker_scrolly_2_w ) { umipoker_state *state = space->machine().driver_data<umipoker_state>(); COMBINE_DATA(&state->m_umipoker_scrolly[2]); }
static WRITE16_HANDLER( umipoker_scrolly_3_w ) { umipoker_state *state = space->machine().driver_data<umipoker_state>(); COMBINE_DATA(&state->m_umipoker_scrolly[3]); }

static WRITE16_HANDLER( umipoker_vram_0_w )
{
	umipoker_state *state = space->machine().driver_data<umipoker_state>();

	COMBINE_DATA(&state->m_vram_0[offset]);
	tilemap_mark_tile_dirty(state->m_tilemap_0,offset >> 1);
}

static WRITE16_HANDLER( umipoker_vram_1_w )
{
	umipoker_state *state = space->machine().driver_data<umipoker_state>();

	COMBINE_DATA(&state->m_vram_1[offset]);
	tilemap_mark_tile_dirty(state->m_tilemap_1,offset >> 1);
}


static WRITE16_HANDLER( umipoker_vram_2_w )
{
	umipoker_state *state = space->machine().driver_data<umipoker_state>();

	COMBINE_DATA(&state->m_vram_2[offset]);
	tilemap_mark_tile_dirty(state->m_tilemap_2,offset >> 1);
}

static WRITE16_HANDLER( umipoker_vram_3_w )
{
	umipoker_state *state = space->machine().driver_data<umipoker_state>();

	COMBINE_DATA(&state->m_vram_3[offset]);
	tilemap_mark_tile_dirty(state->m_tilemap_3,offset >> 1);
}


static ADDRESS_MAP_START( umipoker_map, AS_PROGRAM, 16 )
	ADDRESS_MAP_UNMAP_LOW
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x400000, 0x403fff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x600000, 0x6007ff) AM_RAM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_BASE_GENERIC(paletteram)	// Palette
	AM_RANGE(0x800000, 0x801fff) AM_RAM_WRITE(umipoker_vram_0_w) AM_BASE_MEMBER(umipoker_state, m_vram_0)
	AM_RANGE(0x802000, 0x803fff) AM_RAM_WRITE(umipoker_vram_1_w) AM_BASE_MEMBER(umipoker_state, m_vram_1)
	AM_RANGE(0x804000, 0x805fff) AM_RAM_WRITE(umipoker_vram_2_w) AM_BASE_MEMBER(umipoker_state, m_vram_2)
	AM_RANGE(0x806000, 0x807fff) AM_RAM_WRITE(umipoker_vram_3_w) AM_BASE_MEMBER(umipoker_state, m_vram_3)
	AM_RANGE(0xc00000, 0xc0ffff) AM_READ8(z80_rom_readback_r,0x00ff)
	AM_RANGE(0xc1f000, 0xc1ffff) AM_READWRITE8(z80_shared_ram_r,z80_shared_ram_w,0x00ff)
	AM_RANGE(0xe00000, 0xe00001) AM_READ_PORT("IN0")
	AM_RANGE(0xe00004, 0xe00005) AM_READ_PORT("IN1") // unused?
	AM_RANGE(0xe00008, 0xe00009) AM_READ_PORT("IN2")
	AM_RANGE(0xe0000c, 0xe0000d) AM_WRITENOP // lamps
	AM_RANGE(0xe00010, 0xe00011) AM_WRITENOP // coin counters
	AM_RANGE(0xe00014, 0xe00015) AM_READ_PORT("DSW0")
	AM_RANGE(0xe00018, 0xe00019) AM_READ_PORT("DSW1")
	AM_RANGE(0xe00020, 0xe00021) AM_WRITE(umipoker_scrolly_0_w)
	AM_RANGE(0xe00022, 0xe00023) AM_WRITE(umipoker_irq_ack_w)
	AM_RANGE(0xe00026, 0xe00027) AM_WRITE(umipoker_scrolly_2_w)
	AM_RANGE(0xe0002a, 0xe0002b) AM_WRITE(umipoker_scrolly_1_w)
	AM_RANGE(0xe0002c, 0xe0002d) AM_WRITENOP // unknown meaning, bit 0 goes from 0 -> 1 on IRQ service routine
	AM_RANGE(0xe0002e, 0xe0002f) AM_WRITE(umipoker_scrolly_3_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( umipoker_audio_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xf800, 0xffff) AM_READWRITE(z80_shared_ram_r,z80_shared_ram_w) AM_BASE_MEMBER(umipoker_state, m_z80_wram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( umipoker_audio_io_map, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_DEVREADWRITE_MODERN("oki", okim6295_device, read, write)
	AM_RANGE(0x10, 0x11) AM_DEVREADWRITE("ym", ym3812_r, ym3812_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( common )
	PORT_START("IN0")
	PORT_DIPNAME( 0x0001, 0x0001, "IN0" )
	PORT_DIPSETTING(    0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_GAMBLE_HIGH )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_GAMBLE_BET )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_GAMBLE_LOW )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_DIPNAME( 0x0100, 0x0100, "IN0" )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x0001, 0x0000, "IN1" )
	PORT_DIPSETTING(    0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0000, "IN0" )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x0001, 0x0000, "IN2" )
	PORT_DIPSETTING(    0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_GAMBLE_KEYIN )
	PORT_DIPNAME( 0x0100, 0x0000, "IN0" )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_SERVICE_NO_TOGGLE( 0x4000, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( umipoker )
	PORT_INCLUDE( common )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x0001, 0x0001, "DSW0" )
	PORT_DIPSETTING(    0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "IN0" )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0001, "DSW1" )
	PORT_DIPSETTING(    0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "IN0" )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0000, "Title Type" )
	PORT_DIPSETTING(    0x0000, "Umi de Poker" )
	PORT_DIPSETTING(    0x0800, "Marine Paradise" )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( saiyukip )
	PORT_INCLUDE( common )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x0001, 0x0001, "DSW0" )
	PORT_DIPSETTING(    0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "IN0" )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0001, "DSW1" )
	PORT_DIPSETTING(    0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "IN0" )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout layout_8x8x4 =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4)  },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static GFXDECODE_START( umipoker )
	GFXDECODE_ENTRY( "gfx1", 0, layout_8x8x4,     0, 0x40)
GFXDECODE_END

static MACHINE_START( umipoker )
{
	//umipoker_state *state = machine.driver_data<_umipoker_state>();

}

static MACHINE_RESET( umipoker )
{
	//umipoker_state *state = machine.driver_data<_umipoker_state>();
}

// TODO: clocks
static MACHINE_CONFIG_START( umipoker, umipoker_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M68000,16000000) // TMP68HC000-16
	MCFG_CPU_PROGRAM_MAP(umipoker_map)
	MCFG_CPU_VBLANK_INT("screen", irq6_line_assert)

	MCFG_CPU_ADD("audiocpu",Z80,4000000)
	MCFG_CPU_PROGRAM_MAP(umipoker_audio_map)
	MCFG_CPU_IO_MAP(umipoker_audio_io_map)
	MCFG_CPU_PERIODIC_INT(irq0_line_hold, 120)	// ? controls ym3812 music tempo

	MCFG_NVRAM_ADD_1FILL("nvram")

	MCFG_MACHINE_START(umipoker)
	MCFG_MACHINE_RESET(umipoker)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(8*8, 48*8-1, 2*8, 32*8-1)
	MCFG_SCREEN_UPDATE(umipoker)

	MCFG_GFXDECODE(umipoker)

	MCFG_PALETTE_LENGTH(0x400)

	MCFG_VIDEO_START(umipoker)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym", YM3812, 4000000 / 2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_OKIM6295_ADD("oki", 4000000 / 2, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( umipoker )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sp0.u61",      0x000000, 0x020000, CRC(866eaa02) SHA1(445afdfe010aad1102219a0dbd3a363a22294b4c) )
	ROM_LOAD16_BYTE( "sp1.u60",      0x000001, 0x020000, CRC(8db08696) SHA1(2854d511a8fd30b023e2a2a00b25413f88205d82) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sz.u8",        0x000000, 0x010000, CRC(d874ba1a) SHA1(13c06f3b67694d5d5194023c4f7b75aea8b57129) ) // second half 1-filled

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "sg0.u42",      0x000000, 0x020000, CRC(876f1f4f) SHA1(eca4c397be57812f2c34791736fee7c43925d927) )
	ROM_LOAD( "sg1.u41",      0x020000, 0x020000, CRC(7fcbfb17) SHA1(be2f308a8e8f0941c54125950702ddfbd8538733) )
	ROM_LOAD( "sg2.u40",      0x040000, 0x020000, CRC(eb31649b) SHA1(c0741d85537827e2396e81a1aa3005871dffad78) )
	ROM_LOAD( "sg3.u39",      0x060000, 0x020000, CRC(ebd5f96d) SHA1(968c107ee17f1e92ffc2835e13803347881862f1) )

	ROM_REGION( 0x40000, "oki", 0 )
    ROM_LOAD( "sm.u17",       0x000000, 0x040000, CRC(99503aed) SHA1(011404fad01b3ced708a94143908be3e1d0194d3) ) // first half 1-filled
    ROM_CONTINUE(             0x000000, 0x040000 )
ROM_END

ROM_START( saiyukip )
	ROM_REGION( 0x40000, "maincpu", 0 )
    ROM_LOAD16_BYTE( "slp0-spq.u61", 0x000000, 0x020000, CRC(7fc0f201) SHA1(969170d68278e212dd459744373ed9e704976e45) )
    ROM_LOAD16_BYTE( "slp1-spq.u60", 0x000001, 0x020000, CRC(c8e3547c) SHA1(18bb380a64ed36f45a377b86cbbac892efe879bb) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
    ROM_LOAD( "slz.u8",       0x000000, 0x010000, CRC(4f32ba1c) SHA1(8f1f8c0995bcd05d19120dd3b64b135908caf759) ) // second half 1-filled

	ROM_REGION( 0x80000, "gfx1", 0 )
    ROM_LOAD( "slg0.u42",     0x000000, 0x020000, CRC(49ba7ffd) SHA1(3bbb7656eafbd8c91c9054fca056c8fc3002ed13) )
    ROM_LOAD( "slg1.u41",     0x020000, 0x020000, CRC(59b5f399) SHA1(2b999cebcc53b3b8fd38e3034a12434d82b6fad3) )
    ROM_LOAD( "slg2.u40",     0x040000, 0x020000, CRC(fe6cd717) SHA1(65e59d88a30efd0cec642cda54e2bc38196f0231) )
    ROM_LOAD( "slg3.u39",     0x060000, 0x020000, CRC(e99b2906) SHA1(77884d2dae2e7f7cf27103aa8bbd0eaa39628993) )

	ROM_REGION( 0x40000, "oki", 0 )
    ROM_LOAD( "slm.u17",      0x000000, 0x040000, CRC(b50eb70b) SHA1(342fcb307844f4d0a02a85b2c61e73b5e8bacd44) ) // first half 1-filled
    ROM_CONTINUE(             0x000000, 0x040000 )
ROM_END


GAME( 1997, umipoker,  0,   umipoker,  umipoker,  0, ROT0, "World Station Co.,LTD", "Umi de Poker / Marine Paradise (Japan)", 0 ) // title screen is toggleable thru a dsw
GAME( 1998, saiyukip,  0,   umipoker,  saiyukip,  0, ROT0, "World Station Co.,LTD", "Slot Poker Saiyuki (Japan)", 0 )