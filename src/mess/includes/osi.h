#ifndef __OSI__
#define __OSI__

#define SCREEN_TAG		"screen"
#define M6502_TAG		"m6502"
#define CASSETTE_TAG	"cassette"

#define X1			3932160
#define UK101_X1	XTAL_8MHz

#define OSI600_VIDEORAM_SIZE	0x400
#define OSI630_COLORRAM_SIZE	0x400

class osi_state : public driver_device
{
public:
	osi_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* keyboard state */
	UINT8 m_keylatch;

	/* video state */
	int m_32;
	int m_coloren;
	UINT8 *m_video_ram;
	UINT8 *m_color_ram;

	/* floppy state */
	int m_fdc_index;

	/* devices */
	device_t *m_cassette;
};

/* ---------- defined in video/osi.c ---------- */

MACHINE_CONFIG_EXTERN( osi600_video );
MACHINE_CONFIG_EXTERN( uk101_video );
MACHINE_CONFIG_EXTERN( osi630_video );

#endif
