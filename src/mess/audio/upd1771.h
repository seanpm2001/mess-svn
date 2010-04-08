/**********************************************************************

    NEC uPD1771

**********************************************************************/

#ifndef __UPD1771_H__
#define __UPD1771_H__

#include "devcb.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _upd1771_interface upd1771_interface;
struct _upd1771_interface
{
	devcb_write_line	ack_callback;
};


/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

#define SOUND_UPD1771C DEVICE_GET_INFO_NAME(upd1771c)


/***************************************************************************
    PROTOTYPES
***************************************************************************/

DEVICE_GET_INFO( upd1771c );
WRITE8_DEVICE_HANDLER( upd1771_w );
WRITE_LINE_DEVICE_HANDLER( upd1771_pcm_w );

#endif /* __UPD1771_H__ */
