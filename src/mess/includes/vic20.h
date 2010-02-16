#ifndef __VIC20__
#define __VIC20__

#define SCREEN_TAG		"screen"
#define M6502_TAG		"ue10"
#define M6522_0_TAG		"uab3"
#define M6522_1_TAG		"uab1"
#define M6560_TAG		"ub7"
#define M6561_TAG		"ub7"
#define C1540_TAG		"c1540"
#define C2031_TAG		"c2031"
#define IEC_TAG			"iec"
#define IEEE488_TAG		"ieee488"
#define CASSETTE_TAG	"cassette"
#define TIMER_C1530_TAG	"c1530"

typedef struct _vic20_state vic20_state;
struct _vic20_state
{
	/* keyboard state */
	int key_col;

	/* devices */
	running_device *via0;
	running_device *via1;
	running_device *iec;
	running_device *cassette;
	running_device *vic6560;

	/* timers */
	running_device *cassette_timer;
};

#endif
