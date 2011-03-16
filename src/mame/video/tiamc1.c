/***************************************************************************

  TIA-MC1 video hardware

  driver by Eugene Sandulenko
  special thanks to Shiru for his standalone emulator and documentation

***************************************************************************/

#include "emu.h"
#include "includes/tiamc1.h"


WRITE8_HANDLER( tiamc1_videoram_w )
{
	tiamc1_state *state = space->machine->driver_data<tiamc1_state>();
	if(!(state->layers_ctrl & 2))
		state->charram[offset + 0x0000] = data;
	if(!(state->layers_ctrl & 4))
		state->charram[offset + 0x0800] = data;
	if(!(state->layers_ctrl & 8))
		state->charram[offset + 0x1000] = data;
	if(!(state->layers_ctrl & 16))
		state->charram[offset + 0x1800] = data;

	if ((state->layers_ctrl & (16|8|4|2)) != (16|8|4|2))
		gfx_element_mark_dirty(space->machine->gfx[0], (offset / 8) & 0xff);

	if(!(state->layers_ctrl & 1)) {
		state->tileram[offset] = data;
		if (offset < 1024)
			tilemap_mark_tile_dirty(state->bg_tilemap1, offset & 0x3ff);
		else
			tilemap_mark_tile_dirty(state->bg_tilemap2, offset & 0x3ff);
	}
}

WRITE8_HANDLER( tiamc1_bankswitch_w )
{
	tiamc1_state *state = space->machine->driver_data<tiamc1_state>();
	if ((data & 128) != (state->layers_ctrl & 128))
		tilemap_mark_all_tiles_dirty_all(space->machine);

	state->layers_ctrl = data;
}

WRITE8_HANDLER( tiamc1_sprite_x_w )
{
	tiamc1_state *state = space->machine->driver_data<tiamc1_state>();
	state->spriteram_x[offset] = data;
}

WRITE8_HANDLER( tiamc1_sprite_y_w )
{
	tiamc1_state *state = space->machine->driver_data<tiamc1_state>();
	state->spriteram_y[offset] = data;
}

WRITE8_HANDLER( tiamc1_sprite_a_w )
{
	tiamc1_state *state = space->machine->driver_data<tiamc1_state>();
	state->spriteram_a[offset] = data;
}

WRITE8_HANDLER( tiamc1_sprite_n_w )
{
	tiamc1_state *state = space->machine->driver_data<tiamc1_state>();
	state->spriteram_n[offset] = data;
}

WRITE8_HANDLER( tiamc1_bg_vshift_w )
{
	tiamc1_state *state = space->machine->driver_data<tiamc1_state>();
	state->bg_vshift = data;
}

WRITE8_HANDLER( tiamc1_bg_hshift_w )
{
	tiamc1_state *state = space->machine->driver_data<tiamc1_state>();
	state->bg_hshift = data;
}

WRITE8_HANDLER( tiamc1_palette_w )
{
	tiamc1_state *state = space->machine->driver_data<tiamc1_state>();
	palette_set_color(space->machine, offset, state->palette[data]);
}

PALETTE_INIT( tiamc1 )
{
	tiamc1_state *state = machine->driver_data<tiamc1_state>();
	// Voltage computed by Proteus
	//static const float g_v[8]={1.05f,0.87f,0.81f,0.62f,0.44f,0.25f,0.19f,0.00f};
	//static const float r_v[8]={1.37f,1.13f,1.00f,0.75f,0.63f,0.38f,0.25f,0.00f};
	//static const float b_v[4]={1.16f,0.75f,0.42f,0.00f};

	// Voltage adjusted by Shiru
	static const float g_v[8] = { 1.2071f,0.9971f,0.9259f,0.7159f,0.4912f,0.2812f,0.2100f,0.0000f};
	static const float r_v[8] = { 1.5937f,1.3125f,1.1562f,0.8750f,0.7187f,0.4375f,0.2812f,0.0000f};
	static const float b_v[4] = { 1.3523f,0.8750f,0.4773f,0.0000f};

	int col;
	int r, g, b, ir, ig, ib;
	float tcol;

	state->palette = auto_alloc_array(machine, rgb_t, 256);

	for (col = 0; col < 256; col++) {
		ir = (col >> 3) & 7;
		ig = col & 7;
		ib = (col >> 6) & 3;
		tcol = 255.0f * r_v[ir] / r_v[0];
		r = 255 - (((int)tcol) & 255);
		tcol = 255.0f * g_v[ig] / g_v[0];
		g = 255 - (((int)tcol) & 255);
		tcol = 255.0f * b_v[ib] / b_v[0];
		b = 255 - (((int)tcol) & 255);

		state->palette[col] = MAKE_RGB(r,g,b);
	}
}

static TILE_GET_INFO( get_bg1_tile_info )
{
	tiamc1_state *state = machine->driver_data<tiamc1_state>();
	SET_TILE_INFO(0, state->tileram[tile_index], 0, 0);
}

static TILE_GET_INFO( get_bg2_tile_info )
{
	tiamc1_state *state = machine->driver_data<tiamc1_state>();
	SET_TILE_INFO(0, state->tileram[tile_index + 1024], 0, 0);
}

VIDEO_START( tiamc1 )
{
	tiamc1_state *state = machine->driver_data<tiamc1_state>();
	UINT8 *video_ram;

	video_ram = auto_alloc_array_clear(machine, UINT8, 0x3040);

        state->charram = video_ram + 0x0800;     /* Ram is banked */
        state->tileram = video_ram + 0x0000;

	state->spriteram_y = video_ram + 0x3000;
	state->spriteram_x = video_ram + 0x3010;
	state->spriteram_n = video_ram + 0x3020;
	state->spriteram_a = video_ram + 0x3030;

	state_save_register_global_pointer(machine, video_ram, 0x3040);

	state->bg_tilemap1 = tilemap_create(machine, get_bg1_tile_info, tilemap_scan_rows,
		 8, 8, 32, 32);

	state->bg_tilemap2 = tilemap_create(machine, get_bg2_tile_info, tilemap_scan_rows,
		 8, 8, 32, 32);

	state->bg_vshift = 0;
	state->bg_hshift = 0;

	state_save_register_global(machine, state->layers_ctrl);
	state_save_register_global(machine, state->bg_vshift);
	state_save_register_global(machine, state->bg_hshift);

	gfx_element_set_source(machine->gfx[0], state->charram);
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	tiamc1_state *state = machine->driver_data<tiamc1_state>();
	int offs;

	for (offs = 0; offs < 16; offs++)
	{
		int flipx, flipy, sx, sy, spritecode;

		sx = state->spriteram_x[offs] ^ 0xff;
		sy = state->spriteram_y[offs] ^ 0xff;
		flipx = !(state->spriteram_a[offs] & 0x08);
		flipy = !(state->spriteram_a[offs] & 0x02);
		spritecode = state->spriteram_n[offs] ^ 0xff;

		if (!(state->spriteram_a[offs] & 0x01))
			drawgfx_transpen(bitmap, cliprect, machine->gfx[1],
				spritecode,
				0,
				flipx, flipy,
				sx, sy, 15);
	}
}

SCREEN_UPDATE( tiamc1 )
{
	tiamc1_state *state = screen->machine->driver_data<tiamc1_state>();
#if 0
	int i;

	for (i = 0; i < 32; i++)
	{
		tilemap_set_scrolly(state->bg_tilemap1, i, state->bg_vshift ^ 0xff);
		tilemap_set_scrolly(state->bg_tilemap2, i, state->bg_vshift ^ 0xff);
	}

	for (i = 0; i < 32; i++)
	{
		tilemap_set_scrollx(state->bg_tilemap1, i, state->bg_hshift ^ 0xff);
		tilemap_set_scrollx(state->bg_tilemap2, i, state->bg_hshift ^ 0xff);
	}
#endif

	if (state->layers_ctrl & 0x80)
		tilemap_draw(bitmap, cliprect, state->bg_tilemap2, 0, 0);
	else
		tilemap_draw(bitmap, cliprect, state->bg_tilemap1, 0, 0);


	draw_sprites(screen->machine, bitmap, cliprect);

	return 0;
}

