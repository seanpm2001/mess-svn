/**********************************************************************

    Commodore 2031 Single Disk Drive emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/


#include "c2031.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6502_TAG		"ucd5"
#define M6522_0_TAG		"uab1"
#define M6522_1_TAG		"ucd4"
#define C64H156_TAG		"64h156"


enum
{
	LED_POWER = 0,
	LED_ACT
};



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C2031 = c2031_device_config::static_alloc_device_config;



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  c2031_device_config - constructor
//-------------------------------------------------

c2031_device_config::c2031_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
	: device_config(mconfig, static_alloc_device_config, "C2031", tag, owner, clock),
	  device_config_ieee488_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  static_alloc_device_config - allocate a new
//  configuration object
//-------------------------------------------------

device_config *c2031_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
{
	return global_alloc(c2031_device_config(mconfig, tag, owner, clock));
}


//-------------------------------------------------
//  alloc_device - allocate a new device object
//-------------------------------------------------

device_t *c2031_device_config::alloc_device(running_machine &machine) const
{
	return auto_alloc(machine, c2031_device(machine, *this));
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void c2031_device_config::device_config_complete()
{
	m_shortname = "c2031";
}


//-------------------------------------------------
//  static_set_config - configuration helper
//-------------------------------------------------

void c2031_device_config::static_set_config(device_config *device, int address)
{
	c2031_device_config *c2031 = downcast<c2031_device_config *>(device);

	assert((address > 7) && (address < 12));

	c2031->m_address = address;
}


//-------------------------------------------------
//  ROM( c2031 )
//-------------------------------------------------

ROM_START( c2031 )
	ROM_REGION( 0x4000, M6502_TAG, 0 )
	ROM_LOAD( "901484-03.u5f", 0x0000, 0x2000, CRC(ee4b893b) SHA1(54d608f7f07860f24186749f21c96724dd48bc50) )
	ROM_LOAD( "901484-05.u5h", 0x2000, 0x2000, CRC(6a629054) SHA1(ec6b75ecfdd4744e5d57979ef6af990444c11ae1) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *c2031_device_config::device_rom_region() const
{
	return ROM_NAME( c2031 );
}


//-------------------------------------------------
//  ADDRESS_MAP( c2031_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( c2031_mem, AS_PROGRAM, 8, c2031_device )
	AM_RANGE(0x0000, 0x07ff) AM_MIRROR(0x6000) AM_RAM
	AM_RANGE(0x1800, 0x180f) AM_MIRROR(0x63f0) AM_DEVREADWRITE(M6522_0_TAG, via6522_device, read, write)
	AM_RANGE(0x1c00, 0x1c0f) AM_MIRROR(0x63f0) AM_DEVREADWRITE(M6522_1_TAG, via6522_device, read, write)
	AM_RANGE(0x8000, 0xbfff) AM_MIRROR(0x4000) // AM_ROM
ADDRESS_MAP_END


//-------------------------------------------------
//  via6522_interface via0_intf
//-------------------------------------------------

WRITE_LINE_MEMBER( c2031_device::via0_irq_w )
{
	m_via0_irq = state;

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, (m_via0_irq | m_via1_irq) ? ASSERT_LINE : CLEAR_LINE);
}


READ8_MEMBER( c2031_device::via0_pa_r )
{
	/*

        bit     description

        PA0     DI0
        PA1     DI1
        PA2     DI2
        PA3     DI3
        PA4     DI4
        PA5     DI5
        PA6     DI6
        PA7     DI7

    */

	return m_bus->dio_r();
}


WRITE8_MEMBER( c2031_device::via0_pa_w )
{
	/*

        bit     description

        PA0     DI0
        PA1     DI1
        PA2     DI2
        PA3     DI3
        PA4     DI4
        PA5     DI5
        PA6     DI6
        PA7     DI7

    */

	m_bus->dio_w(this, data);
}


READ8_MEMBER( c2031_device::via0_pb_r )
{
	/*

        bit     description

        PB0     ATNA
        PB1     NRFD
        PB2     NDAC
        PB3     EOI
        PB4     T/_R
        PB5     HD SEL
        PB6     DAV
        PB7     _ATN

    */

	UINT8 data = 0;

	// not ready for data
	data |= m_bus->nrfd_r() << 1;

	// not data accepted
	data |= m_bus->ndac_r() << 2;

	// end or identify
	data |= m_bus->eoi_r() << 3;

	// data valid
	data |= m_bus->dav_r() << 6;

	// attention
	data |= !m_bus->atn_r() << 7;

	return data;
}


WRITE8_MEMBER( c2031_device::via0_pb_w )
{
	/*

        bit     description

        PB0     ATNA
        PB1     NRFD
        PB2     NDAC
        PB3     EOI
        PB4     T/_R
        PB5     HD SEL
        PB6     DAV
        PB7     _ATN

    */

	int atna = BIT(data, 0);
	int nrfd = BIT(data, 1);
	int ndac = BIT(data, 2);

	// not ready for data
	m_nrfd_out = nrfd;

	// not data accepted
	m_ndac_out = ndac;

	// end or identify
	m_bus->eoi_w(this, BIT(data, 3));

	// data valid
	m_bus->dav_w(this, BIT(data, 6));

	// attention acknowledge
	m_atna = atna;

	if (!m_bus->atn_r() ^ atna)
	{
		nrfd = ndac = 0;
	}

	m_bus->nrfd_w(this, nrfd);
	m_bus->ndac_w(this, ndac);

	m_via0->write_ca2(get_device_number());
}


READ_LINE_MEMBER( c2031_device::via0_ca1_r )
{
	return !m_bus->atn_r();
}


READ_LINE_MEMBER( c2031_device::via0_ca2_r )
{
	return get_device_number();
}


static const via6522_interface via0_intf =
{
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, c2031_device, via0_pa_r),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, c2031_device, via0_pb_r),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, c2031_device, via0_ca1_r),
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, c2031_device, via0_ca2_r),
	DEVCB_NULL,

	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, c2031_device, via0_pa_w),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, c2031_device, via0_pb_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL, // PLL SYN

	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, c2031_device, via0_irq_w)
};


//-------------------------------------------------
//  via6522_interface via1_intf
//-------------------------------------------------

WRITE_LINE_MEMBER( c2031_device::via1_irq_w )
{
	m_via1_irq = state;

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, (m_via0_irq | m_via1_irq) ? ASSERT_LINE : CLEAR_LINE);
}


READ8_MEMBER( c2031_device::via1_pb_r )
{
	/*

        bit     signal      description

        PB0     
        PB1     
        PB2     
        PB3     
        PB4     WPS         write protect sense
        PB5     
        PB6     
        PB7     SYNC        SYNC detect line

    */

	UINT8 data = 0;

	// write protect sense
	data |= !floppy_wpt_r(m_image) << 4;

	// SYNC detect line
	data |= m_ga->sync_r() << 7;

	return data;
}


WRITE8_MEMBER( c2031_device::via1_pb_w )
{
	/*

        bit     signal      description

        PB0     STP0        stepping motor bit 0
        PB1     STP1        stepping motor bit 1
        PB2     MTR         motor ON/OFF
        PB3     ACT         drive 0 LED
        PB4     
        PB5     DS0         density select 0
        PB6     DS1         density select 1
        PB7     SYNC        SYNC detect line

    */

	// spindle motor
	m_ga->mtr_w(BIT(data, 2));

	// stepper motor
	m_ga->stp_w(data & 0x03);

	// activity LED
	output_set_led_value(LED_ACT, BIT(data, 3));

	// density select
	m_ga->ds_w((data >> 5) & 0x03);
}


static const via6522_interface via1_intf =
{
	DEVCB_DEVICE_MEMBER(C64H156_TAG, c64h156_device, yb_r),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, c2031_device, via1_pb_r),
	DEVCB_DEVICE_LINE_MEMBER(C64H156_TAG, c64h156_device, byte_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_DEVICE_MEMBER(C64H156_TAG, c64h156_device, yb_w),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, c2031_device, via1_pb_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER(C64H156_TAG, c64h156_device, soe_w),
	DEVCB_DEVICE_LINE_MEMBER(C64H156_TAG, c64h156_device, oe_w),

	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, c2031_device, via1_irq_w)
};


//-------------------------------------------------
//  C64H156_INTERFACE( ga_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( c2031_device::byte_w )
{
	m_maincpu->set_input_line(M6502_SET_OVERFLOW, state);
	m_via1->write_ca1(state);
}


static C64H156_INTERFACE( ga_intf )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, c2031_device, byte_w)
};



//-------------------------------------------------
//  FLOPPY_OPTIONS( c2031 )
//-------------------------------------------------

static FLOPPY_OPTIONS_START( c2031 )
	FLOPPY_OPTION( c2031, "g64", "Commodore 1541 GCR Disk Image", g64_dsk_identify, g64_dsk_construct, NULL )
	FLOPPY_OPTION( c2031, "d64", "Commodore 1541 Disk Image", d64_dsk_identify, d64_dsk_construct, NULL )
FLOPPY_OPTIONS_END


//-------------------------------------------------
//  floppy_config c2031_floppy_config
//-------------------------------------------------

static const floppy_config c2031_floppy_config =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_DSDD,
	FLOPPY_OPTIONS_NAME(c2031),
	"floppy_5_25"
};


//-------------------------------------------------
//  MACHINE_DRIVER( c2031 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c2031 )
	MCFG_CPU_ADD(M6502_TAG, M6502, XTAL_16MHz/16)
	MCFG_CPU_PROGRAM_MAP(c2031_mem)

	MCFG_VIA6522_ADD(M6522_0_TAG, XTAL_16MHz/16, via0_intf)
	MCFG_VIA6522_ADD(M6522_1_TAG, XTAL_16MHz/16, via1_intf)

	MCFG_FLOPPY_DRIVE_ADD(FLOPPY_0, c2031_floppy_config)
	MCFG_64H156_ADD(C64H156_TAG, XTAL_16MHz, ga_intf)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor c2031_device_config::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( c2031 );
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  get_device_number - 
//-------------------------------------------------

inline int c2031_device::get_device_number()
{
	int state = 1;

	switch (m_config.m_address)
	{
	case 8: state = (m_atna & m_nrfd_out);	break;
	case 9: state = m_nrfd_out;				break;
	case 10: state = m_atna;				break;
	case 11: state = 1;						break;
	}

	return state;
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c2031_device - constructor
//-------------------------------------------------

c2031_device::c2031_device(running_machine &_machine, const c2031_device_config &_config)
    : device_t(_machine, _config),
	  device_ieee488_interface(_machine, _config, *this),
	  m_maincpu(*this, M6502_TAG),
	  m_via0(*this, M6522_0_TAG),
	  m_via1(*this, M6522_1_TAG),
	  m_ga(*this, C64H156_TAG),
	  m_image(*this, FLOPPY_0),
	  m_bus(*this->owner(), IEEE488_TAG),
	  m_nrfd_out(1),
	  m_ndac_out(1),
	  m_atna(1),
	  m_via0_irq(0),
	  m_via1_irq(0),
      m_config(_config)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c2031_device::device_start()
{
	// map ROM
	address_space *program = m_maincpu->memory().space(AS_PROGRAM);
	program->install_rom(0x8000, 0xbfff, 0, 0x4000, subregion(M6502_TAG)->base());

	// install image callbacks
	floppy_install_unload_proc(m_image, c2031_device::on_disk_change);
	floppy_install_load_proc(m_image, c2031_device::on_disk_change);

	// register for state saving
	save_item(NAME(m_nrfd_out));
	save_item(NAME(m_ndac_out));
	save_item(NAME(m_atna));
	save_item(NAME(m_via0_irq));
	save_item(NAME(m_via1_irq));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c2031_device::device_reset()
{
}


//-------------------------------------------------
//  ieee488_atn_w - 
//-------------------------------------------------

void c2031_device::ieee488_atn(int state)
{
	int nrfd = m_nrfd_out;
	int ndac = m_ndac_out;

	m_via0->write_ca1(!state);

	if (!state ^ m_atna)
	{
		nrfd = ndac = 0;
	}

	m_bus->nrfd_w(this, nrfd);
	m_bus->ndac_w(this, ndac);
}


//-------------------------------------------------
//  ieee488_ifc_w - 
//-------------------------------------------------

void c2031_device::ieee488_ifc(int state)
{
	if (!state)
	{
		device_reset();
	}
}


//-------------------------------------------------
//  on_disk_change - 
//-------------------------------------------------

void c2031_device::on_disk_change(device_image_interface &image)
{
    c2031_device *c2031 = static_cast<c2031_device *>(image.device().owner());

	c2031->m_ga->on_disk_changed();
}