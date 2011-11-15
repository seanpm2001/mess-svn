/***************************************************************************

    coco12.c

    TRS-80 Radio Shack Color Computer 1/2 Family

***************************************************************************/

#include "includes/coco12.h"

//-------------------------------------------------
//  ctor
//-------------------------------------------------

coco12_state::coco12_state(const machine_config &mconfig, device_type type, const char *tag)
	: coco_state(mconfig, type, tag),
	  m_sam(*this, SAM_TAG),
	  m_vdg(*this, VDG_TAG)
{
}



//-------------------------------------------------
//  device_start
//-------------------------------------------------

void coco12_state::device_start()
{
	coco_state::device_start();
	configure_sam();
}



//-------------------------------------------------
//  configure_sam
//-------------------------------------------------

void coco12_state::configure_sam()
{
	cococart_slot_device *cart = m_cococart;
	UINT8 *ram = m_ram->pointer();
	UINT32 ram_size = m_ram->size();
	UINT8 *rom = machine().region(MAINCPU_TAG)->base();
	UINT8 *cart_rom = cart->get_cart_base();
	
	m_sam->configure_bank(0, ram, ram_size, false);			// $0000-$7FFF
	m_sam->configure_bank(1, &rom[0x0000], 0x2000, true);	// $8000-$9FFF
	m_sam->configure_bank(2, &rom[0x2000], 0x2000, true);	// $A000-$BFFF
	m_sam->configure_bank(3, cart_rom, 0x4000, true);		// $C000-$FEFF

	// $FF00-$FF1F
	m_sam->configure_bank(4, read8_delegate(FUNC(coco12_state::ff00_read), this), write8_delegate(FUNC(coco12_state::ff00_write), this));

	// $FF20-$FF3F
	m_sam->configure_bank(5, read8_delegate(FUNC(coco12_state::ff20_read), this), write8_delegate(FUNC(coco12_state::ff20_write), this));

	// $FF40-$FF5F
	m_sam->configure_bank(6, read8_delegate(FUNC(coco12_state::ff40_read), this), write8_delegate(FUNC(coco12_state::ff40_write), this));

	// $FF60-$FFBF
	m_sam->configure_bank(7, read8_delegate(FUNC(coco12_state::ff60_read), this), write8_delegate(FUNC(coco12_state::ff60_write), this));
}


//-------------------------------------------------
//  horizontal_sync
//-------------------------------------------------

WRITE_LINE_MEMBER( coco12_state::horizontal_sync )
{
	m_pia_0->ca1_w(state);
	m_sam->hs_w(state);
}



//-------------------------------------------------
//  field_sync
//-------------------------------------------------

WRITE_LINE_MEMBER( coco12_state::field_sync )
{
	m_pia_0->cb1_w(state);
}



//-------------------------------------------------
//  sam_read
//-------------------------------------------------

READ8_MEMBER( coco12_state::sam_read )
{
	UINT8 data = m_sam->mpu_address_space()->read_byte(offset);
	m_vdg->as_w(data & 0x80 ? ASSERT_LINE : CLEAR_LINE);
	m_vdg->inv_w(data & 0x40 ? ASSERT_LINE : CLEAR_LINE);
	return data;
}



//-------------------------------------------------
//  pia1_pb_changed
//-------------------------------------------------

void coco12_state::pia1_pb_changed(void)
{
	/* call inherited function */
	coco_state::pia1_pb_changed();

	UINT8 data = m_pia_1->b_output();
	m_vdg->css_w(data & 0x08);
	m_vdg->intext_w(data & 0x10);
	m_vdg->gm0_w(data & 0x10);
	m_vdg->gm1_w(data & 0x20);
	m_vdg->gm2_w(data & 0x40);
	m_vdg->ag_w(data & 0x80);
}



//-------------------------------------------------
//  screen_update
//-------------------------------------------------

bool coco12_state::screen_update(screen_device &screen, bitmap_t &bitmap, const rectangle &cliprect)
{
	return m_vdg->update(&bitmap, &cliprect);
}




//-------------------------------------------------
//  mc6847_config
//-------------------------------------------------

const mc6847_interface coco12_state::mc6847_config =
{
	SCREEN_TAG,
	DEVCB_DEVICE_MEMBER(SAM_TAG, sam6883_device, display_read),
	DEVCB_DRIVER_LINE_MEMBER(coco12_state, horizontal_sync),
	DEVCB_DRIVER_LINE_MEMBER(coco12_state, field_sync)
};



//-------------------------------------------------
//  sam6883_config
//-------------------------------------------------

const sam6883_interface coco12_state::sam6883_config =
{
	MAINCPU_TAG,
	AS_PROGRAM,
	DEVCB_DRIVER_MEMBER(coco12_state, sam_read)
};
