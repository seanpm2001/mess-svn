/*********************************************************************

	artworkx.h

	MESS specific artwork code

*********************************************************************/

#ifndef ARTWORKX_H
#define ARTWORKX_H

#include "fileio.h"
#include "palette.h"
#include "png.h"



/***************************************************************************

	Constants

***************************************************************************/

enum
{
	ARTWORK_CUSTTYPE_JOYSTICK = 0,
	ARTWORK_CUSTTYPE_KEYBOARD,
	ARTWORK_CUSTTYPE_MISC,
	ARTWORK_CUSTTYPE_INVALID
}



/***************************************************************************

	Type definitions

***************************************************************************/

struct inputform_customization
{
	UINT32 ipt;
	int x, y;
	int width, height;
};



/***************************************************************************

	Prototypes

***************************************************************************/

void artwork_use_device_art(mess_image *img, const char *defaultartfile);

int artwork_get_inputscreen_customizations(png_info *png, int cust_type,
	const char *section,
	struct inputform_customization *customizations,
	int customizations_length);



#endif /* ARTWORKX_H */
