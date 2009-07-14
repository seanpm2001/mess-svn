/***************************************************************************

  machine.c

  Functions to emulate general aspects of PMD-85 (RAM, ROM, interrupts,
  I/O ports)

  Krzysztof Strzecha

***************************************************************************/

#include "driver.h"
#include "devices/cassette.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8255a.h"
#include "includes/pmd85.h"
#include "machine/msm8251.h"
#include "machine/pit8253.h"
#include "includes/serial.h"

static UINT8 pmd85_rom_module_present = 0;

static UINT8 pmd85_ppi_port_outputs[4][3];

static UINT8 pmd85_startup_mem_map = 0;
static UINT8 pmd853_memory_mapping = 0x01;
static void (*pmd85_update_memory)(running_machine *);

enum {PMD85_LED_1, PMD85_LED_2, PMD85_LED_3};
enum {PMD85_1, PMD85_2, PMD85_2A, PMD85_2B, PMD85_3, ALFA, MATO, C2717};

static UINT8 pmd85_model;

static emu_timer * pmd85_cassette_timer;

static void pmd851_update_memory(running_machine *machine)
{
	const address_space* space = cputag_get_address_space(machine, "maincpu",ADDRESS_SPACE_PROGRAM);
	
	if (pmd85_startup_mem_map)
	{
		UINT8 *mem = memory_region(machine, "maincpu");

		memory_install_write8_handler(space, 0x0000, 0x0fff, 0, 0, SMH_UNMAP);
		memory_install_write8_handler(space, 0x1000, 0x1fff, 0, 0, SMH_NOP);
		memory_install_write8_handler(space, 0x2000, 0x2fff, 0, 0, SMH_UNMAP);
		memory_install_write8_handler(space, 0x3000, 0x3fff, 0, 0, SMH_NOP);

		memory_install_read8_handler(space, 0x1000, 0x1fff, 0, 0, SMH_NOP);
		memory_install_read8_handler(space, 0x3000, 0x3fff, 0, 0, SMH_NOP);

		memory_set_bankptr(machine, 1, mem + 0x010000);
		memory_set_bankptr(machine, 3, mem + 0x010000);
		memory_set_bankptr(machine, 5, mess_ram + 0xc000);

		memory_set_bankptr(machine, 6, mem + 0x010000);
		memory_set_bankptr(machine, 7, mem + 0x010000);
		memory_set_bankptr(machine, 8, mess_ram + 0xc000);
	}
	else
	{
		memory_install_write8_handler(space, 0x0000, 0x0fff, 0, 0, SMH_BANK(1));
		memory_install_write8_handler(space, 0x1000, 0x1fff, 0, 0, SMH_BANK(2));
		memory_install_write8_handler(space, 0x2000, 0x2fff, 0, 0, SMH_BANK(3));
		memory_install_write8_handler(space, 0x3000, 0x3fff, 0, 0, SMH_BANK(4));
		memory_install_write8_handler(space, 0x4000, 0x7fff, 0, 0, SMH_BANK(5));

		memory_install_read8_handler(space, 0x1000, 0x1fff, 0, 0, SMH_BANK(2));
		memory_install_read8_handler(space, 0x3000, 0x3fff, 0, 0, SMH_BANK(4));

		memory_set_bankptr(machine, 1, mess_ram);
		memory_set_bankptr(machine, 2, mess_ram + 0x1000);
		memory_set_bankptr(machine, 3, mess_ram + 0x2000);
		memory_set_bankptr(machine, 4, mess_ram + 0x3000);
		memory_set_bankptr(machine, 5, mess_ram + 0x4000);
	}
}

static void pmd852a_update_memory(running_machine *machine)
{
	const address_space* space = cputag_get_address_space(machine, "maincpu",ADDRESS_SPACE_PROGRAM);

	if (pmd85_startup_mem_map)
	{
		UINT8 *mem = memory_region(machine, "maincpu");

		memory_install_write8_handler(space, 0x0000, 0x0fff, 0, 0, SMH_UNMAP);
		memory_install_write8_handler(space, 0x2000, 0x2fff, 0, 0, SMH_UNMAP);

		memory_set_bankptr(machine, 1, mem + 0x010000);
		memory_set_bankptr(machine, 2, mess_ram + 0x9000);
		memory_set_bankptr(machine, 3, mem + 0x010000);
		memory_set_bankptr(machine, 4, mess_ram + 0xb000);
		memory_set_bankptr(machine, 5, mess_ram + 0xc000);
		memory_set_bankptr(machine, 6, mem + 0x010000);
		memory_set_bankptr(machine, 7, mess_ram + 0x9000);
		memory_set_bankptr(machine, 8, mem + 0x010000);
		memory_set_bankptr(machine, 9, mess_ram + 0xb000);
		memory_set_bankptr(machine, 10, mess_ram + 0xc000);

	}
	else
	{
		memory_install_write8_handler(space, 0x0000, 0x0fff, 0, 0, SMH_BANK(1));
		memory_install_write8_handler(space, 0x2000, 0x2fff, 0, 0, SMH_BANK(3));

		memory_set_bankptr(machine, 1, mess_ram);
		memory_set_bankptr(machine, 2, mess_ram + 0x1000);
		memory_set_bankptr(machine, 3, mess_ram + 0x2000);
		memory_set_bankptr(machine, 4, mess_ram + 0x5000);
		memory_set_bankptr(machine, 5, mess_ram + 0x4000);
	}
}

static void pmd853_update_memory(running_machine *machine)
{
	if (pmd85_startup_mem_map)
	{
		UINT8 *mem = memory_region(machine, "maincpu");

		memory_set_bankptr(machine,  1, mem + 0x010000);
		memory_set_bankptr(machine,  2, mem + 0x010000);
		memory_set_bankptr(machine,  3, mem + 0x010000);
		memory_set_bankptr(machine,  4, mem + 0x010000);
		memory_set_bankptr(machine,  5, mem + 0x010000);
		memory_set_bankptr(machine,  6, mem + 0x010000);
		memory_set_bankptr(machine,  7, mem + 0x010000);
		memory_set_bankptr(machine,  8, mem + 0x010000);
		memory_set_bankptr(machine,  9, mess_ram);
		memory_set_bankptr(machine, 10, mess_ram + 0x2000);
		memory_set_bankptr(machine, 11, mess_ram + 0x4000);
		memory_set_bankptr(machine, 12, mess_ram + 0x6000);
		memory_set_bankptr(machine, 13, mess_ram + 0x8000);
		memory_set_bankptr(machine, 14, mess_ram + 0xa000);
		memory_set_bankptr(machine, 15, mess_ram + 0xc000);
		memory_set_bankptr(machine, 16, mess_ram + 0xe000);
	}
	else
	{
		memory_set_bankptr(machine,  1, mess_ram);
		memory_set_bankptr(machine,  2, mess_ram + 0x2000);
		memory_set_bankptr(machine,  3, mess_ram + 0x4000);
		memory_set_bankptr(machine,  4, mess_ram + 0x6000);
		memory_set_bankptr(machine,  5, mess_ram + 0x8000);
		memory_set_bankptr(machine,  6, mess_ram + 0xa000);
		memory_set_bankptr(machine,  7, mess_ram + 0xc000);
		memory_set_bankptr(machine,  8, pmd853_memory_mapping ? memory_region(machine, "maincpu") + 0x010000 : mess_ram + 0xe000);
	}
}

static void alfa_update_memory(running_machine *machine)
{
	const address_space* space = cputag_get_address_space(machine, "maincpu",ADDRESS_SPACE_PROGRAM);

	if (pmd85_startup_mem_map)
	{
		UINT8 *mem = memory_region(machine, "maincpu");

		memory_install_write8_handler(space, 0x0000, 0x0fff, 0, 0, SMH_UNMAP);
		memory_install_write8_handler(space, 0x1000, 0x33ff, 0, 0, SMH_UNMAP);
		memory_install_write8_handler(space, 0x3400, 0x3fff, 0, 0, SMH_NOP);

		memory_set_bankptr(machine, 1, mem + 0x010000);
		memory_set_bankptr(machine, 2, mem + 0x011000);
		memory_set_bankptr(machine, 4, mess_ram + 0xc000);
		memory_set_bankptr(machine, 5, mem + 0x010000);
		memory_set_bankptr(machine, 6, mem + 0x011000);
		memory_set_bankptr(machine, 7, mess_ram + 0xc000);
	}
	else
	{
		memory_install_write8_handler(space, 0x0000, 0x0fff, 0, 0, SMH_BANK(1));
		memory_install_write8_handler(space, 0x1000, 0x33ff, 0, 0, SMH_BANK(2));
		memory_install_write8_handler(space, 0x3400, 0x3fff, 0, 0, SMH_BANK(3));

		memory_set_bankptr(machine, 1, mess_ram);
		memory_set_bankptr(machine, 2, mess_ram + 0x1000);
		memory_set_bankptr(machine, 3, mess_ram + 0x3400);
		memory_set_bankptr(machine, 4, mess_ram + 0x4000);
	}
}

static void mato_update_memory(running_machine *machine)
{
	const address_space* space = cputag_get_address_space(machine, "maincpu",ADDRESS_SPACE_PROGRAM);

	if (pmd85_startup_mem_map)
	{
		UINT8 *mem = memory_region(machine, "maincpu");

		memory_install_write8_handler(space, 0x0000, 0x3fff, 0, 0, SMH_UNMAP);

		memory_set_bankptr(machine, 1, mem + 0x010000);
		memory_set_bankptr(machine, 2, mess_ram + 0xc000);
		memory_set_bankptr(machine, 3, mem + 0x010000);
		memory_set_bankptr(machine, 4, mess_ram + 0xc000);
	}
	else
	{
		memory_install_write8_handler(space, 0x0000, 0x3fff, 0, 0, SMH_BANK(1));

		memory_set_bankptr(machine, 1, mess_ram);
		memory_set_bankptr(machine, 2, mess_ram + 0x4000);
	}
}

static void c2717_update_memory(running_machine *machine)
{
	const address_space* space = cputag_get_address_space(machine, "maincpu",ADDRESS_SPACE_PROGRAM);

	UINT8 *mem = memory_region(machine, "maincpu");
	if (pmd85_startup_mem_map)
	{
		memory_install_write8_handler(space, 0x0000, 0x3fff, 0, 0, SMH_UNMAP);

		memory_set_bankptr(machine, 1, mem + 0x010000);
		memory_set_bankptr(machine, 2, mess_ram + 0x4000);
		memory_set_bankptr(machine, 3, mem + 0x010000);
		memory_set_bankptr(machine, 4, mess_ram + 0xc000);
	}
	else
	{
		memory_install_write8_handler(space, 0x0000, 0x3fff, 0, 0, SMH_BANK(1));
		memory_set_bankptr(machine, 1, mess_ram);
		memory_set_bankptr(machine, 2, mess_ram + 0x4000);
	}
}

/*******************************************************************************

	Motherboard 8255 (PMD-85.1, PMD-85.2, PMD-85.3, Didaktik Alfa)
	--------------------------------------------------------------
		keyboard, speaker, LEDs

*******************************************************************************/

static  READ8_DEVICE_HANDLER ( pmd85_ppi_0_porta_r )
{
	return 0xff;
}

static  READ8_DEVICE_HANDLER ( pmd85_ppi_0_portb_r )
{
	static const char *const keynames[] = { "KEY0", "KEY1", "KEY2", "KEY3", "KEY4", "KEY5", "KEY6", "KEY7",
										"KEY8", "KEY9", "KEY10", "KEY11", "KEY12", "KEY13", "KEY14", "KEY15" };

	return input_port_read(device->machine, keynames[(pmd85_ppi_port_outputs[0][0] & 0x0f)]) & input_port_read(device->machine, "KEY15");
}

static  READ8_DEVICE_HANDLER ( pmd85_ppi_0_portc_r )
{
	return 0xff;
}

static WRITE8_DEVICE_HANDLER ( pmd85_ppi_0_porta_w )
{
	pmd85_ppi_port_outputs[0][0] = data;
}

static WRITE8_DEVICE_HANDLER ( pmd85_ppi_0_portb_w )
{
	pmd85_ppi_port_outputs[0][1] = data;
}

static WRITE8_DEVICE_HANDLER ( pmd85_ppi_0_portc_w )
{
	pmd85_ppi_port_outputs[0][2] = data;
	set_led_status(PMD85_LED_2, (data & 0x08) ? 1 : 0);
	set_led_status(PMD85_LED_3, (data & 0x04) ? 1 : 0);
}

/*******************************************************************************

	Motherboard 8255 (Mato)
	-----------------------
		keyboard, speaker, LEDs, tape

*******************************************************************************/

static  READ8_DEVICE_HANDLER ( mato_ppi_0_portb_r )
{
	int i;
	UINT8 data = 0xff;
	static const char *const keynames[] = { "KEY0", "KEY1", "KEY2", "KEY3", "KEY4", "KEY5", "KEY6", "KEY7" };

	for (i = 0; i < 8; i++)
	{
		if (!(pmd85_ppi_port_outputs[0][0] & (1 << i)))
			data &= input_port_read(device->machine, keynames[i]);
	}
	return data;
}

static  READ8_DEVICE_HANDLER ( mato_ppi_0_portc_r )
{
	return input_port_read(device->machine, "KEY8") | 0x8f;
}

static WRITE8_DEVICE_HANDLER ( mato_ppi_0_portc_w )
{
	pmd85_ppi_port_outputs[0][2] = data;
	set_led_status(PMD85_LED_2, (data & 0x08) ? 1 : 0);
	set_led_status(PMD85_LED_3, (data & 0x04) ? 1 : 0);
}

/*******************************************************************************

	I/O board 8255
	--------------
		GPIO/0 (K3 connector), GPIO/1 (K4 connector)

*******************************************************************************/

static READ8_DEVICE_HANDLER ( pmd85_ppi_1_porta_r )
{
	return 0xff;
}

static READ8_DEVICE_HANDLER ( pmd85_ppi_1_portb_r )
{
	return 0xff;
}

static READ8_DEVICE_HANDLER ( pmd85_ppi_1_portc_r )
{
	return 0xff;
}

static WRITE8_DEVICE_HANDLER ( pmd85_ppi_1_porta_w )
{
	pmd85_ppi_port_outputs[1][0] = data;
}

static WRITE8_DEVICE_HANDLER ( pmd85_ppi_1_portb_w )
{
	pmd85_ppi_port_outputs[1][1] = data;
}

static WRITE8_DEVICE_HANDLER ( pmd85_ppi_1_portc_w )
{
	pmd85_ppi_port_outputs[1][2] = data;
}

/*******************************************************************************

	I/O board 8255
	--------------
		IMS-2 (K5 connector)

	- 8251 - cassette recorder and V.24/IFSS (selectable by switch)

	- external interfaces connector (K2)

*******************************************************************************/

static READ8_DEVICE_HANDLER ( pmd85_ppi_2_porta_r )
{
	return 0xff;
}

static READ8_DEVICE_HANDLER ( pmd85_ppi_2_portb_r )
{
	return 0xff;
}

static READ8_DEVICE_HANDLER ( pmd85_ppi_2_portc_r )
{
	return 0xff;
}

static WRITE8_DEVICE_HANDLER ( pmd85_ppi_2_porta_w )
{
	pmd85_ppi_port_outputs[2][0] = data;
}

static WRITE8_DEVICE_HANDLER ( pmd85_ppi_2_portb_w )
{
	pmd85_ppi_port_outputs[2][1] = data;
}

static WRITE8_DEVICE_HANDLER ( pmd85_ppi_2_portc_w )
{
	pmd85_ppi_port_outputs[2][2] = data;
}

/*******************************************************************************

	I/O board 8251
	--------------
		cassette recorder and V.24/IFSS (selectable by switch)

*******************************************************************************/

/*******************************************************************************

	I/O board 8253
	--------------

	Timer 0:
		OUT0	- external interfaces connector (K2)
		CLK0	- external interfaces connector (K2)
		GATE0	- external interfaces connector (K2), default = 1
	Timer 1:
		OUT0	- external interfaces connector (K2), msm8251 (for V24 only)
		CLK0	- hardwired to 2 MHz system clock
		GATE0	- external interfaces connector (K2), default = 1
	Timer 2:
		OUT0	- unused
		CLK0	- hardwired to 1HZ signal generator
		GATE0	- hardwired to 5V, default = 1

*******************************************************************************/

const struct pit8253_config pmd85_pit8253_interface =
{
	{
		{ 0,		NULL },
		{ 2000000,	NULL },
		{ 1,		NULL }
	}
};

/*******************************************************************************

	I/O board external interfaces connector (K2)
	--------------------------------------------

*******************************************************************************/



/*******************************************************************************

	ROM Module 8255
	---------------
		port A - data read
		ports B, C - address select

*******************************************************************************/

static READ8_DEVICE_HANDLER ( pmd85_ppi_3_porta_r )
{
	return memory_region(device->machine, "user1")[pmd85_ppi_port_outputs[3][1]|(pmd85_ppi_port_outputs[3][2]<<8)];
}

static READ8_DEVICE_HANDLER ( pmd85_ppi_3_portb_r )
{
	return 0xff;
}

static READ8_DEVICE_HANDLER ( pmd85_ppi_3_portc_r )
{
	return 0xff;
}

static WRITE8_DEVICE_HANDLER ( pmd85_ppi_3_porta_w )
{
	pmd85_ppi_port_outputs[3][0] = data;
}

static WRITE8_DEVICE_HANDLER ( pmd85_ppi_3_portb_w )
{
	pmd85_ppi_port_outputs[3][1] = data;
}

static WRITE8_DEVICE_HANDLER ( pmd85_ppi_3_portc_w )
{
	pmd85_ppi_port_outputs[3][2] = data;
}

/*******************************************************************************

	I/O ports (PMD-85.1, PMD-85.2, PMD-85.3, Didaktik Alfa)
	-------------------------------------------------------

	I/O board
	1xxx11aa	external interfaces connector (K2)

	0xxx11aa	I/O board interfaces
		000111aa	8251 (casette recorder, V24)
		010011aa	8255 (GPIO/0, GPIO/1)
		010111aa	8253
		011111aa	8255 (IMS-2)

	Motherboard
	1xxx01aa	8255 (keyboard, speaker, LEDs)
			PMD-85.3 memory banking

	ROM Module
	1xxx10aa	8255 (ROM reading)

*******************************************************************************/

READ8_HANDLER ( pmd85_io_r )
{
	if (pmd85_startup_mem_map)
	{
		return 0xff;
	}

	switch (offset & 0x0c)
	{
		case 0x04:	/* Motherboard */
				switch (offset & 0x80)
				{
					case 0x80:	/* Motherboard 8255 */
							return i8255a_r(devtag_get_device(space->machine, "ppi8255_0"), offset & 0x03);
				}
				break;
		case 0x08:	/* ROM module connector */
				switch (pmd85_model)
				{
					case PMD85_1:
					case PMD85_2:
					case PMD85_2A:
					case C2717:
					case PMD85_3:
						if (pmd85_rom_module_present)
						{
							switch (offset & 0x80)
							{
								case 0x80:	/* ROM module 8255 */
										return i8255a_r(devtag_get_device(space->machine, "ppi8255_3"), offset & 0x03);
							}
						}
						break;
				}
				break;
		case 0x0c:	/* I/O board */
				switch (offset & 0x80)
				{
					case 0x00:	/* I/O board interfaces */
							switch (offset & 0x70)
							{
								case 0x10:	/* 8251 (casette recorder, V24) */
										switch (offset & 0x01)
										{
											case 0x00: return msm8251_data_r(devtag_get_device(space->machine, "uart"), offset & 0x01);
											case 0x01: return msm8251_status_r(devtag_get_device(space->machine, "uart"), offset & 0x01);
										}
										break;
								case 0x40:      /* 8255 (GPIO/0, GPIO/1) */
										return i8255a_r(devtag_get_device(space->machine, "ppi8255_1"), offset & 0x03);
								case 0x50:	/* 8253 */
										return pit8253_r( devtag_get_device(space->machine, "pit8253"), offset & 0x03);
								case 0x70:	/* 8255 (IMS-2) */
										return i8255a_r(devtag_get_device(space->machine, "ppi8255_2"), offset & 0x03);
							}
							break;
					case 0x80:	/* external interfaces */
							break;
				}
				break;
	}

	logerror ("Reading from unmapped port: %02x\n", offset);
	return 0xff;
}

WRITE8_HANDLER ( pmd85_io_w )
{
	if (pmd85_startup_mem_map)
	{
		pmd85_startup_mem_map = 0;
		(*pmd85_update_memory)(space->machine);
	}

	switch (offset & 0x0c)
	{
		case 0x04:	/* Motherboard */
				switch (offset & 0x80)
				{
					case 0x80:	/* Motherboard 8255 */
							i8255a_w(devtag_get_device(space-> machine, "ppi8255_0"), offset & 0x03, data);
							/* PMD-85.3 memory banking */
							if ((offset & 0x03) == 0x03)
							{
								pmd853_memory_mapping = data & 0x01;
								(*pmd85_update_memory)(space->machine);
							}
							break;
				}
				break;
		case 0x08:	/* ROM module connector */
				switch (pmd85_model)
				{
					case PMD85_1:
					case PMD85_2:
					case PMD85_2A:
					case C2717:
					case PMD85_3:
						if (pmd85_rom_module_present)
						{
							switch (offset & 0x80)
							{
								case 0x80:	/* ROM module 8255 */
										i8255a_w(devtag_get_device(space->machine, "ppi8255_3"), offset & 0x03, data);
										break;
							}
						}
						break;
				}
				break;
		case 0x0c:	/* I/O board */
				switch (offset & 0x80)
				{
					case 0x00:	/* I/O board interfaces */
							switch (offset & 0x70)
							{
								case 0x10:	/* 8251 (casette recorder, V24) */
										switch (offset & 0x01)
										{
											case 0x00: msm8251_data_w(devtag_get_device(space->machine, "uart"), offset & 0x01, data); break;
											case 0x01: msm8251_control_w(devtag_get_device(space->machine, "uart"), offset & 0x01, data); break;
										}
										break;
								case 0x40:      /* 8255 (GPIO/0, GPIO/0) */
										i8255a_w(devtag_get_device(space->machine, "ppi8255_1"), offset & 0x03, data);
										break;
								case 0x50:	/* 8253 */
										pit8253_w(devtag_get_device(space->machine, "pit8253"), offset & 0x03, data);
										logerror ("8253 writing. Address: %02x, Data: %02x\n", offset, data);
										break;
								case 0x70:	/* 8255 (IMS-2) */
										i8255a_w(devtag_get_device(space->machine, "ppi8255_2"), offset & 0x03, data);
										break;
							}
							break;
					case 0x80:	/* external interfaces */
							break;
				}
				break;
	}
}

/*******************************************************************************

	I/O ports (Mato)
	----------------

	Motherboard
	1xxx01aa	8255 (keyboard, speaker, LEDs, tape)

*******************************************************************************/

 READ8_HANDLER ( mato_io_r )
{
	if (pmd85_startup_mem_map)
	{
		return 0xff;
	}

	switch (offset & 0x0c)
	{
		case 0x04:	/* Motherboard */
				switch (offset & 0x80)
				{
					case 0x80:	/* Motherboard 8255 */
							return i8255a_r(devtag_get_device(space->machine, "ppi8255_0"), offset & 0x03);
				}
				break;
	}

	logerror ("Reading from unmapped port: %02x\n", offset);
	return 0xff;
}

WRITE8_HANDLER ( mato_io_w )
{
	if (pmd85_startup_mem_map)
	{
		pmd85_startup_mem_map = 0;
		(*pmd85_update_memory)(space->machine);
	}

	switch (offset & 0x0c)
	{
		case 0x04:	/* Motherboard */
				switch (offset & 0x80)
				{
					case 0x80:	/* Motherboard 8255 */
							i8255a_w(devtag_get_device(space->machine, "ppi8255_0"), offset & 0x03, data);
							break;
				}
				break;
	}
}

const i8255a_interface pmd85_ppi8255_interface[4] =
{
	{
		DEVCB_HANDLER(pmd85_ppi_0_porta_r),
		DEVCB_HANDLER(pmd85_ppi_0_portb_r),
		DEVCB_HANDLER(pmd85_ppi_0_portc_r),
		DEVCB_HANDLER(pmd85_ppi_0_porta_w),
		DEVCB_HANDLER(pmd85_ppi_0_portb_w),
		DEVCB_HANDLER(pmd85_ppi_0_portc_w)
	},
	{
		DEVCB_HANDLER(pmd85_ppi_1_porta_r),
		DEVCB_HANDLER(pmd85_ppi_1_portb_r),
		DEVCB_HANDLER(pmd85_ppi_1_portc_r),
		DEVCB_HANDLER(pmd85_ppi_1_porta_w),
		DEVCB_HANDLER(pmd85_ppi_1_portb_w),
		DEVCB_HANDLER(pmd85_ppi_1_portc_w)
	},
	{
		DEVCB_HANDLER(pmd85_ppi_2_porta_r),
		DEVCB_HANDLER(pmd85_ppi_2_portb_r),
		DEVCB_HANDLER(pmd85_ppi_2_portc_r),
		DEVCB_HANDLER(pmd85_ppi_2_porta_w),
		DEVCB_HANDLER(pmd85_ppi_2_portb_w),
		DEVCB_HANDLER(pmd85_ppi_2_portc_w)
	},
	{
		DEVCB_HANDLER(pmd85_ppi_3_porta_r),
		DEVCB_HANDLER(pmd85_ppi_3_portb_r),
		DEVCB_HANDLER(pmd85_ppi_3_portc_r),
		DEVCB_HANDLER(pmd85_ppi_3_porta_w),
		DEVCB_HANDLER(pmd85_ppi_3_portb_w),
		DEVCB_HANDLER(pmd85_ppi_3_portc_w)
	}
};

const i8255a_interface alfa_ppi8255_interface[3] =
{
	{
		DEVCB_HANDLER(pmd85_ppi_0_porta_r),
		DEVCB_HANDLER(pmd85_ppi_0_portb_r),
		DEVCB_HANDLER(pmd85_ppi_0_portc_r),
		DEVCB_HANDLER(pmd85_ppi_0_porta_w),
		DEVCB_HANDLER(pmd85_ppi_0_portb_w),
		DEVCB_HANDLER(pmd85_ppi_0_portc_w)
	},
	{
		DEVCB_HANDLER(pmd85_ppi_1_porta_r),
		DEVCB_HANDLER(pmd85_ppi_1_portb_r),
		DEVCB_HANDLER(pmd85_ppi_1_portc_r),
		DEVCB_HANDLER(pmd85_ppi_1_porta_w),
		DEVCB_HANDLER(pmd85_ppi_1_portb_w),
		DEVCB_HANDLER(pmd85_ppi_1_portc_w)
	},
	{
		DEVCB_HANDLER(pmd85_ppi_2_porta_r),
		DEVCB_HANDLER(pmd85_ppi_2_portb_r),
		DEVCB_HANDLER(pmd85_ppi_2_portc_r),
		DEVCB_HANDLER(pmd85_ppi_2_porta_w),
		DEVCB_HANDLER(pmd85_ppi_2_portb_w),
		DEVCB_HANDLER(pmd85_ppi_2_portc_w)
	}
};

I8255A_INTERFACE( mato_ppi8255_interface )
{
	DEVCB_HANDLER(pmd85_ppi_0_porta_r),
	DEVCB_HANDLER(mato_ppi_0_portb_r),
	DEVCB_HANDLER(mato_ppi_0_portc_r),
	DEVCB_HANDLER(pmd85_ppi_0_porta_w),
	DEVCB_HANDLER(pmd85_ppi_0_portb_w),
	DEVCB_HANDLER(mato_ppi_0_portc_w)
};

static struct serial_connection pmd85_cassette_serial_connection;

static void pmd85_cassette_write(running_machine *machine, int id, unsigned long state)
{
	pmd85_cassette_serial_connection.input_state = state;
}

static TIMER_CALLBACK(pmd85_cassette_timer_callback)
{
	int data;
	int current_level;
	static int previous_level = 0;
	static int clk_level = 1;
	static int clk_level_tape = 1;

	if (!(input_port_read(machine, "DSW0") & 0x02))	/* V.24 / Tape Switch */
	{
		/* tape reading */
		if (cassette_get_state(devtag_get_device(machine, "cassette"))&CASSETTE_PLAY)
		{
			switch (pmd85_model)
			{
				case PMD85_1:
					if (clk_level_tape)
					{
						previous_level = (cassette_input(devtag_get_device(machine, "cassette")) > 0.038) ? 1 : 0;
						clk_level_tape = 0;
					}
					else
					{
						current_level = (cassette_input(devtag_get_device(machine, "cassette")) > 0.038) ? 1 : 0;

						if (previous_level!=current_level)
						{
							data = (!previous_level && current_level) ? 1 : 0;

							set_out_data_bit(pmd85_cassette_serial_connection.State, data);
							serial_connection_out(machine, &pmd85_cassette_serial_connection);
							msm8251_receive_clock(devtag_get_device(machine, "uart"));

							clk_level_tape = 1;
						}
					}
					return;
				case PMD85_2:
				case PMD85_2A:
				case C2717:
				case PMD85_3:
				case ALFA:
					/* not hardware data decoding */
					return;
			}
		}

		/* tape writing */
		if (cassette_get_state(devtag_get_device(machine, "cassette"))&CASSETTE_RECORD)
		{
			data = get_in_data_bit(pmd85_cassette_serial_connection.input_state);
			data ^= clk_level_tape;
			cassette_output(devtag_get_device(machine, "cassette"), data&0x01 ? 1 : -1);

			if (!clk_level_tape)
				msm8251_transmit_clock(devtag_get_device(machine, "uart"));

			clk_level_tape = clk_level_tape ? 0 : 1;

			return;
		}

		clk_level_tape = 1;

		if (!clk_level)
			msm8251_transmit_clock(devtag_get_device(machine, "uart"));
		clk_level = clk_level ? 0 : 1;
	}
}

static TIMER_CALLBACK( pmd_reset )
{
	mame_schedule_soft_reset(machine);
}

static DIRECT_UPDATE_HANDLER(pmd85_opbaseoverride)
{
	if (input_port_read(space->machine, "RESET") & 0x01) 
		timer_set(space->machine, ATTOTIME_IN_USEC(10), NULL, 0, pmd_reset);
	return address;
}

static void pmd85_common_driver_init (running_machine *machine)
{
	pmd85_cassette_timer = timer_alloc(machine, pmd85_cassette_timer_callback, NULL);
	timer_adjust_periodic(pmd85_cassette_timer, attotime_zero, 0, ATTOTIME_IN_HZ(2400));

	serial_connection_init(machine, &pmd85_cassette_serial_connection);
	serial_connection_set_in_callback(machine, &pmd85_cassette_serial_connection, pmd85_cassette_write);
}

DRIVER_INIT ( pmd851 )
{
	pmd85_model = PMD85_1;
	pmd85_update_memory = pmd851_update_memory;
	pmd85_common_driver_init(machine);
}

DRIVER_INIT ( pmd852a )
{
	pmd85_model = PMD85_2A;
	pmd85_update_memory = pmd852a_update_memory;
	pmd85_common_driver_init(machine);
}

DRIVER_INIT ( pmd853 )
{
	pmd85_model = PMD85_3;
	pmd85_update_memory = pmd853_update_memory;
	pmd85_common_driver_init(machine);
}

DRIVER_INIT ( alfa )
{
	pmd85_model = ALFA;
	pmd85_update_memory = alfa_update_memory;
	pmd85_common_driver_init(machine);
}

DRIVER_INIT ( mato )
{
	pmd85_model = MATO;
	pmd85_update_memory = mato_update_memory;
}

DRIVER_INIT ( c2717 )
{
	pmd85_model = C2717;
	pmd85_update_memory = c2717_update_memory;
	pmd85_common_driver_init(machine);
}

static TIMER_CALLBACK( setup_machine_state ) 
{
	const device_config *pit8253 = devtag_get_device(machine, "pit8253");

	pit8253_gate_w(pit8253, 0, 1);
	pit8253_gate_w(pit8253, 1, 1);
	pit8253_gate_w(pit8253, 2, 1);
	if (pmd85_model!=MATO) 
	{
		msm8251_connect(devtag_get_device(machine, "uart"), &pmd85_cassette_serial_connection);
	}
}


MACHINE_RESET( pmd85 )
{
	/* checking for Rom Module */
	switch (pmd85_model)
	{
		case PMD85_1:
		case PMD85_2A:
		case PMD85_3:
	  case C2717:
			pmd85_rom_module_present = (input_port_read(machine, "DSW0") & 0x01) ? 1 : 0;
			break;
		case ALFA:
		case MATO:
			break;
	}

	/* memory initialization */
	memset(mess_ram, 0, sizeof(unsigned char)*0x10000);
	pmd85_startup_mem_map = 1;
	pmd85_update_memory(machine);

	timer_set(machine,  attotime_zero, NULL, 0, setup_machine_state );

	memory_set_direct_update_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), pmd85_opbaseoverride);	
}
