//============================================================
//
//   sdlmain.c -- SDL main program
//
//============================================================

// standard sdl header
#include <SDL/SDL.h>
#include <SDL/SDL_version.h>

// standard includes
#include <time.h>
#include <ctype.h>
#include <stdlib.h>

// MAME headers
#include "driver.h"
#include "window.h"
#include "options.h"
#include "clifront.h"
#include "input.h"

#include "osdsdl.h"

// we override SDL's normal startup on Win32
#ifdef SDLMAME_WIN32
#undef main
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <io.h>
#endif

#ifdef SDLMAME_UNIX
#include <signal.h>
#include <unistd.h>
#endif

#if defined(SDLMAME_X11) && (SDL_MAJOR_VERSION == 1) && (SDL_MINOR_VERSION == 2)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

#ifdef SDLMAME_OS2
#define INCL_DOS
#include <os2.h>

void MorphToPM()
{
  PPIB pib;
  PTIB tib;

  DosGetInfoBlocks(&tib, &pib);

  // Change flag from VIO to PM:
  if (pib->pib_ultype==2) pib->pib_ultype = 3;
}
#endif

//============================================================
//	LOCAL VARIABLES
//============================================================

#ifdef MESS
static char cwd[512];
#endif

//============================================================
//  OPTIONS
//============================================================

static const options_entry mame_sdl_options[] =
{
#if defined(SDLMAME_WIN32) || defined(SDLMAME_MACOSX) || defined(SDLMAME_OS2)
	{ "inipath",                              ".;ini",     0,                 "path to ini files" },
#else
#if defined(INI_PATH)
	{ "inipath",                              INI_PATH,     0,                "path to ini files" },
#else
#ifdef MESS
	{ "inipath",                              "$HOME/.mess;.;ini",     0,     "path to ini files" },
#else
	{ SDLOPTION_INIPATH,                     "$HOME/.mame;.;ini",     0,     "path to ini files" },
#endif // MESS
#endif // INI_PATH
#endif // MACOSX
	

	// debugging options
	{ NULL,                                   NULL,       OPTION_HEADER,     "DEBUGGING OPTIONS" },
	{ SDLOPTION_OSLOG,                        "0",        OPTION_BOOLEAN,    "output error.log data to the system debugger" },

	// performance options
	{ NULL,                                   NULL,       OPTION_HEADER,     "PERFORMANCE OPTIONS" },
	{ SDLOPTION_MULTITHREADING ";mt",         "0",        OPTION_BOOLEAN,    "enable multithreading; this enables rendering and blitting on a separate thread" },
	{ SDLOPTION_SDLVIDEOFPS,                  "0",        OPTION_BOOLEAN,    "show sdl video performance" },

	// video options
	{ NULL,                                   NULL,       OPTION_HEADER,     "VIDEO OPTIONS" },
	{ SDLOPTION_VIDEO,                   SDLOPTVAL_SOFT,  0,                 "video output method: soft or opengl" },
	{ SDLOPTION_NUMSCREENS,                   "1",        0,                 "number of screens to create; SDLMAME only supports 1 at this time" },
	{ SDLOPTION_WINDOW ";w",                  "0",        OPTION_BOOLEAN,    "enable window mode; otherwise, full screen mode is assumed" },
	{ SDLOPTION_KEEPASPECT ";ka",             "1",        OPTION_BOOLEAN,    "constrain to the proper aspect ratio" },
	{ SDLOPTION_UNEVENSTRETCH ";ues",         "1",        OPTION_BOOLEAN,    "allow non-integer stretch factors" },
	{ SDLOPTION_EFFECT,                  SDLOPTVAL_NONE,  0,                 "name of a PNG file to use for visual effects, or 'none'" },
	{ SDLOPTION_CENTERH,                      "1",        OPTION_BOOLEAN,    "center horizontally within the view area" },
	{ SDLOPTION_CENTERV,                      "1",        OPTION_BOOLEAN,    "center vertically within the view area" },
	#if (SDL_VERSIONNUM(SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL) >= 1210)
	{ SDLOPTION_WAITVSYNC,                    "0",        OPTION_BOOLEAN,    "enable waiting for the start of VBLANK before flipping screens; reduces tearing effects" },
	#endif
	{ SDLOPTION_YUVMODE ";ym",           SDLOPTVAL_NONE,  0,                 "YUV mode: none, yv12, yuy2, yv12x2, yuy2x2 (-video soft only)" },

#if USE_OPENGL
	// OpenGL specific options
	{ NULL,                                   NULL,   OPTION_HEADER,  "OpenGL-SPECIFIC OPTIONS" },
	{ SDLOPTION_FILTER ";glfilter;flt",       "1",    OPTION_BOOLEAN, "enable bilinear filtering on screen output" },
	{ SDLOPTION_PRESCALE,                     "1",        0,                 "scale screen rendering by this amount in software" },
	{ SDLOPTION_GL_FORCEPOW2TEXTURE,          "0",    OPTION_BOOLEAN, "force power of two textures  (default no)" },
	{ SDLOPTION_GL_NOTEXTURERECT,             "0",    OPTION_BOOLEAN, "don't use OpenGL GL_ARB_texture_rectangle (default on)" },
	{ SDLOPTION_GL_VBO,                       "1",    OPTION_BOOLEAN, "enable OpenGL VBO,  if available (default on)" },
	{ SDLOPTION_GL_PBO,                       "1",    OPTION_BOOLEAN, "enable OpenGL PBO,  if available (default on)" },
	{ SDLOPTION_GL_GLSL,                      "0",    OPTION_BOOLEAN, "enable OpenGL GLSL, if available (default off)" },
 	{ SDLOPTION_GLSL_FILTER,				  "1",    0,              "enable OpenGL GLSL filtering instead of FF filtering 0-plain, 1-bilinear (default)" },
 	{ SDLOPTION_SHADER_MAME("0"),    SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader set mame bitmap 0" },
 	{ SDLOPTION_SHADER_MAME("1"),    SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader set mame bitmap 1" },
 	{ SDLOPTION_SHADER_MAME("2"),    SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader set mame bitmap 2" },
 	{ SDLOPTION_SHADER_MAME("3"),    SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader set mame bitmap 3" },
 	{ SDLOPTION_SHADER_MAME("4"),    SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader set mame bitmap 4" },
 	{ SDLOPTION_SHADER_MAME("5"),    SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader set mame bitmap 5" },
 	{ SDLOPTION_SHADER_MAME("6"),    SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader set mame bitmap 6" },
 	{ SDLOPTION_SHADER_MAME("7"),    SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader set mame bitmap 7" },
 	{ SDLOPTION_SHADER_MAME("8"),    SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader set mame bitmap 8" },
 	{ SDLOPTION_SHADER_MAME("9"),    SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader set mame bitmap 9" },
 	{ SDLOPTION_SHADER_SCREEN("0"),  SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader screen bitmap 0" },
 	{ SDLOPTION_SHADER_SCREEN("1"),  SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader screen bitmap 1" },
 	{ SDLOPTION_SHADER_SCREEN("2"),  SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader screen bitmap 2" },
 	{ SDLOPTION_SHADER_SCREEN("3"),  SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader screen bitmap 3" },
 	{ SDLOPTION_SHADER_SCREEN("4"),  SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader screen bitmap 4" },
 	{ SDLOPTION_SHADER_SCREEN("5"),  SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader screen bitmap 5" },
 	{ SDLOPTION_SHADER_SCREEN("6"),  SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader screen bitmap 6" },
 	{ SDLOPTION_SHADER_SCREEN("7"),  SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader screen bitmap 7" },
 	{ SDLOPTION_SHADER_SCREEN("8"),  SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader screen bitmap 8" },
 	{ SDLOPTION_SHADER_SCREEN("9"),  SDLOPTVAL_NONE,  0,              "custom OpenGL GLSL shader screen bitmap 9" },
 	{ SDLOPTION_GL_GLSL_VID_ATTR,			 "1",    OPTION_BOOLEAN,  "enable OpenGL GLSL handling of brightness and contrast. Better RGB game performance for free. (default)" },
#endif
 	
	// per-window options
	{ NULL,                                   NULL, OPTION_HEADER,    "PER-WINDOW VIDEO OPTIONS" },
	{ SDLOPTION_SCREEN(""),                   SDLOPTVAL_AUTO,   0,    "explicit name of the first screen; 'auto' here will try to make a best guess" },
	{ SDLOPTION_ASPECT("") ";screen_aspect",  SDLOPTVAL_AUTO,   0,    "aspect ratio for all screens; 'auto' here will try to make a best guess" },
	{ SDLOPTION_RESOLUTION("") ";r",          SDLOPTVAL_AUTO,   0,    "preferred resolution for all screens; format is <width>x<height>[@<refreshrate>] or 'auto'" },
	{ SDLOPTION_VIEW(""),                     SDLOPTVAL_AUTO,   0,    "preferred view for all screens" },

	{ SDLOPTION_SCREEN("0"),                  SDLOPTVAL_AUTO,   0,    "explicit name of the first screen; 'auto' here will try to make a best guess" },
	{ SDLOPTION_ASPECT("0"),                  SDLOPTVAL_AUTO,   0,    "aspect ratio of the first screen; 'auto' here will try to make a best guess" },
	{ SDLOPTION_RESOLUTION("0") ";r0",        SDLOPTVAL_AUTO,   0,    "preferred resolution of the first screen; format is <width>x<height>[@<refreshrate>] or 'auto'" },
	{ SDLOPTION_VIEW("0"),                    SDLOPTVAL_AUTO,   0,    "preferred view for the first screen" },

	{ SDLOPTION_SCREEN("1"),                  SDLOPTVAL_AUTO,   0,    "explicit name of the second screen; 'auto' here will try to make a best guess" },
	{ SDLOPTION_ASPECT("1"),                  SDLOPTVAL_AUTO,   0,    "aspect ratio of the second screen; 'auto' here will try to make a best guess" },
	{ SDLOPTION_RESOLUTION("1") ";r1",        SDLOPTVAL_AUTO,   0,    "preferred resolution of the second screen; format is <width>x<height>[@<refreshrate>] or 'auto'" },
	{ SDLOPTION_VIEW("1"),                    SDLOPTVAL_AUTO,   0,    "preferred view for the second screen" },

	{ SDLOPTION_SCREEN("2"),                  SDLOPTVAL_AUTO,   0,    "explicit name of the third screen; 'auto' here will try to make a best guess" },
	{ SDLOPTION_ASPECT("2"),                  SDLOPTVAL_AUTO,   0,    "aspect ratio of the third screen; 'auto' here will try to make a best guess" },
	{ SDLOPTION_RESOLUTION("2") ";r2",        SDLOPTVAL_AUTO,   0,    "preferred resolution of the third screen; format is <width>x<height>[@<refreshrate>] or 'auto'" },
	{ SDLOPTION_VIEW("2"),                    SDLOPTVAL_AUTO,   0,    "preferred view for the third screen" },

	{ SDLOPTION_SCREEN("3"),                  SDLOPTVAL_AUTO,   0,    "explicit name of the fourth screen; 'auto' here will try to make a best guess" },
	{ SDLOPTION_ASPECT("3"),                  SDLOPTVAL_AUTO,   0,    "aspect ratio of the fourth screen; 'auto' here will try to make a best guess" },
	{ SDLOPTION_RESOLUTION("3") ";r3",        SDLOPTVAL_AUTO,   0,    "preferred resolution of the fourth screen; format is <width>x<height>[@<refreshrate>] or 'auto'" },
	{ SDLOPTION_VIEW("3"),                    SDLOPTVAL_AUTO,   0,    "preferred view for the fourth screen" },

	// full screen options
	{ NULL,                                   NULL,  OPTION_HEADER,     "FULL SCREEN OPTIONS" },
	{ SDLOPTION_SWITCHRES,                    "0",   OPTION_BOOLEAN,    "enable resolution switching" },
	#ifdef SDLMAME_X11
	{ SDLOPTION_USEALLHEADS,	             "0",	  OPTION_BOOLEAN,    "split full screen image across monitors" },
	#endif

	// sound options
	{ NULL,                                   NULL,  OPTION_HEADER,     "SOUND OPTIONS" },
	{ SDLOPTION_AUDIO_LATENCY,                "3",   0,                 "set audio latency (increase to reduce glitches, decrease for responsiveness)" },

	// keyboard mapping
	{ NULL, 		                          NULL,  OPTION_HEADER,     "SDL KEYBOARD MAPPING" },
	{ SDLOPTION_KEYMAP,                      "0",    OPTION_BOOLEAN,    "enable keymap" },
	{ SDLOPTION_KEYMAP_FILE,                 "keymap.dat", 0,           "keymap filename" },
#ifdef MESS
#ifdef SDLMAME_MACOSX	// work around for SDL 1.2.11 on Mac - 1.2.12 should not require this
	{ SDLOPTION_UIMODEKEY,			"DELETE", 0,                  "Key to toggle MESS keyboard mode" },
#else
	{ SDLOPTION_UIMODEKEY,			"SCROLLOCK", 0,                  "Key to toggle MESS keyboard mode" },
#endif	// SDLMAME_MACOSX
#endif	// MESS

	// joystick mapping
	{ NULL, 		                         NULL,   OPTION_HEADER,     "SDL JOYSTICK MAPPING" },
	{ SDLOPTION_JOYMAP,                      "0",    OPTION_BOOLEAN,    "enable physical to logical joystick mapping" },
	{ SDLOPTION_JOYMAP_FILE,                "joymap.dat", 0,            "joymap filename" },
	{ SDLOPTION_SIXAXIS,			        "0",	 OPTION_BOOLEAN,    "Use special handling for PS3 Sixaxis controllers" },
	{ NULL }
};

#ifdef MESS
void sdl_mess_options_parse(void);
#endif

//============================================================
//	main
//============================================================

// we do some special sauce on Win32...

/* Show an error message */
static void ShowError(const char *title, const char *message)
{
	fprintf(stderr, "%s: %s\n", title, message);
}

#ifdef SDLMAME_WIN32
int SDL_main(int argc, char **argv);

static BOOL OutOfMemory(void)
{
	ShowError("Fatal Error", "Out of memory - aborting");
	return FALSE;
}

int main(int argc, char *argv[])
{
	size_t n;
	char *bufp, *appname;
	int status;

	/* Get the class name from argv[0] */
	appname = argv[0];
	if ( (bufp=SDL_strrchr(argv[0], '\\')) != NULL ) {
		appname = bufp+1;
	} else
	if ( (bufp=SDL_strrchr(argv[0], '/')) != NULL ) {
		appname = bufp+1;
	}

	if ( (bufp=SDL_strrchr(appname, '.')) == NULL )
		n = SDL_strlen(appname);
	else
		n = (bufp-appname);

	bufp = SDL_stack_alloc(char, n+1);
	if ( bufp == NULL ) {
		return OutOfMemory();
	}
	SDL_strlcpy(bufp, appname, n+1);
	appname = bufp;

	/* Load SDL dynamic link library */
	if ( SDL_Init(SDL_INIT_NOPARACHUTE) < 0 ) {
		ShowError("WinMain() error", SDL_GetError());
		return(FALSE);
	}


	/* Sam:
	   We still need to pass in the application handle so that
	   DirectInput will initialize properly when SDL_RegisterApp()
	   is called later in the video initialization.
	 */
	SDL_SetModuleHandle(GetModuleHandle(NULL));

	/* Run the application main() code */
	status = SDL_main(argc, argv);

	/* Exit cleanly, calling atexit() functions */
	exit(status);

	/* Hush little compiler, don't you cry... */
	return status;
}
#endif

#ifndef SDLMAME_WIN32
int main(int argc, char **argv)
#else
int SDL_main(int argc, char **argv)
#endif
{
	int res = 0;

	#ifdef SDLMAME_OS2
	MorphToPM();
	#endif

	#ifdef MESS
	getcwd(cwd, 511);
	#endif

#if defined(SDLMAME_X11) && (SDL_MAJOR_VERSION == 1) && (SDL_MINOR_VERSION == 2)
	if (SDL_Linked_Version()->patch < 10)
	/* workaround for SDL choosing a 32-bit ARGB visual */
	{
		Display *display;
		if ((display = XOpenDisplay(NULL)) && (DefaultDepth(display, DefaultScreen(display)) >= 24))
		{
			XVisualInfo vi;
			char buf[130];
			if (XMatchVisualInfo(display, DefaultScreen(display), 24, TrueColor, &vi)) {
				snprintf(buf, sizeof(buf), "0x%lx", vi.visualid);
				setenv(SDLENV_IDENTIFIER, buf, 0);
			}
		}
		if (display)
			XCloseDisplay(display);
	}
#endif 

	res = cli_execute(argc, argv, mame_sdl_options);

#ifdef MALLOC_DEBUG
	{
		void check_unfreed_mem(void);
		check_unfreed_mem();
	}
#endif

	SDL_Quit();
	
	exit(res);

	return res;
}



//============================================================
//  output_oslog
//============================================================

static void output_oslog(running_machine *machine, const char *buffer)
{
	fputs(buffer, stderr);
}



//============================================================
//	osd_exit
//============================================================

static void osd_exit(running_machine *machine)
{

	#ifndef SDLMAME_WIN32
	SDL_Quit();
	#endif
}

//============================================================
//	defines_verbose
//============================================================

#define MAC_EXPAND_STR(_m) #_m 
#define MACRO_VERBOSE(_mac) \
	do { \
		if (strcmp(MAC_EXPAND_STR(_mac), #_mac) != 0) \
			mame_printf_verbose("%s=%s ", #_mac, MAC_EXPAND_STR(_mac)); \
	} while (0)

#define _SDL_VER #SDL_MAJOR_VERSION "." #SDL_MINOR_VERSION "." #SDL_PATCHLEVEL

static void defines_verbose(void)
{
	mame_printf_verbose("Build version:      %s\n", build_version);
	mame_printf_verbose("Build architecure:  ");
	MACRO_VERBOSE(SDLMAME_ARCH);
	mame_printf_verbose("\n");
	mame_printf_verbose("Build defines:      ");
	MACRO_VERBOSE(SDLMAME_UNIX);
	MACRO_VERBOSE(SDLMAME_X11);
	MACRO_VERBOSE(SDLMAME_WIN32);
	MACRO_VERBOSE(SDLMAME_OS2);
	MACRO_VERBOSE(SDLMAME_MACOSX);
	MACRO_VERBOSE(SDLMAME_DARWIN);
	MACRO_VERBOSE(SDLMAME_LINUX);
	MACRO_VERBOSE(SDLMAME_SOLARIS);
	MACRO_VERBOSE(SDLMAME_IRIX);
	MACRO_VERBOSE(SDLMAME_FREEBSD);
	MACRO_VERBOSE(LSB_FIRST);
	MACRO_VERBOSE(PTR64);
	MACRO_VERBOSE(MAME_DEBUG);
	MACRO_VERBOSE(NDEBUG);
	MACRO_VERBOSE(ENABLE_DEBUGGER);
	MACRO_VERBOSE(VODOO_DRC);
	mame_printf_verbose("\n");
	mame_printf_verbose("SDL/OpenGL defines: ");
	mame_printf_verbose("SDL_COMPILEDVERSION=%d ", SDL_COMPILEDVERSION);
	MACRO_VERBOSE(USE_OPENGL);
	MACRO_VERBOSE(USE_DISPATCH_GL);
	mame_printf_verbose("\n");
	mame_printf_verbose("Compiler defines A: ");
	MACRO_VERBOSE(__GNUC__);
	MACRO_VERBOSE(__GNUC_MINOR__);
	MACRO_VERBOSE(__GNUC_PATCHLEVEL__);
	MACRO_VERBOSE(__VERSION__);
	mame_printf_verbose("\n");
	mame_printf_verbose("Compiler defines B: ");
	MACRO_VERBOSE(__amd64__);
	MACRO_VERBOSE(__x86_64__);
	MACRO_VERBOSE(__unix__);
	MACRO_VERBOSE(__i386__);
	MACRO_VERBOSE(__ppc__);
	MACRO_VERBOSE(__ppc64__);
	mame_printf_verbose("\n");
}

//============================================================
//	osd_init
//============================================================
void osd_init(running_machine *machine)
{
	#ifndef SDLMAME_WIN32
	if (SDL_Init(SDL_INIT_TIMER|SDL_INIT_AUDIO| SDL_INIT_VIDEO| SDL_INIT_JOYSTICK|SDL_INIT_NOPARACHUTE)) {
		/* mame_printf_* not fully initialized yet */
		ShowError("Could not initialize SDL: %s.\n", SDL_GetError());
		exit(-1);
	}
	#endif
	// must be before sdlvideo_init!
	add_exit_callback(machine, osd_exit);

	defines_verbose();
	
	if (sdlvideo_init(machine))
	{
		osd_exit(machine);
		ShowError("sdlvideo_init", "Initialization failed!\n\n\n");
		fflush(stderr);
		fflush(stdout);
		exit(-1);
	}
	if (sdl_use_unsupported())
		sdlled_init();

	#ifdef MESS
	sdl_mess_options_parse();
	#endif

	sdlinput_init(machine);
	sdlaudio_init(machine);

	if (options_get_bool(mame_options(), SDLOPTION_OSLOG))
		add_logerror_callback(machine, output_oslog);

	#ifdef MESS
	SDL_EnableUNICODE(1);
	#endif
}

#ifdef MESS
char *osd_get_startup_cwd(void)
{
	return cwd;
}
#endif
