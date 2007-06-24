/***************************************************************************

  Atari TIA video emulation

***************************************************************************/

#include "driver.h"
#include "tia.h"
#include "sound/tiaintf.h"
#include <math.h>

#define HMOVE_INACTIVE	-200
#define PLAYER_GFX_SLOTS	4

// Per player graphic
// - pixel number to start drawing from (0-7, from GRPx) / number of pixels drawn from GRPx
// - display position to start drawing
// - size to use
struct player_gfx {
	int	start_pixel[PLAYER_GFX_SLOTS];
	int start_drawing[PLAYER_GFX_SLOTS];
	int size[PLAYER_GFX_SLOTS];
};

struct player_gfx p0gfx;
struct player_gfx p1gfx;

static UINT32 frame_cycles;
static UINT32 paddle_cycles;

static int horzP0;
static int horzP1;
static int horzM0;
static int horzM1;
static int horzBL;
static int motclkP0;
static int motclkP1;
static int motclkM0;
static int motclkM1;
static int motclkBL;
static int startP0;
static int startP1;
static int startM0;
static int startM1;

static int current_bitmap;

static int prev_x;
static int prev_y;

static UINT8 VSYNC;
static UINT8 VBLANK;
static UINT8 COLUP0;
static UINT8 COLUP1;
static UINT8 COLUBK;
static UINT8 COLUPF;
static UINT8 CTRLPF;
static UINT8 GRP0;
static UINT8 GRP1;
static UINT8 REFP0;
static UINT8 REFP1;
static UINT8 HMP0;
static UINT8 HMP1;
static UINT8 HMM0;
static UINT8 HMM1;
static UINT8 HMBL;
static UINT8 VDELP0;
static UINT8 VDELP1;
static UINT8 VDELBL;
static UINT8 NUSIZ0;
static UINT8 NUSIZ1;
static UINT8 ENAM0;
static UINT8 ENAM1;
static UINT8 ENABL;
static UINT8 CXM0P;
static UINT8 CXM1P;
static UINT8 CXP0FB;
static UINT8 CXP1FB;
static UINT8 CXM0FB;
static UINT8 CXM1FB;
static UINT8 CXBLPF;
static UINT8 CXPPMM;
static UINT8 RESMP0;
static UINT8 RESMP1;
static UINT8 PF0;
static UINT8 PF1;
static UINT8 PF2;
static UINT8 INPT4;
static UINT8 INPT5;

static UINT8 prevGRP0;
static UINT8 prevGRP1;
static UINT8 prevENABL;

static int HMOVE_started;
static UINT8 HMP0_latch;
static UINT8 HMP1_latch;
static UINT8 HMM0_latch;
static UINT8 HMM1_latch;
static UINT8 HMBL_latch;
static UINT8 REFLECT;		/* Should playfield be reflected or not */
static UINT8 NUSIZx_changed;

static mame_bitmap *helper[3];


static const int nusiz[8][3] =
{
	{ 1, 1, 0 },
	{ 2, 1, 1 },
	{ 2, 1, 3 },
	{ 3, 1, 1 },
	{ 2, 1, 7 },
	{ 1, 2, 0 },
	{ 3, 1, 3 },
	{ 1, 4, 0 }
};

static read16_handler	tia_read_input_port;
static read8_handler	tia_get_databus;

static void extend_palette(running_machine *machine) {
	int	i,j;

	for( i = 0; i < 128; i ++ )
	{
		rgb_t	new_rgb = palette_get_color( machine, i );
		UINT8	new_r = RGB_RED( new_rgb );
		UINT8	new_g = RGB_GREEN( new_rgb );
		UINT8	new_b = RGB_BLUE( new_rgb );
		
		for ( j = 0; j < 128; j++ )
		{
			rgb_t	old_rgb = palette_get_color( machine, j );
			UINT8	old_r = RGB_RED( old_rgb );
			UINT8	old_g = RGB_GREEN( old_rgb );
			UINT8	old_b = RGB_BLUE( old_rgb );

			palette_set_color_rgb(machine, ( ( i + 1 ) << 7 ) | j,
				( new_r + old_r ) / 2,
				( new_g + old_g ) / 2,
				( new_b + old_b ) / 2 );
		}
	}
}

PALETTE_INIT( tia_NTSC )
{
	int i, j;

	static const double color[16][2] =
	{
		{  0.000,  0.000 },
		{  0.144, -0.189 },
		{  0.231, -0.081 },
		{  0.243,  0.032 },
		{  0.217,  0.121 },
		{  0.117,  0.216 },
		{  0.021,  0.233 },
		{ -0.066,  0.196 },
		{ -0.139,  0.134 },
		{ -0.182,  0.062 },
		{ -0.175, -0.022 },
		{ -0.136, -0.100 },
		{ -0.069, -0.150 },
		{  0.005, -0.159 },
		{  0.071, -0.125 },
		{  0.124, -0.089 }
	};

	for (i = 0; i < 16; i++)
	{
		double I = color[i][0];
		double Q = color[i][1];

		for (j = 0; j < 8; j++)
		{
			double Y = j / 7.0;

			double R = Y + 0.956 * I + 0.621 * Q;
			double G = Y - 0.272 * I - 0.647 * Q;
			double B = Y - 1.106 * I + 1.703 * Q;

			R = pow(R, 0.9) / pow(1, 0.9);
			G = pow(G, 0.9) / pow(1, 0.9);
			B = pow(B, 0.9) / pow(1, 0.9);

			if (R < 0) R = 0;
			if (G < 0) G = 0;
			if (B < 0) B = 0;

			if (R > 1) R = 1;
			if (G > 1) G = 1;
			if (B > 1) B = 1;

			palette_set_color_rgb(machine,8 * i + j,
				(UINT8) (255 * R + 0.5),
				(UINT8) (255 * G + 0.5),
				(UINT8) (255 * B + 0.5));
		}
	}
	extend_palette( machine );
}


PALETTE_INIT( tia_PAL )
{
	int i, j;

	static const double color[16][2] =
	{
		{  0.000,  0.000 },
		{  0.000,  0.000 },
		{ -0.222,  0.032 },
		{ -0.199, -0.027 },
		{ -0.173,  0.117 },
		{ -0.156, -0.197 },
		{ -0.094,  0.200 },
		{ -0.071, -0.229 },
		{  0.070,  0.222 },
		{  0.069, -0.204 },
		{  0.177,  0.134 },
		{  0.163, -0.130 },
		{  0.219,  0.053 },
		{  0.259, -0.042 },
		{  0.000,  0.000 },
		{  0.000,  0.000 }
	};

	for (i = 0; i < 16; i++)
	{
		double U = color[i][0];
		double V = color[i][1];

		for (j = 0; j < 8; j++)
		{
			double Y = j / 7.0;

			double R = Y + 1.403 * V;
			double G = Y - 0.344 * U - 0.714 * V;
			double B = Y + 1.770 * U;

			R = pow(R, 1.2) / pow(1, 1.2);
			G = pow(G, 1.2) / pow(1, 1.2);
			B = pow(B, 1.2) / pow(1, 1.2);

			if (R < 0) R = 0;
			if (G < 0) G = 0;
			if (B < 0) B = 0;

			if (R > 1) R = 1;
			if (G > 1) G = 1;
			if (B > 1) B = 1;

			palette_set_color_rgb(machine,8 * i + j,
				(UINT8) (255 * R + 0.5),
				(UINT8) (255 * G + 0.5),
				(UINT8) (255 * B + 0.5));
		}
	}
	extend_palette( machine );
}


VIDEO_START( tia )
{
	int cx = machine->screen[0].width;
	int cy = machine->screen[0].height;

	helper[0] = auto_bitmap_alloc(cx, cy, machine->screen[0].format);
	helper[1] = auto_bitmap_alloc(cx, cy, machine->screen[0].format);
	helper[2] = auto_bitmap_alloc(cx, cy, machine->screen[0].format);
}


VIDEO_UPDATE( tia )
{
	copybitmap(bitmap, helper[2], 0, 0, 0, 0,
		cliprect, TRANSPARENCY_NONE, 0);
	return 0;
}


static void draw_sprite_helper(UINT8* p, UINT8 *col, struct player_gfx *gfx,
	UINT8 GRP, UINT8 COLUP, UINT8 REFP)
{
	int i;
	int j;
	int k;

	if (REFP & 8)
	{
		GRP = BITSWAP8(GRP, 0, 1, 2, 3, 4, 5, 6, 7);
	}

	for (i = 0; i < PLAYER_GFX_SLOTS; i++)
	{
		int start_pos = gfx->start_drawing[i];
		for (j = gfx->start_pixel[i]; j < 8; j++)
		{
			for (k = 0; k < gfx->size[i]; k++)
			{
				if (GRP & (0x80 >> j))
				{
					p[start_pos % 160] = COLUP >> 1;
					col[start_pos % 160] = COLUP >> 1;
				}
				start_pos++;
			}
		}
	}
}


static void draw_missile_helper(UINT8* p, UINT8* col, int horz, int latch, int start,
	UINT8 RESMP, UINT8 ENAM, UINT8 NUSIZ, UINT8 COLUM)
{
	int num = nusiz[NUSIZ & 7][0];
	int skp = nusiz[NUSIZ & 7][2];

	int width = 1 << ((NUSIZ >> 4) & 3);

	int i;
	int j;

	for (i = 0; i < num; i++)
	{
		if ( i > 0 || start ) {
			for (j = 0; j < width; j++)
			{
				if ((ENAM & 2) && !(RESMP & 2))
				{
					if ( latch ) {
						switch ( horz % 4 ) {
						case 1:
							if ( horz < 156 ) {
								p[(horz + 1) % 160] = COLUM >> 1;
								col[(horz + 1) % 160] = COLUM >> 1;
							}
							p[horz % 160] = COLUM >> 1;
							col[horz % 160] = COLUM >> 1;
							break;
						case 2:
						case 3:
							p[horz % 160] = COLUM >> 1;
							col[horz % 160] = COLUM >> 1;
							break;
						}
					} else {
						p[horz % 160] = COLUM >> 1;
						col[horz % 160] = COLUM >> 1;
					}
				}

				horz++;
			}
		} else {
			horz+= width;
		}

		horz += 8 * (skp + 1) - width;
	}
}


static void draw_playfield_helper(UINT8* p, UINT8* col, int horz,
	UINT8 COLU, UINT8 REFPF)
{
	UINT32 PF =
		(BITSWAP8(PF0, 0, 1, 2, 3, 4, 5, 6, 7) << 0x10) |
		(BITSWAP8(PF1, 7, 6, 5, 4, 3, 2, 1, 0) << 0x08) |
		(BITSWAP8(PF2, 0, 1, 2, 3, 4, 5, 6, 7) << 0x00);

	int i;
	int j;

	if (REFPF)
	{
		UINT32 swap = 0;

		for (i = 0; i < 20; i++)
		{
			swap <<= 1;

			if (PF & 1)
			{
				swap |= 1;
			}

			PF >>= 1;
		}

		PF = swap;
	}

	for (i = 0; i < 20; i++)
	{
		for (j = 0; j < 4; j++)
		{
			if (PF & (0x80000 >> i))
			{
				p[horz] = COLU >> 1;
				col[horz] = COLU >> 1;
			}

			horz++;
		}
	}
}


static void draw_ball_helper(UINT8* p, UINT8* col, int horz, UINT8 ENAB)
{
	int width = 1 << ((CTRLPF >> 4) & 3);

	int i;

	for (i = 0; i < width; i++)
	{
		if (ENAB & 2)
		{
			p[horz % 160] = COLUPF >> 1;
			col[horz % 160] = COLUPF >> 1;
		}

		horz++;
	}
}


static void drawS0(UINT8* p, UINT8* col)
{
	draw_sprite_helper(p, col, &p0gfx, (VDELP0 & 1) ? prevGRP0 : GRP0, COLUP0, REFP0);
}


static void drawS1(UINT8* p, UINT8* col)
{
	draw_sprite_helper(p, col, &p1gfx, (VDELP1 & 1) ? prevGRP1 : GRP1, COLUP1, REFP1);
}


static void drawM0(UINT8* p, UINT8* col)
{
	draw_missile_helper(p, col, horzM0, HMM0_latch, startM0, RESMP0, ENAM0, NUSIZ0, COLUP0);
}


static void drawM1(UINT8* p, UINT8* col)
{
	draw_missile_helper(p, col, horzM1, HMM1_latch, startM1, RESMP1, ENAM1, NUSIZ1, COLUP1);
}


static void drawBL(UINT8* p, UINT8* col)
{
	draw_ball_helper(p, col, horzBL, (VDELBL & 1) ? prevENABL : ENABL);
}


static void drawPF(UINT8* p, UINT8 *col)
{
	draw_playfield_helper(p, col, 0,
		((CTRLPF & 6) == 2) ? COLUP0 : COLUPF, 0);

	draw_playfield_helper(p, col, 80,
		((CTRLPF & 6) == 2) ? COLUP1 : COLUPF, REFLECT);
}


static int collision_check(UINT8* p1, UINT8* p2, int x1, int x2)
{
	int i;

	for (i = x1; i < x2; i++)
	{
		if (p1[i] != 0xFF && p2[i] != 0xFF)
		{
			return 1;
		}
	}

	return 0;
}


INLINE int current_x(void)
{
	return 3 * ((activecpu_gettotalcycles() - frame_cycles) % 76) - 68;
}


INLINE int current_y(void)
{
	return (activecpu_gettotalcycles() - frame_cycles) / 76;
}


static void setup_pXgfx(void)
{
	int i;
	for ( i = 0; i < PLAYER_GFX_SLOTS; i++ )
	{
		if ( i < nusiz[NUSIZ0 & 7][0] && i >= ( startP0 ? 0 : 1 ) )
		{
			p0gfx.size[i] = nusiz[NUSIZ0 & 7][1];
			p0gfx.start_drawing[i] = ( horzP0 + (p0gfx.size[i] > 1 ? 1 : 0)
									 + i * 8 * ( nusiz[NUSIZ0 & 7][2] + p0gfx.size[i] ) ) % 160;
			p0gfx.start_pixel[i] = 0;
		}
		else
		{
			p0gfx.start_pixel[i] = 8;
		}
		if ( i < nusiz[NUSIZ1 & 7][0] && i >= ( startP1 ? 0 : 1 ) )
		{
			p1gfx.size[i] = nusiz[NUSIZ1 & 7][1];
			p1gfx.start_drawing[i] = ( horzP1 + (p1gfx.size[i] > 1 ? 1 : 0)
									 + i * 8 * ( nusiz[NUSIZ1 & 7][2] + p1gfx.size[i] ) ) % 160;
			p1gfx.start_pixel[i] = 0;
		}
		else
		{
			p1gfx.start_pixel[i] = 8;
		}
	}
}

static void update_bitmap(int next_x, int next_y)
{
	int x;
	int y;

	UINT8 linePF[160];
	UINT8 lineP0[160];
	UINT8 lineP1[160];
	UINT8 lineM0[160];
	UINT8 lineM1[160];
	UINT8 lineBL[160];

	UINT8 temp[160];

	if (prev_y >= next_y && prev_x >= next_x)
	{
		return;
	}

	memset(linePF, 0xFF, sizeof linePF);
	memset(lineP0, 0xFF, sizeof lineP0);
	memset(lineP1, 0xFF, sizeof lineP1);
	memset(lineM0, 0xFF, sizeof lineM0);
	memset(lineM1, 0xFF, sizeof lineM1);
	memset(lineBL, 0xFF, sizeof lineBL);

	if (VBLANK & 2)
	{
		memset(temp, 0, 160);
	}
	else
	{
		memset(temp, COLUBK >> 1, 160);

		if (CTRLPF & 4)
		{
			drawS1(temp, lineP1);
			drawM1(temp, lineM1);
			drawS0(temp, lineP0);
			drawM0(temp, lineM0);
			drawPF(temp, linePF);
			drawBL(temp, lineBL);
		}
		else
		{
			drawPF(temp, linePF);
			drawBL(temp, lineBL);
			drawS1(temp, lineP1);
			drawM1(temp, lineM1);
			drawS0(temp, lineP0);
			drawM0(temp, lineM0);
		}
	}

	for (y = prev_y; y <= next_y; y++)
	{
		UINT16* p;

		int x1 = prev_x;
		int x2 = next_x;
		int colx1;

		/* Check if we have crossed a line boundary */
		if ( y !=  prev_y ) {
			int redraw_line = 0;

			if ( HMOVE_started != HMOVE_INACTIVE ) {
				/* Apply pending motion clocks from a HMOVE initiated during the scanline */
				if ( HMOVE_started >= 97 && HMOVE_started < 157 ) {
					horzP0 -= motclkP0;
					horzP1 -= motclkP1;
					horzM0 -= motclkM0;
					horzM1 -= motclkM1;
					horzBL -= motclkBL;
					if (horzP0 < 0)
						horzP0 += 160;
					if (horzP1 < 0)
						horzP1 += 160;
					if (horzM0 < 0)
						horzM0 += 160;
					if (horzM1 < 0)
						horzM1 += 160;
					if (horzBL < 0)
						horzBL += 160;
				}
				HMOVE_started = HMOVE_INACTIVE;
				redraw_line = 1;
			}

			/* Redraw line if the playfield reflect bit was changed after the center of the screen */
			if ( REFLECT != ( CTRLPF & 0x01 ) ) {
				REFLECT = CTRLPF & 0x01;
				redraw_line = 1;
			}

			/* Redraw line if a RESPx or NUSIZx occured during the lastline */
			if ( ! startP0 || ! startP1 || ! startM0 || ! startM1) {
				startP0 = 1;
				startP1 = 1;
				startM0 = 1;
				startM1 = 1;

				redraw_line = 1;
			}

			/* Redraw line if HMP0 latch is still set */
			if ( HMP0_latch ) {
				horzP0 -= 17;
				if ( horzP0 < 0 )
					horzP0 += 160;
				redraw_line = 1;
			}

			/* Redraw line if HMP1 latch is still set */
			if ( HMP1_latch ) {
				horzP1 -= 17;
				if ( horzP1 < 0 )
					horzP1 += 160;
				redraw_line = 1;
			}

			/* Redraw line if HMM0 latch is still set */
			if ( HMM0_latch ) {
				horzM0 -= 17;
				if ( horzM0 < 0 )
					horzM0 += 160;
				redraw_line = 1;
			}

			/* Redraw line if HMM1 latch is still set */
			if ( HMM1_latch ) {
				horzM1 -= 17;
				if ( horzM1 < 0 )
					horzM1 += 160;
				redraw_line = 1;
			}

			/* Redraw line if HMBL latch is still set */
			if ( HMBL_latch ) {
				horzBL -= 17;
				if ( horzBL < 0 )
					horzBL += 160;
				redraw_line = 1;
			}

			/* Redraw line if NUSIZx data was changed */
			if ( NUSIZx_changed ) {
				NUSIZx_changed = 0;
				redraw_line = 1;
			}

			if ( redraw_line ) {
				if (VBLANK & 2)
				{
					memset(temp, 0, 160);
				}
				else
				{
					memset(lineP0, 0xFF, sizeof lineP0);
					memset(lineP1, 0xFF, sizeof lineP1);

					memset(temp, COLUBK >> 1, 160);

					setup_pXgfx();

					if (CTRLPF & 4)
					{
						drawS1(temp, lineP1);
						drawM1(temp, lineM1);
						drawS0(temp, lineP0);
						drawM0(temp, lineM0);
						drawPF(temp, linePF);
						drawBL(temp, lineBL);
					}
					else
					{
						drawPF(temp, linePF);
						drawBL(temp, lineBL);
						drawS1(temp, lineP1);
						drawM1(temp, lineM1);
						drawS0(temp, lineP0);
						drawM0(temp, lineM0);
					}
				}
			}
		}
		if (y != prev_y || x1 < 0)
		{
			x1 = 0;
		}
		if (y != next_y || x2 > 160)
		{
			x2 = 160;
		}

		/* Collision detection also takes place under the extended hblank area */
		colx1 = ( x1 == 8 && HMOVE_started != HMOVE_INACTIVE ) ? 0 : x1;

		if (collision_check(lineM0, lineP1, colx1, x2))
			CXM0P |= 0x80;
		if (collision_check(lineM0, lineP0, colx1, x2))
			CXM0P |= 0x40;
		if (collision_check(lineM1, lineP0, colx1, x2))
			CXM1P |= 0x80;
		if (collision_check(lineM1, lineP1, colx1, x2))
			CXM1P |= 0x40;
		if (collision_check(lineP0, linePF, colx1, x2))
			CXP0FB |= 0x80;
		if (collision_check(lineP0, lineBL, colx1, x2))
			CXP0FB |= 0x40;
		if (collision_check(lineP1, linePF, colx1, x2))
			CXP1FB |= 0x80;
		if (collision_check(lineP1, lineBL, colx1, x2))
			CXP1FB |= 0x40;
		if (collision_check(lineM0, linePF, colx1, x2))
			CXM0FB |= 0x80;
		if (collision_check(lineM0, lineBL, colx1, x2))
			CXM0FB |= 0x40;
		if (collision_check(lineM1, linePF, colx1, x2))
			CXM1FB |= 0x80;
		if (collision_check(lineM1, lineBL, colx1, x2))
			CXM1FB |= 0x40;
		if (collision_check(lineBL, linePF, colx1, x2))
			CXBLPF |= 0x80;
		if (collision_check(lineP0, lineP1, colx1, x2))
			CXPPMM |= 0x80;
		if (collision_check(lineM0, lineM1, colx1, x2))
			CXPPMM |= 0x40;

		p = BITMAP_ADDR16(helper[current_bitmap], y % (helper[current_bitmap]->height), 34);

		for (x = x1; x < x2; x++)
		{
			p[x] = temp[x];
		}

		if ( x2 == 160 && y % (helper[current_bitmap]->height) == (helper[current_bitmap]->height - 1) ) {
			// TODO: create combined screen in helper[2]
			int	t_y;
			for ( t_y = 0; t_y < helper[2]->height; t_y++ ) {
				UINT16*	l0 = BITMAP_ADDR16( helper[current_bitmap], t_y, 0 );
				UINT16*	l1 = BITMAP_ADDR16( helper[1 - current_bitmap], t_y, 0 );
				UINT16*	l2 = BITMAP_ADDR16( helper[2], t_y, 0 );
				int t_x;
				for( t_x = 0; t_x < helper[2]->width; t_x++ ) {
					if ( l0[t_x] != l1[t_x] ) {
						/* Combine both entries */
						l2[t_x] = ( ( l0[t_x] + 1 ) << 7 ) | l1[t_x];
					} else {
						l2[t_x] = l0[t_x];
					}
				}
			}
			current_bitmap ^= 1;
		}
	}

	prev_x = next_x;
	prev_y = next_y;
}


static WRITE8_HANDLER( WSYNC_w )
{
	int cycles = activecpu_gettotalcycles() - frame_cycles;

	if (cycles % 76)
	{
		activecpu_adjust_icount(cycles % 76 - 76);
	}
}


static WRITE8_HANDLER( VSYNC_w )
{
	if (data & 2)
	{
		if (!(VSYNC & 2))
		{
			if ( current_y() > 5 )
				update_bitmap(
					Machine->screen[0].width,
					Machine->screen[0].height);

			prev_y = 0;
			prev_x = 0;

			frame_cycles += 76 * current_y();
		}
	}

	VSYNC = data;
}


static WRITE8_HANDLER( VBLANK_w )
{
	if (data & 0x80)
	{
		paddle_cycles = activecpu_gettotalcycles();
	}
	if ( ! ( VBLANK & 0x40 ) ) {
		INPT4 = 0x80;
		INPT5 = 0x80;
	}
	VBLANK = data;
}


static WRITE8_HANDLER( CTRLPF_w )
{
	int curr_x = current_x();

	CTRLPF = data;
	if ( curr_x < 80 ) {
		REFLECT = CTRLPF & 1;
	}
}

static WRITE8_HANDLER( HMP0_w )
{
	int curr_x = current_x();

	data &= 0xF0;

	if ( data == HMP0 )
		return;

	/* Check if HMOVE cycles are still being applied */
	if ( HMOVE_started != HMOVE_INACTIVE && curr_x < MIN( HMOVE_started + 7 + motclkP0 * 4, 7 ) ) {
		int new_motclkP0 = ( data ^ 0x80 ) >> 4;

		/* Check if new horizontal move can still be applied normally */
		if ( new_motclkP0 > motclkP0 || curr_x <= MIN( HMOVE_started + 7 + new_motclkP0 * 4, 7 ) ) {
			horzP0 -= ( new_motclkP0 - motclkP0 );
			motclkP0 = new_motclkP0;
		} else {
			horzP0 -= ( 15 - motclkP0 );
			motclkP0 = 15;
			if ( data != 0x70 && data != 0x80 ) {
				HMP0_latch = 1;
			}
		}
		if ( horzP0 < 0 )
			horzP0 += 160;
		horzP0 %= 160;
		setup_pXgfx();
	}
	HMP0 = data;
}

static WRITE8_HANDLER( HMP1_w )
{
	int curr_x = current_x();

	data &= 0xF0;

	if ( data == HMP1 )
		return;

	/* Check if HMOVE cycles are still being applied */
	if ( HMOVE_started != HMOVE_INACTIVE && curr_x < MIN( HMOVE_started + 7 + motclkP1 * 4, 7 ) ) {
		int new_motclkP1 = ( data ^ 0x80 ) >> 4;

		/* Check if new horizontal move can still be applied normally */
		if ( new_motclkP1 > motclkP1 || curr_x <= MIN( HMOVE_started + 7 + new_motclkP1 * 4, 7 ) ) {
			horzP1 -= ( new_motclkP1 - motclkP1 );
			motclkP1 = new_motclkP1;
		} else {
			horzP1 -= ( 15 - motclkP1 );
			motclkP1 = 15;
			if ( data != 0x70 && data != 0x80 ) {
				HMP1_latch = 1;
			}
		}
		if ( horzP1 < 0 )
			horzP1 += 160;
		horzP1 %= 160;
		setup_pXgfx();
	}
	HMP1 = data;
}

static WRITE8_HANDLER( HMM0_w )
{
	int curr_x = current_x();

	data &= 0xF0;

	if ( data == HMM0 )
		return;

	/* Check if HMOVE cycles are still being applied */
	if ( HMOVE_started != HMOVE_INACTIVE && curr_x < MIN( HMOVE_started + 7 + motclkM0 * 4, 7 ) ) {
		int new_motclkM0 = ( data ^ 0x80 ) >> 4;

		/* Check if new horizontal move can still be applied normally */
		if ( new_motclkM0 > motclkM0 || curr_x <= MIN( HMOVE_started + 7 + new_motclkM0 * 4, 7 ) ) {
			horzM0 -= ( new_motclkM0 - motclkM0 );
			motclkM0 = new_motclkM0;
		} else {
			horzM0 -= ( 15 - motclkM0 );
			motclkM0 = 15;
			if ( data != 0x70 && data != 0x80 ) {
				HMM0_latch = 1;
			}
		}
		if ( horzM0 < 0 )
			horzM0 += 160;
		horzM0 %= 160;
	}
	HMM0 = data;
}

static WRITE8_HANDLER( HMM1_w )
{
	int curr_x = current_x();

	data &= 0xF0;

	if ( data == HMM1 )
		return;

	/* Check if HMOVE cycles are still being applied */
	if ( HMOVE_started != HMOVE_INACTIVE && curr_x < MIN( HMOVE_started + 7 + motclkM1 * 4, 7 ) ) {
		int new_motclkM1 = ( data ^ 0x80 ) >> 4;

		/* Check if new horizontal move can still be applied normally */
		if ( new_motclkM1 > motclkM1 || curr_x <= MIN( HMOVE_started + 7 + new_motclkM1 * 4, 7 ) ) {
			horzM1 -= ( new_motclkM1 - motclkM1 );
			motclkM1 = new_motclkM1;
		} else {
			horzM1 -= ( 15 - motclkM1 );
			motclkM1 = 15;
			if ( data != 0x70 && data != 0x80 ) {
				HMM1_latch = 1;
			}
		}
		if ( horzM1 < 0 )
			horzM1 += 160;
		horzM1 %= 160;
	}
	HMM1 = data;
}

static WRITE8_HANDLER( HMBL_w )
{
	int curr_x = current_x();

	data &= 0xF0;

	if ( data == HMBL )
		return;

	/* Check if HMOVE cycles are still being applied */
	if ( HMOVE_started != HMOVE_INACTIVE && curr_x < MIN( HMOVE_started + 7 + motclkBL * 4, 7 ) ) {
		int new_motclkBL = ( data ^ 0x80 ) >> 4;

		/* Check if new horizontal move can still be applied normally */
		if ( new_motclkBL > motclkBL || curr_x <= MIN( HMOVE_started + 7 + new_motclkBL * 4, 7 ) ) {
			horzBL -= ( new_motclkBL - motclkBL );
			motclkBL = new_motclkBL;
		} else {
			horzBL -= ( 15 - motclkBL );
			motclkBL = 15;
			if ( data != 0x70 && data != 0x80 ) {
				HMBL_latch = 1;
			}
		}
		if ( horzBL < 0 )
			horzBL += 160;
		horzBL %= 160;
	}
	HMBL = data;
}

static WRITE8_HANDLER( HMOVE_w )
{
	int curr_x = current_x();
	int curr_y = current_y();

	HMOVE_started = curr_x;

	/* Check if we have to undo some of the already applied cycles from an active graphics latch */
	if ( curr_x + 68 < 17 * 4 ) {
		int cycle_fix = 17 - ( ( curr_x + 68 + 7 ) / 4 );
		if ( HMP0_latch )
			horzP0 = ( horzP0 + cycle_fix ) % 160;
		if ( HMP1_latch )
			horzP1 = ( horzP1 + cycle_fix ) % 160;
		if ( HMM0_latch )
			horzM0 = ( horzM0 + cycle_fix ) % 160;
		if ( HMM1_latch )
			horzM1 = ( horzM1 + cycle_fix ) % 160;
		if ( HMBL_latch )
			horzBL = ( horzBL + cycle_fix ) % 160;
	}

	HMP0_latch = 0;
	HMP1_latch = 0;
	HMM0_latch = 0;
	HMM1_latch = 0;
	HMBL_latch = 0;

	/* Check if HMOVE activities can be ignored */
	if ( curr_x >= -5 && curr_x < 97 ) {
		motclkP0 = 0;
		motclkP1 = 0;
		motclkM0 = 0;
		motclkM1 = 0;
		motclkBL = 0;
		HMOVE_started = HMOVE_INACTIVE;
		return;
	}

	motclkP0 = ( HMP0 ^ 0x80 ) >> 4;
	motclkP1 = ( HMP1 ^ 0x80 ) >> 4;
	motclkM0 = ( HMM0 ^ 0x80 ) >> 4;
	motclkM1 = ( HMM1 ^ 0x80 ) >> 4;
	motclkBL = ( HMBL ^ 0x80 ) >> 4;

	/* Adjust number of graphics motion clocks for active display */
	if ( curr_x >= 97 && curr_x < 151 ) {
		int skip_motclks = ( 160 - HMOVE_started - 5 ) / 4;
		motclkP0 -= skip_motclks;
		motclkP1 -= skip_motclks;
		motclkM0 -= skip_motclks;
		motclkM1 -= skip_motclks;
		motclkBL -= skip_motclks;
		if ( motclkP0 < 0 )
			motclkP0 = 0;
		if ( motclkP1 < 0 )
			motclkP1 = 0;
		if ( motclkM0 < 0 )
			motclkM0 = 0;
		if ( motclkM1 < 0 )
			motclkM1 = 0;
		if ( motclkBL < 0 )
			motclkBL = 0;
	}

	if ( curr_x >= -56 && curr_x < -5 ) {
		int max_motclks = ( 7 - ( HMOVE_started + 5 ) ) / 4;
		if ( motclkP0 > max_motclks )
			motclkP0 = max_motclks;
		if ( motclkP1 > max_motclks )
			motclkP1 = max_motclks;
		if ( motclkM0 > max_motclks )
			motclkM0 = max_motclks;
		if ( motclkM1 > max_motclks )
			motclkM1 = max_motclks;
		if ( motclkBL > max_motclks )
			motclkBL = max_motclks;
	}

	/* Apply horizontal motion */
	if ( curr_x < -5 || curr_x >= 157 ) {
		horzP0 += 8 - motclkP0;
		horzP1 += 8 - motclkP1;
		horzM0 += 8 - motclkM0;
		horzM1 += 8 - motclkM1;
		horzBL += 8 - motclkBL;

		if (horzP0 < 0)
			horzP0 += 160;
		if (horzP1 < 0)
			horzP1 += 160;
		if (horzM0 < 0)
			horzM0 += 160;
		if (horzM1 < 0)
			horzM1 += 160;
		if (horzBL < 0)
			horzBL += 160;

		horzP0 %= 160;
		horzP1 %= 160;
		horzM0 %= 160;
		horzM1 %= 160;
		horzBL %= 160;

		/* When HMOVE is triggered on CPU cycle 75, the HBlank period on the
           next line is also extended. */
		if (curr_x >= 157)
		{
			curr_y += 1;
			update_bitmap( -8, curr_y );
		}
		else
		{
			setup_pXgfx();
		}
		if (curr_y < helper[current_bitmap]->height)
		{
			memset(BITMAP_ADDR16(helper[current_bitmap], curr_y, 34), 0, 16);
		}

		prev_x = 8;
	}
}


static WRITE8_HANDLER( RSYNC_w )
{
	/* this address is used in chip testing */
}


static WRITE8_HANDLER( NUSIZ0_w )
{
	int curr_x = current_x();

	/* Check if relevant bits have changed */
	if ( ( data & 7 ) != ( NUSIZ0 & 7 ) ) {
		int i;
		/* Check if we are (about to start) drawing a copy of the player 0 graphics */
		for ( i = 0; i < PLAYER_GFX_SLOTS; i++ ) {
			if ( p0gfx.start_pixel[i] < 8 ) {
				int min_x = p0gfx.start_drawing[i];
				int size = ( 8 - p0gfx.start_pixel[i] ) * p0gfx.size[i];
				if ( curr_x >= ( min_x - 5 ) % 160 && curr_x < ( min_x + size ) % 160 ) {
					if ( curr_x >= min_x % 160 || p0gfx.start_pixel[i] != 0 ) {
						/* This copy has started drawing */
						if ( p0gfx.size[i] == 1 && nusiz[data & 7][1] > 1 ) {
							int delay = 1 + ( ( p0gfx.start_pixel[i] + ( curr_x - p0gfx.start_drawing[i] ) ) & 1 );
							update_bitmap( curr_x + delay, current_y() );
							p0gfx.start_pixel[i] += ( curr_x + delay - p0gfx.start_drawing[i] );
							if ( p0gfx.start_pixel[i] > 8 )
								p0gfx.start_pixel[i] = 8;
							p0gfx.start_drawing[i] = curr_x + delay;
						} else if ( p0gfx.size[1] > 1 && nusiz[data & 7][1] == 1 ) {
							int delay = ( curr_x - p0gfx.start_drawing[i] ) & ( p0gfx.size[i] - 1 );
							if ( delay ) {
								delay = p0gfx.size[i] - delay;
							}
							update_bitmap( curr_x + delay, current_y() );
							p0gfx.start_pixel[i] += ( curr_x - p0gfx.start_drawing[i] ) / p0gfx.size[i];
							p0gfx.start_drawing[i] = curr_x + delay;
						} else {
							p0gfx.start_pixel[i] += ( curr_x - p0gfx.start_drawing[i] ) / p0gfx.size[i];
							p0gfx.start_drawing[i] = curr_x;
						}
						p0gfx.size[i] = nusiz[data & 7][1];
					} else {
						/* This copy was just about to start drawing (meltdown) */
						/* Adjust for 1 clock delay between zoomed and non-zoomed sprites */
						if ( p0gfx.size[i] == 1 && nusiz[data & 7][1] > 1 ) {
							p0gfx.start_drawing[i]++;
						} else if ( p0gfx.size[i] > 1 && nusiz[data & 7][1] == 1 ) {
							p0gfx.start_drawing[i]--;
						}
						p0gfx.size[i] = nusiz[data & 7][1];
					}
				} else {
					/* We are passed the copy or the copy still needs to be done. Mark
                       it as done/invalid, the data will be reset in the next loop. */
					p0gfx.start_pixel[i] = 8;
				}
			}
		}
		/* Apply NUSIZ updates to not yet drawn copies */
		for ( i = ( startP0 ? 0 : 1 ); i < nusiz[data & 7][0]; i++ ) {
			int j;
			/* Find an unused p0gfx entry */
			for ( j = 0; j < PLAYER_GFX_SLOTS; j++ ) {
				if ( p0gfx.start_pixel[j] == 8 )
					break;
			}
			p0gfx.size[j] = nusiz[data & 7][1];
			p0gfx.start_drawing[j] = ( horzP0 + (p0gfx.size[j] > 1 ? 1 : 0)
									+ i * 8 * ( nusiz[data & 7][2] + p0gfx.size[j] ) ) % 160;
			if ( curr_x < p0gfx.start_drawing[j] % 160 ) {
				p0gfx.start_pixel[j] = 0;
			}
		}
		NUSIZx_changed = 1;
	}
	NUSIZ0 = data;
}


static WRITE8_HANDLER( NUSIZ1_w )
{
	int curr_x = current_x();

	/* Check if relevant bits have changed */
	if ( ( data & 7 ) != ( NUSIZ1 & 7 ) ) {
		int i;
		/* Check if we are (about to start) drawing a copy of the player 1 graphics */
		for ( i = 0; i < PLAYER_GFX_SLOTS; i++ ) {
			if ( p1gfx.start_pixel[i] < 8 ) {
				int min_x = p1gfx.start_drawing[i];
				int size = ( 8 - p1gfx.start_pixel[i] ) * p1gfx.size[i];
				if ( curr_x >= ( min_x - 5 ) % 160 && curr_x < ( min_x + size ) % 160 ) {
					if ( curr_x >= min_x % 160 || p1gfx.start_pixel[i] != 0 ) {
						/* This copy has started drawing */
						if ( p1gfx.size[i] == 1 && nusiz[data & 7][1] > 1 ) {
							int delay = 1 + ( ( p0gfx.start_pixel[i] + ( curr_x - p0gfx.start_drawing[i] ) ) & 1 );
							update_bitmap( curr_x + delay, current_y() );
							p1gfx.start_pixel[i] += ( curr_x + delay - p1gfx.start_drawing[i] );
							if ( p1gfx.start_pixel[i] > 8 )
								p1gfx.start_pixel[i] = 8;
							p1gfx.start_drawing[i] = curr_x + delay;
						} else if ( p1gfx.size[1] > 1 && nusiz[data & 7][1] == 1 ) {
							int delay = ( curr_x - p1gfx.start_drawing[i] ) & ( p1gfx.size[i] - 1 );
							if ( delay ) {
								delay = p1gfx.size[i] - delay;
							}
							update_bitmap( curr_x + delay, current_y() );
							p1gfx.start_pixel[i] += ( curr_x - p1gfx.start_drawing[i] ) / p1gfx.size[i];
							p1gfx.start_drawing[i] = curr_x + delay;
						} else {
							p1gfx.start_pixel[i] += ( curr_x - p1gfx.start_drawing[i] ) / p1gfx.size[i];
							p1gfx.start_drawing[i] = curr_x;
						}
						p1gfx.size[i] = nusiz[data & 7][1];
					} else {
						/* This copy was just about to start drawing (meltdown) */
						/* Adjust for 1 clock delay between zoomed and non-zoomed sprites */
						if ( p1gfx.size[i] == 1 && nusiz[data & 7][1] > 1 ) {
							p1gfx.start_drawing[i]++;
						} else if ( p1gfx.size[i] > 1 && nusiz[data & 7][1] == 1 ) {
							p1gfx.start_drawing[i]--;
						}
						p1gfx.size[i] = nusiz[data & 7][1];
					}
				} else {
					/* We are passed the copy or the copy still needs to be done. Mark
                       it as done/invalid, the data will be reset in the next loop. */
					p1gfx.start_pixel[i] = 8;
				}
			}
		}
		/* Apply NUSIZ updates to not yet drawn copies */
		for ( i = ( startP1 ? 0 : 1 ); i < nusiz[data & 7][0]; i++ ) {
			int j;
			/* Find an unused p1gfx entry */
			for ( j = 0; j < PLAYER_GFX_SLOTS; j++ ) {
				if ( p1gfx.start_pixel[j] == 8 )
					break;
			}
			p1gfx.size[j] = nusiz[data & 7][1];
			p1gfx.start_drawing[j] = ( horzP1 + (p1gfx.size[j] > 1 ? 1 : 0)
									+ i * 8 * ( nusiz[data & 7][2] + p1gfx.size[j] ) ) % 160;
			if ( curr_x < p1gfx.start_drawing[j] % 160 ) {
				p1gfx.start_pixel[j] = 0;
			}
		}
		NUSIZx_changed = 1;
	}
	NUSIZ1 = data;
}


static WRITE8_HANDLER( HMCLR_w )
{
	HMP0_w( offset, 0 );
	HMP1_w( offset, 0 );
	HMM0_w( offset, 0 );
	HMM1_w( offset, 0 );
	HMBL_w( offset, 0 );
}


static WRITE8_HANDLER( CXCLR_w )
{
	CXM0P = 0;
	CXM1P = 0;
	CXP0FB = 0;
	CXP1FB = 0;
	CXM0FB = 0;
	CXM1FB = 0;
	CXBLPF = 0;
	CXPPMM = 0;
}


#define RESXX_APPLY_ACTIVE_HMOVE(HORZ,MOTION,MOTCLK)									\
	if ( curr_x < MIN( HMOVE_started + 7 + 16 * 4, 7 ) ) {								\
		int decrements_passed = ( curr_x - ( HMOVE_started + 5 ) ) / 4;					\
		HORZ += 8;																		\
		if ( ( MOTCLK - decrements_passed ) > 0 ) {										\
			HORZ -= ( MOTCLK - decrements_passed );										\
			if ( HORZ < 0 )																\
				HORZ += 160;															\
		}																				\
	}

static WRITE8_HANDLER( RESP0_w )
{
	int curr_x = current_x();
	int new_horzP0;

	if ( HMOVE_started != HMOVE_INACTIVE ) {
		new_horzP0 = ( curr_x < 7 ) ? 3 : ( ( curr_x + 5 ) % 160 );
		/* If HMOVE is active, adjust for remaining horizontal move clocks if any */
		RESXX_APPLY_ACTIVE_HMOVE( new_horzP0, HMP0, motclkP0 );
	} else {
		new_horzP0 = ( curr_x < -2 ) ? 3 : ( ( curr_x + 5 ) % 160 );
	}

	if ( new_horzP0 != horzP0 ) {
		int i;
		horzP0 = new_horzP0;
		startP0 = 0;
		/* Check if we are (about to start) drawing a copy of the player 0 graphics */
		for ( i = 0; i < PLAYER_GFX_SLOTS; i++ ) {
			if ( p0gfx.start_pixel[i] < 8 ) {
				int min_x = p0gfx.start_drawing[i];
				int size = ( 8 - p0gfx.start_pixel[i] ) * p0gfx.size[i];
				if ( curr_x >= ( min_x - 5 ) % 160 && curr_x < ( min_x + size ) % 160 ) {
					if ( curr_x >= min_x ) {
						/* This copy has started drawing */
						p0gfx.start_pixel[i] += ( curr_x - p0gfx.start_drawing[i] ) / p0gfx.size[i];
						p0gfx.start_drawing[i] = curr_x;
					} else {
						/* This copy is waiting to start drawing */
						p0gfx.start_drawing[i] = horzP0;
					}
				} else {
					/* We are passed the copy or the copy still needs to be done. Mark
                       it as done/invalid, the data will be reset in the next loop. */
					p0gfx.start_pixel[i] = 8;
				}
			}
		}
		/* Apply NUSIZ and position updates to not yet drawn copies */
		for ( i = 1; i < nusiz[NUSIZ0 & 7][0]; i++ ) {
			int j;
			/* Find an unused p0gfx entry */
			for ( j = 0; j < PLAYER_GFX_SLOTS; j++ ) {
				if ( p0gfx.start_pixel[j] == 8 )
					break;
			}
			p0gfx.size[j] = nusiz[NUSIZ0 & 7][1];
			p0gfx.start_drawing[j] = ( horzP0 + (p0gfx.size[j] > 1 ? 1 : 0)
									+ i * 8 * ( nusiz[NUSIZ0 & 7][2] + p0gfx.size[j] ) ) % 160;
			if ( curr_x < p0gfx.start_drawing[j] % 160 ) {
				p0gfx.start_pixel[j] = 0;
			}
		}
	}
}


static WRITE8_HANDLER( RESP1_w )
{
	int curr_x = current_x();
	int new_horzP1;

	if ( HMOVE_started != HMOVE_INACTIVE ) {
		new_horzP1 = ( curr_x < 7 ) ? 3 : ( ( curr_x + 5 ) % 160 );
		/* If HMOVE is active, adjust for remaining horizontal move clocks if any */
		RESXX_APPLY_ACTIVE_HMOVE( new_horzP1, HMP1, motclkP1 );
	} else {
		new_horzP1 = ( curr_x < -2 ) ? 3 : ( ( curr_x + 5 ) % 160 );
	}

	if ( new_horzP1 != horzP1 ) {
		int i;
		horzP1 = new_horzP1;
		startP1 = 0;
		/* Check if we are (about to start) drawing a copy of the player 1 graphics */
		for ( i = 0; i < PLAYER_GFX_SLOTS; i++ ) {
			if ( p1gfx.start_pixel[i] < 8 ) {
				int min_x = p1gfx.start_drawing[i];
				int size = ( 8 - p1gfx.start_pixel[i] ) * p1gfx.size[i];
				if ( curr_x >= ( min_x - 5 ) % 160 && curr_x < ( min_x + size ) % 160 ) {
					if ( curr_x >= min_x ) {
						/* This copy has started drawing */
						p1gfx.start_pixel[i] += ( curr_x - p1gfx.start_drawing[i] ) / p1gfx.size[i];
						p1gfx.start_drawing[i] = curr_x;
					} else {
						/* This copy is waiting to start drawing */
						p1gfx.start_drawing[i] = horzP1;
					}
				} else {
					/* We are passed the copy or the copy still needs to be done. Mark
                       it as done/invalid, the data will be reset in the next loop. */
					p1gfx.start_pixel[i] = 8;
				}
			}
		}
		/* Apply NUSIZ and position updates to not yet drawn copies */
		for ( i = 1; i < nusiz[NUSIZ1 & 7][0]; i++ ) {
			int j;
			/* Find an unused p1gfx entry */
			for ( j = 0; j < PLAYER_GFX_SLOTS; j++ ) {
				if ( p1gfx.start_pixel[j] == 8 )
					break;
			}
			p1gfx.size[j] = nusiz[NUSIZ1 & 7][1];
			p1gfx.start_drawing[j] = ( horzP1 + (p1gfx.size[j] > 1 ? 1 : 0)
									+ i * 8 * ( nusiz[NUSIZ1 & 7][2] + p1gfx.size[j] ) ) % 160;
			if ( curr_x < p1gfx.start_drawing[j] % 160 ) {
				p1gfx.start_pixel[j] = 0;
			}
		}
	}
}


static WRITE8_HANDLER( RESM0_w )
{
	int curr_x = current_x();
	int new_horzM0;

	if ( HMOVE_started != HMOVE_INACTIVE ) {
		new_horzM0 = ( curr_x < 7 ) ? 2 : ( ( curr_x + 4 ) % 160 );
		/* If HMOVE is active, adjust for remaining horizontal move clocks if any */
		RESXX_APPLY_ACTIVE_HMOVE( new_horzM0, HMM0, motclkM0 );
	} else {
		new_horzM0 = ( curr_x < 0 ) ? 2 : ( ( curr_x + 4 ) % 160 );
	}
	if ( new_horzM0 != horzM0 ) {
		horzM0 = new_horzM0;
		startM0 = 0;
	}
}


static WRITE8_HANDLER( RESM1_w )
{
	int curr_x = current_x();
	int new_horzM1;

	if ( HMOVE_started != HMOVE_INACTIVE ) {
		new_horzM1 = ( curr_x < 7 ) ? 2 : ( ( curr_x + 4 ) % 160 );
		/* If HMOVE is active, adjust for remaining horizontal move clocks if any */
		RESXX_APPLY_ACTIVE_HMOVE( new_horzM1, HMM1, motclkM1 );
	} else {
		new_horzM1 = ( curr_x < 0 ) ? 2 : ( ( curr_x + 4 ) % 160 );
	}
	if ( new_horzM1 != horzM1 ){
		horzM1 = new_horzM1;
		startM1 = 0;
	}
}


static WRITE8_HANDLER( RESBL_w )
{
	int curr_x = current_x();

	if ( HMOVE_started != HMOVE_INACTIVE ) {
		horzBL = ( curr_x < 7 ) ? 2 : ( ( curr_x + 4 ) % 160 );
		/* If HMOVE is active, adjust for remaining horizontal move clocks if any */
		RESXX_APPLY_ACTIVE_HMOVE( horzBL, HMBL, motclkBL );
	} else {
		horzBL = ( curr_x < 0 ) ? 2 : ( ( curr_x + 4 ) % 160 );
	}
}


static WRITE8_HANDLER( RESMP0_w )
{
	if (RESMP0 & 2)
	{
		horzM0 = (horzP0 + 4 * nusiz[NUSIZ0 & 7][1]) % 160;
	}

	RESMP0 = data;
}


static WRITE8_HANDLER( RESMP1_w )
{
	if (RESMP1 & 2)
	{
		horzM1 = (horzP1 + 4 * nusiz[NUSIZ1 & 7][1]) % 160;
	}

	RESMP1 = data;
}


static WRITE8_HANDLER( GRP0_w )
{
	prevGRP1 = GRP1;

	GRP0 = data;
}


static WRITE8_HANDLER( GRP1_w )
{
	prevGRP0 = GRP0;

	GRP1 = data;

	prevENABL = ENABL;
}


static READ8_HANDLER( INPT_r )
{
	UINT32 elapsed = activecpu_gettotalcycles() - paddle_cycles;
	int input = TIA_INPUT_PORT_ALWAYS_ON;

	if ( tia_read_input_port )
	{
		input = tia_read_input_port(offset & 3, 0xFFFF);
	}

	if ( input == TIA_INPUT_PORT_ALWAYS_ON )
		return 0x80;
	if ( input == TIA_INPUT_PORT_ALWAYS_OFF )
		return 0x00;

	return elapsed > 76 * input ? 0x80 : 0x00;
}


READ8_HANDLER( tia_r )
{
	 /* lower bits 0 - 5 seem to depend on the last byte on the
         data bus. If the driver supplied a routine to retrieve
         that we will call that, otherwise we will use the lower
         bit of the offset.
    */
	UINT8 data = offset & 0x3f;

	if ( tia_get_databus )
	{
		data = tia_get_databus(offset) & 0x3f;
	}

	if (!(offset & 0x8))
	{
		update_bitmap(current_x(), current_y());
	}

	switch (offset & 0xF)
	{
	case 0x0:
		return data | CXM0P;
	case 0x1:
		return data | CXM1P;
	case 0x2:
		return data | CXP0FB;
	case 0x3:
		return data | CXP1FB;
	case 0x4:
		return data | CXM0FB;
	case 0x5:
		return data | CXM1FB;
	case 0x6:
		return data | CXBLPF;
	case 0x7:
		return data | CXPPMM;
	case 0x8:
		return data | INPT_r(0);
	case 0x9:
		return data | INPT_r(1);
	case 0xA:
		return data | INPT_r(2);
	case 0xB:
		return data | INPT_r(3);
	case 0xC:
		{
			int	button = tia_read_input_port ? ( tia_read_input_port(4,0xFFFF) & 0x80 ) : 0x80;
			INPT4 = ( VBLANK & 0x40) ? ( INPT4 & button ) : button;
		}
		return data | INPT4;
	case 0xD:
		{
			int button = tia_read_input_port ? ( tia_read_input_port(5,0xFFFF) & 0x80 ) : 0x80;
			INPT5 = ( VBLANK & 0x40) ? ( INPT5 & button ) : button;
		}
		return data | INPT5;
	}

	return data;
}


WRITE8_HANDLER( tia_w )
{
	static const int delay[0x40] =
	{
		 0,	// VSYNC
		 0,	// VBLANK
		 0,	// WSYNC
		 0,	// RSYNC
		 0,	// NUSIZ0
		 0,	// NUSIZ1
		 0,	// COLUP0
		 0,	// COLUP1
		 0,	// COLUPF
		 0,	// COLUBK
		 0,	// CTRLPF
		 1,	// REFP0
		 1,	// REFP1
		 4,	// PF0
		 4,	// PF1
		 4,	// PF2
		 0,	// RESP0
		 0,	// RESP1
		 0,	// RESM0
		 0,	// RESM1
		 0,	// RESBL
		-1,	// AUDC0
		-1,	// AUDC1
		-1,	// AUDF0
		-1,	// AUDF1
		-1,	// AUDV0
		-1,	// AUDV1
		 1,	// GRP0
		 1,	// GRP1
		 1,	// ENAM0
		 1,	// ENAM1
		 1,	// ENABL
		 0,	// HMP0
		 0,	// HMP1
		 0,	// HMM0
		 0,	// HMM1
		 0,	// HMBL
		 0,	// VDELP0
		 0,	// VDELP1
		 0,	// VDELBL
		 0,	// RESMP0
		 0,	// RESMP1
		 3,	// HMOVE
		 0,	// HMCLR
		 0,	// CXCLR
	};

	int curr_x = current_x();
	int curr_y = current_y();

	offset &= 0x3F;

	if (offset >= 0x0D && offset <= 0x0F)
	{
		curr_x = ( curr_x + 1 ) & ~3;
	}

	if (delay[offset] >= 0)
	{
		update_bitmap(curr_x + delay[offset], curr_y);
	}

	switch (offset)
	{
	case 0x00:
		VSYNC_w(offset, data);
		break;
	case 0x01:
		VBLANK_w(offset, data);
		break;
	case 0x02:
		WSYNC_w(offset, data);
		break;
	case 0x03:
		RSYNC_w(offset, data);
		break;
	case 0x04:
		NUSIZ0_w(offset, data);
		break;
	case 0x05:
		NUSIZ1_w(offset, data);
		break;
	case 0x06:
		COLUP0 = data;
		break;
	case 0x07:
		COLUP1 = data;
		break;
	case 0x08:
		COLUPF = data;
		break;
	case 0x09:
		COLUBK = data;
		break;
	case 0x0A:
		CTRLPF_w(offset, data);
		break;
	case 0x0B:
		REFP0 = data;
		break;
	case 0x0C:
		REFP1 = data;
		break;
	case 0x0D:
		PF0 = data;
		break;
	case 0x0E:
		PF1 = data;
		break;
	case 0x0F:
		PF2 = data;
		break;
	case 0x10:
		RESP0_w(offset, data);
		break;
	case 0x11:
		RESP1_w(offset, data);
		break;
	case 0x12:
		RESM0_w(offset, data);
		break;
	case 0x13:
		RESM1_w(offset, data);
		break;
	case 0x14:
		RESBL_w(offset, data);
		break;

	case 0x15: /* AUDC0 */
	case 0x16: /* AUDC1 */
	case 0x17: /* AUDF0 */
	case 0x18: /* AUDF1 */
	case 0x19: /* AUDV0 */
	case 0x1A: /* AUDV1 */
		tia_sound_w(offset, data);
		break;

	case 0x1B:
		GRP0_w(offset, data);
		break;
	case 0x1C:
		GRP1_w(offset, data);
		break;
	case 0x1D:
		ENAM0 = data;
		break;
	case 0x1E:
		ENAM1 = data;
		break;
	case 0x1F:
		ENABL = data;
		break;
	case 0x20:
		HMP0_w(offset, data);
		break;
	case 0x21:
		HMP1_w(offset, data);
		break;
	case 0x22:
		HMM0_w(offset, data);
		break;
	case 0x23:
		HMM1_w(offset, data);
		break;
	case 0x24:
		HMBL_w(offset, data);
		break;
	case 0x25:
		VDELP0 = data;
		break;
	case 0x26:
		VDELP1 = data;
		break;
	case 0x27:
		VDELBL = data;
		break;
	case 0x28:
		RESMP0_w(offset, data);
		break;
	case 0x29:
		RESMP1_w(offset, data);
		break;
	case 0x2A:
		HMOVE_w(offset, data);
		break;
	case 0x2B:
		HMCLR_w(offset, data);
		break;
	case 0x2C:
		CXCLR_w(offset, 0);
		break;
	}
}


static void tia_reset(running_machine *machine)
{
	frame_cycles = 0;
}



void tia_init(const struct tia_interface* ti)
{
	int	i;

	assert_always(mame_get_phase(Machine) == MAME_PHASE_INIT, "Can only call tia_init at init time!");

	INPT4 = 0x80;
	INPT5 = 0x80;

	HMP0 = 0;
	HMP1 = 0;
	HMM0 = 0;
	HMM1 = 0;
	HMBL = 0;

	startP0 = 1;
	startP1 = 1;

	HMP0_latch = 0;
	HMP1_latch = 0;
	HMM0_latch = 0;
	HMM1_latch = 0;
	HMBL_latch = 0;

	startM0 = 1;
	startM1 = 1;

	REFLECT = 0;

	prev_x = 0;
	prev_y = 0;

	HMOVE_started = HMOVE_INACTIVE;

	motclkP0 = 0;
	motclkP1 = 0;
	motclkM0 = 0;
	motclkM1 = 0;
	motclkBL = 0;

	for( i = 0; i < PLAYER_GFX_SLOTS; i++ )
	{
		p0gfx.start_pixel[i] = 8;
		p0gfx.size[i] = 1;
		p1gfx.start_pixel[i] = 8;
		p1gfx.size[i] = 1;
	}

	NUSIZx_changed = 0;

	if ( ti ) {
		tia_read_input_port = ti->read_input_port;
		tia_get_databus = ti->databus_contents;
	} else {
		tia_read_input_port = NULL;
		tia_get_databus = NULL;
	}

	frame_cycles = 0;
	add_reset_callback(Machine, tia_reset);
}

