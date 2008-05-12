/*********************************************************************

    drivers/oric.c

    Systems supported by this driver:

    Oric 1,
    Oric Atmos,
    Oric Telestrat,
    Pravetz 8D

    Pravetz is a Bulgarian copy of the Oric Atmos and uses
    Apple 2 disc drives for storage.

    This driver originally by Paul Cook, rewritten by Kevin Thacker.

*********************************************************************/

#include "driver.h"
#include "includes/oric.h"
#include "machine/centroni.h"
#include "devices/printer.h"
#include "devices/mflopimg.h"
#include "devices/cassette.h"
#include "formats/ap2_dsk.h"
#include "formats/oric_tap.h"
#include "sound/ay8910.h"
#include "machine/6522via.h"
#include "machine/6551.h"

#include "includes/apple2.h"

/*
    Explaination of memory regions:

    I have split the memory region &c000-&ffff in this way because:

    All roms (os, microdisc and jasmin) use the 6502 IRQ vectors at the end
    of memory &fff8-&ffff, but they are different sizes. The os is 16k, microdisc
    is 8k and jasmin is 2k.

    There is also 16k of ram at &c000-&ffff which is normally masked
    by the os rom, but when the microdisc or jasmin interfaces are used,
    this ram can be accessed. For the microdisc and jasmin, the ram not
    covered by the roms for these interfaces, can be accessed
    if it is enabled.

    SMH_BANK1,SMH_BANK2 and SMH_BANK3 are used for a 16k rom.
    SMH_BANK2 and SMH_BANK3 are used for a 8k rom.
    SMH_BANK3 is used for a 2k rom.

    0x0300-0x03ff is I/O access. It is not defined below because the
    memory is setup dynamically depending on hardware that has been selected (microdisc, jasmin, apple2) etc.

*/


static ADDRESS_MAP_START(oric_mem, ADDRESS_SPACE_PROGRAM, 8)
    AM_RANGE( 0x0000, 0xbfff) AM_RAM AM_BASE( &oric_ram )
    AM_RANGE( 0xc000, 0xdfff) AM_READWRITE( SMH_BANK1, SMH_BANK5 )
	AM_RANGE( 0xe000, 0xf7ff) AM_READWRITE( SMH_BANK2, SMH_BANK6 )
	AM_RANGE( 0xf800, 0xffff) AM_READWRITE( SMH_BANK3, SMH_BANK7 )
ADDRESS_MAP_END

/*
The telestrat has the memory regions split into 16k blocks.
Memory region &c000-&ffff can be ram or rom. */
static ADDRESS_MAP_START(telestrat_mem, ADDRESS_SPACE_PROGRAM, 8)
	AM_RANGE( 0x0000, 0x02ff) AM_RAM
	AM_RANGE( 0x0300, 0x030f) AM_READWRITE( via_0_r, via_0_w )
	AM_RANGE( 0x0310, 0x031b) AM_READWRITE( oric_microdisc_r, oric_microdisc_w )
	AM_RANGE( 0x031c, 0x031f) AM_READWRITE( acia_6551_r, acia_6551_w )
	AM_RANGE( 0x0320, 0x032f) AM_READWRITE( via_1_r, via_1_w )
	AM_RANGE( 0x0400, 0xbfff) AM_RAM
	AM_RANGE( 0xc000, 0xffff) AM_READWRITE( SMH_BANK1, SMH_BANK2 )
ADDRESS_MAP_END


#define INPUT_PORT_ORIC \
	PORT_START /* IN0 */ \
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) \
 \
    PORT_START /* KEY ROW 0 */ \
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3 #") PORT_CODE(KEYCODE_3) \
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("x X") PORT_CODE(KEYCODE_X) \
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1 !") PORT_CODE(KEYCODE_1) \
	PORT_BIT (0x10, 0x00, IPT_UNUSED) \
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("v V") PORT_CODE(KEYCODE_V) \
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5 %") PORT_CODE(KEYCODE_5) \
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("n N") PORT_CODE(KEYCODE_N) \
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7 ^") PORT_CODE(KEYCODE_7) \
 \
	PORT_START /* KEY ROW 1 */ \
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("d D") PORT_CODE(KEYCODE_D) \
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("q Q") PORT_CODE(KEYCODE_Q) \
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) \
	PORT_BIT (0x10, 0x00, IPT_UNUSED) \
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("f F") PORT_CODE(KEYCODE_F) \
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("r R") PORT_CODE(KEYCODE_R) \
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("t T") PORT_CODE(KEYCODE_T) \
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("j J") PORT_CODE(KEYCODE_J) \
 \
	PORT_START /* KEY ROW 2 */ \
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("c C") PORT_CODE(KEYCODE_C) \
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2 @") PORT_CODE(KEYCODE_2) \
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("z Z") PORT_CODE(KEYCODE_Z) \
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) \
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4 $") PORT_CODE(KEYCODE_4) \
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("b B") PORT_CODE(KEYCODE_B) \
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6 &") PORT_CODE(KEYCODE_6) \
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("m M") PORT_CODE(KEYCODE_M) \
 \
	PORT_START /* KEY ROW 3 */ \
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("' \"") PORT_CODE(KEYCODE_QUOTE) \
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("\\ |") PORT_CODE(KEYCODE_BACKSLASH) \
	PORT_BIT (0x20, 0x00, IPT_UNUSED) \
	PORT_BIT (0x10, 0x00, IPT_UNUSED) \
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("- \xC2\xA3") PORT_CODE(KEYCODE_MINUS) \
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("; :") PORT_CODE(KEYCODE_COLON) \
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9 (") PORT_CODE(KEYCODE_9) \
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("k K") PORT_CODE(KEYCODE_K) \
 \
	PORT_START /* KEY ROW 4 */ \
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RIGHT") PORT_CODE(KEYCODE_RIGHT) \
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DOWN") PORT_CODE(KEYCODE_DOWN) \
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LEFT") PORT_CODE(KEYCODE_LEFT) \
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LSHIFT") PORT_CODE(KEYCODE_LSHIFT) \
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("UP") PORT_CODE(KEYCODE_UP) \
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP) \
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA) \
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) \
 \
	PORT_START /* KEY ROW 5 */ \
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("[ {") PORT_CODE(KEYCODE_OPENBRACE) \
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("] }") PORT_CODE(KEYCODE_CLOSEBRACE) \
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DEL") PORT_CODE(KEYCODE_BACKSPACE) \
	PORT_BIT (0x10, 0x00, IPT_UNUSED) \
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("p P") PORT_CODE(KEYCODE_P) \
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("o O") PORT_CODE(KEYCODE_O) \
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("i I") PORT_CODE(KEYCODE_I) \
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("u U") PORT_CODE(KEYCODE_U) \
 \
	PORT_START /* KEY ROW 6 */ \
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("w W") PORT_CODE(KEYCODE_W) \
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("s S") PORT_CODE(KEYCODE_S) \
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("a A") PORT_CODE(KEYCODE_A) \
	PORT_BIT (0x10, 0x00, IPT_UNUSED) \
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("e E") PORT_CODE(KEYCODE_E) \
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("g G") PORT_CODE(KEYCODE_G) \
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("h H") PORT_CODE(KEYCODE_H) \
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("y Y") PORT_CODE(KEYCODE_Y) \
 \
	PORT_START /* KEY ROW 7 */ \
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("= +") PORT_CODE(KEYCODE_EQUALS) \
	PORT_BIT (0x40, 0x00, IPT_UNUSED) \
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) \
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RSHIFT") PORT_CODE(KEYCODE_RSHIFT) \
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH) \
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0 )") PORT_CODE(KEYCODE_0) \
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("l L") PORT_CODE(KEYCODE_L) \
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8 *") PORT_CODE(KEYCODE_8)

#define INPUT_PORT_ORICA \
	PORT_START /* IN0 */ \
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) \
 \
    PORT_START /* KEY ROW 0 */ \
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3 #") PORT_CODE(KEYCODE_3) \
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("x X") PORT_CODE(KEYCODE_X) \
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1 !") PORT_CODE(KEYCODE_1) \
	PORT_BIT (0x10, 0x00, IPT_UNUSED) \
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("v V") PORT_CODE(KEYCODE_V) \
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5 %") PORT_CODE(KEYCODE_5) \
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("n N") PORT_CODE(KEYCODE_N) \
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7 ^") PORT_CODE(KEYCODE_7) \
 \
	PORT_START /* KEY ROW 1 */ \
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("d D") PORT_CODE(KEYCODE_D) \
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("q Q") PORT_CODE(KEYCODE_Q) \
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) \
	PORT_BIT (0x10, 0x00, IPT_UNUSED) \
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("f F") PORT_CODE(KEYCODE_F) \
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("r R") PORT_CODE(KEYCODE_R) \
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("t T") PORT_CODE(KEYCODE_T) \
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("j J") PORT_CODE(KEYCODE_J) \
 \
	PORT_START /* KEY ROW 2 */ \
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("c C") PORT_CODE(KEYCODE_C) \
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2 @") PORT_CODE(KEYCODE_2) \
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("z Z") PORT_CODE(KEYCODE_Z) \
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) \
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4 $") PORT_CODE(KEYCODE_4) \
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("b B") PORT_CODE(KEYCODE_B) \
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6 &") PORT_CODE(KEYCODE_6) \
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("m M") PORT_CODE(KEYCODE_M) \
 \
	PORT_START /* KEY ROW 3 */ \
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("' \"") PORT_CODE(KEYCODE_QUOTE) \
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("\\ |") PORT_CODE(KEYCODE_BACKSLASH) \
	PORT_BIT (0x20, 0x00, IPT_UNUSED) \
	PORT_BIT (0x10, 0x00, IPT_UNUSED) \
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("- \xC2\xA3") PORT_CODE(KEYCODE_MINUS) \
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("; :") PORT_CODE(KEYCODE_COLON) \
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9 (") PORT_CODE(KEYCODE_9) \
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("k K") PORT_CODE(KEYCODE_K) \
 \
	PORT_START /* KEY ROW 4 */ \
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RIGHT") PORT_CODE(KEYCODE_RIGHT) \
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DOWN") PORT_CODE(KEYCODE_DOWN) \
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LEFT") PORT_CODE(KEYCODE_LEFT) \
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LSHIFT") PORT_CODE(KEYCODE_LSHIFT) \
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("UP") PORT_CODE(KEYCODE_UP) \
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA) \
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP) \
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) \
 \
	PORT_START /* KEY ROW 5 */ \
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("[ {") PORT_CODE(KEYCODE_OPENBRACE) \
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("] }") PORT_CODE(KEYCODE_CLOSEBRACE) \
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DEL") PORT_CODE(KEYCODE_BACKSPACE) \
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("FUNCT") PORT_CODE(KEYCODE_INSERT) \
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("p P") PORT_CODE(KEYCODE_P) \
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("o O") PORT_CODE(KEYCODE_O) \
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("i I") PORT_CODE(KEYCODE_I) \
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("u U") PORT_CODE(KEYCODE_U) \
 \
	PORT_START /* KEY ROW 6 */ \
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("w W") PORT_CODE(KEYCODE_W) \
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("s S") PORT_CODE(KEYCODE_S) \
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("a A") PORT_CODE(KEYCODE_A) \
	PORT_BIT (0x10, 0x00, IPT_UNUSED) \
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("e E") PORT_CODE(KEYCODE_E) \
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("g G") PORT_CODE(KEYCODE_G) \
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("h H") PORT_CODE(KEYCODE_H) \
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("y Y") PORT_CODE(KEYCODE_Y) \
 \
	PORT_START /* KEY ROW 7 */ \
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("= +") PORT_CODE(KEYCODE_EQUALS) \
	PORT_BIT (0x40, 0x00, IPT_UNUSED) \
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) \
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RSHIFT") PORT_CODE(KEYCODE_RSHIFT) \
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH) \
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0 )") PORT_CODE(KEYCODE_0) \
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("l L") PORT_CODE(KEYCODE_L) \
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8 *") PORT_CODE(KEYCODE_8)

#define INPUT_PORT_PRAV8D \
	PORT_START /* IN0 */ \
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) \
 \
    PORT_START /* KEY ROW 0 */ \
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3 #") PORT_CODE(KEYCODE_3) \
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X (MYAGKEEY ZNAHK)") PORT_CODE(KEYCODE_X) \
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1 !") PORT_CODE(KEYCODE_1) \
        PORT_BIT (0x10, 0x00, IPT_UNUSED) \
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("V (ZHEH)") PORT_CODE(KEYCODE_V) \
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5 %") PORT_CODE(KEYCODE_5) \
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N (EHN)") PORT_CODE(KEYCODE_N) \
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7 '") PORT_CODE(KEYCODE_7) \
 \
	PORT_START /* KEY ROW 1 */ \
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D (DEH)") PORT_CODE(KEYCODE_D) \
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Q (YAH)") PORT_CODE(KEYCODE_Q) \
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("OCB") PORT_CODE(KEYCODE_ESC) \
        PORT_BIT (0x10, 0x00, IPT_UNUSED) \
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F (EHF)") PORT_CODE(KEYCODE_F) \
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R (ERH)") PORT_CODE(KEYCODE_R) \
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("T (TEH)") PORT_CODE(KEYCODE_T) \
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("J (EE KRATKOYEH)") PORT_CODE(KEYCODE_J) \
 \
	PORT_START /* KEY ROW 2 */ \
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C (TSEH)") PORT_CODE(KEYCODE_C) \
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2 \"") PORT_CODE(KEYCODE_2) \
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z (ZEH)") PORT_CODE(KEYCODE_Z) \
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MK") PORT_CODE(KEYCODE_LCONTROL) \
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4 $") PORT_CODE(KEYCODE_4) \
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B (BEH)") PORT_CODE(KEYCODE_B) \
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6 &") PORT_CODE(KEYCODE_6) \
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M (EHM)") PORT_CODE(KEYCODE_M) \
 \
	PORT_START /* KEY ROW 3 */ \
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("] (SCHYAH)") PORT_CODE(KEYCODE_CLOSEBRACE) \
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("; +") PORT_CODE(KEYCODE_EQUALS) \
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C/L") PORT_CODE(KEYCODE_INSERT) \
        PORT_BIT (0x10, 0x00, IPT_UNUSED) \
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(": *") PORT_CODE(KEYCODE_COLON) \
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("[ (SHAH)") PORT_CODE(KEYCODE_OPENBRACE) \
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9 )") PORT_CODE(KEYCODE_9) \
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("K (KAH)") PORT_CODE(KEYCODE_K) \
 \
	PORT_START /* KEY ROW 4 */ \
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RIGHT") PORT_CODE(KEYCODE_RIGHT) \
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DOWN") PORT_CODE(KEYCODE_DOWN) \
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LEFT") PORT_CODE(KEYCODE_LEFT) \
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LSHIFT") PORT_CODE(KEYCODE_LSHIFT) \
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("UP") PORT_CODE(KEYCODE_UP) \
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA) \
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP) \
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) \
 \
	PORT_START /* KEY ROW 5 */ \
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("@ (YOO)") PORT_CODE(KEYCODE_TILDE) \
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("\\ (EH)") PORT_CODE(KEYCODE_BACKSLASH) \
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DEL") PORT_CODE(KEYCODE_BACKSPACE) \
        PORT_BIT (0x10, 0x00, IPT_UNUSED) \
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P (PEH)") PORT_CODE(KEYCODE_P) \
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("O (OH)") PORT_CODE(KEYCODE_O) \
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("I (EE)") PORT_CODE(KEYCODE_I) \
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("U (OO)") PORT_CODE(KEYCODE_U) \
 \
	PORT_START /* KEY ROW 6 */ \
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("W (VEH)") PORT_CODE(KEYCODE_W) \
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("S (EHS)") PORT_CODE(KEYCODE_S) \
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A (AH)") PORT_CODE(KEYCODE_A) \
	PORT_BIT (0x10, 0x00, IPT_UNUSED) \
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E (YEH)") PORT_CODE(KEYCODE_E) \
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G (GEH)") PORT_CODE(KEYCODE_G) \
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H (KHAH)") PORT_CODE(KEYCODE_H) \
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y (TVYORDIY ZNAHK)") PORT_CODE(KEYCODE_Y) \
 \
	PORT_START /* KEY ROW 7 */ \
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("- =") PORT_CODE(KEYCODE_MINUS) \
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("^ (CHEH)") PORT_CODE(KEYCODE_HOME) \
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) \
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RSHIFT") PORT_CODE(KEYCODE_RSHIFT) \
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH) \
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) \
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L (EHL)") PORT_CODE(KEYCODE_L) \
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8 (") PORT_CODE(KEYCODE_8)

static INPUT_PORTS_START(oric)
	INPUT_PORT_ORIC
	PORT_START_TAG("oric_floppy_interface")
	/* floppy interface  */
	PORT_CONFNAME( 0x03, 0x00, "Floppy disc interface" )
	PORT_CONFSETTING(    0x00, DEF_STR( None ) )
	PORT_CONFSETTING(    0x01, "Microdisc" )
	PORT_CONFSETTING(    0x02, "Jasmin" )
/*  PORT_CONFSETTING(    0x03, "Low 8D DOS" ) */
/*  PORT_CONFSETTING(    0x04, "High 8D DOS" ) */

	/* vsync cable hardware. This is a simple cable connected to the video output
    to the monitor/television. The sync signal is connected to the cassette input
    allowing interrupts to be generated from the vsync signal. */
	PORT_CONFNAME(0x08, 0x00, "Vsync cable hardware")
	PORT_CONFSETTING(0x0, DEF_STR( Off) )
	PORT_CONFSETTING(0x8, DEF_STR( On) )
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_VBLANK)
INPUT_PORTS_END

static INPUT_PORTS_START(orica)
	INPUT_PORT_ORICA
	PORT_START_TAG("oric_floppy_interface")
	/* floppy interface  */
	PORT_CONFNAME( 0x03, 0x00, "Floppy disc interface" )
	PORT_CONFSETTING(    0x00, DEF_STR( None ) )
	PORT_CONFSETTING(    0x01, "Microdisc" )
	PORT_CONFSETTING(    0x02, "Jasmin" )
/*  PORT_CONFSETTING(    0x03, "Low 8D DOS" ) */
/*  PORT_CONFSETTING(    0x04, "High 8D DOS" ) */

	/* vsync cable hardware. This is a simple cable connected to the video output
    to the monitor/television. The sync signal is connected to the cassette input
    allowing interrupts to be generated from the vsync signal. */
    PORT_CONFNAME(0x08, 0x00, "Vsync cable hardware")
	PORT_CONFSETTING(0x0, DEF_STR( Off) )
	PORT_CONFSETTING(0x8, DEF_STR( On) )
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_VBLANK)
INPUT_PORTS_END

static INPUT_PORTS_START(prav8d)
	INPUT_PORT_PRAV8D
	/* force apple2 disc interface for pravetz */
	PORT_START_TAG("oric_floppy_interface")
	PORT_CONFNAME( 0x07, 0x00, "Floppy disc interface" )
	PORT_CONFSETTING(    0x00, DEF_STR( None ) )
	PORT_CONFSETTING(    0x03, "Low 8D DOS" )
	PORT_CONFSETTING(    0x04, "High 8D DOS" )
	PORT_CONFNAME(0x08, 0x00, "Vsync cable hardware")
	PORT_CONFSETTING(0x0, DEF_STR( Off ) )
	PORT_CONFSETTING(0x8, DEF_STR( On ) )
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_VBLANK)
INPUT_PORTS_END

static INPUT_PORTS_START(telstrat)
	INPUT_PORT_ORICA

	PORT_START_TAG("oric_floppy_interface")
	/* vsync cable hardware. This is a simple cable connected to the video output
    to the monitor/television. The sync signal is connected to the cassette input
    allowing interrupts to be generated from the vsync signal. */
	PORT_BIT (0x07, 0x00, IPT_UNUSED)
	PORT_CONFNAME(0x08, 0x00, "Vsync cable hardware")
	PORT_CONFSETTING(0x0, DEF_STR( Off ) )
	PORT_CONFSETTING(0x8, DEF_STR( On ) )
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_VBLANK)
	/* left joystick port */
	PORT_START
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("JOYSTICK 0 UP") PORT_CODE(JOYCODE_X_RIGHT_SWITCH)
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("JOYSTICK 0 DOWN") PORT_CODE(JOYCODE_X_LEFT_SWITCH)
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("JOYSTICK 0 LEFT") PORT_CODE(JOYCODE_BUTTON1)
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("JOYSTICK 0 RIGHT") PORT_CODE(JOYCODE_Y_DOWN_SWITCH)
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("JOYSTICK 0 FIRE 1") PORT_CODE(JOYCODE_Y_UP_SWITCH)
	/* right joystick port */
	PORT_START
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("JOYSTICK 1 UP") PORT_CODE(JOYCODE_X_RIGHT_SWITCH)
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("JOYSTICK 1 DOWN") PORT_CODE(JOYCODE_X_LEFT_SWITCH)
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("JOYSTICK 1 LEFT") PORT_CODE(JOYCODE_BUTTON1)
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("JOYSTICK 1 RIGHT") PORT_CODE(JOYCODE_Y_DOWN_SWITCH)
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("JOYSTICK 1 FIRE 1") PORT_CODE(JOYCODE_Y_UP_SWITCH)
INPUT_PORTS_END

static const unsigned char oric_palette[8*3] =
{
	0x00, 0x00, 0x00, 0xff, 0x00, 0x00,
	0x00, 0xff, 0x00, 0xff, 0xff, 0x00,
	0x00, 0x00, 0xff, 0xff, 0x00, 0xff,
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff,
};

/* Initialise the palette */
static PALETTE_INIT( oric )
{
	int i;

	for ( i = 0; i < sizeof(oric_palette) / 3; i++ ) {
		palette_set_color_rgb(machine, i, oric_palette[i*3], oric_palette[i*3+1], oric_palette[i*3+2]);
	}
}



static const struct AY8910interface oric_ay_interface =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	0,
	0,
	oric_psg_porta_write,
	0
};


static MACHINE_DRIVER_START( oric )
	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("main", M6502, 1000000)
	MDRV_CPU_PROGRAM_MAP(oric_mem, 0)
	MDRV_INTERLEAVE(1)

	MDRV_MACHINE_START( oric )
	MDRV_MACHINE_RESET( oric )

    /* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(40*6, 28*8)
	MDRV_SCREEN_VISIBLE_AREA(0, 40*6-1, 0, 28*8-1)
	MDRV_PALETTE_LENGTH(8)
	MDRV_PALETTE_INIT( oric )

	MDRV_VIDEO_START( oric )
	MDRV_VIDEO_UPDATE( oric )

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD(WAVE, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MDRV_SOUND_ADD(AY8912, 1000000)
	MDRV_SOUND_CONFIG(oric_ay_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* printer */
	MDRV_DEVICE_ADD("printer", PRINTER)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( telstrat)
	MDRV_IMPORT_FROM( oric )
	MDRV_CPU_MODIFY( "main" )
	MDRV_CPU_PROGRAM_MAP( telestrat_mem, 0 )

	MDRV_MACHINE_START( telestrat )
	MDRV_MACHINE_RESET( NULL )
MACHINE_DRIVER_END


ROM_START(oric1)
	ROM_REGION(0x10000+0x04000+0x02000+0x0800,REGION_CPU1,0)
	ROM_LOAD ("basic10.rom", 0x10000, 0x4000, CRC(f18710b4) SHA1(333116e6884d85aaa4dfc7578a91cceeea66d016))
	ROM_LOAD_OPTIONAL ("microdis.rom",0x014000, 0x02000, CRC(a9664a9c) SHA1(0d2ef6e67322f48f4b7e08d8bbe68827e2074561))
	ROM_LOAD_OPTIONAL ("jasmin.rom", 0x016000, 0x800, CRC(37220e89) SHA1(70e59b8abd67092f050462abc6cb5271e4c15f01))
ROM_END

ROM_START(orica)
	ROM_REGION(0x10000+0x04000+0x02000+0x0800,REGION_CPU1,0)
	ROM_LOAD ("basic11b.rom", 0x10000, 0x4000, CRC(c3a92bef) SHA1(9451a1a09d8f75944dbd6f91193fc360f1de80ac))
	ROM_LOAD_OPTIONAL ("microdis.rom",0x014000, 0x02000, CRC(a9664a9c) SHA1(0d2ef6e67322f48f4b7e08d8bbe68827e2074561))
	ROM_LOAD_OPTIONAL ("jasmin.rom", 0x016000, 0x800, CRC(37220e89) SHA1(70e59b8abd67092f050462abc6cb5271e4c15f01))
ROM_END

ROM_START(telstrat)
	ROM_REGION(0x010000+(0x04000*4), REGION_CPU1,0)
	ROM_LOAD ("telmatic.rom", 0x010000, 0x02000, CRC(94358dc6) SHA1(35f92a0477a88f5cf564971125047ffcfa02ec10))
	ROM_LOAD ("teleass.rom", 0x014000, 0x04000, CRC(68b0fde6) SHA1(9e9af51dae3199cccf49ab3f0d47e2b9be4ba97d))
	ROM_LOAD ("hyperbas.rom", 0x018000, 0x04000, CRC(1d96ab50) SHA1(f5f70a0eb59f8cd6c261e179ae78ef906f68ed63))
	ROM_LOAD ("telmon24.rom", 0x01c000, 0x04000, CRC(aa727c5d) SHA1(86fc8dc0932f983efa199e31ae05a4424772f959))
ROM_END

ROM_START(prav8d)
    ROM_REGION( 0x10000+0x4000+0x0100+0x0200, REGION_CPU1, 0 )
    ROM_LOAD( "pravetzt.rom", 0x10000, 0x4000, CRC(58079502) SHA1(7afc276cb118adff72e4f16698f94bf3b2c64146) )
	ROM_LOAD_OPTIONAL( "8ddoslo.rom", 0x014000, 0x0100, CRC(0c82f636) SHA1(b29d151a0dfa3c7cd50439b51d0a8f95559bc2b6) )
    ROM_LOAD_OPTIONAL( "8ddoshi.rom", 0x014100, 0x0200, CRC(66309641) SHA1(9c2e82b3c4d385ade6215fcb89f8b92e6fd2bf4b) )
ROM_END

ROM_START(prav8dd)
    ROM_REGION( 0x10000+0x4000+0x0100+0x0200, REGION_CPU1, 0 )
	ROM_SYSTEM_BIOS( 0, "default", "Disk ROM, 1989")
	ROMX_LOAD( "8d.rom",       0x10000, 0x4000, CRC(b48973ef) SHA1(fd47c977fc215a3b577596a7483df53e8a1e9c83), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "radosoft", "RadoSoft Disk ROM, 1992")
	ROMX_LOAD( "pravetzd.rom", 0x10000, 0x4000, CRC(f8d23821) SHA1(f87ad3c5832773b6e0614905552a80c98dc8e2a5), ROM_BIOS(2) )
    ROM_LOAD_OPTIONAL( "8ddoslo.rom", 0x014000, 0x0100, CRC(0c82f636) SHA1(b29d151a0dfa3c7cd50439b51d0a8f95559bc2b6) )
    ROM_LOAD_OPTIONAL( "8ddoshi.rom", 0x014100, 0x0200, CRC(66309641) SHA1(9c2e82b3c4d385ade6215fcb89f8b92e6fd2bf4b) )
ROM_END

static void oric_common_cassette_getinfo(const mess_device_class *devclass, UINT32 state, union devinfo *info)
{
	/* cassette */
	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case MESS_DEVINFO_INT_COUNT:							info->i = 1; break;
		case MESS_DEVINFO_INT_CASSETTE_DEFAULT_STATE:				info->i = CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED; break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case MESS_DEVINFO_PTR_CASSETTE_FORMATS:				info->p = (void *) oric_cassette_formats; break;

		default:										cassette_device_getinfo(devclass, state, info); break;
	}
}

SYSTEM_CONFIG_START(oric_common)
	CONFIG_DEVICE(oric_common_cassette_getinfo)
SYSTEM_CONFIG_END

static void oric1_floppy_getinfo(const mess_device_class *devclass, UINT32 state, union devinfo *info)
{
	/* floppy */
	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case MESS_DEVINFO_INT_TYPE:							info->i = IO_FLOPPY; break;
		case MESS_DEVINFO_INT_READABLE:						info->i = 1; break;
		case MESS_DEVINFO_INT_WRITEABLE:						info->i = 1; break;
		case MESS_DEVINFO_INT_CREATABLE:						info->i = 1; break;
		case MESS_DEVINFO_INT_COUNT:							info->i = 4; break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case MESS_DEVINFO_PTR_START:							info->start = DEVICE_START_NAME(oric_floppy); break;
		case MESS_DEVINFO_PTR_LOAD:							info->load = DEVICE_IMAGE_LOAD_NAME(oric_floppy); break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case MESS_DEVINFO_STR_FILE_EXTENSIONS:				strcpy(info->s = device_temp_str(), "dsk"); break;
	}
}

SYSTEM_CONFIG_START(oric1)
	CONFIG_IMPORT_FROM(oric_common)
	CONFIG_DEVICE(oric1_floppy_getinfo)
SYSTEM_CONFIG_END

static void prav8_floppy_getinfo(const mess_device_class *devclass, UINT32 state, union devinfo *info)
{
	/* floppy */
	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case MESS_DEVINFO_INT_COUNT:							info->i = 1; break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case MESS_DEVINFO_PTR_FLOPPY_OPTIONS:				info->p = (void *) floppyoptions_apple2; break;

		default:										floppy_device_getinfo(devclass, state, info); break;
	}
}

SYSTEM_CONFIG_START(prav8)
	CONFIG_IMPORT_FROM(oric_common)
	CONFIG_DEVICE(prav8_floppy_getinfo)
SYSTEM_CONFIG_END


/*    YEAR   NAME       PARENT  COMPAT  MACHINE     INPUT       INIT    CONFIG    COMPANY         FULLNAME */
COMP( 1983, oric1,     0,		0,		oric,       oric,	    0,	    oric1,    "Tangerine",    "Oric 1" , 0)
COMP( 1984, orica,     oric1,	0,		oric,	    orica,	    0,	    oric1,    "Tangerine",    "Oric Atmos" , 0)
COMP( 1985, prav8d,    oric1,	0,		oric,       prav8d,     0,      prav8,    "Pravetz",      "Pravetz 8D", 0)
COMP( 1989, prav8dd,   oric1,	0,		oric,       prav8d,     0,      prav8,    "Pravetz",      "Pravetz 8D (Disk ROM)", GAME_COMPUTER_MODIFIED)
COMP( 1986, telstrat,  oric1,	0,		telstrat,   telstrat,   0,      oric1,    "Tangerine",    "Oric Telestrat", GAME_NOT_WORKING )
