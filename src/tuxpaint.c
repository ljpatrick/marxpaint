/*
  tuxpaint.c
  
  Tux Paint - A simple drawing program for children.
  
  Copyright (c) 2004 by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/tuxpaint/

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
  
  June 14, 2002 - December 12, 2004
*/


#define VER_VERSION     "0.9.15"
#define VER_DATE        "2004-12-12"


//#define VIDEO_BPP 15 // saves memory
//#define VIDEO_BPP 16 // causes discoloration
//#define VIDEO_BPP 24 // compromise
#define VIDEO_BPP 32 // might be fastest, if conversion funcs removed


/* #define DEBUG */
/* #define LOW_QUALITY_THUMBNAILS */
/* #define LOW_QUALITY_COLOR_SELECTOR */
/* #define LOW_QUALITY_STAMP_OUTLINE */
/* #define LOW_QUALITY_FLOOD_FILL */
/* #define NO_PROMPT_SHADOWS */
/* #define USE_HWSURFACE */

/* Use high quality 4x filter when scaling stamps up: */
/* #define USE_HQ4X */


/* Disable fancy cursors in fullscreen mode, to avoid SDL bug: */
#define LARGE_CURSOR_FULLSCREEN_BUG

#define HEIGHTOFFSET (((WINDOW_HEIGHT - 480) / 48) * 48)
#define TOOLOFFSET (HEIGHTOFFSET / 48 * 2)
#define PROMPTOFFSETX (WINDOW_WIDTH - 640) / 2
#define PROMPTOFFSETY (HEIGHTOFFSET / 2)

// control the color selector
#define COLORSEL_DISABLE 0  // disable and draw the (greyed out) colors
#define COLORSEL_ENABLE  1  // enable and draw the colors
#define COLORSEL_CLOBBER 2  // colors get scribbled over
#define COLORSEL_REFRESH 4  // redraw the colors, either on or off

static void draw_colors(int action);

/////////////////////////////////////////////////////////////////////
// hide all scale-related values here

typedef struct scaleparams {
  unsigned numer, denom;
} scaleparams;
static scaleparams scaletable[] = {
  {  1,256}, //  0.00390625
  {  3,512}, //  0.005859375
  {  1,128}, //  0.0078125
  {  3,256}, //  0.01171875
  {  1, 64}, //  0.015625
  {  3,128}, //  0.0234375
  {  1, 32}, //  0.03125
  {  3, 64}, //  0.046875
  {  1, 16}, //  0.0625
  {  3, 32}, //  0.09375
  {  1,  8}, //  0.125
  {  3, 16}, //  0.1875
  {  1,  4}, //  0.25
  {  3,  8}, //  0.375
  {  1,  2}, //  0.5
  {  3,  4}, //  0.75
  {  1,  1}, //  1
  {  3,  2}, //  1.5
  {  2,  1}, //  2
  {  3,  1}, //  3
  {  4,  1}, //  4
  {  6,  1}, //  6
  {  8,  1}, //  8
  { 12,  1}, // 12
  { 16,  1}, // 16
  { 24,  1}, // 24
  { 32,  1}, // 32
  { 48,  1}, // 48
};

#define HARD_MIN_STAMP_SIZE 0  // bottom of scaletable
#define HARD_MAX_STAMP_SIZE (sizeof scaletable / sizeof scaletable[0] - 1)

#define MIN_STAMP_SIZE (state_stamps[cur_stamp]->min)
#define MAX_STAMP_SIZE (state_stamps[cur_stamp]->max)

#define CAN_USE_HQ4X (scaletable[state_stamps[cur_stamp]->size] == (scaleparams){4,1})
// to scale some offset, in pixels, like the current stamp is scaled
#define SCALE_LIKE_STAMP(x) ( ((x) * scaletable[state_stamps[cur_stamp]->size].numer + scaletable[state_stamps[cur_stamp]->size].denom-1) / scaletable[state_stamps[cur_stamp]->size].denom )
// pixel dimensions of the current stamp, as scaled
#define CUR_STAMP_W SCALE_LIKE_STAMP(img_stamps[cur_stamp]->w)
#define CUR_STAMP_H SCALE_LIKE_STAMP(img_stamps[cur_stamp]->h)

///////////////////////////////////////////////////////////////////////////////


// #define MAX_FILES 2048  /* Max. # of files in a dir. to worry about... */

#define REPEAT_SPEED 300  /* Initial repeat speed for scrollbars */
#define CURSOR_BLINK_SPEED 500  /* Initial repeat speed for cursor */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

#ifdef USE_HQ4X
#include "hqxx.h"
#include "hq3x.h"
#include "hq4x.h"
#endif

#include <locale.h>
#include <iconv.h>

#ifndef OLD_UPPERCASE_CODE
#include <wctype.h>
#endif

#ifdef WIN32_OLD
/* The following are required by libintl.h, so must be defined first: */
#define LC_MESSAGES       1729
#define HAVE_LC_MESSAGES  1
#define ENABLE_NLS        1
#define HAVE_LOCALE_H     1
#define HAVE_GETTEXT      1
#define HAVE_DCGETTEXT    1
#endif

#if    defined(sun) && defined(__svr4__)
/* Solaris needs locale.h */
#endif

#include <libintl.h>
#ifndef gettext_noop
#define gettext_noop(String) String
#endif


#ifdef DEBUG
#define gettext(String) debug_gettext(String)
#endif


#ifndef M_PI
#define M_PI 3.14159265
#endif

#include <sys/types.h>
#include <sys/stat.h>



#ifndef WIN32
#include <unistd.h>
#include <dirent.h>
#ifdef __BEOS__
#include "BeOS_print.h"
// workaround dirent handling bug in TuxPaint code
typedef struct safer_dirent {
       dev_t                   d_dev;
       dev_t                   d_pdev;
       ino_t                   d_ino;
       ino_t                   d_pino;
       unsigned short  d_reclen;
       char                    d_name[NAME_MAX];
} safer_dirent;
#define dirent safer_dirent
#endif
#ifdef __APPLE__
#include "macosx_print.h"
#endif
#else
#include "win32_dirent.h"
#include "win32_print.h"
#include <io.h>
#include <direct.h>


/* Enables win32 apps to get a GNU compatible locale string */
extern char* g_win32_getlocale(void);

/* Set this to 0 during developement and testing in Visual-Studio
   Set this to 1 to make the final executable   */

#if 1

#define DOC_PREFIX "docs/"
#define DATA_PREFIX "data/"
#define LOCALEDIR "locale"

#else

#define DOC_PREFIX  "../../docs/"
#define DATA_PREFIX "../../data/"
#define LOCALEDIR   "../../locale"

#endif /* 1/0 */

#define mkdir(path,access)    _mkdir(path)
#define strcasecmp            stricmp
#define strncasecmp           strnicmp
#define snprintf              _snprintf
#define S_ISDIR(i)            ((i&_S_IFDIR)!=0)
#define alloca                _alloca

#endif /* WIN32 */

#include <errno.h>
#include <sys/stat.h>

#include "SDL.h"
#ifndef _SDL_H
#error "---------------------------------------------------"
#error "If you installed SDL from a package, be sure to get"
#error "the development package, as well!"
#error "(e.g., 'libsdl1.2-devel.rpm')"
#error "---------------------------------------------------"
#endif

#include "SDL_image.h"
#ifndef _IMG_h
#error "---------------------------------------------------"
#error "If you installed SDL_image from a package, be sure"
#error "to get the development package, as well!"
#error "(e.g., 'libsdl-image1.2-devel.rpm')"
#error "---------------------------------------------------"
#endif

#include "SDL_ttf.h"
#ifndef _SDLttf_h
#error "---------------------------------------------------"
#error "If you installed SDL_ttf from a package, be sure"
#error "to get the development package, as well!"
#error "(e.g., 'libsdl-ttf1.2-devel.rpm')"
#error "---------------------------------------------------"
#endif

#ifndef NOSOUND
#include "SDL_mixer.h"
#ifndef _MIXER_H_
#error "---------------------------------------------------"
#error "If you installed SDL_mixer from a package, be sure"
#error "to get the development package, as well!"
#error "(e.g., 'libsdl-mixer1.2-devel.rpm')"
#error "---------------------------------------------------"
#endif
#endif

#ifndef SAVE_AS_BMP
#include <png.h>
#define FNAME_EXTENSION ".png"
#ifndef PNG_H
#error "---------------------------------------------------"
#error "If you installed the PNG libraries from a package,"
#error "be sure to get the development package, as well!"
#error "(e.g., 'libpng2-devel.rpm')"
#error "---------------------------------------------------"
#endif
#else
#define FNAME_EXTENSION ".bmp"
#endif

#define THUMB_W ((WINDOW_WIDTH - 96 - 96) / 4)
#define THUMB_H (((48 * 7 + 40 + HEIGHTOFFSET) - 72) / 4)

#include "tools.h"
#include "titles.h"
#include "colors.h"
#include "shapes.h"
#include "magic.h"
#include "sounds.h"
#include "tip_tux.h"
#include "great.h"


#include "watch.xbm"
#include "watch-mask.xbm"

#include "hand.xbm"
#include "hand-mask.xbm"

#include "wand.xbm"
#include "wand-mask.xbm"

#include "insertion.xbm"
#include "insertion-mask.xbm"

#include "brush.xbm"
#include "brush-mask.xbm"

#include "crosshair.xbm"
#include "crosshair-mask.xbm"

#include "rotate.xbm"
#include "rotate-mask.xbm"

#include "up.xbm"
#include "up-mask.xbm"

#include "down.xbm"
#include "down-mask.xbm"

#include "tiny.xbm"
#include "tiny-mask.xbm"

#include "arrow.xbm"
#include "arrow-mask.xbm"



#ifdef WIN32
/*
  The SDL stderr redirection trick doesn't seem to work for perror().
  This does pretty much the same thing.
*/
static void win32_perror(const char * const str)
{
  if ( str && *str )
    fprintf(stderr,"%s : ",str);
  fprintf(stderr,
	  "%s [%d]\n",
	  (errno<_sys_nerr)?_sys_errlist[errno]:"unknown",errno );
}
#define perror         win32_perror
#endif


#ifndef WIN32
#ifdef __GNUC__
// This version has strict type checking for safety.
// See the "unnecessary" pointer comparison. (from Linux)
#define min(x,y) ({ \
  typeof(x) _x = (x);     \
  typeof(y) _y = (y);     \
  (void) (&_x == &_y);            \
  _x < _y ? _x : _y; })
#define max(x,y) ({ \
  typeof(x) _x = (x);     \
  typeof(y) _y = (y);     \
  (void) (&_x == &_y);            \
  _x > _y ? _x : _y; })
#else
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#endif

#define clamp(lo,value,hi)    (min(max(value,lo),hi))


// since gcc-2.5
#ifdef __GNUC__
#define NORETURN __attribute__((__noreturn__))
#define FUNCTION __attribute__((__const__))  // no access to global mem, even via ptr, and no side effect
#else
#define NORETURN
#define FUNCTION
#endif

#if !defined(restrict) && __STDC_VERSION__ < 199901
#if __GNUC__ > 2 || __GNUC_MINOR__ >= 92
#define restrict __restrict__
#else
#warning No restrict keyword?
#define restrict
#endif
#endif

#if __GNUC__ > 2 || __GNUC_MINOR__ >= 96
// won't alias anything, and aligned enough for anything
#define MALLOC __attribute__ ((__malloc__))
// no side effect, may read globals
#define PURE __attribute__ ((__pure__))
// tell gcc what to expect:   if(unlikely(err)) die(err);
#define likely(x)       __builtin_expect(!!(x),1)
#define unlikely(x)     __builtin_expect(!!(x),0)
#define expected(x,y)   __builtin_expect((x),(y))
#else
#define MALLOC
#define PURE
#define likely(x)       (x)
#define unlikely(x)     (x)
#define expected(x,y)   (x)
#endif


/* Unfortunately, there is a bug in SDL_ttf-2.0.6, the current version
   that causes a segmentation fault if an attempt is made to call 
   TTF_OpenFont() with the filename of a font that doesn't exist. This 
   is an old and well documented bug that is fixed in CVS.
*/
static TTF_Font *BUGFIX_TTF_OpenFont206(const char * const file, int ptsize)
{
    FILE    *fp;

    if ((fp = fopen(file, "rb")) == NULL) return NULL;
    fclose(fp);
    return TTF_OpenFont(file, ptsize);
}
#define TTF_OpenFont    BUGFIX_TTF_OpenFont206


#if VIDEO_BPP==32
#ifdef __GNUC__
#define SDL_GetRGBA(p,f,rp,gp,bp,ap) ({ \
  unsigned u_p = p;                     \
  *(ap) = (u_p >> 24) & 0xff;           \
  *(rp) = (u_p >> 16) & 0xff;           \
  *(gp) = (u_p >>  8) & 0xff;           \
  *(bp) = (u_p >>  0) & 0xff;           \
})
#define SDL_GetRGB(p,f,rp,gp,bp) ({ \
  unsigned u_p = p;                     \
  *(rp) = (u_p >> 16) & 0xff;           \
  *(gp) = (u_p >>  8) & 0xff;           \
  *(bp) = (u_p >>  0) & 0xff;           \
})
#endif
#define SDL_MapRGBA(f,r,g,b,a) ( \
  (((a) & 0xffu) << 24)          \
  |                              \
  (((r) & 0xffu) << 16)          \
  |                              \
  (((g) & 0xffu) <<  8)          \
  |                              \
  (((b) & 0xffu) <<  0)          \
)
#define SDL_MapRGB(f,r,g,b) (   \
  (((r) & 0xffu) << 16)          \
  |                              \
  (((g) & 0xffu) <<  8)          \
  |                              \
  (((b) & 0xffu) <<  0)          \
)
#endif

/* Possible languages: */

enum {
  LANG_AF,     /* Afrikaans */
  LANG_BE,     /* Belarusian */
  LANG_BG,     /* Bulgarian */
  LANG_BR,     /* Breton */
  LANG_CA,     /* Catalan */
  LANG_CS,     /* Czech */
  LANG_CY,     /* Welsh */
  LANG_DA,     /* Danish */
  LANG_DE,     /* German */
  LANG_EL,     /* Greek */
  LANG_EN,     /* English (American) (DEFAULT) */
  LANG_EN_GB,  /* English (British) */
  LANG_ES,     /* Spanish */
  LANG_EU,     /* Basque */
  LANG_FI,     /* Finnish */
  LANG_FR,     /* French */
  LANG_GL,     /* Galician */
  LANG_HE,     /* Hebrew */
  LANG_HI,     /* Hindi */
  LANG_HR,     /* Croatian */
  LANG_HU,     /* Hungarian */
  LANG_I_KLINGON_ROMANIZED,     /* Klingon (Romanized) */
  LANG_ID,     /* Indonesian */
  LANG_IS,     /* Icelandic */
  LANG_IT,     /* Italian */
  LANG_JA,     /* Japanese */
  LANG_KO,     /* Korean */
  LANG_LT,     /* Lithuanian */
  LANG_MS,     /* Malay */
  LANG_NB,     /* Norwegian Bokmal */
  LANG_NL,     /* Dutch */
  LANG_NN,     /* Norwegian Nynorsk */
  LANG_PL,     /* Polish */
  LANG_PT_BR,  /* Portuguese (Brazilian) */
  LANG_PT_PT,     /* Portuguese (Portugal) */
  LANG_RO,     /* Romanian */
  LANG_RU,     /* Russian */
  LANG_SK,     /* Slovak */
  LANG_SL,     /* Slovenian */
  LANG_SQ,     /* Albanian */
  LANG_SR,     /* Serbian */
  LANG_SV,     /* Swedish */
  LANG_TA,     /* Tamil */
  LANG_TR,     /* Turkish */
  LANG_VI,     /* Vietnamese */
  LANG_WA,     /* Walloon */
  LANG_ZH_CN,  /* Chinese (Simplified) */
  LANG_ZH_TW,  /* Chinese (Traditional) */
  NUM_LANGS
};

static const char * lang_prefixes[NUM_LANGS] = {
  "af",
  "be",
  "bg",
  "br",
  "ca",
  "cs",
  "cy",
  "da",
  "de",
  "el",
  "en",
  "en_gb",
  "es",
  "eu",
  "fi",
  "fr",
  "gl",
  "he",
  "hi",
  "hr",
  "hu",
  "tlh",
  "id",
  "is",
  "it",
  "ja",
  "ko",
  "lt",
  "ms",
  "nb",
  "nl",
  "nn",
  "pl",
  "pt_br",
  "pt_pt",
  "ro",
  "ru",
  "sk",
  "sl",
  "sq",
  "sr",
  "sv",
  "ta",
  "tr",
  "vi",
  "wa",
  "zh_cn",
  "zh_tw"
};


/* List of languages which doesn't use the default font: */


static int lang_use_own_font[] = {
  LANG_EL,
  LANG_HE,
  LANG_HI,
  LANG_JA,
  LANG_KO,
  LANG_TA,
  LANG_ZH_CN,
  LANG_ZH_TW,
  -1
};

static int lang_use_right_to_left[] = {
  LANG_HE,
  -1
};


typedef struct info_type {
  double ratio;
  unsigned tinter;
  int colorable;
  int tintable;
  int mirrorable;
  int flipable;
} info_type;


typedef struct state_type {
  int mirrored;
  int flipped;
  unsigned min,size,max;
} state_type;


enum {
  SAVE_OVER_PROMPT,
  SAVE_OVER_ALWAYS,
  SAVE_OVER_NO
};


enum {
  STARTER_OUTLINE,
  STARTER_SCENE
};


/* Globals: */

static int use_sound, fullscreen, disable_quit, simple_shapes, language,
  disable_print, print_delay, only_uppercase, promptless_save, grab_input,
  wheely, no_fancy_cursors, keymouse, mouse_x, mouse_y,
  mousekey_up, mousekey_down, mousekey_left, mousekey_right,
  dont_do_xor, use_print_config, dont_load_stamps, noshortcuts,
  mirrorstamps, disable_stamp_controls, disable_save, ok_to_use_lockfile;
static int recording, playing;
static char * playfile;
static FILE * demofi;
static int WINDOW_WIDTH, WINDOW_HEIGHT;
static const char * printcommand;
static int prog_bar_ctr;
static SDL_Surface * screen;

static SDL_Surface * canvas;
static SDL_Surface * img_starter, * img_starter_bkgd;
static int starter_mirrored, starter_flipped;

enum {
  UNDO_STARTER_NONE,
  UNDO_STARTER_MIRRORED,
  UNDO_STARTER_FLIPPED
};

#define NUM_UNDO_BUFS 20
static SDL_Surface * undo_bufs[NUM_UNDO_BUFS];
static int undo_starters[NUM_UNDO_BUFS];
static int cur_undo, oldest_undo, newest_undo;

static SDL_Surface * img_title, * img_progress;
static SDL_Surface * img_btn_up, * img_btn_down, * img_btn_off;
static SDL_Surface * img_yes, * img_no;
static SDL_Surface * img_open, * img_erase, * img_back;
static SDL_Surface * img_cursor_up, * img_cursor_down;
static SDL_Surface * img_cursor_starter_up, * img_cursor_starter_down;
static SDL_Surface * img_scroll_up, * img_scroll_down;
static SDL_Surface * img_scroll_up_off, * img_scroll_down_off;
static SDL_Surface * img_paintcan;
static SDL_Surface * img_grow, * img_shrink;

static SDL_Surface * img_sparkles;
static SDL_Surface * img_grass;

static SDL_Surface * img_title_on, * img_title_off,
  * img_title_large_on, * img_title_large_off;
static SDL_Surface * img_title_names[NUM_TITLES];
static SDL_Surface * img_tools[NUM_TOOLS], * img_tool_names[NUM_TOOLS];

#define MAX_STAMPS 512
#define MAX_BRUSHES 64
#define MAX_FONTS 64

static int num_brushes, num_stamps;
static SDL_Surface * img_brushes[MAX_BRUSHES];
static SDL_Surface * img_stamps[MAX_STAMPS];
static SDL_Surface * img_stamps_premirror[MAX_STAMPS];
static char * txt_stamps[MAX_STAMPS];
static info_type * inf_stamps[MAX_STAMPS];
static state_type * state_stamps[MAX_STAMPS];
#ifndef NOSOUND
static Mix_Chunk * snd_stamps[MAX_STAMPS];
#endif
static SDL_Surface * img_stamp_thumbs[MAX_STAMPS],
  * img_stamp_thumbs_premirror[MAX_STAMPS];

static SDL_Surface * img_shapes[NUM_SHAPES], * img_shape_names[NUM_SHAPES];
static SDL_Surface * img_magics[NUM_MAGICS], * img_magic_names[NUM_MAGICS];
static SDL_Surface * img_openlabels_open, * img_openlabels_erase,
  * img_openlabels_back;

static SDL_Surface * img_tux[NUM_TIP_TUX];

#ifndef LOW_QUALITY_COLOR_SELECTOR
static SDL_Surface * img_color_btns[NUM_COLORS*2];
static SDL_Surface * img_color_btn_off;
#endif

static int colors_are_selectable;

static SDL_Surface * img_cur_brush;
static int brush_counter, rainbow_color;

static TTF_Font * font, * small_font, * large_font, * locale_font;
static TTF_Font * fonts[MAX_FONTS];
static int num_fonts;

#ifndef NOSOUND
static Mix_Chunk * sounds[NUM_SOUNDS];
#endif

#define NUM_ERASERS 6 /* How many sizes of erasers (from ERASER_MIN to _MAX) */
#define ERASER_MIN 13 
#define ERASER_MAX 128 


static SDL_Cursor * cursor_hand, * cursor_arrow, * cursor_watch,
  * cursor_up, * cursor_down, * cursor_tiny, * cursor_crosshair,
  * cursor_brush, * cursor_wand, * cursor_insertion, * cursor_rotate;


static int cur_tool, cur_color, cur_brush, cur_stamp, cur_shape, cur_magic;
static int cur_font, cur_eraser;
static int cursor_left, cursor_x, cursor_y, cursor_textwidth;
static int been_saved;
static char file_id[32];
static char starter_id[32];
static int brush_scroll, stamp_scroll, font_scroll, magic_scroll;
static int eraser_sound;

static char texttool_str[256];
static unsigned int texttool_len;

static int tool_avail[NUM_TOOLS], tool_avail_bak[NUM_TOOLS];

typedef struct edge_type {
  int y_upper;
  float x_intersect, dx_per_scan;
  struct edge_type * next;
} edge;


typedef struct point_type {
  int x, y;
} point_type;

typedef struct fpoint_type {
  float x, y;
} fpoint_type;

typedef enum { Left, Right, Bottom, Top } an_edge;
#define NUM_EDGES 4

static SDL_Event scrolltimer_event;

static char * langstr;
static char * savedir;

#ifdef USE_HQ4X
static int RGBtoYUV[65536];
#endif

typedef struct dirent2 {
  struct dirent f;
  int place;
} dirent2;


/* Local function prototypes: */

static void mainloop(void);
static void brush_draw(int x1, int y1, int x2, int y2, int update);
static void blit_brush(int x, int y);
static void magic_draw(int x1, int y1, int x2, int y2, int button_down);
static void blit_magic(int x, int y, int button_down);
static void stamp_draw(int x, int y);
static void rec_undo_buffer(void);
static void update_canvas(int x1, int y1, int x2, int y2);
static void show_usage(FILE * f, char * prg);
static void show_lang_usage(FILE * f, char * prg);
static void show_locale_usage(FILE * f, char * prg);
static void setup(int argc, char * argv[]);
static SDL_Cursor * get_cursor(char * bits, char * mask_bits,
		        int w, int h, int x, int y);
static void seticon(void);
static SDL_Surface * loadimage(const char * const fname);
static SDL_Surface * do_loadimage(const char * const fname, int abort_on_error);
static SDL_Surface * loadaltimage(const char * const fname);
static void draw_toolbar(void);
static void draw_magic(void);
static void draw_brushes(void);
static void draw_stamps(void);
static void draw_shapes(void);
static void draw_erasers(void);
static void draw_fonts(void);
static void draw_none(void);
#ifndef NOSOUND
static void loadarbitrary(SDL_Surface * surfs[], SDL_Surface * altsurfs[],
		   char * descs[], info_type * infs[],
		   Mix_Chunk * sounds[], int * count, int starting, int max,
		   const char * const dir, int fatal, int maxw, int maxh);
#else
static void loadarbitrary(SDL_Surface * surfs[], SDL_Surface * altsurfs[],
		   char * descs[], info_type * infs[],
		   int * count, int starting, int max,
		   const char * const dir, int fatal, int maxw, int maxh);
#endif
static SDL_Surface * thumbnail(SDL_Surface * src, int max_x, int max_y,
			int keep_aspect);

static Uint32 getpixel(SDL_Surface * surface, int x, int y);
static void putpixel(SDL_Surface * surface, int x, int y, Uint32 pixel);
static void clipped_putpixel(SDL_Surface * dest, int x, int y, Uint32 c);

static void debug(const char * const str);
static void do_undo(void);
static void do_redo(void);
static void render_brush(void);
static void playsound(int chan, int s, int override);
static void line_xor(int x1, int y1, int x2, int y2);
static void rect_xor(int x1, int y1, int x2, int y2);

#ifdef LOW_QUALITY_STAMP_OUTLINE
#define stamp_xor(x,y) rect_xor( \
  (x) - (CUR_STAMP_W+1)/2, \
  (y) - (CUR_STAMP_H+1)/2, \
  (x) + (CUR_STAMP_W+1)/2, \
  (y) + (CUR_STAMP_H+1)/2 \
)
#define update_stamp_xor()
#else
static void stamp_xor(int x1, int y1);
static void update_stamp_xor(void);
#endif

static void do_eraser(int x, int y);
static void disable_avail_tools(void);
static void enable_avail_tools(void);
static void reset_avail_tools(void);
static void update_screen(int x1, int y1, int x2, int y2);
static int compare_strings(char * * s1, char * * s2);
static int compare_dirent2s(struct dirent2 * f1, struct dirent2 * f2);
static void draw_tux_text(int which_tux, const char * const str,
		          int want_right_to_left);
static void wordwrap_text(const char * const str, SDL_Color color,
		   int left, int top, int right,
		   int want_right_to_left);
static char * loaddesc(const char * const fname);
static info_type * loadinfo(const char * const fname);
#ifndef NOSOUND
static Mix_Chunk * loadsound(const char * const fname);
#endif
static void do_wait(void);
static void load_current(void);
static void save_current(void);
static char * get_fname(const char * const name);
static int do_prompt(const char * const text, const char * const btn_yes, const char * const btn_no);
static void cleanup(void);
static void free_cursor(SDL_Cursor ** cursor);
static void free_surface(SDL_Surface **surface_array);
static void free_surface_array(SDL_Surface *surface_array[], int count);
//static void update_shape(int cx, int ox1, int ox2, int cy, int oy1, int oy2,
//		  int fixed);
static void do_shape(int cx, int cy, int ox, int oy, int rotn, int use_brush);
static int rotation(int ctr_x, int ctr_y, int ox, int oy);
static int do_save(void);
static int do_png_save(FILE * fi, const char * const fname, SDL_Surface * surf);
static void get_new_file_id(void);
static int do_quit(void);
static int do_open(int want_new_tool);
#ifdef SCAN_FILL
static void scan_fill(int cnt, point_type * pts);
#endif
#ifdef SCANLINE_POLY_FILL
static int clip_polygon(int n, fpoint_type * pin, fpoint_type * pout);
#endif
static void wait_for_sfx(void);
static int current_language(void);
static int stamp_colorable(int stamp);
static int stamp_tintable(int stamp);
static void rgbtohsv(Uint8 r8, Uint8 g8, Uint8 b8, float *h, float *s, float *v);
static void hsvtorgb(float h, float s, float v, Uint8 *r8, Uint8 *g8, Uint8 *b8);
static void show_progress_bar(void);
static void do_print(void);
static void strip_trailing_whitespace(char * buf);
static void do_render_cur_text(int do_blit);
static void loadfonts(const char * const dir, int fatal);
static char * uppercase(char * str);
static unsigned char * textdir(const unsigned char * const str);
static SDL_Surface * do_render_button_label(const char * const label);
static void create_button_labels(void);
static int colors_close(Uint32 c1, Uint32 c2);
static void do_flood_fill(int x, int y, Uint32 cur_colr, Uint32 old_colr);
static Uint32 scrolltimer_callback(Uint32 interval, void *param);
static Uint32 drawtext_callback(Uint32 interval, void *param);
static void control_drawtext_timer(Uint32 interval, const char * const text);
static void parse_options(FILE * fi);
static void do_setcursor(SDL_Cursor * c);
static const char * great_str(void);
static void draw_image_title(int t, int x);
static int need_own_font(int l);
static int need_right_to_left(int l);
static void handle_keymouse(SDLKey key, Uint8 updown);
static void handle_active(SDL_Event * event);
static char * remove_slash(char * path);
static void anti_carriage_return(int left, int right, int cur_top, int new_top,
		          int cur_bot, int line_width);
static int mySDL_WaitEvent(SDL_Event *event);
static int mySDL_PollEvent(SDL_Event *event);
static void load_starter_id(char * saved_id);
static void load_starter(char * img_id);
static SDL_Surface * duplicate_surface(SDL_Surface * orig);
static TTF_Font *try_alternate_font(int language);
static void mirror_starter(void);
static void flip_starter(void);

#ifdef DEBUG
static char * debug_gettext(const char * str);
static int charsize(char c);
#endif


#define MAX_UTF8_CHAR_LENGTH 6

#define USEREVENT_TEXT_UPDATE 1


/* --- MAIN --- */

int main(int argc, char * argv[])
{
  SDL_Surface * tmp_surf;
  SDL_Color black = {0, 0, 0, 0};
  SDL_Rect dest;
  char tmp_str[128];

  /* Set up locale support */
  
  setlocale(LC_ALL, "");


  /* Set up! */

  setup(argc, argv);


  do_setcursor(cursor_arrow);

  SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 255, 255, 255));

  dest.x = (WINDOW_WIDTH - img_title->w) / 2;
  dest.y = (WINDOW_HEIGHT - img_title->h);

  SDL_BlitSurface(img_title, NULL, screen, &dest);

  snprintf(tmp_str, sizeof(tmp_str), "%s – %s", VER_VERSION, VER_DATE);
  tmp_surf = TTF_RenderUTF8_Blended(font, tmp_str, black);
  dest.x = 20 + (WINDOW_WIDTH - img_title->w) / 2;
  dest.y = WINDOW_HEIGHT - 60;
  SDL_BlitSurface(tmp_surf, NULL, screen, &dest);
  SDL_FreeSurface(tmp_surf);
  SDL_Flip(screen);
  playsound(0, SND_HARP, 1);
  
  do_wait();
  SDL_FreeSurface(img_title);


  /* Set defaults! */

  cur_undo = 0;
  oldest_undo = 0;
  newest_undo = 0;

  cur_tool = TOOL_BRUSH;
  cur_color = COLOR_BLACK;
  colors_are_selectable = 1;
  cur_brush = 0;
  cur_stamp = 0;
  cur_shape = SHAPE_SQUARE;
  cur_magic = 0;
  cur_font = 0;
  cur_eraser = 0;
  cursor_left = -1;
  cursor_x = -1;
  cursor_y = -1;
  cursor_textwidth = 0;

  mouse_x = WINDOW_WIDTH / 2;
  mouse_y = WINDOW_HEIGHT / 2;
  SDL_WarpMouse(mouse_x, mouse_y);

  mousekey_up = SDL_KEYUP;
  mousekey_down = SDL_KEYUP;
  mousekey_left = SDL_KEYUP;
  mousekey_right = SDL_KEYUP;

  eraser_sound = 0;

  img_cur_brush = NULL;
  render_brush();

  brush_scroll = 0;
  stamp_scroll = 0;
  font_scroll = 0;
  magic_scroll = 0;


  reset_avail_tools();


  /* Load current image (if any): */

  load_current();
  been_saved = 1;
  tool_avail[TOOL_SAVE] = 0;


  /* Draw the screen! */

  SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 255, 255, 255));

  draw_toolbar();
  draw_colors(COLORSEL_REFRESH);
  draw_brushes();
  update_canvas(0, 0, WINDOW_WIDTH - 96, (48 * 7) + 40 + HEIGHTOFFSET);

  SDL_Flip(screen);


  /* Main loop! */

  mainloop();


  /* Close and quit! */

  save_current();

  wait_for_sfx();


  cleanup();

  return 0;
}


/* FIXME: Move elsewhere!!! */

#define PROMPT_QUIT_TXT gettext_noop("Do you really want to quit?")
#define PROMPT_QUIT_YES gettext_noop("Yes")
#define PROMPT_QUIT_NO  gettext_noop("No")

#define PROMPT_QUIT_SAVE_TXT gettext_noop("If you quit, you’ll lose your picture! Save it?")
#define PROMPT_QUIT_SAVE_YES gettext_noop("Yes")
#define PROMPT_QUIT_SAVE_NO gettext_noop("No")

#define PROMPT_OPEN_SAVE_TXT gettext_noop("Save your picture first?")
#define PROMPT_OPEN_SAVE_YES gettext_noop("Yes")
#define PROMPT_OPEN_SAVE_NO gettext_noop("No")

#define PROMPT_OPEN_UNOPENABLE_TXT gettext_noop("Can’t open that picture!")
#define PROMPT_OPEN_UNOPENABLE_YES gettext_noop("OK")

#define PROMPT_NEW_TXT gettext_noop("Starting a new picture will erase the current one!")
#define PROMPT_NEW_YES gettext_noop("That’s OK!")
#define PROMPT_NEW_NO gettext_noop("Never mind!")

#define PROMPT_OPEN_NOFILES_TXT gettext_noop("There are no saved files!")
#define PROMPT_OPEN_NOFILES_YES gettext_noop("OK")

#define PROMPT_PRINT_NOW_TXT gettext_noop("Print your picture now?")
#define PROMPT_PRINT_NOW_YES gettext_noop("Yes")
#define PROMPT_PRINT_NOW_NO gettext_noop("No")

#define PROMPT_PRINT_TXT gettext_noop("Your picture has been printed!")
#define PROMPT_PRINT_YES gettext_noop("OK")

#define PROMPT_PRINT_TOO_SOON_TXT gettext_noop("You can’t print yet!")
#define PROMPT_PRINT_TOO_SOON_YES gettext_noop("OK")

#define PROMPT_ERASE_TXT gettext_noop("Erase this picture?")
#define PROMPT_ERASE_YES gettext_noop("Yes")
#define PROMPT_ERASE_NO gettext_noop("No")


enum {
  SHAPE_TOOL_MODE_STRETCH,
  SHAPE_TOOL_MODE_ROTATE,
  SHAPE_TOOL_MODE_DONE
};


/* --- MAIN LOOP! --- */

static void mainloop(void)
{
  int done, off_y, which, button_down, old_x, old_y, new_x, new_y,
    line_start_x, line_start_y, w, h, shape_tool_mode,
    shape_ctr_x, shape_ctr_y, shape_outer_x, shape_outer_y;
  int num_things, thing_scroll, cur_thing, old_thing, do_draw, old_tool,
    tmp_int, max;
  int cur_time, last_print_time, scrolling, ignoring_motion;
  SDL_TimerID scrolltimer;
  SDL_Event event;
  SDLKey key, key_down;
  Uint16 key_unicode;
  SDLMod mod;
  Uint32 last_cursor_blink, cur_cursor_blink,
    pre_event_time, current_event_time;


  num_things = 0;
  thing_scroll = 0;
  cur_thing = 0;
  old_thing = 0;
  do_draw = 0;
  old_x = 0;
  old_y = 0;
  new_x = 0;
  new_y = 0;
  line_start_x = 0;
  line_start_y = 0;
  shape_ctr_x = 0;
  shape_ctr_y = 0;
  shape_outer_x = 0;
  shape_outer_y =0;
  shape_tool_mode = SHAPE_TOOL_MODE_DONE;
  button_down = 0;
  last_print_time = -print_delay;
  last_cursor_blink = 0;
  texttool_len = 0;
  scrolling = 0;
  scrolltimer = 0;
  key_down = SDLK_LAST;
  key_unicode = 0;


  done = 0;

  do
    {
      ignoring_motion = 0;

      pre_event_time = SDL_GetTicks();


      while (mySDL_PollEvent(&event))
	{
	  current_event_time = SDL_GetTicks();

	  if (current_event_time > pre_event_time + 250)
	    ignoring_motion = 1;


	  if (event.type == SDL_QUIT)
	    {
	      done = do_quit();
	    }
	  else if (event.type == SDL_ACTIVEEVENT)
	    {
	      handle_active(&event);
	    }
	  else if (event.type == SDL_KEYUP)
	    {
	      key = event.key.keysym.sym;

	      handle_keymouse(key, SDL_KEYUP);
	    }
	  else if (event.type == SDL_KEYDOWN)
	    {
	      key = event.key.keysym.sym;
	      mod = event.key.keysym.mod;
	      
	      handle_keymouse(key, SDL_KEYDOWN);

	      if (key == SDLK_ESCAPE)
		{
		  done = do_quit();
		}
#ifdef WIN32
	      else if (key == SDLK_F4 && (mod & KMOD_ALT))
	        {
		  done = do_quit();
	        }
#endif
	      else if (key == SDLK_z &&
		       (mod & KMOD_CTRL ||
			mod & KMOD_LCTRL ||
			mod & KMOD_RCTRL) &&
		       !noshortcuts)
		{
		  /* Ctrl-Z - Undo */
		  
		  if (tool_avail[TOOL_UNDO])
		    {
		      if (cur_undo == newest_undo)
			{
			  rec_undo_buffer();
			  do_undo();
			}
		      do_undo();
		      SDL_UpdateRect(screen,
				     0, 0,
				     96, (48 * (7 + TOOLOFFSET / 2)) + 40);
		      shape_tool_mode = SHAPE_TOOL_MODE_DONE;
		    }
		}
	      else if (key == SDLK_r &&
		       (mod & KMOD_CTRL ||
			mod & KMOD_LCTRL ||
			mod & KMOD_RCTRL) &&
		       !noshortcuts)
		{
		  /* Ctrl-R - Redo */
		  
		  if (tool_avail[TOOL_REDO])
		    {
		      do_redo();
		      SDL_UpdateRect(screen,
				     0, 0,
				     96, (48 * (7 + TOOLOFFSET / 2)) + 40);
		      shape_tool_mode = SHAPE_TOOL_MODE_DONE;
		    }
		}
	      else if (key == SDLK_o &&
		       (mod & KMOD_CTRL ||
			mod & KMOD_LCTRL ||
			mod & KMOD_RCTRL) &&
		       !noshortcuts)
		{
		  /* Ctrl-O - Open */
		  
		  tmp_int = tool_avail[TOOL_NEW];
		  disable_avail_tools();
		  draw_toolbar();
		  draw_colors(COLORSEL_CLOBBER);
		  draw_none();
		  
		  tmp_int = do_open(tmp_int);
		  
		  enable_avail_tools();
		  tool_avail[TOOL_NEW] = tmp_int;
		  
		  draw_toolbar();
		  SDL_UpdateRect(screen,
				 0, 0,
				 96, (48 * (7 + TOOLOFFSET / 2)) + 40);
		  draw_colors(COLORSEL_REFRESH);

		  if (cur_tool == TOOL_BRUSH || cur_tool == TOOL_LINES)
		    draw_brushes();
		  else if (cur_tool == TOOL_MAGIC)
		    draw_magic();
		  else if (cur_tool == TOOL_STAMP)
		    draw_stamps();
		  else if (cur_tool == TOOL_TEXT)
		    draw_fonts();
		  else if (cur_tool == TOOL_SHAPES)
		    draw_shapes();
		  else if (cur_tool == TOOL_ERASER)
		    draw_erasers();
		  
		  draw_tux_text(TUX_GREAT, tool_tips[cur_tool], 1);

		  /* FIXME: Make delay configurable: */
		  control_drawtext_timer(1000, tool_tips[cur_tool]);
		}
	      else if ((key == SDLK_n &&
			((mod & KMOD_CTRL ||
			  mod & KMOD_LCTRL ||
			  mod & KMOD_RCTRL))) && tool_avail[TOOL_NEW] &&
		       !noshortcuts)
		{
		  /* Ctrl-N - New */
		  
		  if (do_prompt(PROMPT_NEW_TXT,
				PROMPT_NEW_YES,
				PROMPT_NEW_NO))
		    {
		      free_surface(&img_starter);
		      free_surface(&img_starter_bkgd);
		      starter_mirrored = 0;
		      starter_flipped = 0;
		      
		      SDL_FillRect(canvas, NULL,
				   SDL_MapRGB(canvas->format, 255, 255, 255));
		      update_canvas(0, 0,
				    WINDOW_WIDTH - 96,
				    (48 * 7) + 40 + HEIGHTOFFSET);

		      cur_undo = 0;
		      oldest_undo = 0;
		      newest_undo = 0;
		      shape_tool_mode = SHAPE_TOOL_MODE_DONE;
		      
		      been_saved = 1;
		      reset_avail_tools();
		      
		      file_id[0] = '\0';
		      starter_id[0] = '\0';
		      
		      playsound(1, SND_HARP, 1);
		    }
		  else
		    {
		      draw_tux_text(tool_tux[TUX_DEFAULT], TIP_NEW_ABORT,
				    1);
		    }
		  
		  draw_toolbar();
		  SDL_UpdateRect(screen, 0, 0,
				 96, (48 * (7 + TOOLOFFSET / 2)) + 40);
		}
	      else if (key == SDLK_s &&
		       (mod & KMOD_CTRL ||
			mod & KMOD_LCTRL ||
			mod & KMOD_RCTRL) &&
		       !noshortcuts)
		{
		  /* Ctrl-S - Save */
		  
		  if (do_save())
		    {
		      /* Only think it's been saved if it HAS been saved :^) */
		      
		      been_saved = 1;
		      tool_avail[TOOL_SAVE] = 0;
		    }

		  /* cur_tool = old_tool; */
		  draw_toolbar();
		  
		  SDL_UpdateRect(screen,
				 0, 0,
				 96, (48 * (7 + TOOLOFFSET / 2)) + 40);
		}
	      else
		{
		  /* Handle key in text tool: */

		  if (cur_tool == TOOL_TEXT &&
		      cursor_x != -1 && cursor_y != -1)
		    {
		      key_down = key;
		      key_unicode = event.key.keysym.unicode;

#ifdef DEBUG
		      printf("charsize(%c) = %d\n", event.key.keysym.unicode,
			     charsize(event.key.keysym.unicode));
#endif
	
		      if (key_down == SDLK_BACKSPACE)
			{

			  if (texttool_len > 0)
			    {
			      texttool_len--;
			      texttool_str[texttool_len] = '\0';
			      playsound(0, SND_KEYCLICK, 1);
	        
			      do_render_cur_text(0);
			    }
			}
		      else if (key_down == SDLK_RETURN)
			{
			  if (texttool_len > 0)
			    {
			      rec_undo_buffer();
			      do_render_cur_text(1);
			      texttool_len = 0;
			      cursor_textwidth = 0;
			    }

			  cursor_x = cursor_left;
			  cursor_y = cursor_y + TTF_FontHeight(fonts[cur_font]);

			  if (cursor_y > ((48 * 7 + 40 + HEIGHTOFFSET) -
					  TTF_FontHeight(fonts[cur_font])))
			    {
			      cursor_y = ((48 * 7 + 40 + HEIGHTOFFSET) -
					  TTF_FontHeight(fonts[cur_font]));
			    }
	    
			  playsound(0, SND_RETURN, 1);
			}
		      else if (isprint(key_unicode))
			{
			  if (texttool_len < sizeof(texttool_str) - MAX_UTF8_CHAR_LENGTH)
			    {
#ifdef DEBUG
			      printf("    key = %c\n"
				     "unicode = %c (%d)\n\n",
				     key_down, key_unicode, key_unicode);
#endif
	  	     
			        texttool_str[texttool_len++] = key_unicode;
		
			      texttool_str[texttool_len] = '\0';
			      playsound(0, SND_KEYCLICK, 1);
			      do_render_cur_text(0);
			    }
			}
		    }
		}
	    }
	  else if ((event.type == SDL_MOUSEBUTTONDOWN &&
		    event.button.button >= 1 &&
		    event.button.button <= 3))
	    {
	      if (/* event.button.x >= 0 && */
		  event.button.x < 96 &&
		  event.button.y >= 40 &&
		  event.button.y < (48 * 7) + 40) /* FIXME: FIX ME? */
		{
		  /* A tool on the left has been pressed! */
		  
		  which = ((event.button.y - 40) / 48) * 2 +
		    (event.button.x / 48);
		  
		  if (which < NUM_TOOLS && tool_avail[which])
		    {
		      /* Render any current text: */
		      
		      if (cur_tool == TOOL_TEXT && which != TOOL_TEXT &&
			  texttool_len > 0)
			{
			  if (cursor_x != -1 && cursor_y != -1)
			    {
			      if (texttool_len > 0)
				{
				  rec_undo_buffer();
				  do_render_cur_text(1);
				  texttool_len = 0;
				  cursor_textwidth = 0;
				}
			    }
			}
		      
		      
		      old_tool = cur_tool;
		      cur_tool = which;
		      draw_toolbar();
		      SDL_UpdateRect(screen,
				     0, 0,
				     96, (48 * (7 + TOOLOFFSET / 2)) + 40);
		      
		      playsound(1, SND_CLICK, 0);
		      
		      draw_tux_text(tool_tux[cur_tool], tool_tips[cur_tool],
				    1);
		      
		      
		      /* Draw items for this tool: */
		      
		      if (cur_tool == TOOL_BRUSH)
			{
			  cur_thing = cur_brush;
			  draw_brushes();
			  draw_colors(COLORSEL_ENABLE);
			}
		      else if (cur_tool == TOOL_ERASER)
		        {
			  cur_thing = cur_eraser;
			  draw_erasers();
			  draw_colors(COLORSEL_DISABLE);
		        }
		      else if (cur_tool == TOOL_STAMP)
			{
			  cur_thing = cur_stamp;
			  draw_stamps();
			  draw_colors(stamp_colorable(cur_stamp) ||
				      stamp_tintable(cur_stamp));

			  update_stamp_xor();
			}
		      else if (cur_tool == TOOL_LINES)
			{
			  cur_thing = cur_brush;
			  draw_brushes();
			  draw_colors(COLORSEL_ENABLE);
			}
		      else if (cur_tool == TOOL_SHAPES)
			{
			  cur_thing = cur_shape;
			  draw_shapes();
			  draw_colors(COLORSEL_ENABLE);
			  shape_tool_mode = SHAPE_TOOL_MODE_DONE;
			}
		      else if (cur_tool == TOOL_TEXT)
			{
			  cur_thing = cur_font;
			  draw_fonts();
			  draw_colors(COLORSEL_ENABLE);
			}
		      else if (cur_tool == TOOL_ERASER)
			{
			  cur_thing = cur_eraser;
			  draw_erasers();
			  draw_colors(COLORSEL_DISABLE);
			}
		      else if (cur_tool == TOOL_UNDO)
			{
			  if (cur_undo == newest_undo)
			    {
			      rec_undo_buffer();
			      do_undo();
			    }
			  do_undo();
			  
			  been_saved = 0;

			  if (!disable_save)
			    tool_avail[TOOL_SAVE] = 1;
			  
			  cur_tool = old_tool;
			  draw_toolbar();
			  SDL_UpdateRect(screen,
					 0, 0,
					 96, (48 * (7 + TOOLOFFSET / 2)) + 40);
			  shape_tool_mode = SHAPE_TOOL_MODE_DONE;
			}
		      else if (cur_tool == TOOL_REDO)
			{
			  do_redo();
			  
			  been_saved = 0;

			  if (!disable_save)
			    tool_avail[TOOL_SAVE] = 1;
			  
			  cur_tool = old_tool;
			  draw_toolbar();
			  SDL_UpdateRect(screen,
					 0, 0,
					 96, (48 * (7 + TOOLOFFSET / 2)) + 40);
			  shape_tool_mode = SHAPE_TOOL_MODE_DONE;
			}
		      else if (cur_tool == TOOL_OPEN)
			{
			  tmp_int = tool_avail[TOOL_NEW];
			  disable_avail_tools();
			  draw_toolbar();
		          draw_colors(COLORSEL_CLOBBER);
		          draw_none();
			  
			  tmp_int = do_open(tmp_int);
			  
			  enable_avail_tools();
			  tool_avail[TOOL_NEW] = tmp_int;
			  
			  cur_tool = old_tool;
			  draw_toolbar();
			  SDL_UpdateRect(screen,
					 0, 0,
					 96, (48 * (7 + TOOLOFFSET / 2)) + 40);
			  
			  draw_tux_text(TUX_GREAT, tool_tips[cur_tool], 1);

			  draw_colors(COLORSEL_REFRESH);

			  if (cur_tool == TOOL_BRUSH || cur_tool == TOOL_LINES)
			    draw_brushes();
			  else if (cur_tool == TOOL_MAGIC)
			    draw_magic();
			  else if (cur_tool == TOOL_STAMP)
			    draw_stamps();
			  else if (cur_tool == TOOL_TEXT)
			    draw_fonts();
			  else if (cur_tool == TOOL_SHAPES)
			    draw_shapes();
			  else if (cur_tool == TOOL_ERASER)
			    draw_erasers();
			}
		      else if (cur_tool == TOOL_SAVE)
			{
			  if (do_save())
			    {
			      been_saved = 1;
			      tool_avail[TOOL_SAVE] = 0;
			    }
			  
			  cur_tool = old_tool;
			  draw_toolbar();
			  SDL_UpdateRect(screen,
					 0, 0,
					 96, (48 * (7 + TOOLOFFSET / 2)) + 40);
			}
		      else if (cur_tool == TOOL_NEW)
			{
			  if (do_prompt(PROMPT_NEW_TXT,
					PROMPT_NEW_YES,
					PROMPT_NEW_NO))
			    {
		      	      free_surface(&img_starter);
			      free_surface(&img_starter_bkgd);
			      starter_mirrored = 0;
			      starter_flipped = 0;
		      
			      SDL_FillRect(canvas, NULL,
					   SDL_MapRGB(canvas->format,
						      255, 255, 255));
			      update_canvas(0, 0,
					    WINDOW_WIDTH - 96,
					    (48 * 7) + 40 + HEIGHTOFFSET);
			      
			      cur_undo = 0;
			      oldest_undo = 0;
			      newest_undo = 0;
			      shape_tool_mode = SHAPE_TOOL_MODE_DONE;
			      
			      been_saved = 1;
			      reset_avail_tools();
			      
			      file_id[0] = '\0';
			      starter_id[0] = '\0';
			      
			      playsound(1, SND_HARP, 1);
			    }
			  else
			    {
			      draw_tux_text(tool_tux[TUX_DEFAULT],
					    TIP_NEW_ABORT, 1);
			    }
			  
			  cur_tool = old_tool;
			  draw_toolbar();
			  SDL_UpdateRect(screen, 0, 0, 96, (48 * (7 + TOOLOFFSET / 2)) + 40);
			}
		      else if (cur_tool == TOOL_PRINT)
			{
			  cur_time = SDL_GetTicks() / 1000;
			  
#ifdef DEBUG
			  printf("Current time = %d\n", cur_time);
#endif
			  
			  if (cur_time >= last_print_time + print_delay)
			    {
		              if (do_prompt(PROMPT_PRINT_NOW_TXT,
				            PROMPT_PRINT_NOW_YES,
				            PROMPT_PRINT_NOW_NO))
			      {
			        do_print();
			      
			        last_print_time = cur_time;
			      }
			    }
			  else
			    {
			      do_prompt(PROMPT_PRINT_TOO_SOON_TXT,
					PROMPT_PRINT_TOO_SOON_YES,
					"");
			    }
			  
			  cur_tool = old_tool;
			  draw_toolbar();
			  SDL_UpdateRect(screen, 0, 0, 96, (48 * (7 + TOOLOFFSET / 2)) + 40);
			}
		      else if (cur_tool == TOOL_MAGIC)
			{
			  cur_thing = cur_magic;
			  rainbow_color = 0;
			  draw_magic();
			  draw_colors(magic_colors[cur_magic]);
			}
		      else if (cur_tool == TOOL_QUIT)
			{
			  done = do_quit();
			  cur_tool = old_tool;
			  draw_toolbar();
			  SDL_UpdateRect(screen, 0, 0, 96, (48 * (7 + TOOLOFFSET / 2)) + 40);
			}
		      
		      SDL_UpdateRect(screen,
				     WINDOW_WIDTH - 96, 0,
				     96, (48 * 7) + 40 + HEIGHTOFFSET);
		      SDL_UpdateRect(screen,
				     0, (48 * 7) + 40 + HEIGHTOFFSET,
				     WINDOW_WIDTH, 48);
		    }
		}
	      else if (event.button.x >= WINDOW_WIDTH - 96 &&
		       event.button.x < WINDOW_WIDTH &&
		       event.button.y >= 40 &&
		       event.button.y < (48 * (7 + TOOLOFFSET / 2)) + 40)
		{
		  /* Options on the right have been pressed! */
		  
		  if (cur_tool == TOOL_BRUSH || cur_tool == TOOL_STAMP ||
		      cur_tool == TOOL_SHAPES || cur_tool == TOOL_LINES ||
		      cur_tool == TOOL_MAGIC || cur_tool == TOOL_TEXT ||
		      cur_tool == TOOL_ERASER)
		    {
		      /* Note set of things we're dealing with */
		      /* (stamps, brushes, etc.) */
		      
		      if (cur_tool == TOOL_BRUSH || cur_tool == TOOL_LINES)
			{
			  num_things = num_brushes;
			  thing_scroll = brush_scroll;
			}
		      else if (cur_tool == TOOL_STAMP)
			{
			  num_things = num_stamps;
			  thing_scroll = stamp_scroll;
			}
		      else if (cur_tool == TOOL_TEXT)
			{
			  num_things = num_fonts;
			  thing_scroll = font_scroll;
			}
		      else if (cur_tool == TOOL_SHAPES)
			{
			  num_things = NUM_SHAPES;
			  thing_scroll = 0;
			}
		      else if (cur_tool == TOOL_MAGIC)
			{
			  num_things = NUM_MAGICS;
			  thing_scroll = magic_scroll;
			}
		      else if (cur_tool == TOOL_ERASER)
		        {
			  num_things = NUM_ERASERS;
			  thing_scroll = 0;
		        }
		      
		      do_draw = 0;

		      
		      /* Deal with scroll buttons: */
		      
		      max = 14;

		      if (cur_tool == TOOL_STAMP && !disable_stamp_controls)
			max = 10;

		      if (num_things > max + TOOLOFFSET)
			{
			  if (event.button.y < 40 + 24)
			    {
			      if (thing_scroll > 0)
				{
				  thing_scroll = thing_scroll - 2;
				  do_draw = 1;
				  
				  playsound(1, SND_SCROLL, 1);

				  if (scrolling == 0)
				    {
				      memcpy(&scrolltimer_event,
					     &event,
					     sizeof(SDL_Event));

				    
				      /* FIXME: Make delay value changable: */

				      scrolltimer =
					SDL_AddTimer(REPEAT_SPEED,
						     scrolltimer_callback,
						     (void*) &scrolltimer_event);

				      scrolling = 1;
				    }
				  else
				    {
				      SDL_RemoveTimer(scrolltimer);
				    
				      scrolltimer =
					SDL_AddTimer(REPEAT_SPEED / 3,
						     scrolltimer_callback,
						     (void*) &scrolltimer_event);
				    }
				  
				  
				  if (thing_scroll == 0)
				    {
				      do_setcursor(cursor_arrow);

				      if (scrolling == 1)
					{
					  SDL_RemoveTimer(scrolltimer);
					  scrolling = 0;
					}
				    }
				}
			    }
			  else if (event.button.y >=
				   (48 * ((max - 2) / 2 + TOOLOFFSET / 2)) + 40 + 24 &&
				   event.button.y <
				   (48 * ((max - 2) / 2 + TOOLOFFSET / 2)) + 40 + 24 + 24)
			    {
			      if (thing_scroll < num_things - (max - 2) - TOOLOFFSET)
				{
				  thing_scroll = thing_scroll + 2;
				  do_draw = 1;
				  
				  playsound(1, SND_SCROLL, 1);
				  
				  if (scrolling == 0)
				    {
				      memcpy(&scrolltimer_event,
					     &event,
					     sizeof(SDL_Event));

				      /* FIXME: Make delay value changable: */

				      scrolltimer =
					SDL_AddTimer(REPEAT_SPEED,
						     scrolltimer_callback,
						     (void*) &scrolltimer_event);

				      scrolling = 1;
				    }
				  else
				    {
				      SDL_RemoveTimer(scrolltimer);
				    
				      scrolltimer =
					SDL_AddTimer(REPEAT_SPEED / 3,
						     scrolltimer_callback,
						     (void*) &scrolltimer_event);
				    }
				  
				  if (thing_scroll == 0)
				    {
				      do_setcursor(cursor_arrow);

				      if (scrolling == 1)
					{
					  SDL_RemoveTimer(scrolltimer);
					  scrolling = 0;
					}
				    }
				}
			    }
			  
			  off_y = 24;
			}
		      else
			{
			  off_y = 0;
			}


		      
		      if (event.button.y > 40 + off_y &&
			  event.button.y <
			  (48 * ((max / 2) + TOOLOFFSET / 2)) + 40 - off_y)
			{
			  which = ((event.button.y - 40 - off_y) / 48) * 2 +
			    ((event.button.x - (WINDOW_WIDTH - 96)) / 48) +
			    thing_scroll;
			  
			  if (which < num_things)
			    {
#ifndef NOSOUND
			      if (cur_tool != TOOL_STAMP ||
				  snd_stamps[which] == NULL)
				{ 
				  playsound(1, SND_BLEEP, 0);
				}
#endif
			      
			      old_thing = cur_thing;
			      cur_thing = which;
			      do_draw = 1;
			    }
			  else
			    {
			      cur_thing = num_things - 1;
			      do_draw = 1;
			    }
			}
		      else if (cur_tool == TOOL_STAMP &&
			       event.button.y >= (48 * ((max / 2) + TOOLOFFSET / 2)) + 40 &&
			       event.button.y < (48 * ((max / 2) + TOOLOFFSET / 2)) + 40 + 96 &&
			       !disable_stamp_controls)
			{
			  /* Stamp controls! */

			  if (event.button.y >= (48 * ((max / 2) + TOOLOFFSET / 2)) + 40 &&
			      event.button.y < (48 * ((max / 2) + TOOLOFFSET / 2)) + 40 + 48)
			    {
			      /* One of the top buttons: */

			      if (event.button.x >= WINDOW_WIDTH - 96 &&
				  event.button.x <= WINDOW_WIDTH - 48)
				{
				  /* Top left button: Mirror: */
			    
				  if (inf_stamps[cur_stamp]->mirrorable)
				    {
				      state_stamps[cur_stamp]->mirrored =
					!state_stamps[cur_stamp]->mirrored;
			      
				      playsound(0, SND_MIRROR, 0);
				      draw_stamps();
			
				      SDL_UpdateRect(screen,
						     WINDOW_WIDTH - 96, 0,
						     96, (48 * 7) + 40 + HEIGHTOFFSET);
				    }
				}
			      else
				{
				  /* Top right button: Flip: */
				  
				  if (inf_stamps[cur_stamp]->flipable)
				    {
				      state_stamps[cur_stamp]->flipped =
					!state_stamps[cur_stamp]->flipped;

				      playsound(0, SND_FLIP, 0);
				      draw_stamps();
			
				      SDL_UpdateRect(screen,
						     WINDOW_WIDTH - 96, 0,
						     96, (48 * 7) + 40 + HEIGHTOFFSET);
				    }
				}
			    }
			  else
			    {
			      /* One of the bottom buttons: */

			      if (event.button.x >= WINDOW_WIDTH - 96 &&
				  event.button.x <= WINDOW_WIDTH - 48)
				{

				  /* Bottom left button: Shrink: */

				  if (state_stamps[cur_stamp]->size > MIN_STAMP_SIZE)
				    {
				      state_stamps[cur_stamp]->size--;
			    
				      playsound(0, SND_SHRINK, 0);
				      draw_stamps();
			
				      SDL_UpdateRect(screen,
						     WINDOW_WIDTH - 96, 0,
						     96, (48 * 7) + 40 + HEIGHTOFFSET);
				    }
				}
			      else
				{
				  /* Bottom right button: Grow: */

				  if (state_stamps[cur_stamp]->size < MAX_STAMP_SIZE)
				    {
				      state_stamps[cur_stamp]->size++;
			    
				      playsound(0, SND_GROW, 0);
				      draw_stamps();
			
				      SDL_UpdateRect(screen,
						     WINDOW_WIDTH - 96, 0,
						     96, (48 * 7) + 40 + HEIGHTOFFSET);
				    }
				}
			    }
			  update_stamp_xor();
			}
		      
		      
		      /* Assign the change(s), if any / redraw, if needed: */
		      
		      if (cur_tool == TOOL_BRUSH || cur_tool == TOOL_LINES)
			{
			  cur_brush = cur_thing;
			  brush_scroll = thing_scroll;
			  render_brush();
			  
			  if (do_draw)
			    draw_brushes();
			}
		      else if (cur_tool == TOOL_ERASER)
		        {
			  cur_eraser = cur_thing;
			  
			  if (do_draw)
			    draw_erasers();
		        }
		      else if (cur_tool == TOOL_TEXT)
			{
			  cur_font = cur_thing;
			  font_scroll = thing_scroll;
			  
			  if (do_draw)
			    draw_fonts();
			  
			  do_render_cur_text(0);
			}
		      else if (cur_tool == TOOL_STAMP)
			{
#ifndef NOSOUND
			  if (cur_stamp != cur_thing ||
			      stamp_scroll == thing_scroll)
			    {
			      /* Only play when picking a different stamp, not
				 simply scrolling */
			      
			      if (snd_stamps[cur_thing] != NULL)
				Mix_PlayChannel(2, snd_stamps[cur_thing], 0);
			    }
#endif
		  

			  if (cur_thing != cur_stamp)
			  {
			    cur_stamp = cur_thing;
			    update_stamp_xor();
			  }

			  stamp_scroll = thing_scroll;
			  

			  if (do_draw)
			    draw_stamps();
			  
			  if (txt_stamps[cur_stamp] != NULL)
			    {
#ifdef DEBUG
			      printf("txt_stamps[cur_stamp] = %s\n",
				     txt_stamps[cur_stamp]);
#endif

			      draw_tux_text(TUX_GREAT,
						txt_stamps[cur_stamp], 1);
			    }
			  else
			    draw_tux_text(TUX_GREAT, "", 0);
			  
			  
			  /* Enable or disable color selector: */
			  
			  if ((stamp_colorable(cur_stamp) ||
			       stamp_tintable(cur_stamp)) !=
			      (stamp_colorable(old_thing) ||
			       stamp_tintable(old_thing)))
			    {
			      draw_colors(stamp_colorable(cur_stamp) ||
					  stamp_tintable(cur_stamp));
			      SDL_UpdateRect(screen,
					     0, (48 * 7) + 40 + HEIGHTOFFSET,
					     WINDOW_WIDTH, 48);
			    }
			}
		      else if (cur_tool == TOOL_SHAPES)
			{
			  cur_shape = cur_thing;
			  
			  draw_tux_text(TUX_GREAT, shape_tips[cur_shape], 1);
			  
			  if (do_draw)
			    draw_shapes();
			}
		      else if (cur_tool == TOOL_MAGIC)
			{
			  if (cur_thing != cur_magic)
			    {
			      cur_magic = cur_thing;
			      draw_colors(magic_colors[cur_magic]);

			      SDL_UpdateRect(screen,
					     0, (48 * 7) + 40 + HEIGHTOFFSET,
					     WINDOW_WIDTH, 48);
			    }
			  
			  magic_scroll = thing_scroll;

			  draw_tux_text(TUX_GREAT, magic_tips[cur_magic], 1);
			  
			  if (do_draw)
			    draw_magic();
			}
		      
		      /* Update the screen: */
		      
		      if (do_draw)
			SDL_UpdateRect(screen,
				       WINDOW_WIDTH - 96, 0,
				       96, (48 * 7) + 40 + HEIGHTOFFSET);
		    }
		}
	      else if (event.button.x > 96 &&
		       /* FIXME: Need exact number here! */
		       event.button.x < WINDOW_WIDTH &&
		       event.button.y > (48 * (7 + TOOLOFFSET / 2)) + 40 &&
		       event.button.y <= (48 * (7 + TOOLOFFSET / 2)) + 48 + 48 &&
		       colors_are_selectable)
		{
		  /* Color! */
		  
		  which = ((event.button.x - 96) /
			   ((WINDOW_WIDTH - 96) / NUM_COLORS));
		  
		  if (which < NUM_COLORS)
		    {
		      cur_color = which;
		      playsound(1, SND_BUBBLE, 1);
		      draw_colors(COLORSEL_REFRESH);
		      SDL_UpdateRect(screen,
				     0, (48 * 7) + 40 + HEIGHTOFFSET,
				     WINDOW_WIDTH, 48);
		      render_brush();
		      draw_tux_text(TUX_KISS, color_names[cur_color], 1);
		
		      if (cur_tool == TOOL_TEXT)
		        do_render_cur_text(0);
		    }
		}
	      else if (event.button.x > 96 &&
		       event.button.x < WINDOW_WIDTH - 96 &&
		       /* event.button.y >= 0 && */
		       event.button.y < (48 * 7) + 40 + HEIGHTOFFSET)
		{
		  /* Draw something! */
		  
		  old_x = event.button.x - 96;
		  old_y = event.button.y;
		  
		  if (been_saved)
		    {
		      been_saved = 0;

		      if (!disable_save)
		        tool_avail[TOOL_SAVE] = 1;

		      draw_toolbar();
		      SDL_UpdateRect(screen,
				     0, 0,
				     96, (48 * (7 + TOOLOFFSET / 2)) + 40);
		    }
		  
		  if (cur_tool == TOOL_BRUSH)
		    {
		      /* Start painting! */
		      
		      rec_undo_buffer();
		      
		      /* (Arbitrarily large, so we draw once now) */
		      brush_counter = 999;
  	
		      brush_draw(old_x, old_y, old_x, old_y, 1);
		      playsound(0, SND_PAINT1 + (img_cur_brush->w) / 12, 1);
		    }
		  else if (cur_tool == TOOL_STAMP)
		    {
		      /* Draw a stamp! */
		      
		      rec_undo_buffer();
		      
		      stamp_draw(old_x, old_y);
		      stamp_xor(old_x, old_y);
		      playsound(1, SND_STAMP, 1);

		      draw_tux_text(TUX_GREAT, great_str(), 1);

		      /* FIXME: Make delay configurable: */

		      control_drawtext_timer(1000, txt_stamps[cur_stamp]);
		    }
		  else if (cur_tool == TOOL_LINES)
		    {
		      /* Start a line! */
		      
		      rec_undo_buffer();
		      
		      line_start_x = old_x;
		      line_start_y = old_y;
		      
		      /* (Arbitrarily large, so we draw once now) */
		      brush_counter = 999;
		      
		      brush_draw(old_x, old_y, old_x, old_y, 1);
		      
		      playsound(1, SND_LINE_START, 1);
		      draw_tux_text(TUX_BORED, TIP_LINE_START, 1);
		    }
		  else if (cur_tool == TOOL_SHAPES)
		    {
		      if (shape_tool_mode == SHAPE_TOOL_MODE_DONE)
			{
			  /* Start drawing a shape! */
			  
			  rec_undo_buffer();
			  
			  shape_ctr_x = old_x;
			  shape_ctr_y = old_y;
			  
			  shape_tool_mode = SHAPE_TOOL_MODE_STRETCH;
			  
			  playsound(1, SND_LINE_START, 1);
			  draw_tux_text(TUX_BORED, TIP_SHAPE_START, 1);
			}
		      else if (shape_tool_mode == SHAPE_TOOL_MODE_ROTATE)
			{
			  /* Draw the shape with the brush! */
			  
			  /* (Arbitrarily large...) */
			  brush_counter = 999;
			  
			  playsound(1, SND_LINE_END, 1);
			  do_shape(shape_ctr_x, shape_ctr_y,
				   shape_outer_x, shape_outer_y,
				   rotation(shape_ctr_x, shape_ctr_y,
					    event.button.x - 96,
					    event.button.y),
				   1);
			  
			  shape_tool_mode = SHAPE_TOOL_MODE_DONE;
			  draw_tux_text(TUX_GREAT, tool_tips[TOOL_SHAPES], 1);
			}
		    }
		  else if (cur_tool == TOOL_MAGIC)
		    {
		      /* Start doing magic! */
		     
		      tmp_int = cur_undo;
		      rec_undo_buffer();


		      /* Mirror or flip, make a note so we record it for
		         the starters, too! */

		      if (cur_magic == MAGIC_MIRROR)
			undo_starters[tmp_int] = UNDO_STARTER_MIRRORED;
		      else if (cur_magic == MAGIC_FLIP)
			undo_starters[tmp_int] = UNDO_STARTER_FLIPPED;

		      
		      /* (Arbitrarily large, so we draw once now) */
		      brush_counter = 999;
		     
		      if (cur_magic != MAGIC_FILL)
			{
			  magic_draw(old_x, old_y, old_x, old_y, button_down);
			}
		      else
			{
			  do_flood_fill(old_x, old_y,
					SDL_MapRGB(canvas->format,
						   color_hexes[cur_color][0],
						   color_hexes[cur_color][1],
						   color_hexes[cur_color][2]),
					getpixel(canvas, old_x, old_y));

			  draw_tux_text(TUX_GREAT, magic_tips[MAGIC_FILL], 1);
			}
		      
		      if (cur_magic == MAGIC_FLIP ||
			  cur_magic == MAGIC_MIRROR ||
			  cur_magic == MAGIC_FILL)
			{
			  update_canvas(0, 0, canvas->w, canvas->h);
			}
		    }
		  else if (cur_tool == TOOL_ERASER)
		    {
		      /* Erase! */
		      
		      rec_undo_buffer();
		      
		      do_eraser(old_x, old_y);
		    }
		  else if (cur_tool == TOOL_TEXT)
		    {
		      /* Text Tool! */
		      
		      if (cursor_x != -1 && cursor_y != -1)
			{
		 	  /*
			    if (texttool_len > 0)
			    {
			    rec_undo_buffer();
			    do_render_cur_text(1);
			    texttool_len = 0;
			    }
			  */
			}
		      
		      cursor_x = old_x;
		      cursor_y = old_y;
		      cursor_left = old_x;

		      do_render_cur_text(0);
		    }
		  
		  button_down = 1;

		  /* Make sure these commands are available now: */
		  
		  if (tool_avail[TOOL_NEW] == 0)
		    {
		      tool_avail[TOOL_NEW] = 1;
		      
		      draw_toolbar();
		      SDL_UpdateRect(screen,
				     0, 0,
				     96, (48 * (7 + TOOLOFFSET / 2)) + 40);
		    }
		}
	    }
	  else if (event.type == SDL_MOUSEBUTTONDOWN &&
		   wheely &&
		   event.button.button >= 4 &&
		   event.button.button <= 5)
	    {
	      if (cur_tool == TOOL_BRUSH || cur_tool == TOOL_STAMP ||
		  cur_tool == TOOL_SHAPES || cur_tool == TOOL_LINES ||
		  cur_tool == TOOL_MAGIC || cur_tool == TOOL_TEXT)
		{
		  /* Note set of things we're dealing with */
		  /* (stamps, brushes, etc.) */

		  if (cur_tool == TOOL_BRUSH || cur_tool == TOOL_LINES)
		    {
		      num_things = num_brushes;
		      thing_scroll = brush_scroll;
		    }
		  else if (cur_tool == TOOL_STAMP)
		    {
		      num_things = num_stamps;
		      thing_scroll = stamp_scroll;
		    }
		  else if (cur_tool == TOOL_TEXT)
		    {
		      num_things = num_fonts;
		      thing_scroll = font_scroll;
		    }
		  else if (cur_tool == TOOL_SHAPES)
		    {
		      num_things = NUM_SHAPES;
		      thing_scroll = 0;
		    }
		  else if (cur_tool == TOOL_MAGIC)
		    {
		      num_things = NUM_MAGICS;
		      thing_scroll = magic_scroll;
		    }

		  do_draw = 0;



		  /* Deal with scroll wheels: */

		  max = 14;

		  if (cur_tool == TOOL_STAMP && !disable_stamp_controls)
		    max = 10;

		  if (num_things > max + TOOLOFFSET)
		    {
		      if (event.button.button == 4)
			{
			  /* Wheelmouse - UP "button" */
			       
			  if (thing_scroll > 0)
			    {
			      thing_scroll = thing_scroll - 2;
			      do_draw = 1;

			      playsound(1, SND_SCROLL, 1);

			      if (thing_scroll == 0)
				do_setcursor(cursor_arrow);
			    }
			}
		      else if (event.button.button == 5)
			{
			  /* Wheelmouse - DOWN "button" */
				   
			  if (thing_scroll < num_things - (max - 2))
			    {
			      thing_scroll = thing_scroll + 2;
			      do_draw = 1;

			      playsound(1, SND_SCROLL, 1);

			      if (thing_scroll == 0)
				do_setcursor(cursor_arrow);
			    }
			}

		      off_y = 24;
		    }
		  else
		    {
		      off_y = 0;
		    }
		}

		 
	      /* Assign the change(s), if any / redraw, if needed: */

	      if (cur_tool == TOOL_BRUSH || cur_tool == TOOL_LINES)
		{
		  cur_brush = cur_thing;
		  brush_scroll = thing_scroll;
		  render_brush();

		  if (do_draw)
		    draw_brushes();
		}
	      else if (cur_tool == TOOL_ERASER)
	        {
		  cur_eraser = cur_thing;
		  
		  if (do_draw)
		    draw_erasers();
	        }
	      else if (cur_tool == TOOL_TEXT)
		{
		  cur_font = cur_thing;
		  font_scroll = thing_scroll;

		  if (do_draw)
		    draw_fonts();

		  do_render_cur_text(0);
		}
	      else if (cur_tool == TOOL_STAMP)
		{
#ifndef NOSOUND
		  if (cur_stamp != cur_thing)
		    {
		      /* Only play when picking a different stamp, not
			 simply scrolling */

		      if (snd_stamps[cur_thing] != NULL)
			Mix_PlayChannel(2, snd_stamps[cur_thing], 0);
		    }
#endif
  
		  cur_stamp = cur_thing;
		  stamp_scroll = thing_scroll;

		  update_stamp_xor();

		  if (do_draw)
		    draw_stamps();

		  if (txt_stamps[cur_stamp] != NULL)
		    {
		      draw_tux_text(TUX_GREAT, txt_stamps[cur_stamp], 1);
		    }
		  else
		    draw_tux_text(TUX_GREAT, "", 0);


		  /* Enable or disable color selector: */

		  if ((stamp_colorable(cur_stamp) ||
		       stamp_tintable(cur_stamp)) !=
		      (stamp_colorable(old_thing) ||
		       stamp_tintable(old_thing)))
		    {
		      draw_colors(stamp_colorable(cur_stamp) ||
				  stamp_tintable(cur_stamp));
		      SDL_UpdateRect(screen,
				     0, (48 * 7) + 40 + HEIGHTOFFSET,
				     WINDOW_WIDTH, 48);
		    }
		}
	      else if (cur_tool == TOOL_SHAPES)
		{
		  cur_shape = cur_thing;
                       
		  draw_tux_text(TUX_GREAT, shape_tips[cur_shape], 1);

		  if (do_draw)
		    draw_shapes();
		}
	      else if (cur_tool == TOOL_MAGIC)
		{
		  if (cur_thing != cur_magic)
		    {
		      draw_colors(magic_colors[cur_magic]);

		      SDL_UpdateRect(screen,
				     0, (48 * 7) + 40 + HEIGHTOFFSET,
				     WINDOW_WIDTH, 48);
		    }

		  cur_magic = cur_thing;
		  magic_scroll = thing_scroll;

		  draw_tux_text(TUX_GREAT, magic_tips[cur_magic], 1);

		  if (do_draw)
		    draw_magic();
		}

                           
	      /* Update the screen: */

	      if (do_draw)
		SDL_UpdateRect(screen,
			       WINDOW_WIDTH - 96, 0,
			       96, (48 * 7) + 40 + HEIGHTOFFSET);
	    }
	  else if (event.type == SDL_USEREVENT)
	    {
	      if (event.user.code == USEREVENT_TEXT_UPDATE)
		{
		  /* Time to replace "Great!" with old tip text: */
		    
		  if (event.user.data1 != NULL)
		    {
		      if (((unsigned char *) event.user.data1)[0] == '=')
			{
			  draw_tux_text(TUX_GREAT,
					(char *) event.user.data1 + 1, 1);
			}
		      else
			{
			  draw_tux_text(TUX_GREAT,
					(char *) event.user.data1, 0);
			}
		    }
		  else
		    draw_tux_text(TUX_GREAT, "", 1);
		}
	    }
	  else if (event.type == SDL_MOUSEBUTTONUP)
	    {
	      if (scrolling)
		{
		  SDL_RemoveTimer(scrolltimer);
		  scrolling = 0;
		}
	      
	      if (button_down)
		{
		  if (cur_tool == TOOL_LINES)
		    {
		      /* (Arbitrarily large, so we draw once now) */
		      brush_counter = 999;
		      
		      brush_draw(line_start_x, line_start_y,
				 event.button.x - 96, event.button.y, 1);
		      brush_draw(event.button.x - 96, event.button.y,
				 event.button.x - 96, event.button.y, 1);
		      
		      playsound(1, SND_LINE_END, 1);
		      draw_tux_text(TUX_GREAT, tool_tips[TOOL_LINES], 1);
		    }
		  else if (cur_tool == TOOL_SHAPES)
		    {
		      if (shape_tool_mode == SHAPE_TOOL_MODE_STRETCH)
			{
			  /* Now we can rotate the shape... */
			  
			  shape_outer_x = event.button.x - 96;
			  shape_outer_y = event.button.y;
			  
			  if (!simple_shapes && !shape_no_rotate[cur_shape])
			    {
			      shape_tool_mode = SHAPE_TOOL_MODE_ROTATE;
		      
			      SDL_WarpMouse(shape_outer_x + 96, shape_ctr_y);
			      do_setcursor(cursor_rotate);
			      
			      
			      /* Erase stretchy XOR: */
			      
			      do_shape(shape_ctr_x, shape_ctr_y, old_x, old_y,
				       0, 0);
			      
			      /* Make an initial rotation XOR to be erased: */
			      
			      do_shape(shape_ctr_x, shape_ctr_y,
				       shape_outer_x, shape_outer_y,
				       rotation(shape_ctr_x, shape_ctr_y,
						shape_outer_x, shape_outer_y),
				       0);
			      
			      playsound(1, SND_LINE_START, 1);
			      draw_tux_text(TUX_BORED, TIP_SHAPE_NEXT, 1);
			      
			      
			      /* FIXME: Do something less intensive! */
			      
			      SDL_Flip(screen);
			    }
			  else
			    {
			      brush_counter = 999; /* arbitrarily large... */
			      
		  
			      playsound(1, SND_LINE_END, 1);
			      do_shape(shape_ctr_x, shape_ctr_y,
				       shape_outer_x, shape_outer_y,
				       0, 1);

			      SDL_Flip(screen);
			      
			      shape_tool_mode = SHAPE_TOOL_MODE_DONE;
			      draw_tux_text(TUX_GREAT,
					    tool_tips[TOOL_SHAPES], 1);
			    }
			}
		    }
		}
	      
	      button_down = 0;
	    }
	  else if (event.type == SDL_MOUSEMOTION && !ignoring_motion)
	    {
	      new_x = event.button.x - 96;
	      new_y = event.button.y;
	      
	      
	      /* FIXME: Is doing this every event too intensive? */
	      /* Should I check current cursor first? */
	      
	      if (event.button.x < 96 && event.button.y < (48 * 7) + 40 &&
		  event.button.y > 40)
		{
		  /* Tools: */
		  
		  if (tool_avail[(event.button.x / 48) +
				 ((event.button.y - 40) / 48) * 2])
		    {
		      do_setcursor(cursor_hand);
		    }
		  else
		    {
		      do_setcursor(cursor_arrow);
		    }
		}
	      else if (event.button.x > 96 &&
		       event.button.y >= (48 * 7) + 40 + HEIGHTOFFSET &&
		       event.button.y <= (48 * 7) + 40 + 48 + HEIGHTOFFSET)
		{
		  /* Color picker: */
		 
	  	  if (colors_are_selectable)
		    do_setcursor(cursor_hand);
		  else
		    do_setcursor(cursor_arrow);
		}
	      else if (event.button.x >= WINDOW_WIDTH - 96 &&
		       event.button.y > 40 &&
		       event.button.y <= (48 * (7 + TOOLOFFSET / 2)) + 40)
		{
		  /* Selector: */
		  
		  /* Note set of things we're dealing with */
		  /* (stamps, brushes, etc.) */
		  
		  if (cur_tool == TOOL_BRUSH || cur_tool == TOOL_LINES)
		    {
		      num_things = num_brushes;
		      thing_scroll = brush_scroll;
		    }
		  else if (cur_tool == TOOL_STAMP)
		    {
		      num_things = num_stamps;
		      thing_scroll = stamp_scroll;
		    }
		  else if (cur_tool == TOOL_TEXT)
		    {
		      num_things = num_fonts;
		      thing_scroll = font_scroll;
		    }
		  else if (cur_tool == TOOL_SHAPES)
		    {
		      num_things = NUM_SHAPES;
		      thing_scroll = 0;
		    }
		  else if (cur_tool == TOOL_MAGIC)
		    {
		      num_things = NUM_MAGICS;
		      thing_scroll = magic_scroll;
		    }


		  max = 14;
		  if (cur_tool == TOOL_STAMP && !disable_stamp_controls)
		    max = 10;
		  
		  
		  if (num_things > max + TOOLOFFSET)
		    {
		      /* Are there scroll buttons? */
		      
		      if (event.button.y < 40 + 24)
			{
			  /* Up button; is it available? */
			  
			  if (thing_scroll > 0)
			    do_setcursor(cursor_up);
			  else
			    do_setcursor(cursor_arrow);
			}
		      else if (event.button.y > (48 * ((max - 2) / 2 + TOOLOFFSET / 2)) + 40 + 24 &&
		               event.button.y <= (48 * ((max - 2) / 2 + TOOLOFFSET / 2)) + 40 + 24 + 24)
			{
			  /* Down button; is it available? */
			  
			  if (thing_scroll < num_things - (max - 2))
			    do_setcursor(cursor_down);
			  else
			    do_setcursor(cursor_arrow);
			}
		      else
			{
			  /* One of the selectors: */
			 
			  which = ((event.button.y - 40 - 24) / 48) * 2 +
			    (event.button.x - (WINDOW_WIDTH - 96)) / 48;

			  if (which < num_things)
			    do_setcursor(cursor_hand);
			  else
			    do_setcursor(cursor_arrow);
			}
		    }
		  else
		    {
		      /* No scroll buttons - must be a selector: */
		      
		      which = ((event.button.y - 40) / 48) * 2 +
			(event.button.x - (WINDOW_WIDTH - 96)) / 48;

		      if (which < num_things)
			do_setcursor(cursor_hand);
		      else
			do_setcursor(cursor_arrow);
		    }
		}
	      else if (event.button.x > 96 &&
		       event.button.x < WINDOW_WIDTH - 96 &&
		       event.button.y < (48 * 7) + 40 + HEIGHTOFFSET)
		{
		  /* Canvas: */
		  
		  if (cur_tool == TOOL_BRUSH)
		    do_setcursor(cursor_brush);
		  else if (cur_tool == TOOL_STAMP)
		    do_setcursor(cursor_tiny);
		  else if (cur_tool == TOOL_LINES)
		    do_setcursor(cursor_crosshair);
		  else if (cur_tool == TOOL_SHAPES)
		    {
		      if (shape_tool_mode != SHAPE_TOOL_MODE_ROTATE)
			do_setcursor(cursor_crosshair);
		      else
			do_setcursor(cursor_rotate);
		    }
		  else if (cur_tool == TOOL_TEXT)
		    do_setcursor(cursor_insertion);
		  else if (cur_tool == TOOL_MAGIC)
		    do_setcursor(cursor_wand);
		  else if (cur_tool == TOOL_ERASER)
		    do_setcursor(cursor_tiny);
		}
	      else
		{
		  do_setcursor(cursor_arrow);
		}
	      
	      
	      if (button_down)
		{
		  if (cur_tool == TOOL_BRUSH)
		    {
		      /* Pushing button and moving: Draw with the brush: */
		      
		      brush_draw(old_x, old_y, new_x, new_y, 1);
		      
		      playsound(0, SND_PAINT1 + (img_cur_brush->w) / 12, 0);
		    }
		  else if (cur_tool == TOOL_LINES)
		    {
		      /* Still pushing button, while moving:
			 Draw XOR where line will go: */
		      
		      line_xor(line_start_x, line_start_y, old_x, old_y);
		      
		      line_xor(line_start_x, line_start_y, new_x, new_y);
		      
		      update_screen(line_start_x + 96, line_start_y,
				    old_x + 96, old_y);
		      update_screen(line_start_x + 96, line_start_y,
				    new_x + 96, new_y);
		    }
		  else if (cur_tool == TOOL_SHAPES)
		    {
		      /* Still pushing button, while moving:
			 Draw XOR where shape will go: */
		      
		      if (shape_tool_mode == SHAPE_TOOL_MODE_STRETCH)
			{
			  do_shape(shape_ctr_x, shape_ctr_y, old_x, old_y,
				   0, 0);
			  
			  do_shape(shape_ctr_x, shape_ctr_y,
				   new_x, new_y,
				   0, 0);
			  
			  
			  /* FIXME: Fix update shape function! */
			  
			  /* update_shape(shape_ctr_x, old_x, new_x,
			     shape_ctr_y, old_y, new_y,
			     shape_locked[cur_shape]); */
			  
			  SDL_Flip(screen);
			}
		    }
		  else if (cur_tool == TOOL_MAGIC)
		    {
		      /* Pushing button and moving: Do the magic: */
		      
		      if (cur_magic != MAGIC_FLIP &&
			  cur_magic != MAGIC_MIRROR &&
			  cur_magic != MAGIC_FILL)
			{
			  magic_draw(old_x, old_y, new_x, new_y, button_down);
			}
		    }
		  else if (cur_tool == TOOL_ERASER)
		    {
		      /* Still pushing, and moving - Erase! */
		      
		      do_eraser(new_x, new_y);
		    }
		}
	      
	      
	      if (cur_tool == TOOL_STAMP ||
		  (cur_tool == TOOL_ERASER && !button_down))
		{
		  /* Moving: Draw XOR where stamp/eraser will apply: */
		  
		  if (cur_tool == TOOL_STAMP)
		    {
		      w = img_stamps[cur_stamp]->w;
		      h = img_stamps[cur_stamp]->h;
		    }
		  else if (cur_tool == TOOL_ERASER)
		    {
		      w = (ERASER_MIN +
			   ((NUM_ERASERS - cur_eraser - 1) *
			    ((ERASER_MAX - ERASER_MIN) / (NUM_ERASERS - 1))));

		      h = w;
		    }
		  
		  if (old_x >= 0 && old_x < WINDOW_WIDTH - 96 - 96 &&
		      old_y >= 0 && old_y < (48 * 7) + 40 + HEIGHTOFFSET)
		    {
		      if (cur_tool == TOOL_STAMP)
			{
			  stamp_xor(old_x, old_y);

			  update_screen(old_x - (CUR_STAMP_W+1)/2 + 96,
					old_y - (CUR_STAMP_H+1)/2,
					old_x + (CUR_STAMP_W+1)/2 + 96,
					old_y + (CUR_STAMP_H+1)/2);
			}
		      else
			{
			  rect_xor(old_x - w / 2, old_y - h / 2,
				   old_x + w / 2, old_y + h / 2);

			  update_screen(old_x - w / 2 + 96, old_y - h / 2,
					old_x + w / 2 + 96, old_y + h / 2);
			}
		    }
		  
		  if (new_x >= 0 && new_x < WINDOW_WIDTH - 96 - 96 &&
		      new_y >= 0 && new_y < (48 * 7) + 40 + HEIGHTOFFSET)
		    {
		      if (cur_tool == TOOL_STAMP)
			{
			  stamp_xor(new_x, new_y);

			  update_screen(old_x - (CUR_STAMP_W+1)/2 + 96,
					old_y - (CUR_STAMP_H+1)/2,
					old_x + (CUR_STAMP_W+1)/2 + 96,
					old_y + (CUR_STAMP_H+1)/2);
			}
		      else
			{
			  rect_xor(new_x - w / 2, new_y - h / 2,
				   new_x + w / 2, new_y + h / 2);

			  update_screen(new_x - w / 2 + 96, new_y - h / 2,
					new_x + w / 2 + 96, new_y + h / 2);
			}
		    }
		}
	      else if (cur_tool == TOOL_SHAPES &&
		       shape_tool_mode == SHAPE_TOOL_MODE_ROTATE)
		{
		  do_shape(shape_ctr_x, shape_ctr_y,
			   shape_outer_x, shape_outer_y,
			   rotation(shape_ctr_x, shape_ctr_y,
				    old_x, old_y), 0);
			      
		  
		  do_shape(shape_ctr_x, shape_ctr_y,
			   shape_outer_x, shape_outer_y,
			   rotation(shape_ctr_x, shape_ctr_y,
				    new_x, new_y), 0);
		  
		  
		  /* FIXME: Do something less intensive! */
		  
		  SDL_Flip(screen);
		}
	      
	      old_x = new_x;
	      old_y = new_y;
	    }
	}


      SDL_Delay(10);
      
      cur_cursor_blink = SDL_GetTicks();
      
      
      if (cur_tool == TOOL_TEXT && cursor_x != -1 && cursor_y != -1 &&
	  cur_cursor_blink > last_cursor_blink + CURSOR_BLINK_SPEED)
	{
	  last_cursor_blink = SDL_GetTicks();
	  
	  line_xor(cursor_x + cursor_textwidth, cursor_y,
	           cursor_x + cursor_textwidth,
		   cursor_y + TTF_FontHeight(fonts[cur_font]));

          update_screen(cursor_x + 96 + cursor_textwidth, cursor_y,
			cursor_x + 96 + cursor_textwidth,
			cursor_y + TTF_FontHeight(fonts[cur_font]));
	}
    }
  while (!done);
}


/* Draw using the current brush: */

static void brush_draw(int x1, int y1, int x2, int y2, int update)
{
  int dx, dy, y;
  int orig_x1, orig_y1, orig_x2, orig_y2, tmp;
  float m, b;

  orig_x1 = x1;
  orig_y1 = y1;

  orig_x2 = x2;
  orig_y2 = y2;


  x1 = x1 - (img_brushes[cur_brush]->w / 2);
  y1 = y1 - (img_brushes[cur_brush]->h / 2);

  x2 = x2 - (img_brushes[cur_brush]->w / 2);
  y2 = y2 - (img_brushes[cur_brush]->h / 2);


  dx = x2 - x1;
  dy = y2 - y1;

  if (dx != 0)
    {
      m = ((float) dy) / ((float) dx);
      b = y1 - m * x1;

      if (x2 >= x1)
	dx = 1;
      else
	dx = -1;


      while (x1 != x2)
	{
	  y1 = m * x1 + b;
	  y2 = m * (x1 + dx) + b;

	  if (y1 > y2)
	    {
	      y = y1;
	      y1 = y2;
	      y2 = y;
	    }
	
	  for (y = y1; y <= y2; y++)
	    blit_brush(x1, y);
	
	  x1 = x1 + dx;
	}
    }
  else
    {
      if (y1 > y2)
	{
	  y = y1;
	  y1 = y2;
	  y2 = y;
	}

      for (y = y1; y <= y2; y++)
	blit_brush(x1, y);
    }

  if (orig_x1 > orig_x2)
    {
      tmp = orig_x1;
      orig_x1 = orig_x2;
      orig_x2 = tmp;
    }

  if (orig_y1 > orig_y2)
    {
      tmp = orig_y1;
      orig_y1 = orig_y2;
      orig_y2 = tmp;
    }


  if (update)
    {
      update_canvas(orig_x1 - (img_brushes[cur_brush]->w / 2),
		    orig_y1 - (img_brushes[cur_brush]->h / 2),
		    orig_x2 + (img_brushes[cur_brush]->w / 2),
		    orig_y2 + (img_brushes[cur_brush]->h / 2));
    }
}


/* Draw the current brush in the current color: */

static void blit_brush(int x, int y)
{
  SDL_Rect dest;

  brush_counter++;

  if (brush_counter >= (img_cur_brush->h / 4))
    {
      brush_counter = 0;

      dest.x = x;
      dest.y = y;

      SDL_BlitSurface(img_cur_brush, NULL, canvas, &dest);
    }
}


//////////////////////////////////////////////////////////////////////////
// stamp tinter

#define TINTER_ANYHUE 0 // like normal, but remaps all hues in the stamp
#define TINTER_NARROW 1 // like normal, but narrow hue angle
#define TINTER_NORMAL 2 // normal
#define TINTER_VECTOR 3 // map black->white to black->destination

// This goes from 8-bit sRGB (range 0 to 255) to linear (range 0 to 1).
// The math to produce a table entry:
//    tmp = oldvalue / 255.0;
//    result = (tmp<=0.03928) ? tmp/12.92 : pow((tmp+0.055)/1.055,2.4);
static const float sRGB_to_linear_table[256] = {
  0.000000, 0.000304, 0.000607, 0.000911, 0.001214, 0.001518, 0.001821,
  0.002125, 0.002428, 0.002732, 0.003035, 0.003347, 0.003677, 0.004025,
  0.004391, 0.004777, 0.005182, 0.005605, 0.006049, 0.006512, 0.006995,
  0.007499, 0.008023, 0.008568, 0.009134, 0.009721, 0.010330, 0.010960,
  0.011612, 0.012286, 0.012983, 0.013702, 0.014444, 0.015209, 0.015996,
  0.016807, 0.017642, 0.018500, 0.019382, 0.020289, 0.021219, 0.022174,
  0.023153, 0.024158, 0.025187, 0.026241, 0.027321, 0.028426, 0.029557,
  0.030713, 0.031896, 0.033105, 0.034340, 0.035601, 0.036889, 0.038204,
  0.039546, 0.040915, 0.042311, 0.043735, 0.045186, 0.046665, 0.048172,
  0.049707, 0.051269, 0.052861, 0.054480, 0.056128, 0.057805, 0.059511,
  0.061246, 0.063010, 0.064803, 0.066626, 0.068478, 0.070360, 0.072272,
  0.074214, 0.076185, 0.078187, 0.080220, 0.082283, 0.084376, 0.086500,
  0.088656, 0.090842, 0.093059, 0.095307, 0.097587, 0.099899, 0.102242,
  0.104616, 0.107023, 0.109462, 0.111932, 0.114435, 0.116971, 0.119538,
  0.122139, 0.124772, 0.127438, 0.130136, 0.132868, 0.135633, 0.138432,
  0.141263, 0.144128, 0.147027, 0.149960, 0.152926, 0.155926, 0.158961,
  0.162029, 0.165132, 0.168269, 0.171441, 0.174647, 0.177888, 0.181164,
  0.184475, 0.187821, 0.191202, 0.194618, 0.198069, 0.201556, 0.205079,
  0.208637, 0.212231, 0.215861, 0.219526, 0.223228, 0.226966, 0.230740,
  0.234551, 0.238398, 0.242281, 0.246201, 0.250158, 0.254152, 0.258183,
  0.262251, 0.266356, 0.270498, 0.274677, 0.278894, 0.283149, 0.287441,
  0.291771, 0.296138, 0.300544, 0.304987, 0.309469, 0.313989, 0.318547,
  0.323143, 0.327778, 0.332452, 0.337164, 0.341914, 0.346704, 0.351533,
  0.356400, 0.361307, 0.366253, 0.371238, 0.376262, 0.381326, 0.386429,
  0.391572, 0.396755, 0.401978, 0.407240, 0.412543, 0.417885, 0.423268,
  0.428690, 0.434154, 0.439657, 0.445201, 0.450786, 0.456411, 0.462077,
  0.467784, 0.473531, 0.479320, 0.485150, 0.491021, 0.496933, 0.502886,
  0.508881, 0.514918, 0.520996, 0.527115, 0.533276, 0.539479, 0.545724,
  0.552011, 0.558340, 0.564712, 0.571125, 0.577580, 0.584078, 0.590619,
  0.597202, 0.603827, 0.610496, 0.617207, 0.623960, 0.630757, 0.637597,
  0.644480, 0.651406, 0.658375, 0.665387, 0.672443, 0.679542, 0.686685,
  0.693872, 0.701102, 0.708376, 0.715694, 0.723055, 0.730461, 0.737910,
  0.745404, 0.752942, 0.760525, 0.768151, 0.775822, 0.783538, 0.791298,
  0.799103, 0.806952, 0.814847, 0.822786, 0.830770, 0.838799, 0.846873,
  0.854993, 0.863157, 0.871367, 0.879622, 0.887923, 0.896269, 0.904661,
  0.913099, 0.921582, 0.930111, 0.938686, 0.947307, 0.955973, 0.964686,
  0.973445, 0.982251, 0.991102, 1.000000
};

// this goes the other way; range checking will be required
static const unsigned char linear_to_sRGB_table[4096] =
  "\x00\x01\x02\x03\x03\x04\x05\x06\x07\x08\x08\x09\x0a\x0b\x0b\x0c\x0d\x0d"
  "\x0e\x0f\x10\x10\x11\x11\x12\x12\x13\x13\x14\x14\x15\x15\x16\x16\x17\x17"
  "\x18\x18\x18\x19\x19\x1a\x1a\x1a\x1b\x1b\x1c\x1c\x1c\x1d\x1d\x1d\x1e\x1e"
  "\x1e\x1f\x1f\x1f\x20\x20\x20\x21\x21\x21\x22\x22\x22\x23\x23\x23\x23\x24"
  "\x24\x24\x25\x25\x25\x25\x26\x26\x26\x26\x27\x27\x27\x28\x28\x28\x28\x29"
  "\x29\x29\x29\x2a\x2a\x2a\x2a\x2b\x2b\x2b\x2b\x2c\x2c\x2c\x2c\x2c\x2d\x2d"
  "\x2d\x2d\x2e\x2e\x2e\x2e\x2f\x2f\x2f\x2f\x2f\x30\x30\x30\x30\x30\x31\x31"
  "\x31\x31\x31\x32\x32\x32\x32\x33\x33\x33\x33\x33\x34\x34\x34\x34\x34\x35"
  "\x35\x35\x35\x35\x35\x36\x36\x36\x36\x36\x37\x37\x37\x37\x37\x38\x38\x38"
  "\x38\x38\x38\x39\x39\x39\x39\x39\x39\x3a\x3a\x3a\x3a\x3a\x3a\x3b\x3b\x3b"
  "\x3b\x3b\x3c\x3c\x3c\x3c\x3c\x3c\x3d\x3d\x3d\x3d\x3d\x3d\x3d\x3e\x3e\x3e"
  "\x3e\x3e\x3e\x3f\x3f\x3f\x3f\x3f\x3f\x40\x40\x40\x40\x40\x40\x41\x41\x41"
  "\x41\x41\x41\x41\x42\x42\x42\x42\x42\x42\x42\x43\x43\x43\x43\x43\x43\x44"
  "\x44\x44\x44\x44\x44\x44\x45\x45\x45\x45\x45\x45\x45\x46\x46\x46\x46\x46"
  "\x46\x46\x46\x47\x47\x47\x47\x47\x47\x47\x48\x48\x48\x48\x48\x48\x48\x48"
  "\x49\x49\x49\x49\x49\x49\x49\x4a\x4a\x4a\x4a\x4a\x4a\x4a\x4a\x4b\x4b\x4b"
  "\x4b\x4b\x4b\x4b\x4c\x4c\x4c\x4c\x4c\x4c\x4c\x4c\x4d\x4d\x4d\x4d\x4d\x4d"
  "\x4d\x4d\x4e\x4e\x4e\x4e\x4e\x4e\x4e\x4e\x4f\x4f\x4f\x4f\x4f\x4f\x4f\x4f"
  "\x50\x50\x50\x50\x50\x50\x50\x50\x50\x51\x51\x51\x51\x51\x51\x51\x51\x51"
  "\x52\x52\x52\x52\x52\x52\x52\x52\x53\x53\x53\x53\x53\x53\x53\x53\x53\x54"
  "\x54\x54\x54\x54\x54\x54\x54\x54\x55\x55\x55\x55\x55\x55\x55\x55\x55\x56"
  "\x56\x56\x56\x56\x56\x56\x56\x56\x57\x57\x57\x57\x57\x57\x57\x57\x57\x58"
  "\x58\x58\x58\x58\x58\x58\x58\x58\x58\x59\x59\x59\x59\x59\x59\x59\x59\x59"
  "\x5a\x5a\x5a\x5a\x5a\x5a\x5a\x5a\x5a\x5a\x5b\x5b\x5b\x5b\x5b\x5b\x5b\x5b"
  "\x5b\x5b\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5d\x5d\x5d\x5d\x5d\x5d"
  "\x5d\x5d\x5d\x5e\x5e\x5e\x5e\x5e\x5e\x5e\x5e\x5e\x5e\x5e\x5f\x5f\x5f\x5f"
  "\x5f\x5f\x5f\x5f\x5f\x5f\x60\x60\x60\x60\x60\x60\x60\x60\x60\x60\x60\x61"
  "\x61\x61\x61\x61\x61\x61\x61\x61\x61\x62\x62\x62\x62\x62\x62\x62\x62\x62"
  "\x62\x62\x63\x63\x63\x63\x63\x63\x63\x63\x63\x63\x63\x64\x64\x64\x64\x64"
  "\x64\x64\x64\x64\x64\x64\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x66"
  "\x66\x66\x66\x66\x66\x66\x66\x66\x66\x66\x67\x67\x67\x67\x67\x67\x67\x67"
  "\x67\x67\x67\x67\x68\x68\x68\x68\x68\x68\x68\x68\x68\x68\x68\x69\x69\x69"
  "\x69\x69\x69\x69\x69\x69\x69\x69\x6a\x6a\x6a\x6a\x6a\x6a\x6a\x6a\x6a\x6a"
  "\x6a\x6a\x6b\x6b\x6b\x6b\x6b\x6b\x6b\x6b\x6b\x6b\x6b\x6b\x6c\x6c\x6c\x6c"
  "\x6c\x6c\x6c\x6c\x6c\x6c\x6c\x6c\x6c\x6d\x6d\x6d\x6d\x6d\x6d\x6d\x6d\x6d"
  "\x6d\x6d\x6d\x6e\x6e\x6e\x6e\x6e\x6e\x6e\x6e\x6e\x6e\x6e\x6e\x6f\x6f\x6f"
  "\x6f\x6f\x6f\x6f\x6f\x6f\x6f\x6f\x6f\x70\x70\x70\x70\x70\x70\x70\x70\x70"
  "\x70\x70\x70\x70\x71\x71\x71\x71\x71\x71\x71\x71\x71\x71\x71\x71\x71\x72"
  "\x72\x72\x72\x72\x72\x72\x72\x72\x72\x72\x72\x72\x73\x73\x73\x73\x73\x73"
  "\x73\x73\x73\x73\x73\x73\x73\x74\x74\x74\x74\x74\x74\x74\x74\x74\x74\x74"
  "\x74\x74\x75\x75\x75\x75\x75\x75\x75\x75\x75\x75\x75\x75\x75\x75\x76\x76"
  "\x76\x76\x76\x76\x76\x76\x76\x76\x76\x76\x76\x77\x77\x77\x77\x77\x77\x77"
  "\x77\x77\x77\x77\x77\x77\x77\x78\x78\x78\x78\x78\x78\x78\x78\x78\x78\x78"
  "\x78\x78\x78\x79\x79\x79\x79\x79\x79\x79\x79\x79\x79\x79\x79\x79\x79\x7a"
  "\x7a\x7a\x7a\x7a\x7a\x7a\x7a\x7a\x7a\x7a\x7a\x7a\x7b\x7b\x7b\x7b\x7b\x7b"
  "\x7b\x7b\x7b\x7b\x7b\x7b\x7b\x7b\x7b\x7c\x7c\x7c\x7c\x7c\x7c\x7c\x7c\x7c"
  "\x7c\x7c\x7c\x7c\x7c\x7d\x7d\x7d\x7d\x7d\x7d\x7d\x7d\x7d\x7d\x7d\x7d\x7d"
  "\x7d\x7e\x7e\x7e\x7e\x7e\x7e\x7e\x7e\x7e\x7e\x7e\x7e\x7e\x7e\x7e\x7f\x7f"
  "\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x80\x80\x80\x80\x80"
  "\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x81\x81\x81\x81\x81\x81\x81\x81"
  "\x81\x81\x81\x81\x81\x81\x81\x82\x82\x82\x82\x82\x82\x82\x82\x82\x82\x82"
  "\x82\x82\x82\x82\x83\x83\x83\x83\x83\x83\x83\x83\x83\x83\x83\x83\x83\x83"
  "\x83\x83\x84\x84\x84\x84\x84\x84\x84\x84\x84\x84\x84\x84\x84\x84\x84\x85"
  "\x85\x85\x85\x85\x85\x85\x85\x85\x85\x85\x85\x85\x85\x85\x85\x86\x86\x86"
  "\x86\x86\x86\x86\x86\x86\x86\x86\x86\x86\x86\x86\x86\x87\x87\x87\x87\x87"
  "\x87\x87\x87\x87\x87\x87\x87\x87\x87\x87\x87\x88\x88\x88\x88\x88\x88\x88"
  "\x88\x88\x88\x88\x88\x88\x88\x88\x88\x89\x89\x89\x89\x89\x89\x89\x89\x89"
  "\x89\x89\x89\x89\x89\x89\x89\x8a\x8a\x8a\x8a\x8a\x8a\x8a\x8a\x8a\x8a\x8a"
  "\x8a\x8a\x8a\x8a\x8a\x8b\x8b\x8b\x8b\x8b\x8b\x8b\x8b\x8b\x8b\x8b\x8b\x8b"
  "\x8b\x8b\x8b\x8b\x8c\x8c\x8c\x8c\x8c\x8c\x8c\x8c\x8c\x8c\x8c\x8c\x8c\x8c"
  "\x8c\x8c\x8c\x8d\x8d\x8d\x8d\x8d\x8d\x8d\x8d\x8d\x8d\x8d\x8d\x8d\x8d\x8d"
  "\x8d\x8d\x8e\x8e\x8e\x8e\x8e\x8e\x8e\x8e\x8e\x8e\x8e\x8e\x8e\x8e\x8e\x8e"
  "\x8e\x8f\x8f\x8f\x8f\x8f\x8f\x8f\x8f\x8f\x8f\x8f\x8f\x8f\x8f\x8f\x8f\x8f"
  "\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x91"
  "\x91\x91\x91\x91\x91\x91\x91\x91\x91\x91\x91\x91\x91\x91\x91\x91\x91\x92"
  "\x92\x92\x92\x92\x92\x92\x92\x92\x92\x92\x92\x92\x92\x92\x92\x92\x92\x93"
  "\x93\x93\x93\x93\x93\x93\x93\x93\x93\x93\x93\x93\x93\x93\x93\x93\x94\x94"
  "\x94\x94\x94\x94\x94\x94\x94\x94\x94\x94\x94\x94\x94\x94\x94\x94\x95\x95"
  "\x95\x95\x95\x95\x95\x95\x95\x95\x95\x95\x95\x95\x95\x95\x95\x95\x96\x96"
  "\x96\x96\x96\x96\x96\x96\x96\x96\x96\x96\x96\x96\x96\x96\x96\x96\x96\x97"
  "\x97\x97\x97\x97\x97\x97\x97\x97\x97\x97\x97\x97\x97\x97\x97\x97\x97\x98"
  "\x98\x98\x98\x98\x98\x98\x98\x98\x98\x98\x98\x98\x98\x98\x98\x98\x98\x98"
  "\x99\x99\x99\x99\x99\x99\x99\x99\x99\x99\x99\x99\x99\x99\x99\x99\x99\x99"
  "\x9a\x9a\x9a\x9a\x9a\x9a\x9a\x9a\x9a\x9a\x9a\x9a\x9a\x9a\x9a\x9a\x9a\x9a"
  "\x9a\x9b\x9b\x9b\x9b\x9b\x9b\x9b\x9b\x9b\x9b\x9b\x9b\x9b\x9b\x9b\x9b\x9b"
  "\x9b\x9b\x9c\x9c\x9c\x9c\x9c\x9c\x9c\x9c\x9c\x9c\x9c\x9c\x9c\x9c\x9c\x9c"
  "\x9c\x9c\x9c\x9c\x9d\x9d\x9d\x9d\x9d\x9d\x9d\x9d\x9d\x9d\x9d\x9d\x9d\x9d"
  "\x9d\x9d\x9d\x9d\x9d\x9e\x9e\x9e\x9e\x9e\x9e\x9e\x9e\x9e\x9e\x9e\x9e\x9e"
  "\x9e\x9e\x9e\x9e\x9e\x9e\x9e\x9f\x9f\x9f\x9f\x9f\x9f\x9f\x9f\x9f\x9f\x9f"
  "\x9f\x9f\x9f\x9f\x9f\x9f\x9f\x9f\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0"
  "\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa1\xa1\xa1\xa1\xa1\xa1\xa1\xa1"
  "\xa1\xa1\xa1\xa1\xa1\xa1\xa1\xa1\xa1\xa1\xa1\xa1\xa2\xa2\xa2\xa2\xa2\xa2"
  "\xa2\xa2\xa2\xa2\xa2\xa2\xa2\xa2\xa2\xa2\xa2\xa2\xa2\xa2\xa3\xa3\xa3\xa3"
  "\xa3\xa3\xa3\xa3\xa3\xa3\xa3\xa3\xa3\xa3\xa3\xa3\xa3\xa3\xa3\xa3\xa3\xa4"
  "\xa4\xa4\xa4\xa4\xa4\xa4\xa4\xa4\xa4\xa4\xa4\xa4\xa4\xa4\xa4\xa4\xa4\xa4"
  "\xa4\xa5\xa5\xa5\xa5\xa5\xa5\xa5\xa5\xa5\xa5\xa5\xa5\xa5\xa5\xa5\xa5\xa5"
  "\xa5\xa5\xa5\xa5\xa6\xa6\xa6\xa6\xa6\xa6\xa6\xa6\xa6\xa6\xa6\xa6\xa6\xa6"
  "\xa6\xa6\xa6\xa6\xa6\xa6\xa7\xa7\xa7\xa7\xa7\xa7\xa7\xa7\xa7\xa7\xa7\xa7"
  "\xa7\xa7\xa7\xa7\xa7\xa7\xa7\xa7\xa7\xa8\xa8\xa8\xa8\xa8\xa8\xa8\xa8\xa8"
  "\xa8\xa8\xa8\xa8\xa8\xa8\xa8\xa8\xa8\xa8\xa8\xa8\xa8\xa9\xa9\xa9\xa9\xa9"
  "\xa9\xa9\xa9\xa9\xa9\xa9\xa9\xa9\xa9\xa9\xa9\xa9\xa9\xa9\xa9\xa9\xaa\xaa"
  "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
  "\xaa\xaa\xab\xab\xab\xab\xab\xab\xab\xab\xab\xab\xab\xab\xab\xab\xab\xab"
  "\xab\xab\xab\xab\xab\xac\xac\xac\xac\xac\xac\xac\xac\xac\xac\xac\xac\xac"
  "\xac\xac\xac\xac\xac\xac\xac\xac\xac\xad\xad\xad\xad\xad\xad\xad\xad\xad"
  "\xad\xad\xad\xad\xad\xad\xad\xad\xad\xad\xad\xad\xad\xae\xae\xae\xae\xae"
  "\xae\xae\xae\xae\xae\xae\xae\xae\xae\xae\xae\xae\xae\xae\xae\xae\xae\xaf"
  "\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf"
  "\xaf\xaf\xaf\xb0\xb0\xb0\xb0\xb0\xb0\xb0\xb0\xb0\xb0\xb0\xb0\xb0\xb0\xb0"
  "\xb0\xb0\xb0\xb0\xb0\xb0\xb0\xb0\xb1\xb1\xb1\xb1\xb1\xb1\xb1\xb1\xb1\xb1"
  "\xb1\xb1\xb1\xb1\xb1\xb1\xb1\xb1\xb1\xb1\xb1\xb1\xb2\xb2\xb2\xb2\xb2\xb2"
  "\xb2\xb2\xb2\xb2\xb2\xb2\xb2\xb2\xb2\xb2\xb2\xb2\xb2\xb2\xb2\xb2\xb2\xb3"
  "\xb3\xb3\xb3\xb3\xb3\xb3\xb3\xb3\xb3\xb3\xb3\xb3\xb3\xb3\xb3\xb3\xb3\xb3"
  "\xb3\xb3\xb3\xb3\xb4\xb4\xb4\xb4\xb4\xb4\xb4\xb4\xb4\xb4\xb4\xb4\xb4\xb4"
  "\xb4\xb4\xb4\xb4\xb4\xb4\xb4\xb4\xb4\xb5\xb5\xb5\xb5\xb5\xb5\xb5\xb5\xb5"
  "\xb5\xb5\xb5\xb5\xb5\xb5\xb5\xb5\xb5\xb5\xb5\xb5\xb5\xb5\xb5\xb6\xb6\xb6"
  "\xb6\xb6\xb6\xb6\xb6\xb6\xb6\xb6\xb6\xb6\xb6\xb6\xb6\xb6\xb6\xb6\xb6\xb6"
  "\xb6\xb6\xb7\xb7\xb7\xb7\xb7\xb7\xb7\xb7\xb7\xb7\xb7\xb7\xb7\xb7\xb7\xb7"
  "\xb7\xb7\xb7\xb7\xb7\xb7\xb7\xb7\xb8\xb8\xb8\xb8\xb8\xb8\xb8\xb8\xb8\xb8"
  "\xb8\xb8\xb8\xb8\xb8\xb8\xb8\xb8\xb8\xb8\xb8\xb8\xb8\xb8\xb9\xb9\xb9\xb9"
  "\xb9\xb9\xb9\xb9\xb9\xb9\xb9\xb9\xb9\xb9\xb9\xb9\xb9\xb9\xb9\xb9\xb9\xb9"
  "\xb9\xba\xba\xba\xba\xba\xba\xba\xba\xba\xba\xba\xba\xba\xba\xba\xba\xba"
  "\xba\xba\xba\xba\xba\xba\xba\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb"
  "\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbc\xbc\xbc\xbc"
  "\xbc\xbc\xbc\xbc\xbc\xbc\xbc\xbc\xbc\xbc\xbc\xbc\xbc\xbc\xbc\xbc\xbc\xbc"
  "\xbc\xbc\xbd\xbd\xbd\xbd\xbd\xbd\xbd\xbd\xbd\xbd\xbd\xbd\xbd\xbd\xbd\xbd"
  "\xbd\xbd\xbd\xbd\xbd\xbd\xbd\xbd\xbd\xbe\xbe\xbe\xbe\xbe\xbe\xbe\xbe\xbe"
  "\xbe\xbe\xbe\xbe\xbe\xbe\xbe\xbe\xbe\xbe\xbe\xbe\xbe\xbe\xbe\xbe\xbf\xbf"
  "\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf"
  "\xbf\xbf\xbf\xbf\xbf\xc0\xc0\xc0\xc0\xc0\xc0\xc0\xc0\xc0\xc0\xc0\xc0\xc0"
  "\xc0\xc0\xc0\xc0\xc0\xc0\xc0\xc0\xc0\xc0\xc0\xc0\xc1\xc1\xc1\xc1\xc1\xc1"
  "\xc1\xc1\xc1\xc1\xc1\xc1\xc1\xc1\xc1\xc1\xc1\xc1\xc1\xc1\xc1\xc1\xc1\xc1"
  "\xc1\xc2\xc2\xc2\xc2\xc2\xc2\xc2\xc2\xc2\xc2\xc2\xc2\xc2\xc2\xc2\xc2\xc2"
  "\xc2\xc2\xc2\xc2\xc2\xc2\xc2\xc2\xc3\xc3\xc3\xc3\xc3\xc3\xc3\xc3\xc3\xc3"
  "\xc3\xc3\xc3\xc3\xc3\xc3\xc3\xc3\xc3\xc3\xc3\xc3\xc3\xc3\xc3\xc3\xc4\xc4"
  "\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4"
  "\xc4\xc4\xc4\xc4\xc4\xc4\xc5\xc5\xc5\xc5\xc5\xc5\xc5\xc5\xc5\xc5\xc5\xc5"
  "\xc5\xc5\xc5\xc5\xc5\xc5\xc5\xc5\xc5\xc5\xc5\xc5\xc5\xc5\xc6\xc6\xc6\xc6"
  "\xc6\xc6\xc6\xc6\xc6\xc6\xc6\xc6\xc6\xc6\xc6\xc6\xc6\xc6\xc6\xc6\xc6\xc6"
  "\xc6\xc6\xc6\xc6\xc6\xc7\xc7\xc7\xc7\xc7\xc7\xc7\xc7\xc7\xc7\xc7\xc7\xc7"
  "\xc7\xc7\xc7\xc7\xc7\xc7\xc7\xc7\xc7\xc7\xc7\xc7\xc7\xc8\xc8\xc8\xc8\xc8"
  "\xc8\xc8\xc8\xc8\xc8\xc8\xc8\xc8\xc8\xc8\xc8\xc8\xc8\xc8\xc8\xc8\xc8\xc8"
  "\xc8\xc8\xc8\xc9\xc9\xc9\xc9\xc9\xc9\xc9\xc9\xc9\xc9\xc9\xc9\xc9\xc9\xc9"
  "\xc9\xc9\xc9\xc9\xc9\xc9\xc9\xc9\xc9\xc9\xc9\xc9\xca\xca\xca\xca\xca\xca"
  "\xca\xca\xca\xca\xca\xca\xca\xca\xca\xca\xca\xca\xca\xca\xca\xca\xca\xca"
  "\xca\xca\xca\xcb\xcb\xcb\xcb\xcb\xcb\xcb\xcb\xcb\xcb\xcb\xcb\xcb\xcb\xcb"
  "\xcb\xcb\xcb\xcb\xcb\xcb\xcb\xcb\xcb\xcb\xcb\xcb\xcc\xcc\xcc\xcc\xcc\xcc"
  "\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc"
  "\xcc\xcc\xcc\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd"
  "\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xce\xce\xce\xce\xce"
  "\xce\xce\xce\xce\xce\xce\xce\xce\xce\xce\xce\xce\xce\xce\xce\xce\xce\xce"
  "\xce\xce\xce\xce\xcf\xcf\xcf\xcf\xcf\xcf\xcf\xcf\xcf\xcf\xcf\xcf\xcf\xcf"
  "\xcf\xcf\xcf\xcf\xcf\xcf\xcf\xcf\xcf\xcf\xcf\xcf\xcf\xcf\xd0\xd0\xd0\xd0"
  "\xd0\xd0\xd0\xd0\xd0\xd0\xd0\xd0\xd0\xd0\xd0\xd0\xd0\xd0\xd0\xd0\xd0\xd0"
  "\xd0\xd0\xd0\xd0\xd0\xd0\xd1\xd1\xd1\xd1\xd1\xd1\xd1\xd1\xd1\xd1\xd1\xd1"
  "\xd1\xd1\xd1\xd1\xd1\xd1\xd1\xd1\xd1\xd1\xd1\xd1\xd1\xd1\xd1\xd1\xd2\xd2"
  "\xd2\xd2\xd2\xd2\xd2\xd2\xd2\xd2\xd2\xd2\xd2\xd2\xd2\xd2\xd2\xd2\xd2\xd2"
  "\xd2\xd2\xd2\xd2\xd2\xd2\xd2\xd2\xd3\xd3\xd3\xd3\xd3\xd3\xd3\xd3\xd3\xd3"
  "\xd3\xd3\xd3\xd3\xd3\xd3\xd3\xd3\xd3\xd3\xd3\xd3\xd3\xd3\xd3\xd3\xd3\xd3"
  "\xd4\xd4\xd4\xd4\xd4\xd4\xd4\xd4\xd4\xd4\xd4\xd4\xd4\xd4\xd4\xd4\xd4\xd4"
  "\xd4\xd4\xd4\xd4\xd4\xd4\xd4\xd4\xd4\xd4\xd4\xd5\xd5\xd5\xd5\xd5\xd5\xd5"
  "\xd5\xd5\xd5\xd5\xd5\xd5\xd5\xd5\xd5\xd5\xd5\xd5\xd5\xd5\xd5\xd5\xd5\xd5"
  "\xd5\xd5\xd5\xd5\xd6\xd6\xd6\xd6\xd6\xd6\xd6\xd6\xd6\xd6\xd6\xd6\xd6\xd6"
  "\xd6\xd6\xd6\xd6\xd6\xd6\xd6\xd6\xd6\xd6\xd6\xd6\xd6\xd6\xd6\xd7\xd7\xd7"
  "\xd7\xd7\xd7\xd7\xd7\xd7\xd7\xd7\xd7\xd7\xd7\xd7\xd7\xd7\xd7\xd7\xd7\xd7"
  "\xd7\xd7\xd7\xd7\xd7\xd7\xd7\xd7\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8"
  "\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8\xd8"
  "\xd8\xd9\xd9\xd9\xd9\xd9\xd9\xd9\xd9\xd9\xd9\xd9\xd9\xd9\xd9\xd9\xd9\xd9"
  "\xd9\xd9\xd9\xd9\xd9\xd9\xd9\xd9\xd9\xd9\xd9\xd9\xd9\xda\xda\xda\xda\xda"
  "\xda\xda\xda\xda\xda\xda\xda\xda\xda\xda\xda\xda\xda\xda\xda\xda\xda\xda"
  "\xda\xda\xda\xda\xda\xda\xda\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb"
  "\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb"
  "\xdb\xdc\xdc\xdc\xdc\xdc\xdc\xdc\xdc\xdc\xdc\xdc\xdc\xdc\xdc\xdc\xdc\xdc"
  "\xdc\xdc\xdc\xdc\xdc\xdc\xdc\xdc\xdc\xdc\xdc\xdc\xdc\xdd\xdd\xdd\xdd\xdd"
  "\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd"
  "\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xde\xde\xde\xde\xde\xde\xde\xde\xde\xde\xde"
  "\xde\xde\xde\xde\xde\xde\xde\xde\xde\xde\xde\xde\xde\xde\xde\xde\xde\xde"
  "\xde\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf"
  "\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xe0\xe0\xe0\xe0"
  "\xe0\xe0\xe0\xe0\xe0\xe0\xe0\xe0\xe0\xe0\xe0\xe0\xe0\xe0\xe0\xe0\xe0\xe0"
  "\xe0\xe0\xe0\xe0\xe0\xe0\xe0\xe0\xe0\xe1\xe1\xe1\xe1\xe1\xe1\xe1\xe1\xe1"
  "\xe1\xe1\xe1\xe1\xe1\xe1\xe1\xe1\xe1\xe1\xe1\xe1\xe1\xe1\xe1\xe1\xe1\xe1"
  "\xe1\xe1\xe1\xe1\xe2\xe2\xe2\xe2\xe2\xe2\xe2\xe2\xe2\xe2\xe2\xe2\xe2\xe2"
  "\xe2\xe2\xe2\xe2\xe2\xe2\xe2\xe2\xe2\xe2\xe2\xe2\xe2\xe2\xe2\xe2\xe2\xe3"
  "\xe3\xe3\xe3\xe3\xe3\xe3\xe3\xe3\xe3\xe3\xe3\xe3\xe3\xe3\xe3\xe3\xe3\xe3"
  "\xe3\xe3\xe3\xe3\xe3\xe3\xe3\xe3\xe3\xe3\xe3\xe3\xe4\xe4\xe4\xe4\xe4\xe4"
  "\xe4\xe4\xe4\xe4\xe4\xe4\xe4\xe4\xe4\xe4\xe4\xe4\xe4\xe4\xe4\xe4\xe4\xe4"
  "\xe4\xe4\xe4\xe4\xe4\xe4\xe4\xe4\xe5\xe5\xe5\xe5\xe5\xe5\xe5\xe5\xe5\xe5"
  "\xe5\xe5\xe5\xe5\xe5\xe5\xe5\xe5\xe5\xe5\xe5\xe5\xe5\xe5\xe5\xe5\xe5\xe5"
  "\xe5\xe5\xe5\xe5\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6"
  "\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe7"
  "\xe7\xe7\xe7\xe7\xe7\xe7\xe7\xe7\xe7\xe7\xe7\xe7\xe7\xe7\xe7\xe7\xe7\xe7"
  "\xe7\xe7\xe7\xe7\xe7\xe7\xe7\xe7\xe7\xe7\xe7\xe7\xe7\xe8\xe8\xe8\xe8\xe8"
  "\xe8\xe8\xe8\xe8\xe8\xe8\xe8\xe8\xe8\xe8\xe8\xe8\xe8\xe8\xe8\xe8\xe8\xe8"
  "\xe8\xe8\xe8\xe8\xe8\xe8\xe8\xe8\xe8\xe9\xe9\xe9\xe9\xe9\xe9\xe9\xe9\xe9"
  "\xe9\xe9\xe9\xe9\xe9\xe9\xe9\xe9\xe9\xe9\xe9\xe9\xe9\xe9\xe9\xe9\xe9\xe9"
  "\xe9\xe9\xe9\xe9\xe9\xe9\xea\xea\xea\xea\xea\xea\xea\xea\xea\xea\xea\xea"
  "\xea\xea\xea\xea\xea\xea\xea\xea\xea\xea\xea\xea\xea\xea\xea\xea\xea\xea"
  "\xea\xea\xea\xeb\xeb\xeb\xeb\xeb\xeb\xeb\xeb\xeb\xeb\xeb\xeb\xeb\xeb\xeb"
  "\xeb\xeb\xeb\xeb\xeb\xeb\xeb\xeb\xeb\xeb\xeb\xeb\xeb\xeb\xeb\xeb\xeb\xec"
  "\xec\xec\xec\xec\xec\xec\xec\xec\xec\xec\xec\xec\xec\xec\xec\xec\xec\xec"
  "\xec\xec\xec\xec\xec\xec\xec\xec\xec\xec\xec\xec\xec\xec\xec\xed\xed\xed"
  "\xed\xed\xed\xed\xed\xed\xed\xed\xed\xed\xed\xed\xed\xed\xed\xed\xed\xed"
  "\xed\xed\xed\xed\xed\xed\xed\xed\xed\xed\xed\xed\xee\xee\xee\xee\xee\xee"
  "\xee\xee\xee\xee\xee\xee\xee\xee\xee\xee\xee\xee\xee\xee\xee\xee\xee\xee"
  "\xee\xee\xee\xee\xee\xee\xee\xee\xee\xef\xef\xef\xef\xef\xef\xef\xef\xef"
  "\xef\xef\xef\xef\xef\xef\xef\xef\xef\xef\xef\xef\xef\xef\xef\xef\xef\xef"
  "\xef\xef\xef\xef\xef\xef\xf0\xf0\xf0\xf0\xf0\xf0\xf0\xf0\xf0\xf0\xf0\xf0"
  "\xf0\xf0\xf0\xf0\xf0\xf0\xf0\xf0\xf0\xf0\xf0\xf0\xf0\xf0\xf0\xf0\xf0\xf0"
  "\xf0\xf0\xf0\xf0\xf1\xf1\xf1\xf1\xf1\xf1\xf1\xf1\xf1\xf1\xf1\xf1\xf1\xf1"
  "\xf1\xf1\xf1\xf1\xf1\xf1\xf1\xf1\xf1\xf1\xf1\xf1\xf1\xf1\xf1\xf1\xf1\xf1"
  "\xf1\xf1\xf2\xf2\xf2\xf2\xf2\xf2\xf2\xf2\xf2\xf2\xf2\xf2\xf2\xf2\xf2\xf2"
  "\xf2\xf2\xf2\xf2\xf2\xf2\xf2\xf2\xf2\xf2\xf2\xf2\xf2\xf2\xf2\xf2\xf2\xf2"
  "\xf3\xf3\xf3\xf3\xf3\xf3\xf3\xf3\xf3\xf3\xf3\xf3\xf3\xf3\xf3\xf3\xf3\xf3"
  "\xf3\xf3\xf3\xf3\xf3\xf3\xf3\xf3\xf3\xf3\xf3\xf3\xf3\xf3\xf3\xf3\xf4\xf4"
  "\xf4\xf4\xf4\xf4\xf4\xf4\xf4\xf4\xf4\xf4\xf4\xf4\xf4\xf4\xf4\xf4\xf4\xf4"
  "\xf4\xf4\xf4\xf4\xf4\xf4\xf4\xf4\xf4\xf4\xf4\xf4\xf4\xf4\xf4\xf5\xf5\xf5"
  "\xf5\xf5\xf5\xf5\xf5\xf5\xf5\xf5\xf5\xf5\xf5\xf5\xf5\xf5\xf5\xf5\xf5\xf5"
  "\xf5\xf5\xf5\xf5\xf5\xf5\xf5\xf5\xf5\xf5\xf5\xf5\xf5\xf5\xf6\xf6\xf6\xf6"
  "\xf6\xf6\xf6\xf6\xf6\xf6\xf6\xf6\xf6\xf6\xf6\xf6\xf6\xf6\xf6\xf6\xf6\xf6"
  "\xf6\xf6\xf6\xf6\xf6\xf6\xf6\xf6\xf6\xf6\xf6\xf6\xf7\xf7\xf7\xf7\xf7\xf7"
  "\xf7\xf7\xf7\xf7\xf7\xf7\xf7\xf7\xf7\xf7\xf7\xf7\xf7\xf7\xf7\xf7\xf7\xf7"
  "\xf7\xf7\xf7\xf7\xf7\xf7\xf7\xf7\xf7\xf7\xf7\xf8\xf8\xf8\xf8\xf8\xf8\xf8"
  "\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8"
  "\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf9\xf9\xf9\xf9\xf9\xf9\xf9"
  "\xf9\xf9\xf9\xf9\xf9\xf9\xf9\xf9\xf9\xf9\xf9\xf9\xf9\xf9\xf9\xf9\xf9\xf9"
  "\xf9\xf9\xf9\xf9\xf9\xf9\xf9\xf9\xf9\xf9\xfa\xfa\xfa\xfa\xfa\xfa\xfa\xfa"
  "\xfa\xfa\xfa\xfa\xfa\xfa\xfa\xfa\xfa\xfa\xfa\xfa\xfa\xfa\xfa\xfa\xfa\xfa"
  "\xfa\xfa\xfa\xfa\xfa\xfa\xfa\xfa\xfa\xfa\xfb\xfb\xfb\xfb\xfb\xfb\xfb\xfb"
  "\xfb\xfb\xfb\xfb\xfb\xfb\xfb\xfb\xfb\xfb\xfb\xfb\xfb\xfb\xfb\xfb\xfb\xfb"
  "\xfb\xfb\xfb\xfb\xfb\xfb\xfb\xfb\xfb\xfc\xfc\xfc\xfc\xfc\xfc\xfc\xfc\xfc"
  "\xfc\xfc\xfc\xfc\xfc\xfc\xfc\xfc\xfc\xfc\xfc\xfc\xfc\xfc\xfc\xfc\xfc\xfc"
  "\xfc\xfc\xfc\xfc\xfc\xfc\xfc\xfc\xfc\xfd\xfd\xfd\xfd\xfd\xfd\xfd\xfd\xfd"
  "\xfd\xfd\xfd\xfd\xfd\xfd\xfd\xfd\xfd\xfd\xfd\xfd\xfd\xfd\xfd\xfd\xfd\xfd"
  "\xfd\xfd\xfd\xfd\xfd\xfd\xfd\xfd\xfd\xfd\xfe\xfe\xfe\xfe\xfe\xfe\xfe\xfe"
  "\xfe\xfe\xfe\xfe\xfe\xfe\xfe\xfe\xfe\xfe\xfe\xfe\xfe\xfe\xfe\xfe\xfe\xfe"
  "\xfe\xfe\xfe\xfe\xfe\xfe\xfe\xfe\xfe\xfe\xff\xff\xff\xff\xff\xff\xff\xff"
  "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
;

static unsigned char linear_to_sRGB (float linear) FUNCTION;
static unsigned char linear_to_sRGB (float linear)
{
  unsigned slot;
  slot = linear*4096.0 + 0.5;
  if(slot>4095)
    {
      if(linear>0.5)
        slot = 4095;
      else
        slot = 0;
    }
  return linear_to_sRGB_table[slot];
}

typedef struct multichan {
  double L,hue,sat; // L,a,b would be better -- 2-way formula unknown
  unsigned char or,og,ob,alpha;  // old 8-bit values
} multichan;

#define X0 ((double)0.9505)
#define Y0 ((double)1.0000)
#define Z0 ((double)1.0890)
#define u0_prime ( (4.0 * X0) / (X0 + 15.0*Y0 + 3.0*Z0) )
#define v0_prime ( (9.0 * Y0) / (X0 + 15.0*Y0 + 3.0*Z0) )


static void fill_multichan(multichan *mc, double *up, double *vp)
{
  double X,Y,Z,u,v;
  double u_prime, v_prime; /* temp, part of official formula */
  double Y_norm, fract; /* severely temp */

  double r = sRGB_to_linear_table[mc->or];
  double g = sRGB_to_linear_table[mc->og];
  double b = sRGB_to_linear_table[mc->ob];

  // coordinate change, RGB --> XYZ
  X = 0.4124*r + 0.3576*g + 0.1805*b;
  Y = 0.2126*r + 0.7152*g + 0.0722*b;
  Z = 0.0193*r + 0.1192*g + 0.9505*b;
  
  // XYZ --> Luv
  Y_norm = Y/Y0;
  fract = 1.0 / (X + 15.0*Y + 3.0*Z);
  u_prime = 4.0*X*fract;
  v_prime = 9.0*Y*fract;
  mc->L = (Y_norm>0.008856) ? 116.0*pow(Y_norm,1.0/3.0)-16.0 : 903.3*Y_norm;
  u = 13.0*mc->L*(u_prime - u0_prime);
  v = 13.0*mc->L*(v_prime - v0_prime);

  mc->sat = sqrt(u*u + v*v);
  mc->hue = atan2(u,v);
  if(up) *up = u;
  if(vp) *vp = v;
}


static double tint_part_1(multichan *work, SDL_Surface *in)
{
  int xx,yy;
  double u_total = 0;
  double v_total = 0;

  SDL_LockSurface(in);
  for (yy = 0; yy < in->h; yy++)
    {
      for (xx = 0; xx < in->w; xx++)
        {
          multichan *mc = work+yy*in->w+xx;
          // put pixels into a more tolerable form
          SDL_GetRGBA(getpixel(in, xx, yy),
                      in->format,
                      &mc->or, &mc->og, &mc->ob, &mc->alpha);
          double u,v;
          fill_multichan(mc,&u,&v);
          // average out u and v, giving more weight to opaque high-saturation pixels
          // (this is to take an initial guess at the primary hue)
          u_total += mc->alpha * u * mc->sat;
          v_total += mc->alpha * v * mc->sat;
        }
    }
  SDL_UnlockSurface(in);

  return atan2(u_total,v_total);
}


static void change_colors(SDL_Surface *out, multichan *work, double hue_range, multichan *key_color_ptr)
{
  double lower_hue_1,upper_hue_1,lower_hue_2,upper_hue_2;
  int xx,yy;

  // prepare source and destination color info
  // should reset hue_range or not? won't bother for now
  multichan key_color = *key_color_ptr; // want to work from a copy, for safety
  lower_hue_1 = key_color.hue - hue_range;
  upper_hue_1 = key_color.hue + hue_range;
  if (lower_hue_1 < -M_PI)
    {
      lower_hue_2 = lower_hue_1 + 2 * M_PI;
      upper_hue_2 = upper_hue_1 + 2 * M_PI;
    }
  else
    {
      lower_hue_2 = lower_hue_1 - 2 * M_PI;
      upper_hue_2 = upper_hue_1 - 2 * M_PI;
    }

  // get the destination color set up
  multichan dst;
  dst.or = color_hexes[cur_color][0];
  dst.og = color_hexes[cur_color][1];
  dst.ob = color_hexes[cur_color][2];
  fill_multichan(&dst,NULL,NULL);
  double satratio = dst.sat / key_color.sat;
  double slope = (dst.L-key_color.L)/dst.sat;

  SDL_LockSurface(out);
  for (yy = 0; yy < out->h; yy++)
    {
      for (xx = 0; xx < out->w; xx++)
	{
	  multichan *mc = work+yy*out->w+xx;

          double oldhue = mc->hue;

          // if not in the first range, and not in the second range, skip this one
          // (really should alpha-blend as a function of hue angle difference)
          if( (oldhue<lower_hue_1 || oldhue>upper_hue_1) && (oldhue<lower_hue_2 || oldhue>upper_hue_2) )
            {
              putpixel(out, xx, yy, SDL_MapRGBA(out->format, mc->or, mc->og, mc->ob, mc->alpha));
	      continue;
	    }

          // Modify the pixel
          double old_sat = mc->sat;
          double newsat = old_sat * satratio;
          double L = mc->L;
          if(dst.sat>0)
	    L  += newsat * slope; // not greyscale destination
          else
	    L  += old_sat*(dst.L-key_color.L)/key_color.sat;

          // convert from L,u,v all the way back to sRGB with 8-bit channels
          double X,Y,Z;
          double u_prime, v_prime; /* temp, part of official formula */
          unsigned tries = 3;
trysat:;
          double u = newsat * sin(dst.hue);
          double v = newsat * cos(dst.hue);

          // Luv to XYZ
          u_prime = u/(13.0*L)+u0_prime;
          v_prime = v/(13.0*L)+v0_prime;
          Y = (L>7.99959199307) ? Y0*pow((L+16.0)/116.0,3.0) : Y0*L/903.3;
          X = 2.25*Y*u_prime/v_prime;
          Z = (3.0*Y - 0.75*Y*u_prime)/v_prime - 5.0*Y;
          
          // coordinate change: XYZ to RGB
          double r =  3.2410*X + -1.5374*Y + -0.4986*Z;
          double g = -0.9692*X +  1.8760*Y +  0.0416*Z;
          double b =  0.0556*X + -0.2040*Y +  1.0570*Z;
          
          // If it is out of gamut, try to de-saturate it a few times before truncating.
          // (the linear_to_sRGB function will truncate)
          if((r<=-0.5 || g<=-0.5 || b<=-0.5 || r>=255.5 || g>=255.5 || b>=255.5) && tries--)
            {
              newsat *= 0.8;
              goto trysat;
            }

	  putpixel(out, xx, yy,
		   SDL_MapRGBA(out->format, linear_to_sRGB(r), linear_to_sRGB(g), linear_to_sRGB(b), mc->alpha));
	}
    }
  SDL_UnlockSurface(out);
}


static multichan *find_most_saturated(double initial_hue, multichan * work,
				      unsigned num, double * hue_range_ptr)
{
  // find the most saturated pixel near the initial hue guess
  multichan *key_color_ptr = NULL;
  double hue_range;
  unsigned i;
  
  switch (inf_stamps[cur_stamp]->tinter)
  {
    default:
    case TINTER_NORMAL:
      hue_range = 18*M_PI/180.0; // plus or minus 18 degrees search, 27 replace
      break;
    case TINTER_NARROW:
      hue_range = 6*M_PI/180.0;  // plus or minus 6 degrees search, 9 replace
      break;
    case TINTER_ANYHUE:
      hue_range = M_PI;          // plus or minus 180 degrees
      break;
  }

hue_range_retry:;
  double max_sat = 0;
  double lower_hue_1 = initial_hue - hue_range;
  double upper_hue_1 = initial_hue + hue_range;
  double lower_hue_2;
  double upper_hue_2;
  multichan * mc;
  
  if (lower_hue_1 < -M_PI)
    {
      lower_hue_2 = lower_hue_1 + 2 * M_PI;
      upper_hue_2 = upper_hue_1 + 2 * M_PI;
    }
  else
    {
      lower_hue_2 = lower_hue_1 - 2 * M_PI;
      upper_hue_2 = upper_hue_1 - 2 * M_PI;
    }
  
  i = num;
  while (i--)
    {
      mc = work + i;

      // if not in the first range, and not in the second range, skip this one
      // 
      if ((mc->hue<lower_hue_1 || mc->hue>upper_hue_1) &&
	  (mc->hue<lower_hue_2 || mc->hue>upper_hue_2))
	continue;
      
      if(mc->sat > max_sat)
      {
	max_sat = mc->sat;
	key_color_ptr = mc;
      }
    }
  
  if (!key_color_ptr)
    {
      hue_range *= 1.5;
      
      if (hue_range < M_PI)
	goto hue_range_retry;
    }
  
  *hue_range_ptr = hue_range;
  
  return key_color_ptr;
}


static void vector_tint_surface(SDL_Surface * out, SDL_Surface * in)
{
  int xx,yy;

  double r = sRGB_to_linear_table[color_hexes[cur_color][0]];
  double g = sRGB_to_linear_table[color_hexes[cur_color][1]];
  double b = sRGB_to_linear_table[color_hexes[cur_color][2]];

  SDL_LockSurface(in);
  for (yy = 0; yy < in->h; yy++)
    {
      for (xx = 0; xx < in->w; xx++)
        {
          unsigned char r8, g8, b8, a8;
          SDL_GetRGBA(getpixel(in, xx, yy),
                      in->format,
                      &r8, &g8, &b8, &a8);
          // get the linear greyscale value
          double old = sRGB_to_linear_table[r8]*0.2126 + sRGB_to_linear_table[g8]*0.7152 + sRGB_to_linear_table[b8]*0.0722;

	  putpixel(out, xx, yy,
		   SDL_MapRGBA(out->format, linear_to_sRGB(r*old), linear_to_sRGB(g*old), linear_to_sRGB(b*old), a8));
        }
    }
  SDL_UnlockSurface(in);
}


static void tint_surface(SDL_Surface * tmp_surf, SDL_Surface * surf_ptr)
{
  unsigned width = surf_ptr->w;
  unsigned height = surf_ptr->h;
  multichan * work;
  multichan * key_color_ptr;
  double initial_hue;
  double hue_range;
  
 
  work = malloc(sizeof(multichan) * width * height);

  if (work)
  {
    initial_hue = tint_part_1(work, surf_ptr);

    key_color_ptr = find_most_saturated(initial_hue, work,
		    			width * height, &hue_range);

    if (key_color_ptr)
    {
      // wider for processing than for searching
      hue_range *= 1.5;
      
      change_colors(tmp_surf, work, hue_range, key_color_ptr);
    
      free(work);
      return;
    }
  
    free(work);
  }

  /* Failed!  Fall back: */

  //fprintf(stderr, "Falling back to tinter=vector, "
  //		    "this should be in the *.dat file\n");
  
  vector_tint_surface(tmp_surf, surf_ptr);
}


//////////////////////////////////////////////////////////////////////

/* Draw using the current stamp: */

static void stamp_draw(int x, int y)
{
  SDL_Rect src, dest;
  SDL_Surface * tmp_surf, * surf_ptr, * final_surf;
  Uint32 amask;
  Uint8 r, g, b, a;
  int xx, yy, dont_free_tmp_surf, base_x, base_y;
  

  /* Use a pre-mirrored version, if there is one? */
  
  if (state_stamps[cur_stamp]->mirrored)
    {
      if (img_stamps_premirror[cur_stamp] != NULL)
	{
	  /* Use pre-mirrored one! */
            
	  surf_ptr = img_stamps_premirror[cur_stamp];
	}
      else
	{
	  /* Use normal (only) one, and mirror it ourselves: */
            
	  surf_ptr = img_stamps[cur_stamp];
	}
    }
  else
    {
      /* Not mirrored: */

      surf_ptr = img_stamps[cur_stamp];
    }


  /* Create a temp surface to play with: */
  
  if (stamp_colorable(cur_stamp) || stamp_tintable(cur_stamp))
    {
      amask = ~(surf_ptr->format->Rmask |
		surf_ptr->format->Gmask |
		surf_ptr->format->Bmask);

      tmp_surf =
	SDL_CreateRGBSurface(SDL_SWSURFACE,
			     surf_ptr->w,
			     surf_ptr->h,
			     surf_ptr->format->BitsPerPixel,
			     surf_ptr->format->Rmask,
			     surf_ptr->format->Gmask,
			     surf_ptr->format->Bmask,
			     amask);

      if (tmp_surf == NULL)
	{
	  fprintf(stderr, "\nError: Can't render the colored stamp!\n"
		  "The Simple DirectMedia Layer error that occurred was:\n"
		  "%s\n\n", SDL_GetError());

	  cleanup();
	  exit(1);
	}

      dont_free_tmp_surf = 0;
    }
  else
    {
      /* Not altering color; no need to create temp surf if we don't use it! */

      tmp_surf = NULL;
      dont_free_tmp_surf = 1;
    }


  /* Alter the stamp's color, if needed: */

  if (stamp_colorable(cur_stamp))
    {
      /* Render the stamp in the chosen color: */

      /* FIXME: It sucks to render this EVERY TIME.  Why not just when
	 they pick the color, or pick the stamp, like with brushes? */

      /* Render the stamp: */

      SDL_LockSurface(surf_ptr);
      SDL_LockSurface(tmp_surf);

      for (yy = 0; yy < surf_ptr->h; yy++)
	{
	  for (xx = 0; xx < surf_ptr->w; xx++)
	    {
	      SDL_GetRGBA(getpixel(surf_ptr, xx, yy),
			  surf_ptr->format,
			  &r, &g, &b, &a);
	
	      putpixel(tmp_surf, xx, yy,
		       SDL_MapRGBA(tmp_surf->format,
				   color_hexes[cur_color][0],
				   color_hexes[cur_color][1],
				   color_hexes[cur_color][2],
				   a));
	    }
	}

      SDL_UnlockSurface(tmp_surf);
      SDL_UnlockSurface(surf_ptr);
    }
  else if (stamp_tintable(cur_stamp))
    {
      if (inf_stamps[cur_stamp]->tinter == TINTER_VECTOR)
        vector_tint_surface(tmp_surf, surf_ptr);
      else
        tint_surface(tmp_surf, surf_ptr);
    }
  else
    {
      /* No color change, just use it! */

      tmp_surf = surf_ptr;
    }

  
  /* Shrink or grow it! */

#ifdef USE_HQ4X
  if (CAN_USE_HQ4X)
    {
      /* Use high quality 4x filter! */
      
      
      /* Make the new surface for the scaled image: */
      
      amask = ~(img_stamps[cur_stamp]->format->Rmask |
		img_stamps[cur_stamp]->format->Gmask |
		img_stamps[cur_stamp]->format->Bmask);
      
      final_surf = SDL_CreateRGBSurface(SDL_SWSURFACE,
					img_stamps[cur_stamp]->w * 4,
					img_stamps[cur_stamp]->h * 4,
					img_stamps[cur_stamp]->format->BitsPerPixel,
					img_stamps[cur_stamp]->format->Rmask,
					img_stamps[cur_stamp]->format->Gmask,
					img_stamps[cur_stamp]->format->Bmask,
					amask);
      
      if (final_surf == NULL)
	{
	  fprintf(stderr, "\nError: Can't build stamp thumbnails\n"
		  "The Simple DirectMedia Layer error that occurred was:\n"
		  "%s\n\n", SDL_GetError());
	  
	  cleanup();
	  exit(1);
	}
      
      
      hq4x_32(tmp_surf, final_surf, RGBtoYUV);
    }
  else
#endif
    {
      final_surf = thumbnail(tmp_surf, CUR_STAMP_W, CUR_STAMP_H, 0);
    }
  
  
  /* Where it will go? */
  
  base_x = x - (CUR_STAMP_W+1)/2;
  base_y = y - (CUR_STAMP_H+1)/2;
   

  /* And blit it! */
  
  if (state_stamps[cur_stamp]->flipped)
    {
      /* Flipped! */
	  
      if (state_stamps[cur_stamp]->mirrored &&
	  img_stamps_premirror[cur_stamp] == NULL)
	{
	  /* Mirrored, too! */

	  for (yy = 0; yy < final_surf->h; yy++)
	    {
	      for (xx = 0; xx < final_surf->w; xx++)
		{
		  src.x = final_surf->w - 1 - xx;
		  src.y = final_surf->h - 1 - yy;
		  src.w = 1;
		  src.h = 1;

		  dest.x = base_x + xx;
		  dest.y = base_y + yy;

		  SDL_BlitSurface(final_surf, &src, canvas, &dest);
		}
	    }
	}
      else
	{
	  /* (Only flipped) */
      
	  for (yy = 0; yy < final_surf->h; yy++)
	    {
	      src.x = 0;
	      src.y = final_surf->h - 1 - yy;
	      src.w = final_surf->w;
	      src.h = 1;

	      dest.x = base_x;
	      dest.y = base_y + yy;

	      SDL_BlitSurface(final_surf, &src, canvas, &dest);
	    }
	}
    }
  else
    {
      if (state_stamps[cur_stamp]->mirrored &&
	  img_stamps_premirror[cur_stamp] == NULL)
	{
	  /* Mirrored! */

	  for (xx = 0; xx < final_surf->w; xx++)
	    {
	      src.x = final_surf->w - 1 - xx;
	      src.y = 0;
	      src.w = 1;
	      src.h = final_surf->h;

	      dest.x = base_x + xx;
	      dest.y = base_y;

	      SDL_BlitSurface(final_surf, &src, canvas, &dest);
	    }
	}
      else
	{
	  /* Not altered at all */

	  dest.x = base_x;
	  dest.y = base_y;
      
	  SDL_BlitSurface(final_surf, NULL, canvas, &dest);
	}
    }

  update_canvas(x - (CUR_STAMP_W+1)/2,
		y - (CUR_STAMP_H+1)/2,
                x + (CUR_STAMP_W+1)/2,
		y + (CUR_STAMP_H+1)/2);
  
  /* Free the temporary surfaces */

  if (!dont_free_tmp_surf)
    SDL_FreeSurface(tmp_surf);
  
  SDL_FreeSurface(final_surf);
}


/* Draw using the current brush: */

static void magic_draw(int x1, int y1, int x2, int y2, int button_down)
{
  int dx, dy, y;
  int orig_x1, orig_y1, orig_x2, orig_y2, tmp;
  float m, b;

  if (cur_magic == MAGIC_RAINBOW)
    rainbow_color = (rainbow_color + 1) % NUM_RAINBOW_COLORS;

  orig_x1 = x1;
  orig_y1 = y1;

  orig_x2 = x2;
  orig_y2 = y2;


  dx = x2 - x1;
  dy = y2 - y1;

  if (dx != 0)
    {
      m = ((float) dy) / ((float) dx);
      b = y1 - m * x1;

      if (x2 >= x1)
	dx = 1;
      else
	dx = -1;


      while (x1 != x2)
	{
	  y1 = m * x1 + b;
	  y2 = m * (x1 + dx) + b;

          if (y1 > y2)
	    {
	      for (y = y1; y >= y2; y--)
		blit_magic(x1, y, button_down);
	    }
          else
	    {
	      for (y = y1; y <= y2; y++)
		blit_magic(x1, y, button_down);
	    }
	
	  x1 = x1 + dx;
	}
    }
  else
    {
      if (y1 > y2)
	{
	  for (y = y1; y >= y2; y--)
	    blit_magic(x1, y, button_down);
	}
      else
	{
	  for (y = y1; y <= y2; y++)
	    blit_magic(x1, y, button_down);
	}
    }

  if (orig_x1 > orig_x2)
    {
      tmp = orig_x1;
      orig_x1 = orig_x2;
      orig_x2 = tmp;
    }

  if (orig_y1 > orig_y2)
    {
      tmp = orig_y1;
      orig_y1 = orig_y2;
      orig_y2 = tmp;
    }


  /* Play sound: */

  if (cur_magic == MAGIC_DRIP)
    playsound(0, SND_DRIP, 0);
  else if (cur_magic == MAGIC_CHALK)
    playsound(0, SND_CHALK, 0);
  else if (cur_magic == MAGIC_SPARKLES)
    playsound(0, SND_SPARKLES1 + (rand() % 2), 0);
  else if (cur_magic == MAGIC_FLIP)
    playsound(0, SND_FLIP, 0);
  else if (cur_magic == MAGIC_MIRROR)
    playsound(0, SND_MIRROR, 0);
  else if (cur_magic == MAGIC_NEGATIVE)
    playsound(0, SND_NEGATIVE, 0);
  else if (cur_magic == MAGIC_BLUR)
    playsound(0, SND_BLUR, 0);
  else if (cur_magic == MAGIC_BLOCKS && ((rand() % 10) < 5))
    playsound(0, SND_BLOCKS, 0);
  else if (cur_magic == MAGIC_FADE)
    playsound(0, SND_FADE, 0);
  else if (cur_magic == MAGIC_DARKEN)
    playsound(0, SND_DARKEN, 0);
  else if (cur_magic == MAGIC_RAINBOW)
    playsound(0, SND_RAINBOW, 0);
  else if (cur_magic == MAGIC_SMUDGE)
    playsound(0, SND_SMUDGE, 0);
  else if (cur_magic == MAGIC_CARTOON)
    playsound(0, SND_CARTOON, 0);
  else if (cur_magic == MAGIC_TINT)
    playsound(0, SND_TINT, 0);

  /* FIXME: Need sounds for:
     Smudge
     Tint
     Grass (mower?)
     Cartoon */


  /* FIXME: Arbitrary? */

  update_canvas(orig_x1 - 32, orig_y1 - 32,
		orig_x2 + 32, orig_y2 + 32);
}


// this one rounds down
static int log2int(int x)
{
  int y = 0;
  if(x<=1)
    return 0;
  x >>= 1;
  while(x)
    {
      x >>= 1;
      y++;
    }
  return y;
}

static void do_brick(int x, int y, int w, int h)
{
  SDL_Rect dest;

  dest.x = x;
  dest.y = y;
  dest.w = w;
  dest.h = h;

  // brick color: 127,76,73
  double rand_r = rand()/(double)RAND_MAX;
  double rand_g = rand()/(double)RAND_MAX;
  double base_r = sRGB_to_linear_table[color_hexes[cur_color][0]]*1.5 + sRGB_to_linear_table[127]*5.0 + rand_r;
  double base_g = sRGB_to_linear_table[color_hexes[cur_color][1]]*1.5 + sRGB_to_linear_table[76] *5.0 + rand_g;
  double base_b = sRGB_to_linear_table[color_hexes[cur_color][2]]*1.5 + sRGB_to_linear_table[73] *5.0 + (rand_r+rand_g*2.0)/3.0;

  Uint8 r = linear_to_sRGB(base_r/7.5);
  Uint8 g = linear_to_sRGB(base_g/7.5);
  Uint8 b = linear_to_sRGB(base_b/7.5);


  SDL_FillRect(canvas, &dest, SDL_MapRGB(canvas->format, r, g, b));

// This is better for debugging brick layout:
//  SDL_FillRect(canvas, &dest, SDL_MapRGB(canvas->format, rand()&255, rand()&255, rand()&255));
}

/* Draw the current brush in the current color: */

static void blit_magic(int x, int y, int button_down)
{
  int xx, yy, w, h;
  Uint32 colr;
  Uint8 r, g, b, a;
  SDL_Surface * last;
  SDL_Rect src, dest;
  int undo_ctr;


  /* In case we need to use the current canvas (just saved to undo buf)... */

  if (cur_undo > 0)
    undo_ctr = cur_undo - 1;
  else
    undo_ctr = NUM_UNDO_BUFS - 1;

  last = undo_bufs[undo_ctr];


  brush_counter++;

  if (brush_counter >= 4)  /* FIXME: Arbitrary? */
    {
      brush_counter = 0;


      if (cur_magic == MAGIC_BLUR)
	{
	  double state[32][32][3];
	  unsigned i = 32*32;

	  SDL_LockSurface(canvas);

	  while (i--)
	    {
	      int iy = i>>5;
	      int ix = i&0x1f;
	      // is it not on the circle of radius sqrt(220) at location 16,16?
	      if ( (ix-16)*(ix-16) + (iy-16)*(iy-16) > 220)
	        continue;
	      // it is on the circle, so grab it

	      SDL_GetRGB(getpixel(canvas, x+ix-16, y+iy-16), last->format, &r, &g, &b);
	      state[ix][iy][0] = sRGB_to_linear_table[r];
	      state[ix][iy][1] = sRGB_to_linear_table[g];
	      state[ix][iy][2] = sRGB_to_linear_table[b];
	    }
	  i = 32*32;
	  while (i--)
	    {
	      double lr, lg, lb;  // linear red,green,blue
	      double weight;
	      int iy = i>>5;
	      int ix = i&0x1f;
	      int r2 = (ix-16)*(ix-16) + (iy-16)*(iy-16); // radius squared

	      // is it not on the circle of radius sqrt(140) at location 16,16?
	      if ( r2 > 140)
	        continue;

	      // It is on the circle, but how strongly will it be affected?
	      // This is lame; we should use something like a gaussian or cosine
	      // via a lookup table. (inverted, because this is the center weight)
	      weight = r2/16.0+3.0;

	      // Sampling more points would be good too, though it'd be slower.

	      lr =                     state[ix][iy-1][0]
	        + state[ix-1][iy][0] + state[ix][iy  ][0]*weight + state[ix+1][iy][0]
	                             + state[ix][iy+1][0];

	      lg =                     state[ix][iy-1][1]
	        + state[ix-1][iy][1] + state[ix][iy  ][1]*weight + state[ix+1][iy][1]
	                             + state[ix][iy+1][1];

	      lb =                     state[ix][iy-1][2]
	        + state[ix-1][iy][2] + state[ix][iy  ][2]*weight + state[ix+1][iy][2]
	                             + state[ix][iy+1][2];

              lr /= weight+4.0;
              lg /= weight+4.0;
              lb /= weight+4.0;
	      putpixel(canvas, x+ix-16, y+iy-16, SDL_MapRGB(canvas->format, linear_to_sRGB(lr), linear_to_sRGB(lg), linear_to_sRGB(lb)));
	    }
	  SDL_UnlockSurface(canvas);
	}
      else if (cur_magic == MAGIC_BLOCKS)
	{
	  /* Put x/y on exact grid points: */

	  x = (x / 4) * 4;
	  y = (y / 4) * 4;

	  SDL_LockSurface(last);
	  SDL_LockSurface(canvas);

	  for (yy = y - 8; yy < y + 8; yy = yy + 4)
	    {
	      for (xx = x - 8; xx < x + 8; xx = xx + 4)
		{
		  Uint32 pix[16];
		  Uint32 p_or = 0;
		  Uint32 p_and = ~0;
		  unsigned i = 16;
		  while(i--)
		    {
		      Uint32 p_tmp;
		      p_tmp = getpixel(last, xx+(i>>2), yy+(i&3));
		      p_or |= p_tmp;
		      p_and &= p_tmp;
		      pix[i] = p_tmp;
		    }
		  if(p_or==p_and)  // if all pixels the same already
		    {
		      SDL_GetRGB(p_or, last->format, &r, &g, &b);
		    }
		  else  // nope, must average them
		    {
		      double r_sum = 0.0;
		      double g_sum = 0.0;
		      double b_sum = 0.0;
		      i = 16;
		      while(i--)
		        {
		          SDL_GetRGB(pix[i], last->format, &r, &g, &b);
		          r_sum += sRGB_to_linear_table[r];
		          g_sum += sRGB_to_linear_table[g];
		          b_sum += sRGB_to_linear_table[b];
		        }
		      r = linear_to_sRGB(r_sum/16.0);
		      g = linear_to_sRGB(g_sum/16.0);
		      b = linear_to_sRGB(b_sum/16.0);
		    }
	
		  /* Draw block: */

		  dest.x = xx;
		  dest.y = yy;
		  dest.w = 4;
		  dest.h = 4;

		  SDL_FillRect(canvas, &dest, SDL_MapRGB(canvas->format, r, g, b));
		}
	    }
	
	  SDL_UnlockSurface(canvas);
	  SDL_UnlockSurface(last);
	}
      else if (cur_magic == MAGIC_LARGEBRICK || cur_magic == MAGIC_SMALLBRICK)
	{
	  // "specified" means the brick itself, w/o morter
	  // "nominal" means brick-to-brick (includes morter)
	  int specified_width, specified_height, specified_length;
	  int brick_x, brick_y;

	  int vertical_joint   = 2; // between a brick and the one above/below
	  int horizontal_joint = 2; // between a brick and the one to the side
	  int nominal_width    = 18;
	  int nominal_height   = 12; // 11 to 14, for joints of 2
	  int nominal_length   = 36; // 3x the above, 2x the width

#if 0
	  if (cur_magic == MAGIC_SMALLBRICK)
	    {
	      vertical_joint   = 1; // between a brick and the one above/below
	      horizontal_joint = 1; // between a brick and the one to the side
	      nominal_width    = 9;
	      nominal_height   = 6; // 11 to 14, for joints of 2
	      nominal_length   = 18; // 3x the above, 2x the width
	    }
#endif
#if 0
	  if (cur_magic == MAGIC_LARGEBRICK)
	    {
	      vertical_joint   = 3; // between a brick and the one above/below
	      horizontal_joint = 3; // between a brick and the one to the side
	      nominal_width    = 27;
	      nominal_height   = 18; // 11 to 14, for joints of 2
	      nominal_length   = 54; // 3x the above, 2x the width
	    }
#endif
#if 1
	  if (cur_magic == MAGIC_LARGEBRICK)
	    {
	      vertical_joint   = 4; // between a brick and the one above/below
	      horizontal_joint = 4; // between a brick and the one to the side
	      nominal_width    = 36;
	      nominal_height   = 24; // 11 to 14, for joints of 2
	      nominal_length   = 72; // 3x the above, 2x the width
	    }
#endif

	  specified_width  = nominal_width - horizontal_joint;
	  specified_height = nominal_height - vertical_joint;
	  specified_length = nominal_length - horizontal_joint;
	  static unsigned char *map;
	  static int x_count;
	  static int y_count;
          if (!button_down)
            {
              if (map)
                free(map);
              // the "+ 3" allows for both ends and misalignment
              x_count = (canvas->w + nominal_width  - 1) / nominal_width + 3;
              y_count = (canvas->h + nominal_height - 1) / nominal_height + 3;
              map = calloc(x_count,y_count);
            }

	  brick_x = x / nominal_width;
	  brick_y = y / nominal_height;

          unsigned char *mybrick = map + brick_x+1 + (brick_y+1)*x_count;

          if ( (unsigned)x < (unsigned)canvas->w && (unsigned)y < (unsigned)canvas->h )
            {
              if(!*mybrick)
                {
                  int my_x = brick_x*nominal_width;
                  int my_w = specified_width;
                  *mybrick = 1;
                  SDL_LockSurface(canvas);
                  if((brick_y^brick_x)&1)
                    {
                      if(mybrick[1])
                        my_w = specified_length;
                    }
                  else
                    if(mybrick[-1])
                      {
                        my_x -= nominal_width;
                        my_w = specified_length;
                      }
                  do_brick(my_x, brick_y*nominal_height, my_w,  specified_height);
                  SDL_UnlockSurface(canvas);
                  //            upper left corner        and     lower right corner
                  update_canvas(brick_x*nominal_width-nominal_width, brick_y*nominal_height-vertical_joint, brick_x*nominal_width+specified_length,  (brick_y+1)*nominal_height);
                }
            }
	}
      else if (cur_magic == MAGIC_SMUDGE)
	{
	  static double state[32][32][3];
	  unsigned i = 32*32;
	  double rate = button_down ? 0.5 : 0.0;

	  SDL_LockSurface(canvas);

	  while (i--)
	    {
	      int iy = i>>5;
	      int ix = i&0x1f;
	      // is it not on the circle of radius sqrt(120) at location 16,16?
	      if ( (ix-16)*(ix-16) + (iy-16)*(iy-16) > 120)
	        continue;
	      // it is on the circle, so grab it

	      SDL_GetRGB(getpixel(canvas, x+ix-16, y+iy-16), last->format, &r, &g, &b);
	      state[ix][iy][0] = rate*state[ix][iy][0] + (1.0-rate)*sRGB_to_linear_table[r];
	      state[ix][iy][1] = rate*state[ix][iy][1] + (1.0-rate)*sRGB_to_linear_table[g];
	      state[ix][iy][2] = rate*state[ix][iy][2] + (1.0-rate)*sRGB_to_linear_table[b];

	      // opacity 100% --> new data not blended w/ existing data
	      putpixel(canvas, x+ix-16, y+iy-16, SDL_MapRGB(canvas->format, linear_to_sRGB(state[ix][iy][0]), linear_to_sRGB(state[ix][iy][1]), linear_to_sRGB(state[ix][iy][2])));
	    }
	  SDL_UnlockSurface(canvas);
	}
      else if (cur_magic == MAGIC_NEGATIVE)
	{
	  SDL_LockSurface(last);
	  SDL_LockSurface(canvas);

	  for (yy = y - 16; yy < y + 16; yy++)
	    {
	      for (xx = x - 16; xx < x + 16; xx++)
		{
		  SDL_GetRGB(getpixel(last, xx, yy), last->format,
			     &r, &g, &b);
	
		  r = 0xFF - r;
		  g = 0xFF - g;
		  b = 0xFF - b;

		  putpixel(canvas, xx, yy, SDL_MapRGB(canvas->format, r, g, b));
		}
	    }

	  SDL_UnlockSurface(canvas);
	  SDL_UnlockSurface(last);
	}
      else if (cur_magic == MAGIC_FADE || cur_magic == MAGIC_DARKEN)
	{
	  SDL_LockSurface(last);
	  SDL_LockSurface(canvas);

	  for (yy = y - 16; yy < y + 16; yy++)
	    {
	      for (xx = x - 16; xx < x + 16; xx++)
		{
		  /* Get original color: */
		
		  SDL_GetRGB(getpixel(last, xx, yy), last->format,
			     &r, &g, &b);

		  if (cur_magic == MAGIC_FADE)
		  {
		    r = min(r + 48, 255);
		    g = min(g + 48, 255);
		    b = min(b + 48, 255);
		  }
		  else
		  {
		    r = max(r - 48, 0);
		    g = max(g - 48, 0);
		    b = max(b - 48, 0);
		  }

		  putpixel(canvas, xx, yy, SDL_MapRGB(canvas->format, r, g, b));
		}
	    }

	  SDL_UnlockSurface(canvas);
	  SDL_UnlockSurface(last);
	}
      else if (cur_magic == MAGIC_TINT)
	{
	  double rd = sRGB_to_linear_table[color_hexes[cur_color][0]];
	  double gd = sRGB_to_linear_table[color_hexes[cur_color][1]];
	  double bd = sRGB_to_linear_table[color_hexes[cur_color][2]];
	  double old;

	  SDL_LockSurface(last);
	  SDL_LockSurface(canvas);

	  for (yy = y - 16; yy < y + 16; yy++)
	    {
	      for (xx = x - 16; xx < x + 16; xx++)
		{
		  /* Get original pixel: */
		
		  SDL_GetRGB(getpixel(last, xx, yy), last->format,
			     &r, &g, &b);
  
                  old = sRGB_to_linear_table[r] * 0.2126 +
			sRGB_to_linear_table[g] * 0.7152 +
			sRGB_to_linear_table[b] * 0.0722;

		  putpixel(canvas, xx, yy,
			   SDL_MapRGB(canvas->format,
			              linear_to_sRGB(rd * old),
				      linear_to_sRGB(gd * old),
				      linear_to_sRGB(bd * old)));
		}
	    }

	  SDL_UnlockSurface(canvas);
	  SDL_UnlockSurface(last);
	}
      else if (cur_magic == MAGIC_CARTOON)
	{
	  float hue, sat, val;
	  
	  SDL_LockSurface(last);
	  SDL_LockSurface(canvas);

	  for (yy = y - 16; yy < y + 16; yy++)
	    {
	      for (xx = x - 16; xx < x + 16; xx++)
		{
		  /* Get original color: */
		
		  SDL_GetRGB(getpixel(last, xx, yy), last->format,
			     &r, &g, &b);

		  rgbtohsv(r, g, b, &hue, &sat, &val);

		  if (sat <= 0.1)
		    sat = 0;
		  else
		    sat = 1.0;
		 
		  val = val - 0.5;
		  val = val * 4;
		  val = val + 0.5;

		  if (val < 0)
		    val = 0;
		  else if (val > 1.0)
		    val = 1.0;

		  val = floor(val * 10) / 10;

		  hue = floor(hue * 10) / 10;

		  hsvtorgb(hue, sat, val, &r, &g, &b);

		  putpixel(canvas, xx, yy, SDL_MapRGB(canvas->format, r, g, b));
		}
	    }

	  SDL_UnlockSurface(canvas);
	  SDL_UnlockSurface(last);
	}
      else if (cur_magic == MAGIC_RAINBOW)
	{
	  /* Pick next color: */
	
	  colr = SDL_MapRGB(canvas->format,
			    rainbow_hexes[rainbow_color][0],
			    rainbow_hexes[rainbow_color][1],
			    rainbow_hexes[rainbow_color][2]);
	

	  /* Draw the shape: */

	  for (yy = 0; yy <= 16; yy++)
	    {
	      w = (yy * yy) / 16;


	      /* Top half: */
	
	      dest.x = x - 16 + w;
	      dest.w = 32 - (w * 2);
	      dest.y = y - yy;;
	      dest.h = 1;

	      SDL_FillRect(canvas, &dest, colr);
	

	      /* Bottom half: */

	      dest.x = x - 16 + w;
	      dest.w = 32 - (w * 2);
	      dest.y = y + yy;
	      dest.h = 1;

	      SDL_FillRect(canvas, &dest, colr);
	    }
	}
      else if (cur_magic == MAGIC_CHALK)
	{
	  SDL_LockSurface(last);

	  for (yy = y - 8; yy <= y + 8; yy = yy + 4)
	    {
	      for (xx = x - 8; xx <= x + 8; xx = xx + 4)
		{
		  dest.x = xx + ((rand() % 5) - 2);
		  dest.y = yy + ((rand() % 5) - 2);
		  dest.w = (rand() % 4) + 2;
		  dest.h = (rand() % 4) + 2;
	
		  colr = getpixel(last, clamp(0, xx, canvas->w-1),
				  clamp(0, yy, canvas->h-1));
		  SDL_FillRect(canvas, &dest, colr);
		}
	    }

	  SDL_UnlockSurface(last);
	}
      else if (cur_magic == MAGIC_DRIP)
	{
	  for (xx = x - 8; xx <= x + 8; xx++)
	    {
	      h = (rand() % 8) + 8;

	      for (yy = y; yy <= y + h; yy++)
		{
		  src.x = xx;
		  src.y = y;
		  src.w = 1;
		  src.h = 16;

		  dest.x = xx;
		  dest.y = yy;
	
		  SDL_BlitSurface(last, &src, canvas, &dest);
		}
	    }
	}
      else if (cur_magic == MAGIC_SPARKLES)
	{
	  if ((rand() % 10) < 2)
	    {
	      src.x = 0;
	      src.y = (rand() % 4) * 32;
	      src.w = 32;
	      src.h = 32;

	      dest.x = x - 16;
	      dest.y = y - 16;

	      SDL_BlitSurface(img_sparkles, &src, canvas, &dest);
	    }
	}
      else if (cur_magic == MAGIC_GRASS)
	{
	  // grass color: 82,180,17
	  static int bucket;
	  if (!button_down)
	    bucket = 0;
	  bucket += (3.5+(rand()/(double)RAND_MAX)) * 7.0;
	  while (bucket >= 0)
	    {
	      int rank = log2int( ((double)y/canvas->h) * (0.99+(rand()/(double)RAND_MAX)) * 64 );
	      int ah = 1<<rank;
	      bucket -= ah;
	      src.x = (rand() % 4) * 64;
	      src.y = ah;
	      src.w = 64;
	      src.h = ah;

	      dest.x = x - 32;
	      dest.y = y - 30 + (int)((rand()/(double)RAND_MAX)*30);

#if 1
              // grass color: 82,180,17
              double tmp_red   = sRGB_to_linear_table[color_hexes[cur_color][0]]*2.0 + (rand()/(double)RAND_MAX);
              double tmp_green = sRGB_to_linear_table[color_hexes[cur_color][1]]*2.0 + (rand()/(double)RAND_MAX);
              double tmp_blue  = sRGB_to_linear_table[color_hexes[cur_color][2]]*2.0 + sRGB_to_linear_table[17];
	      for (yy = 0; yy < ah; yy++)
	        {
	          for (xx = 0; xx < 64; xx++)
		    {
		      SDL_GetRGBA(getpixel(img_grass, xx+src.x, yy+src.y), img_grass->format,
			         &r, &g, &b, &a);

                      double rd = sRGB_to_linear_table[r]*8.0 + tmp_red;
                      rd = rd * (a/255.0) / 11.0;
                      double gd = sRGB_to_linear_table[g]*8.0 + tmp_green;
                      gd = gd * (a/255.0) / 11.0;
                      double bd = sRGB_to_linear_table[b]*8.0 + tmp_blue;
                      bd = bd * (a/255.0) / 11.0;

		      SDL_GetRGB(getpixel(canvas, xx+dest.x, yy+dest.y), canvas->format,
			         &r, &g, &b);
	
		      r = linear_to_sRGB(sRGB_to_linear_table[r]*(1.0-a/255.0) + rd);
		      g = linear_to_sRGB(sRGB_to_linear_table[g]*(1.0-a/255.0) + gd);
		      b = linear_to_sRGB(sRGB_to_linear_table[b]*(1.0-a/255.0) + bd);

		      putpixel(canvas, xx+dest.x, yy+dest.y, SDL_MapRGB(canvas->format, r, g, b));
		    }
	        }
#else
              // untinted
	      SDL_BlitSurface(img_grass, &src, canvas, &dest);
#endif
	    }
	}
      else if (cur_magic == MAGIC_FLIP)
	{
	  /* Flip the canvas: */

	  for (yy = 0; yy < canvas->h; yy++)
	    {
	      src.x = 0;
	      src.y = yy;
	      src.w = canvas->w;
	      src.h = 1;

	      dest.x = 0;
	      dest.y = canvas->h - yy - 1;
	
	      SDL_BlitSurface(last, &src, canvas, &dest);
	    }
	  
	  
	  /* Flip starter, too! */

	  starter_flipped = !starter_flipped;

	  if (img_starter != NULL)
	    flip_starter();
	}
      else if (cur_magic == MAGIC_MIRROR)
	{
	  /* Mirror-image the canvas: */

	  for (xx = 0; xx < canvas->w; xx++)
	    {
	      src.x = xx;
	      src.y = 0;
	      src.w = 1;
	      src.h = canvas->h;

	      dest.x = canvas->w - xx - 1;
	      dest.y = 0;
	
	      SDL_BlitSurface(last, &src, canvas, &dest);
	    }


	  /* Mirror starter, too! */

	  starter_mirrored = !starter_mirrored;

	  if (img_starter != NULL)
	    mirror_starter();
	}
#if 0
      else if (cur_magic == MAGIC_THIN || cur_magic == MAGIC_THICK)
	{
	  SDL_LockSurface(last);
	  SDL_LockSurface(canvas);

	  for (xx = -8; xx <= 8; xx++)
	    {
	      for (yy = -8; yy <= 8; yy++)
		{
		  SDL_GetRGB(getpixel(last, x + xx, y + yy), last->format,
			     &r, &g, &b);

		  r = min(r, (Uint8)255);
		  g = min(g, (Uint8)255);
		  b = min(b, (Uint8)255);
	
		  if ((cur_magic == MAGIC_THIN && (((r + g + b) / 3) > 128)) ||
		      (cur_magic == MAGIC_THICK && (((r + g + b) / 3) <= 128)))
		    {
		      putpixel(canvas, x + xx + 0, y + yy - 1,
			       SDL_MapRGB(canvas->format, r, g, b));
		      putpixel(canvas, x + xx - 1, y + yy + 0,
			       SDL_MapRGB(canvas->format, r, g, b));
		      putpixel(canvas, x + xx + 1, y + yy + 0,
			       SDL_MapRGB(canvas->format, r, g, b));
		      putpixel(canvas, x + xx + 0, y + yy + 1,
			       SDL_MapRGB(canvas->format, r, g, b));
	      
		      putpixel(canvas, x + xx - 1, y + yy - 1,
			       SDL_MapRGB(canvas->format, r, g, b));
		      putpixel(canvas, x + xx - 1, y + yy + 1,
			       SDL_MapRGB(canvas->format, r, g, b));
		      putpixel(canvas, x + xx + 1, y + yy - 1,
			       SDL_MapRGB(canvas->format, r, g, b));
		      putpixel(canvas, x + xx + 1, y + yy + 1,
			       SDL_MapRGB(canvas->format, r, g, b));
	      
		      putpixel(canvas, x + xx + 0, y + yy - 2,
			       SDL_MapRGB(canvas->format, r, g, b));
		      putpixel(canvas, x + xx - 2, y + yy + 0,
			       SDL_MapRGB(canvas->format, r, g, b));
		      putpixel(canvas, x + xx + 2, y + yy + 0,
			       SDL_MapRGB(canvas->format, r, g, b));
		      putpixel(canvas, x + xx + 0, y + yy + 2,
			       SDL_MapRGB(canvas->format, r, g, b));
		    }
		}
	    }
	
	  SDL_UnlockSurface(canvas);
	  SDL_UnlockSurface(last);
	}
#endif
    }
}


/* Store canvas into undo buffer: */

static void rec_undo_buffer(void)
{
  int wanna_update_toolbar;

  wanna_update_toolbar = 0;


  SDL_BlitSurface(canvas, NULL, undo_bufs[cur_undo], NULL);
  undo_starters[cur_undo] = UNDO_STARTER_NONE;

  cur_undo = (cur_undo + 1) % NUM_UNDO_BUFS;

  if (cur_undo == oldest_undo)
    oldest_undo = (oldest_undo + 1) % NUM_UNDO_BUFS;

  newest_undo = cur_undo;

#ifdef DEBUG
  printf("DRAW: Current=%d  Oldest=%d  Newest=%d\n",
	 cur_undo, oldest_undo, newest_undo);
#endif


  /* Update toolbar buttons, if needed: */

  if (tool_avail[TOOL_UNDO] == 0)
    {
      tool_avail[TOOL_UNDO] = 1;
      wanna_update_toolbar = 1;
    }

  if (tool_avail[TOOL_REDO])
    {
      tool_avail[TOOL_REDO] = 0;
      wanna_update_toolbar = 1;
    }

  if (wanna_update_toolbar)
    {
      draw_toolbar();
      SDL_UpdateRect(screen, 0, 0, 96, (48 * (7 + TOOLOFFSET / 2)) + 40);
    }
}


/* Update the screen with the new canvas: */

static void update_canvas(int x1, int y1, int x2, int y2)
{
  SDL_Rect src, dest;

  if (img_starter != NULL)
  {
    /* If there was a starter, cover this part of the drawing with
       the corresponding part of the starter's foreground! */

    src.x = x1;
    src.y = y1;
    src.w = x2 - x1 + 1;
    src.h = y2 - y1 + 1;

    dest.x = x1;
    dest.y = y1;
    dest.w = src.w;
    dest.h = src.h;

    SDL_BlitSurface(img_starter, &dest, canvas, &dest);
  }

  dest.x = 96;
  dest.y = 0;
  SDL_BlitSurface(canvas, NULL, screen, &dest);
  update_screen(x1 + 96, y1, x2 + 96, y2);
}


/* Show program version: */

static void show_version(void)
{
  printf("\nTux Paint\n");
  printf("  Version " VER_VERSION " (" VER_DATE ")\n");

#ifdef LOW_QUALITY_THUMBNAILS
  printf("  Low Quality Thumbnails enabled\n");
#endif

#ifdef LOW_QUALITY_COLOR_SELECTOR
  printf("  Low Quality Color Selector enabled\n");
#endif

#ifdef LOW_QUALITY_STAMP_OUTLINE
  printf("  Low Quality Stamp Outline enabled\n");
#endif

#ifdef LOW_QUALITY_FLOOD_FILL
  printf("  Low Quality Flood Fill enabled\n");
#endif

#ifdef NO_PROMPT_SHADOWS
  printf("  Prompt Shadows disabled\n");
#endif

#ifdef NOSOUND
  printf("  Sound disabled\n");
#endif

#ifdef DEBUG
  printf("  Verbose debugging enabled\n");
#endif
  
  printf("\n");
}


/* Show usage display: */

static void show_usage(FILE * f, char * prg)
{
  char * blank;
  unsigned i;

  blank = strdup(prg);

  for (i = 0; i < strlen(blank); i++)
    blank[i] = ' ';

  fprintf(f,
	  "\n"
	  "Usage: %s {--usage | --help | --version | --copying}\n"
	  "\n"
	  "  %s [--windowed | --fullscreen]\n"
	  "  %s [--640x480   | --800x600   | --1024x768 |\n"
	  "  %s  --1280x1024 | --1400x1050 | --1600x1200]\n"
	  "  %s [--sound | --nosound]         [--quit | --noquit]\n"
	  "  %s [--print | --noprint]         [--complexshapes | --simpleshapes]\n"
	  "  %s [--mixedcase | --uppercase]   [--fancycursors | --nofancycursors]\n"
	  "  %s [--mouse | --keyboard]        [--dontgrab | --grab]\n"
	  "  %s [--noshortcuts | --shortcuts] [--wheelmouse | --nowheelmouse]\n"
	  "  %s [--outlines | --nooutlines]   [--stamps | --nostamps]\n"
	  "  %s [--nostampcontrols | --stampcontrols]\n"
	  "  %s [--mirrorstamps | --dontmirrorstamps]\n"
	  "  %s [--saveoverask | --saveover | --saveovernew]\n"
	  "  %s [--nosave | --save]\n"
	  "  %s [--savedir DIRECTORY]\n"
#ifdef WIN32
	  "  %s [--printcfg | --noprintcfg]\n"
#endif
	  "  %s [--printdelay=SECONDS]\n"
	  "  %s [--lang LANGUAGE | --locale LOCALE | --lang help]\n"
	  "  %s [--nosysconfig] [--nolockfile]\n"
	  /* "  %s [--record FILE | --playback FILE]\n" */
	  "\n",
	  prg, prg,
	  blank, blank, blank,
	  blank, blank, blank,
	  blank, blank, blank,
	  blank, blank, blank,
	  blank, blank, blank,
#ifdef WIN32
	  blank,
#endif
	  blank);

  free(blank);
}


/* FIXME: All this should REALLY be array-based!!! */

/* Show available languages: */

static void show_lang_usage(FILE * f, char * prg)
{
  fprintf(f,
	  "\n"
	  "Usage: %s [--lang LANGUAGE]\n"
	  "\n"
	  "LANGUAGE may be one of:\n"
/* C */	     "  english      american-english\n"
/* af */     "  afrikaans\n"
/* sq */     "  albanian\n"
/* eu */     "  basque       euskara\n"
/* be */     "  belarusian   bielaruskaja\n"
/* nb */     "  bokmal\n"
/* pt_BR */  "  brazilian    brazilian-portuguese   portugues-brazilian\n"
/* br */     "  breton       brezhoneg\n"
/* en_GB */  "  british      british-english\n"
/* bg_BG */  "  bulgarian\n"
/* ca */     "  catalan      catala\n"
/* zh_CN */  "  chinese      simplified-chinese\n"
/* zh_TW */  "               traditional-chinese\n"
/* hr */     "  croatian     hrvatski\n"
/* cs */     "  czech        cesky\n"
/* da */     "  danish       dansk\n"
/* nl */     "  dutch        nederlands\n"
/* fi */     "  finnish      suomi\n"
/* fr */     "  french       francais\n"
/* gl */     "  galician     galego\n"
/* de */     "  german       deutsch\n"
/* el */     "  greek\n"
/* he */     "  hebrew\n"
/* hi */     "  hindi\n"
/* hu */     "  hungarian    magyar\n"
/* is */     "  icelandic    islenska\n"
/* id */     "  indonesian   bahasa-indonesia\n"
/* it */     "  italian      italiano\n"
/* ja */     "  japanese\n"
/* tlh */    "  klingon      tlhIngan\n"
/* ko */     "  korean\n"
/* lt */     "  lithuanian   lietuviu\n"
/* ms */     "  malay\n"
/* nn */     "  norwegian    nynorsk                norsk\n"
/* pl */     "  polish       polski\n"
/* pt_PT */  "  portuguese   portugues\n"
/* ro */     "  romanian\n"
/* ru */     "  russian      russkiy\n"
/* sr */     "  serbian\n"
/* sk */     "  slovak\n"
/* sl */     "  slovenian    slovensko\n"
/* es */     "  spanish      espanol\n"
/* sv */     "  swedish      svenska\n"
/* ta */     "  tamil\n"
/* tr */     "  turkish\n"
/* vi */     "  vietnamese\n"
/* wa */     "  walloon      walon\n"
/* cy */     "  welsh        cymraeg\n"
	  "\n",
	  prg);
}


/* FIXME: Add accented characters to the descriptions */

/* Show available locales: */

static void show_locale_usage(FILE * f, char * prg)
{
  fprintf(f,
	  "\n"
	  "Usage: %s [--locale LOCALE]\n"
	  "\n"
	  "LOCALE may be one of:\n"
	  "  C       (English      American English)\n"
	  "  af_ZA   (Afrikaans)\n"
	  "  eu_ES   (Baque        Euskara)\n"
	  "  be_BY   (Belarusian   Bielaruskaja)\n"
	  "  nb_NO   (Bokmal)\n"
	  "  pt_BR   (Brazilian    Brazilian Portuguese   Portugues Brazilian)\n"
	  "  br_FR   (Breton       Brezhoneg)\n"
	  "  en_GB   (British      British English)\n"
          "  bg_BG   (Bulgarian)\n"
	  "  ca_ES   (Catalan      Catala)\n"
	  "  zh_CN   (Chinese-Simplified)\n"
	  "  zh_TW   (Chinese-Traditional)\n"
	  "  cs_CZ   (Czech        Cesky)\n"
	  "  da_DK   (Danish       Dansk)\n"
	  "  nl_NL   (Dutch)\n"
	  "  fi_FI   (Finnish      Suomi)\n"
	  "  fr_FR   (French       Francais)\n"
	  "  gl_ES   (Galician     Galego)\n"
	  "  de_DE   (German       Deutsch)\n"
	  "  el_GR   (Greek)\n"
	  "  he_IL   (Hebrew)\n"
	  "  hi_IN   (Hindi)\n"
	  "  hr_HR   (Croatian     Hrvatski)\n"
	  "  hu_HU   (Hungarian    Magyar)\n"
	  "  tlh     (Klingon      tlhIngan)\n"
	  "  is_IS   (Icelandic    Islenska)\n"
	  "  id_ID   (Indonesian   Bahasa Indonesia)\n"
	  "  it_IT   (Italian      Italiano)\n"
	  "  ja_JP   (Japanese)\n"
	  "  ko_KR   (Korean)\n"
	  "  ms_MY   (Malay)\n"
	  "  lt_LT   (Lithuanian   Lietuviu)\n"
	  "  nn_NO   (Norwegian    Nynorsk                Norsk)\n"
	  "  pl_PL   (Polish       Polski)\n"
	  "  pt_PT   (Portuguese   Portugues)\n"
	  "  ro_RO   (Romanian)\n"
	  "  ru_RU   (Russian      Russkiy)\n"
	  "  sk_SK   (Slovak)\n"
	  "  sl_SI   (Slovenian)\n"
	  "  sq_AL   (Albanian)\n"
	  "  sr_YU   (Serbian)\n"
	  "  es_ES   (Spanish      Espanol)\n"
	  "  sv_SE   (Swedish      Svenska)\n"
	  "  tr_TR   (Turkish)\n"
	  "  vi_VN   (Vietnamese)\n"
	  "  wa_BE   (Walloon)\n"
	  "  cy_GB   (Welsh        Cymraeg)\n"
	  "\n",
	  prg);
}


static const char *getfilename(const char* path)
{
  char    *p;

  if ( (p = strrchr( path, '\\' )) != NULL )
    return p+1;
  if ( (p = strrchr( path, '/' )) != NULL )
    return p+1;
  return path;
}


// The original Tux Paint canvas was 608x472. The canvas can be
// other sizes now, but many old stamps are sized for the small
// canvas. So, with larger canvases, we must choose a good scale
// factor to compensate. As the canvas size grows, the user will
// want a balance of "more stamps on the screen" and "stamps not
// getting tiny". This will calculate the needed scale factor.
static unsigned compute_default_scale_factor(double ratio)
{
  double old_diag = sqrt(608*608+472*472);
  double new_diag = sqrt(canvas->w*canvas->w+canvas->h*canvas->h);
  double good_def = ratio*sqrt(new_diag/old_diag);
  double good_log = log(good_def);
  unsigned defsize = HARD_MAX_STAMP_SIZE;
  while(defsize>0)
    {
      double this_err = good_log - log(scaletable[defsize].numer / (double)scaletable[defsize].denom);
      double next_err = good_log - log(scaletable[defsize-1].numer / (double)scaletable[defsize-1].denom);
      if( fabs(next_err) > fabs(this_err) ) break;
      defsize--;
    }
  return defsize;
}


/* Setup: */

static void setup(int argc, char * argv[])
{
  int i, ok_to_use_sysconfig;
  char str[128];
  char * upstr;
  SDL_Color black = {0, 0, 0, 0};
  char * homedirdir;
  FILE * fi;
  SDL_Surface * tmp_surf;
  SDL_Rect dest;
  int scale;
#ifndef LOW_QUALITY_COLOR_SELECTOR
  int x, y;
  SDL_Surface * tmp_btn_up;
  SDL_Surface * tmp_btn_down;
  Uint8 r, g, b;
#endif
  SDL_Surface * tmp_imgcurup, * tmp_imgcurdown;


#ifdef __BEOS__
  /* if run from gui, like OpenTracker in BeOS or Explorer in Windows,
     find path from which binary was run and change dir to it
     so all files will be local :) */
  /* UPDATE (2004.10.06): Since SDL 1.2.7 SDL sets that path correctly,
     so this code wouldn't be needed if SDL was init before anything else,
     (just basic init, window shouldn't be needed). */

  if (argc && argv[0])
    {
      char * slash = strrchr(argv[0], '/');
      *(slash + 1) = '\0';
      chdir(argv[0]);
      *(slash + 1) = '/';
    }
#endif


  /* Set default options: */

  use_sound = 1;
  fullscreen = 0;
  noshortcuts = 0;
  dont_do_xor = 0;
  keymouse = 0;
  wheely = 1;
  grab_input = 0;
  no_fancy_cursors = 0;
  simple_shapes = 0;
  only_uppercase = 0;
  promptless_save = SAVE_OVER_PROMPT;
  disable_quit = 0;
  disable_save = 0;
  disable_print = 0;
  dont_load_stamps = 0;
  print_delay = 0;
  printcommand = "pngtopnm | pnmtops | lpr";
  langstr = NULL;
  use_print_config = 0;
  mirrorstamps = 0;
  disable_stamp_controls = 0;
  WINDOW_WIDTH = 640;
  WINDOW_HEIGHT = 480;
  playfile = NULL;
  recording = 0;
  playing = 0;
  ok_to_use_lockfile = 1;
  

#ifdef __BEOS__
  /* Fancy cursors on BeOS are buggy in SDL */

  no_fancy_cursors = 1;
#endif


#ifdef WIN32
  savedir = strdup("userdata");
#elif __BEOS__
  savedir = strdup("./userdata");
#else
  savedir = NULL;
#endif



  /* Load options from global config file: */

#ifndef WIN32

  /* Check to see if it's ok first: */

  ok_to_use_sysconfig = 1;
  
  for (i = 1; i < argc; i++)
    {
      if (strcmp(argv[i], "--nosysconfig") == 0)
	{
	  ok_to_use_sysconfig = 0;
	  i = argc;  /* aka break; */
	}
    }
  

  if (ok_to_use_sysconfig)
    {
      snprintf(str, sizeof(str), "%s/tuxpaint.conf", CONFDIR);
    
      fi = fopen(str, "r");
      if (fi != NULL)
	{
	  parse_options(fi);
	  fclose(fi);
	}
      else
	debug(str);
    }

#endif


  /* Load options from user's own configuration (".rc" / ".cfg") file: */

#if defined(WIN32) || defined(__BEOS__)
  /* Windows and BeOS: Use a "tuxpaint.cfg" file: */
  
  strcpy(str, "tuxpaint.cfg");

  //#elif __APPLE__
  /* Mac: ??? */
  /* FIXME! */
  
#else
  /* Linux and other Unixes:  Use 'rc' style (~/.tuxpaintrc) */
  
  if (getenv("HOME") != NULL)
    {
      /* Should it be "~/.tuxpaint/tuxpaintrc" instead???
	 Comments welcome ... bill@newbreedsoftware.com */

      snprintf(str, sizeof(str), "%s/.tuxpaintrc", getenv("HOME"));
    }
  else
    {
      /* WOAH! We don't know what our home directory is!? Last resort,
	 do it Windows/BeOS way: */

      strcpy(str, "tuxpaint.cfg");
    }
#endif


  fi = fopen(str, "r");
  if (fi != NULL)
    {
      parse_options(fi);
      fclose(fi);
    }
  else
    debug(str);


  /* Handle command-line arguments: */

  for (i = 1; i < argc; i++)
    {
      if (strcmp(argv[i], "--fullscreen") == 0 || strcmp(argv[i], "-f") == 0)
	{
	  fullscreen = 1;
	}
      else if (strcmp(argv[i], "--windowed") == 0 || strcmp(argv[i], "-w") == 0)
	{
	  fullscreen = 0;
	}
      else if (strcmp(argv[i], "--mirrorstamps") == 0)
	{
	  mirrorstamps = 1;
	}
      else if (strcmp(argv[i], "--dontmirrorstamps") == 0)
	{
	  mirrorstamps = 0;
	}
      else if (strcmp(argv[i], "--nostampcontrols") == 0)
	{
	  disable_stamp_controls = 1;
	}
      else if (strcmp(argv[i], "--stampcontrols") == 0)
	{
	  disable_stamp_controls = 0;
	}
      else if (strcmp(argv[i], "--noshortcuts") == 0)
	{
	  noshortcuts = 1;
	}
      else if (strcmp(argv[i], "--shortcuts") == 0)
	{
	  noshortcuts = 0;
	}
      else if ( argv[i][0]=='-' && argv[i][1]=='-' && argv[i][2]>='1' && argv[i][2]<='9' )
	{
	  char *endp1;
	  char *endp2;
	  int w,h;
	  w = strtoul(argv[i]+2, &endp1, 10);
	  h = strtoul(endp1+1,   &endp2, 10);
	  // sanity check it
	  if(argv[i]+2==endp1 || endp1+1==endp2 || *endp1!='x' || *endp2 || w<500 || h<480 || h>w*3 || w>h*4)
	    {
	      show_usage(stderr, (char *) getfilename(argv[0]));
	      exit(1);
	    }
	  WINDOW_WIDTH  = w;
	  WINDOW_HEIGHT = h;
	}
      else if (strcmp(argv[i], "--nooutlines") == 0)
	{
	  dont_do_xor = 1;
	}
      else if (strcmp(argv[i], "--outlines") == 0)
	{
	  dont_do_xor = 0;
	}
      else if (strcmp(argv[i], "--keyboard") == 0)
	{
	  keymouse = 1;
	}
      else if (strcmp(argv[i], "--mouse") == 0)
	{
	  keymouse = 0;
	}
      else if (strcmp(argv[i], "--nowheelmouse") == 0)
	{
	  wheely = 0;
	}
      else if (strcmp(argv[i], "--wheelmouse") == 0)
	{
	  wheely = 1;
	}
      else if (strcmp(argv[i], "--grab") == 0)
	{
	  grab_input = 1;
	}
      else if (strcmp(argv[i], "--dontgrab") == 0)
	{
	  grab_input = 0;
	}
      else if (strcmp(argv[i], "--nofancycursors") == 0)
	{
	  no_fancy_cursors = 1;
	}
      else if (strcmp(argv[i], "--fancycursors") == 0)
	{
	  no_fancy_cursors = 0;
	}
      else if (strcmp(argv[i], "--saveover") == 0)
	{
	  promptless_save = SAVE_OVER_ALWAYS;
	}
      else if (strcmp(argv[i], "--saveoverask") == 0)
	{
	  promptless_save = SAVE_OVER_PROMPT;
	}
      else if (strcmp(argv[i], "--saveovernew") == 0)
	{
	  promptless_save = SAVE_OVER_NO;
	}
      else if (strcmp(argv[i], "--uppercase") == 0 || strcmp(argv[i], "-u") == 0)
	{
	  only_uppercase = 1;
	}
      else if (strcmp(argv[i], "--mixedcase") == 0 || strcmp(argv[i], "-m") == 0)
	{
	  only_uppercase = 0;
	}
      else if (strcmp(argv[i], "--simpleshapes") == 0 ||
	       strcmp(argv[i], "-s") == 0)
	{
	  simple_shapes = 1;
	}
      else if (strcmp(argv[i], "--complexshapes") == 0)
	{
	  simple_shapes = 0;
	}
      else if (strcmp(argv[i], "--noquit") == 0 || strcmp(argv[i], "-x") == 0)
	{
	  disable_quit = 1;
	}
      else if (strcmp(argv[i], "--quit") == 0)
	{
	  disable_quit = 0;
	}
      else if (strcmp(argv[i], "--nosave") == 0)
	{
	  disable_save = 1;
	}
      else if (strcmp(argv[i], "--save") == 0)
	{
	  disable_save = 0;
	}
      else if (strcmp(argv[i], "--nostamps") == 0)
	{
	  dont_load_stamps = 1;
	}
      else if (strcmp(argv[i], "--stamps") == 0)
	{
	  dont_load_stamps = 0;
	}
      else if (strcmp(argv[i], "--noprint") == 0 || strcmp(argv[i], "-p") == 0)
	{
	  disable_print = 1;
	}
      else if (strcmp(argv[i], "--print") == 0)
	{
	  disable_print = 0;
	}
      else if (strcmp(argv[i], "--noprintcfg") == 0)
	{
#ifndef WIN32
	  fprintf(stderr, "Note: printcfg option only applies to Windows!\n");
#endif
	  use_print_config = 0;
	}
      else if (strcmp(argv[i], "--printcfg") == 0)
	{
#ifndef WIN32
	  fprintf(stderr, "Note: printcfg option only applies to Windows!\n");
#endif
	  use_print_config = 1;
	}
      else if (strstr(argv[i], "--printdelay=") == argv[i])
	{
	  sscanf(strstr(argv[i], "--printdelay=") + 13, "%d", &print_delay);
#ifdef DEBUG
	  printf("Print delay set to %d seconds\n", print_delay);
#endif
	}
      else if (strcmp(argv[i], "--nosound") == 0 || strcmp(argv[i], "-q") == 0)
	{
	  use_sound = 0;
	}
      else if (strcmp(argv[i], "--sound") == 0)
	{
	  use_sound = 1;
	}
      else if (strcmp(argv[i], "--locale") == 0 || strcmp(argv[i], "-L") == 0)
	{
	  if (i < argc - 1)
	    {
	      snprintf(str, sizeof(str), "LANG=%s", argv[i + 1]);
	      putenv(str);
	
	      setlocale(LC_ALL, ""); /* argv[i + 1]) ? */
	      i++;
	    }
	  else
	    {
	      /* Forgot to specify the language (locale)! */
	
	      fprintf(stderr, "%s takes an argument\n", argv[i]);
	      show_locale_usage(stderr, (char *) getfilename(argv[0]));
	      exit(1);
	    }
	}
      else if (strstr(argv[i], "--lang=") == argv[i])
	{
	  if (langstr != NULL)
	    free(langstr);

	  langstr = strdup(argv[i] + 7);
	}
      else if (strcmp(argv[i], "--lang") == 0 || strcmp(argv[i], "-l") == 0)
	{
	  if (i < argc - 1)
	    {
	      if (langstr != NULL)
		free(langstr);

	      langstr = strdup(argv[i + 1]);
	      i++;
	    }
	  else
	    {
	      /* Forgot to specify the language! */
	
	      fprintf(stderr, "%s takes an argument\n", argv[i]);
	      show_lang_usage(stderr, (char *) getfilename(argv[0]));
	      exit(1);
	    }
	}
      else if (strcmp(argv[i], "--savedir") == 0)
	{
	  if (i < argc - 1)
	    {
	      if (savedir != NULL)
		free(savedir);

	      savedir = strdup(argv[i + 1]);
	      remove_slash(savedir);
	      i++;
	    }
	  else
	    {
	      /* Forgot to specify the directory name! */

	      fprintf(stderr, "%s takes an argument\n", argv[i]);
	      show_usage(stderr, (char *) getfilename(argv[0]));
	      exit(1);
	    }
	}
      else if (strcmp(argv[i], "--record") == 0 ||
	       strcmp(argv[i], "--playback") == 0)
	{
	  if (i < argc - 1)
	    {
	      playfile = strdup(argv[i + 1]);

	      if (strcmp(argv[i], "--record") == 0)
		recording = 1;
	      else if (strcmp(argv[i], "--playback") == 0)
		playing = 1;
	
	      i++;
	    }
	  else
	    {
	      /* Forgot to specify the filename! */

	      fprintf(stderr, "%s takes an argument\n", argv[i]);
	      show_usage(stderr, (char *) getfilename(argv[0]));
	      exit(1);
	    }
	}
      else if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0)
	{
	  show_version();
	  exit(0);
	}
      else if (strcmp(argv[i], "--copying") == 0 || strcmp(argv[i], "-c") == 0)
	{
	  show_version();
	  printf(
		 "\n"
		 "This program is free software; you can redistribute it\n"
		 "and/or modify it under the terms of the GNU General Public\n"
		 "License as published by the Free Software Foundation;\n"
		 "either version 2 of the License, or (at your option) any\n"
		 "later version.\n"
		 "\n"
		 "This program is distributed in the hope that it will be\n"
		 "useful and entertaining, but WITHOUT ANY WARRANTY; without\n"
		 "even the implied warranty of MERCHANTABILITY or FITNESS\n"
		 "FOR A PARTICULAR PURPOSE.  See the GNU General Public\n"
		 "License for more details.\n"
		 "\n"
		 "You should have received a copy of the GNU General Public\n"
		 "License along with this program; if not, write to the Free\n"
		 "Software Foundation, Inc., 59 Temple Place, Suite 330,\n"
		 "Boston, MA  02111-1307  USA\n"
		 "\n"
		 );
	  exit(0);
	}
      else if (strcmp(argv[i], "--help") == 0 ||
	       strcmp(argv[i], "-h") == 0)
	{
	  show_version();
	  show_usage(stdout, (char *) getfilename(argv[0]));

	  printf(
		 "See: " DOC_PREFIX "README.txt\n"
		 "\n");
	  exit(0);
	}
      else if (strcmp(argv[i], "--usage") == 0 ||
	       strcmp(argv[i], "-u") == 0)
	{
	  show_usage(stdout, (char *) getfilename(argv[0]));
	  exit(0);
	}
      else if (strcmp(argv[i], "--nosysconfig") == 0)
	{
	  debug("Not using system config.");   
	}
      else if (strcmp(argv[i], "--nolockfile") == 0)
        {
	  debug("Not using lockfile");   
          ok_to_use_lockfile = 0;
        }
      else
	{
	  show_usage(stderr, (char *) getfilename(argv[0]));
	  exit(1);
	}
    }


  /* Set up language: */

  if (langstr == NULL && getenv("LANG") != NULL &&
      strncasecmp(getenv("LANG"), "lt_LT", 5) == 0)
    {
      langstr = strdup("lithuanian");
    }

  if (langstr == NULL && getenv("LANG") != NULL &&
      strncasecmp(getenv("LANG"), "pl_PL", 5) == 0)
    {
      langstr = strdup("polish");
    }

  if (langstr != NULL)
    {
      if (strcmp(langstr, "english") == 0 ||
	  strcmp(langstr, "american-english") == 0)
	{
	  putenv((char *) "LANGUAGE=C");
	  putenv((char *) "LC_ALL=C");
	}
      else if (strcmp(langstr, "croatian") == 0 ||
	       strcmp(langstr, "hrvatski") == 0)
	{
	  putenv((char *) "LANGUAGE=hr_HR.UTF-8");
	  putenv((char *) "LC_ALL=hr_HR.UTF-8");
	}
      else if (strcmp(langstr, "catalan") == 0 ||
	       strcmp(langstr, "catala") == 0)
	{
	  putenv((char *) "LANGUAGE=ca_ES.UTF-8");
	  putenv((char *) "LC_ALL=ca_ES.UTF-8");
	}
      else if (strcmp(langstr, "belarusian") == 0 ||
	       strcmp(langstr, "bielaruskaja") == 0)
        {
	  putenv((char *) "LANGUAGE=be_BY.UTF-8");
	  putenv((char *) "LC_ALL=be_BY.UTF-8");
        }
      else if (strcmp(langstr, "czech") == 0 ||
	       strcmp(langstr, "cesky") == 0)
	{
	  putenv((char *) "LANGUAGE=cs_CZ.UTF-8");
	  putenv((char *) "LC_ALL=cs_CZ.UTF-8");
	}
      else if (strcmp(langstr, "danish") == 0 ||
	       strcmp(langstr, "dansk") == 0)
	{
	  putenv((char *) "LANGUAGE=da_DK.UTF-8");
	  putenv((char *) "LC_ALL=da_DK.UTF-8");
	}
      else if (strcmp(langstr, "german") == 0 ||
	       strcmp(langstr, "deutsch") == 0)
	{
	  putenv((char *) "LANGUAGE=de_DE.UTF-8");
	  putenv((char *) "LC_ALL=de_DE.UTF-8");
	}
      else if (strcmp(langstr, "greek") == 0)
	{
	  putenv((char *) "LANGUAGE=el_GR.UTF-8");
	  putenv((char *) "LC_ALL=el_GR.UTF-8");
	}
      else if (strcmp(langstr, "british-english") == 0 ||
	       strcmp(langstr, "british") == 0)
	{
	  putenv((char *) "LANGUAGE=en_GB.UTF-8");
	  putenv((char *) "LC_ALL=en_GB.UTF-8");
	}
      else if (strcmp(langstr, "spanish") == 0 ||
	       strcmp(langstr, "espanol") == 0)
	{
	  putenv((char *) "LANGUAGE=es_ES.UTF-8");
	  putenv((char *) "LC_ALL=es_ES.UTF-8");
	}
      else if (strcmp(langstr, "finnish") == 0 ||
	       strcmp(langstr, "suomi") == 0)
	{
	  putenv((char *) "LANGUAGE=fi_FI.UTF-8");
	  putenv((char *) "LC_ALL=fi_FI.UTF-8");
	}
      else if (strcmp(langstr, "french") == 0 ||
	       strcmp(langstr, "francais") == 0)
	{
	  putenv((char *) "LANGUAGE=fr_FR.UTF-8");
	  putenv((char *) "LC_ALL=fr_FR.UTF-8");
	}
      else if (strcmp(langstr, "galician") == 0 ||
	       strcmp(langstr, "galego") == 0)
	{
	  putenv((char *) "LANGUAGE=gl_ES.UTF-8");
	  putenv((char *) "LC_ALL=gl_ES.UTF-8");
	}
      else if (strcmp(langstr, "hebrew") == 0)
	{
	  putenv((char *) "LANGUAGE=he_IL.UTF-8");
	  putenv((char *) "LC_ALL=he_IL.UTF-8");
	}
      else if (strcmp(langstr, "hindi") == 0)
	{
	  putenv((char *) "LANGUAGE=hi_IN.UTF-8");
	  putenv((char *) "LC_ALL=hi_IN.UTF-8");
	}
      else if (strcmp(langstr, "hungarian") == 0 ||
	       strcmp(langstr, "magyar") == 0)
	{
	  putenv((char *) "LANGUAGE=hu_HU.UTF-8");
	}
      else if (strcmp(langstr, "indonesian") == 0 ||
	       strcmp(langstr, "bahasa-indonesia") == 0)
	{
	  putenv((char *) "LANGUAGE=id_ID.UTF-8");
	  putenv((char *) "LC_ALL=id_ID.UTF-8");
	}
      else if (strcmp(langstr, "icelandic") == 0 ||
	       strcmp(langstr, "islenska") == 0)
	{
	  putenv((char *) "LANGUAGE=is_IS.UTF-8");
	  putenv((char *) "LC_ALL=is_IS.UTF-8");
	}
      else if (strcmp(langstr, "italian") == 0 ||
	       strcmp(langstr, "italiano") == 0)
	{
	  putenv((char *) "LANGUAGE=it_IT.UTF-8");
	  putenv((char *) "LC_ALL=it_IT.UTF-8");
	}
      else if (strcmp(langstr, "japanese") == 0)
	{
	  putenv((char *) "LANGUAGE=ja_JP.UTF-8");
	  putenv((char *) "LC_ALL=ja_JP.UTF-8");
	}
      else if (strcmp(langstr, "vietnamese") == 0)
	{
	  putenv((char *) "LANGUAGE=vi_VN.UTF-8");
	  putenv((char *) "LC_ALL=vi_VN.UTF-8");
	}
      else if (strcmp(langstr, "afrikaans") == 0)
	{
	  putenv((char *) "LANGUAGE=af_ZA.UTF-8");
	  putenv((char *) "LC_ALL=af_ZA.UTF-8");
	}
      else if (strcmp(langstr, "albanian") == 0)
	{
	  putenv((char *) "LANGUAGE=sq_AL.UTF-8");
	  putenv((char *) "LC_ALL=sq_AL.UTF-8");
	}
      else if (strcmp(langstr, "breton") == 0 ||
	       strcmp(langstr, "brezhoneg") == 0)
	{
	  putenv((char *) "LANGUAGE=br_FR.UTF-8");
	  putenv((char *) "LC_ALL=br_FR.UTF-8");
	}
      else if (strcmp(langstr, "bulgarian") == 0)
	{
	  putenv((char *) "LANGUAGE=bg_BG.UTF-8");
	  putenv((char *) "LC_ALL=bg_BG.UTF-8");
	}
      else if (strcmp(langstr, "welsh") == 0 ||
	       strcmp(langstr, "cymraeg") == 0)
	{
	  putenv((char *) "LANGUAGE=cy_GB.UTF-8");
	  putenv((char *) "LC_ALL=cy_GB.UTF-8");
	}
      else if (strcmp(langstr, "bokmal") == 0)
	{
	  putenv((char *) "LANGUAGE=nb_NO.UTF-8");
	  putenv((char *) "LC_ALL=nb_NO.UTF-8");
	}
      else if (strcmp(langstr, "basque") == 0 ||
	       strcmp(langstr, "euskara") == 0)
	{
	  putenv((char *) "LANGUAGE=eu_ES.UTF-8");
	  putenv((char *) "LC_ALL=eu_ES.UTF-8");
	}
      else if (strcmp(langstr, "korean") == 0)
	{
	  putenv((char *) "LANGUAGE=ko_KR.UTF-8");
	  putenv((char *) "LC_ALL=ko_KR.UTF-8");
	}
      else if (strcmp(langstr, "klingon") == 0 ||
	       strcmp(langstr, "tlhIngan") == 0 ||
	       strcmp(langstr, "tlhingan") == 0)
	{
	  putenv((char *) "LANGUAGE=tlh.UTF-8");
	  putenv((char *) "LC_ALL=tlh.UTF-8");
	}
      else if (strcmp(langstr, "tamil") == 0)
	{
	  putenv((char *) "LANGUAGE=ta_IN.UTF-8");
	  putenv((char *) "LC_ALL=ta_IN.UTF-8");
	}
      else if (strcmp(langstr, "lithuanian") == 0 ||
	       strcmp(langstr, "lietuviu") == 0)
	{
	  putenv((char *) "LANGUAGE=lt_LT.UTF-8");
	  putenv((char *) "LC_ALL=lt_LT.UTF-8");
	}
      else if (strcmp(langstr, "malay") == 0)
	{
	  putenv((char *) "LANGUAGE=ms_MY.UTF-8");
	  putenv((char *) "LC_ALL=ms_MY.UTF-8");
	}
      else if (strcmp(langstr, "dutch") == 0 ||
	       strcmp(langstr, "nederlands") == 0)
	{
	  putenv((char *) "LANGUAGE=nl_NL.UTF-8");
	  putenv((char *) "LC_ALL=nl_NL.UTF-8");
	}
      else if (strcmp(langstr, "norwegian") == 0 ||
	       strcmp(langstr, "nynorsk") == 0 ||
	       strcmp(langstr, "norsk") == 0)
	{
	  putenv((char *) "LANGUAGE=nn_NO.UTF-8");
	  putenv((char *) "LC_ALL=nn_NO.UTF-8");
	}
      else if (strcmp(langstr, "polish") == 0 ||
	       strcmp(langstr, "polski") == 0)
	{
	  putenv((char *) "LANGUAGE=pl_PL.UTF-8");
	  putenv((char *) "LC_ALL=pl_PL.UTF-8");
	}
      else if (strcmp(langstr, "brazilian-portuguese") == 0 ||
	       strcmp(langstr, "portugues-brazilian") == 0 ||
	       strcmp(langstr, "brazilian") == 0)
	{
	  putenv((char *) "LANGUAGE=pt_BR.UTF-8");
	  putenv((char *) "LC_ALL=pt_BR.UTF-8");
	}
      else if (strcmp(langstr, "portuguese") == 0 ||
	       strcmp(langstr, "portugues") == 0)
	{
	  putenv((char *) "LANGUAGE=pt_PT.UTF-8");
	  putenv((char *) "LC_ALL=pt_PT.UTF-8");
	}
      else if (strcmp(langstr, "romanian") == 0)
	{
	  putenv((char *) "LANGUAGE=ro_RO.UTF-8");
	  putenv((char *) "LC_ALL=ro_RO.UTF-8");
	}
      else if (strcmp(langstr, "russian") == 0 ||
	       strcmp(langstr, "russkiy") == 0)
	{
	  putenv((char *) "LANGUAGE=ru_RU.UTF-8");
	  putenv((char *) "LC_ALL=ru_RU.UTF-8");
	}
      else if (strcmp(langstr, "slovak") == 0)
	{
	  putenv((char *) "LANGUAGE=sk_SK.UTF-8");
	  putenv((char *) "LC_ALL=sk_SK.UTF-8");
	}
      else if (strcmp(langstr, "slovenian") == 0 ||
	       strcmp(langstr, "slovensko") == 0)
	{
	  putenv((char *) "LANGUAGE=sl_SI.UTF-8");
	  putenv((char *) "LC_ALL=sl_SI.UTF-8");
	}
      else if (strcmp(langstr, "serbian") == 0)
	{
	  putenv((char *) "LANGUAGE=sr_YU.UTF-8");
	  putenv((char *) "LC_ALL=sr_YU.UTF-8");
	}
      else if (strcmp(langstr, "swedish") == 0 ||
	       strcmp(langstr, "svenska") == 0)
	{
	  putenv((char *) "LANGUAGE=sv_SE.UTF-8");
	  putenv((char *) "LC_ALL=sv_SE.UTF-8");
	}
      else if (strcmp(langstr, "turkish") == 0)
	{
	  putenv((char *) "LANGUAGE=tr_TR.UTF-8");
	  putenv((char *) "LC_ALL=tr_TR.UTF-8");
	}
      else if (strcmp(langstr, "walloon") == 0 ||
	       strcmp(langstr, "walon") == 0)
	{
	  putenv((char *) "LANGUAGE=wa_BE.UTF-8");
	  putenv((char *) "LC_ALL=wa_BE.UTF-8");
	}
      else if (strcmp(langstr, "chinese") == 0 ||
               strcmp(langstr, "simplified-chinese") == 0)
	{
	  putenv((char *) "LANGUAGE=zh_CN.UTF-8");
	  putenv((char *) "LC_ALL=zh_CN.UTF-8");
	}
      else if (strcmp(langstr, "traditional-chinese") == 0)
	{
	  putenv((char *) "LANGUAGE=zh_TW.UTF-8");
	  putenv((char *) "LC_ALL=zh_TW.UTF-8");
	}
      else if (strcmp(langstr, "help") == 0 || strcmp(langstr, "list") == 0)
	{
	  show_lang_usage(stdout, (char *) getfilename(argv[0]));
	  free(langstr);
	  exit(0);
	}
      else
	{
	  fprintf(stderr, "%s is an invalid language\n", langstr);
	  show_lang_usage(stderr, (char *) getfilename(argv[0]));
	  free(langstr);
	  exit(1);
	}
    
      setlocale(LC_ALL, "");
      free(langstr);
    }
 
  bindtextdomain("tuxpaint", LOCALEDIR);
  
  /* Old version of glibc does not have bind_textdomain_codeset() */
#if defined __GLIBC__ && __GLIBC__ == 2 && __GLIBC_MINOR__ >=2 || __GLIBC__ > 2
  bind_textdomain_codeset("tuxpaint", "UTF-8");
#endif

  textdomain("tuxpaint");

  language = current_language();


#ifdef DEBUG
  printf("DEBUG: Language is %s (%d)\n", lang_prefixes[language], language);
#endif

#ifndef WIN32
  putenv((char *) "SDL_VIDEO_X11_WMCLASS=TuxPaint.TuxPaint");
#endif
  
  
  /* Test for lockfile, if we're using one: */

  if (ok_to_use_lockfile)
  {
    char * lock_fname;
    FILE * fi;
    time_t time_lock, time_now;


    /* Get the current time: */
    
    time_now = time(NULL);
    

    /* Look for the lockfile... */

    lock_fname = get_fname("lockfile.dat");

    fi = fopen(lock_fname, "r");
    if (fi != NULL)
    {
      /* If it exists, read its contents: */

      if (fread(&time_lock, sizeof(time_t), 1, fi) > 0)
      {
	/* Has it not been 30 seconds yet? */

	if (time_now < time_lock + 30)
	{
          /* FIXME: Wrap in gettext() */
          printf("\nYou're already running a copy of Tux Paint!\n\n");

	  free(lock_fname);

	  fclose(fi);
          exit(0);
	}
      }

      fclose(fi);
    }


    /* Okay to run; create/update the lockfile */

    fi = fopen(lock_fname, "w");
    if (fi != NULL)
    {
      /* If we can write to it, do so! */

      fwrite(&time_now, sizeof(time_t), 1, fi);
      fclose(fi);
    }
    else
    {
      fprintf(stderr,
 	      "\nWarning: I couldn't create the lockfile (%s)\n"
	      "The error that occurred was:\n"
	      "%s\n\n", lock_fname, strerror(errno));
    }

    free(lock_fname);
  }


  /* Init SDL Video: */

  if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
      fprintf(stderr,
	      "\nError: I could not initialize video!\n"
	      "The Simple DirectMedia Layer error that occurred was:\n"
	      "%s\n\n", SDL_GetError());
      exit(1);
    }


  /* Set-up Key-Repeat: */

  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,
		      SDL_DEFAULT_REPEAT_INTERVAL);
 

  /* Init SDL Timer: */

  if (SDL_Init(SDL_INIT_TIMER) < 0)
    {
      fprintf(stderr,
	      "\nError: I could not initialize timer!\n"
	      "The Simple DirectMedia Layer error that occurred was:\n"
	      "%s\n\n", SDL_GetError());
      exit(1);
    }


  /* Init TTF stuff: */

  if (TTF_Init() < 0)
    {
      fprintf(stderr,
	      "\nError: I could not initialize the font (TTF) library!\n"
	      "The Simple DirectMedia Layer error that occurred was:\n"
	      "%s\n\n", SDL_GetError());

      SDL_Quit();
      exit(1);
    }


  /* Init SDL Audio and set-up Mixer: */

#ifndef NOSOUND
  if (use_sound)
    {
      if (SDL_Init(SDL_INIT_AUDIO) < 0)
	{
	  fprintf(stderr,
		  "\nWarning: I could not initialize audio!\n"
		  "The Simple DirectMedia Layer error that occurred was:\n"
		  "%s\n\n", SDL_GetError());
	  use_sound = 0;
	}
      else
	{
#ifndef WIN32
	  if (Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 1024) < 0)
#else
	    if (Mix_OpenAudio(44100, AUDIO_S16, 2, 2048) < 0)
#endif
	      {
		fprintf(stderr,
			"\nWarning: I could not set up audio for 44100 Hz "
			"16-bit stereo.\n"
			"The Simple DirectMedia Layer error that occurred was:\n"
			"%s\n\n", SDL_GetError());
		use_sound = 0;
	      }
	    else
	      {
		/* Load sounds: */

		for (i = 0; i < NUM_SOUNDS; i++)
		  {
		    sounds[i] = Mix_LoadWAV(sound_fnames[i]);
	
		    if (sounds[i] == NULL)
		      {
			fprintf(stderr,
				"\nWarning: I couldn't open a sound file:\n%s\n"
				"The Simple DirectMedia Layer error that occurred was:\n"
				"%s\n\n", sound_fnames[i], SDL_GetError());
			use_sound = 0;
		      }
		  }
	      }
	}
    }
#endif


  /* Set window icon and caption: */

#ifndef __APPLE__
  seticon();
#endif
  SDL_WM_SetCaption("Tux Paint", "Tux Paint");

  /* Open Window: */

  if (fullscreen)
    {
#ifdef USE_HWSURFACE
      screen = SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT,
				VIDEO_BPP, SDL_FULLSCREEN | SDL_HWSURFACE);
#else
      screen = SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT,
				VIDEO_BPP, SDL_FULLSCREEN | SDL_SWSURFACE);
#endif

      if (screen == NULL)
	{
	  fprintf(stderr,
		  "\nWarning: I could not open the display in fullscreen mode.\n"
		  "The Simple DirectMedia Layer error that occurred was:\n"
		  "%s\n\n", SDL_GetError());

	  fullscreen = 0;
	}
    }


  if (!fullscreen)
    {
#ifdef USE_HWSURFACE
      screen = SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT,
		      		VIDEO_BPP, SDL_HWSURFACE);
#else
      screen = SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT,
		      		VIDEO_BPP, SDL_SWSURFACE);
#endif
    }

  if (screen == NULL)
    {
      fprintf(stderr,
	      "\nError: I could not open the display.\n"
	      "The Simple DirectMedia Layer error that occurred was:\n"
	      "%s\n\n", SDL_GetError());

      cleanup();
      exit(1);
    }


#if defined(WIN32) && defined(LARGE_CURSOR_FULLSCREEN_BUG)
  if (fullscreen && no_fancy_cursors == 0)
    {
      fprintf(stderr, "Warning: An SDL bug causes the fancy cursors to leave\n"
	      "trails in fullscreen mode.  Disabling fancy cursors.\n"
	      "(You can do this yourself with 'nofancycursors' option,\n"
	      "to avoid this warning in the future.)\n");
      no_fancy_cursors = 1;
    }
#endif


  /* Create cursors: */

  scale = 1;

#ifdef SMALL_CURSOR_SHAPES
  scale = 2;
#endif

  cursor_hand = get_cursor(hand_bits, hand_mask_bits,
			   hand_width, hand_height,
			   12 / scale, 1 / scale);
  
  cursor_wand = get_cursor(wand_bits, wand_mask_bits,
		           wand_width, wand_height,
		           4 / scale, 4 / scale);
  
  cursor_insertion = get_cursor(insertion_bits, insertion_mask_bits,
		                insertion_width, insertion_height,
		                7 / scale, 4 / scale);
  
  cursor_brush = get_cursor(brush_bits, brush_mask_bits,
			    brush_width, brush_height,
			    4 / scale, 28 / scale);
  
  cursor_crosshair = get_cursor(crosshair_bits, crosshair_mask_bits,
				crosshair_width, crosshair_height,
				15 / scale, 15 / scale);
  
  cursor_rotate = get_cursor(rotate_bits, rotate_mask_bits,
		             rotate_width, rotate_height,
		             15 / scale, 15 / scale);
  
  cursor_watch = get_cursor(watch_bits, watch_mask_bits,
			    watch_width, watch_height,
			    14 / scale, 14 / scale);
  
  cursor_arrow = get_cursor(arrow_bits, arrow_mask_bits,
			    arrow_width, arrow_height,
			    0, 0);

  cursor_up = get_cursor(up_bits, up_mask_bits,
			 up_width, up_height,
			 15 / scale, 1 / scale);

  cursor_down = get_cursor(down_bits, down_mask_bits,
			   down_width, down_height,
			   15 / scale, 30 / scale);

  cursor_tiny = get_cursor(tiny_bits, tiny_mask_bits,
			   tiny_width, tiny_height,
			   3, 3);   /* Exactly the same in SMALL (16x16) size! */

  do_setcursor(cursor_watch);
  


  /* Create drawing canvas: */

  canvas = SDL_CreateRGBSurface(screen->flags,
				WINDOW_WIDTH - (96 * 2),
				(48 * 7) + 40 + HEIGHTOFFSET,
				screen->format->BitsPerPixel,
				screen->format->Rmask,
				screen->format->Gmask,
				screen->format->Bmask,
				0);

  img_starter = NULL;
  img_starter_bkgd = NULL;
  starter_mirrored = 0;
  starter_flipped = 0;

  if (canvas == NULL)
    {
      fprintf(stderr, "\nError: Can't build drawing canvas!\n"
	      "The Simple DirectMedia Layer error that occurred was:\n"
	      "%s\n\n", SDL_GetError());

      cleanup();
      exit(1);
    }

  SDL_FillRect(canvas, NULL, SDL_MapRGB(canvas->format, 255, 255, 255));


  /* Create undo buffer space: */

  for (i = 0; i < NUM_UNDO_BUFS; i++)
    {
      undo_bufs[i] = SDL_CreateRGBSurface(screen->flags,
					  WINDOW_WIDTH - (96 * 2),
					  (48 * 7) + 40 + HEIGHTOFFSET,
					  screen->format->BitsPerPixel,
					  screen->format->Rmask,
					  screen->format->Gmask,
					  screen->format->Bmask,
					  0);


      if (undo_bufs[i] == NULL)
	{
	  fprintf(stderr, "\nError: Can't build undo buffer! (%d of %d)\n"
		  "The Simple DirectMedia Layer error that occurred was:\n"
		  "%s\n\n", i + 1, NUM_UNDO_BUFS, SDL_GetError());
	
	  cleanup();
	  exit(1);
	}

      undo_starters[i] = UNDO_STARTER_NONE;
    }


  /* Load and display title image: */

  img_title = loadimage(DATA_PREFIX "images/title.png");
  img_progress = loadimage(DATA_PREFIX "images/ui/progress.png");

  SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 255, 255, 255));

  dest.x = (WINDOW_WIDTH - img_title->w) / 2;
  dest.y = (WINDOW_HEIGHT - img_title->h);

  SDL_BlitSurface(img_title, NULL, screen, &dest);

  prog_bar_ctr = 0;
  show_progress_bar();

  SDL_Flip(screen);
  
 
#ifdef USE_HQ4X
  /* Init high quality scaling stuff: */
  
  InitLUTs(RGBtoYUV);
#endif


  /* Load other images: */

  for (i = 0; i < NUM_TOOLS; i++)
    img_tools[i] = loadimage(tool_img_fnames[i]);

  img_title_on = loadimage(DATA_PREFIX "images/ui/title.png");
  img_title_large_on = loadimage(DATA_PREFIX "images/ui/title_large.png");
  img_title_off = loadimage(DATA_PREFIX "images/ui/no_title.png");
  img_title_large_off = loadimage(DATA_PREFIX "images/ui/no_title_large.png");
  
  img_btn_up = loadimage(DATA_PREFIX "images/ui/btn_up.png");
  img_btn_down = loadimage(DATA_PREFIX "images/ui/btn_down.png");
  img_btn_off = loadimage(DATA_PREFIX "images/ui/btn_off.png");

  show_progress_bar();

  img_yes = loadimage(DATA_PREFIX "images/ui/yes.png");
  img_no = loadimage(DATA_PREFIX "images/ui/no.png");

  img_open = loadimage(DATA_PREFIX "images/ui/open.png");
  img_erase = loadimage(DATA_PREFIX "images/ui/erase.png");
  img_back = loadimage(DATA_PREFIX "images/ui/back.png");

  img_grow = loadimage(DATA_PREFIX "images/ui/grow.png");
  img_shrink = loadimage(DATA_PREFIX "images/ui/shrink.png");
  
  show_progress_bar();

  tmp_imgcurup    = loadimage(DATA_PREFIX "images/ui/cursor_up_large.png");
  tmp_imgcurdown  = loadimage(DATA_PREFIX "images/ui/cursor_down_large.png");
  img_cursor_up   = thumbnail(tmp_imgcurup,   THUMB_W, THUMB_H, 0);
  img_cursor_down = thumbnail(tmp_imgcurdown, THUMB_W, THUMB_H, 0);

  tmp_imgcurup    = loadimage(DATA_PREFIX "images/ui/cursor_starter_up.png");
  tmp_imgcurdown  = loadimage(DATA_PREFIX "images/ui/cursor_starter_down.png");
  img_cursor_starter_up   = thumbnail(tmp_imgcurup,   THUMB_W, THUMB_H, 0);
  img_cursor_starter_down = thumbnail(tmp_imgcurdown, THUMB_W, THUMB_H, 0);
  SDL_FreeSurface(tmp_imgcurup);
  SDL_FreeSurface(tmp_imgcurdown);

  show_progress_bar();

  img_scroll_up = loadimage(DATA_PREFIX "images/ui/scroll_up.png");
  img_scroll_down = loadimage(DATA_PREFIX "images/ui/scroll_down.png");

  img_scroll_up_off = loadimage(DATA_PREFIX "images/ui/scroll_up_off.png");
  img_scroll_down_off = loadimage(DATA_PREFIX "images/ui/scroll_down_off.png");

  img_paintcan = loadimage(DATA_PREFIX "images/ui/paintcan.png");

  show_progress_bar();

  img_sparkles = loadimage(DATA_PREFIX "images/ui/sparkles.png");
  img_grass    = loadimage(DATA_PREFIX "images/ui/grass.png");


  /* Load brushes: */

#ifndef NOSOUND
  loadarbitrary(img_brushes, NULL, NULL, NULL, NULL, &num_brushes, 0,
	        MAX_BRUSHES, DATA_PREFIX "brushes", 1, 40, 40);
#else
  loadarbitrary(img_brushes, NULL, NULL, NULL, &num_brushes, 0,
	        MAX_BRUSHES, DATA_PREFIX "brushes", 1, 40, 40);
#endif


  homedirdir = get_fname("brushes");
#ifndef NOSOUND
  loadarbitrary(img_brushes, NULL, NULL, NULL, NULL, &num_brushes, num_brushes,
	        MAX_BRUSHES, homedirdir, 0, 40, 40);
#else
  loadarbitrary(img_brushes, NULL, NULL, NULL, &num_brushes, num_brushes,
	        MAX_BRUSHES, homedirdir, 0, 40, 40);
#endif

  if (num_brushes == 0)
    {
      fprintf(stderr,
	      "\nError: No brushes found in " DATA_PREFIX "brushes/\n"
	      "or %s\n\n", homedirdir);
      cleanup();
      exit(1);
    }

  free(homedirdir);
  

  /* Load system fonts: */

  font = TTF_OpenFont(DATA_PREFIX "fonts/default_font.ttf",
		      18 - (only_uppercase * 3));

  if (font == NULL)
    {
      fprintf(stderr,
	      "\nError: Can't load font file: "
	      DATA_PREFIX "fonts/default_font.ttf\n"
	      "The Simple DirectMedia Layer error that occurred was:\n"
	      "%s\n\n", SDL_GetError());

      cleanup();
      exit(1);
    }

  
  large_font = TTF_OpenFont(DATA_PREFIX "fonts/default_font.ttf",
			    36 - (only_uppercase * 3));

  if (large_font == NULL)
    {
      fprintf(stderr,
	      "\nError: Can't load font file: "
	      DATA_PREFIX "fonts/default_font.ttf\n"
	      "The Simple DirectMedia Layer error that occurred was:\n"
	      "%s\n\n", SDL_GetError());

      cleanup();
      exit(1);
    }


  small_font = TTF_OpenFont(DATA_PREFIX "fonts/default_font.ttf",
		            14 - (only_uppercase * 2));

  if (small_font == NULL)
    {
      fprintf(stderr,
	      "\nError: Can't load font file: "
	      DATA_PREFIX "fonts/default_font.ttf\n"
	      "The Simple DirectMedia Layer error that occurred was:\n"
	      "%s\n\n", SDL_GetError());

      cleanup();
      exit(1);
    }


  if (need_own_font(language))
    {
      snprintf(str, sizeof(str), "%sfonts/locale/%s.ttf",
	       DATA_PREFIX, lang_prefixes[language]);

      locale_font = TTF_OpenFont(str, 18);

      if (locale_font == NULL)
      {
          locale_font = try_alternate_font(language);
          if (locale_font == NULL)
	  {
	      fprintf(stderr,
		      "\nWarning: Can't load font for this locale:\n"
		      "%s\n"
		      "The Simple DirectMedia Layer error that occurred was:\n"
		      "%s\n\n"
		      "Will use default (American English) instead.\n\n",
		      str, SDL_GetError());


	      /* Revert to default: */
      
	      putenv((char *) "LANG=C");
	      putenv((char *) "OUTPUT_CHARSET=C");
	      setlocale(LC_ALL, "C");

	      bindtextdomain("tuxpaint", LOCALEDIR);
	      textdomain("tuxpaint");
	      language = current_language();
	  }
      }
    }


  if (locale_font == NULL)
    locale_font = font;
  

  /* Load other available fonts: */

  num_fonts = 0;
  
  loadfonts(DATA_PREFIX "fonts", 1);

  homedirdir = get_fname("fonts");
  loadfonts(homedirdir, 0);
  free(homedirdir);


  /* Load stamps: */

  if (dont_load_stamps == 0)
    {
      homedirdir = get_fname("stamps");
#ifndef NOSOUND
      loadarbitrary(img_stamps, img_stamps_premirror,
		    txt_stamps, inf_stamps, snd_stamps,
		    &num_stamps, 0,
		    MAX_STAMPS, homedirdir, 0, -1, -1);
#else
      loadarbitrary(img_stamps, img_stamps_premirror,
		    txt_stamps, inf_stamps, &num_stamps, 0,
		    MAX_STAMPS, homedirdir, 0, -1, -1);
#endif


#ifndef NOSOUND
      loadarbitrary(img_stamps, img_stamps_premirror,
		    txt_stamps, inf_stamps, snd_stamps, &num_stamps,
		    num_stamps, MAX_STAMPS, DATA_PREFIX "stamps", 0, -1, -1);
#else
      loadarbitrary(img_stamps, img_stamps_premirror,
		    txt_stamps, inf_stamps, &num_stamps,
		    num_stamps, MAX_STAMPS, DATA_PREFIX "stamps", 0, -1, -1);
#endif

#ifdef __APPLE__
#ifndef NOSOUND
      loadarbitrary(img_stamps, img_stamps_premirror,
		    txt_stamps, inf_stamps, snd_stamps, &num_stamps,
		    num_stamps, MAX_STAMPS, "/Library/Application Support/TuxPaint/stamps", 0, -1, -1);
#else
      loadarbitrary(img_stamps, img_stamps_premirror,
		    txt_stamps, inf_stamps, &num_stamps,
		    num_stamps, MAX_STAMPS, "/Library/Application Support/TuxPaint/stamps", 0, -1, -1);
#endif
#endif

      if (num_stamps == 0)
	{
	  fprintf(stderr,
		  "\nWarning: No stamps found in " DATA_PREFIX "stamps/\n"
		  "or %s\n\n", homedirdir);
	}

      free(homedirdir);


      unsigned default_stamp_size = compute_default_scale_factor(1.0);

      /* Create stamp thumbnails: */

      for (i = 0; i < num_stamps; i++)
	{
	  if (img_stamps[i]->w > 40 ||
	      img_stamps[i]->h > 40)
	    {
	      img_stamp_thumbs[i] = thumbnail(img_stamps[i], 40, 40, 1);
	    }
	  else
	    {
	      img_stamp_thumbs[i] = NULL;
	    }

      
	  if (img_stamps_premirror[i] != NULL && !disable_stamp_controls)
	    {
	      /* Also thumbnail the pre-drawn mirror version, if any: */

	      if (img_stamps_premirror[i]->w > 40 ||
		  img_stamps_premirror[i]->h > 40)
		{
		  img_stamp_thumbs_premirror[i] =
		    thumbnail(img_stamps_premirror[i], 40, 40, 1);
		}
	      else
		{
		  img_stamp_thumbs_premirror[i] = NULL;
		}
	    }
	  else
	    {
	      img_stamps_premirror[i] = NULL;
	    }

	  state_stamps[i] = malloc(sizeof(state_type));

	  if (inf_stamps[i] == NULL)
	    {
	      /* Didn't load one for this stamp, assume defaults: */
	      
	      inf_stamps[i] = malloc(sizeof(info_type));
	      inf_stamps[i]->tintable = 0;
	      inf_stamps[i]->colorable = 0;
	      inf_stamps[i]->mirrorable = 1;
	      inf_stamps[i]->flipable = 1;
	      inf_stamps[i]->ratio = 1.0;
	      inf_stamps[i]->tinter = TINTER_NORMAL;
	    }

	  {
	    unsigned int upper = HARD_MAX_STAMP_SIZE;
	    unsigned int lower = 0;

	    do
	    {
	      scaleparams *s = &scaletable[upper];
	      int pw, ph; // proposed width and height

	      pw = (img_stamps[i]->w * s->numer + s->denom - 1) / s->denom;
	      ph = (img_stamps[i]->h * s->numer + s->denom - 1) / s->denom;

	      // OK to let a stamp stick off the sides in one direction, not two
	      if (pw < canvas->w * 2 && ph < canvas->h * 1)
		break;
	      if (pw < canvas->w * 1 && ph < canvas->h * 2)
		break;
	    }
	    while (--upper);


	    do
	    {
	      scaleparams *s = &scaletable[lower];
	      int pw, ph; // proposed width and height

	      pw = (img_stamps[i]->w * s->numer + s->denom - 1) / s->denom;
	      ph = (img_stamps[i]->h * s->numer + s->denom - 1) / s->denom;

	      if (pw*ph > 20)
		break;
	    }
	    while (++lower < HARD_MAX_STAMP_SIZE);


	    if(upper<lower)
	    {
	      // this, if it ever happens, is very bad
	      upper = (upper+lower)/2;
	      lower = upper;
	    }

	    unsigned mid = default_stamp_size;
	    if(inf_stamps[i]->ratio != 1.0)
	       mid = compute_default_scale_factor(inf_stamps[i]->ratio);

	    if(mid > upper)
	      mid = upper;
	    
	    if(mid < lower)
	      mid = lower;

	    state_stamps[i]->min  = lower;
	    state_stamps[i]->size = mid;
	    state_stamps[i]->max  = upper;
	  }


	  /* If Tux Paint is in mirror-image-by-default mode, mirror, if we can: */
      
	  if (mirrorstamps && inf_stamps[i]->mirrorable)
	    state_stamps[i]->mirrored = 1;
	  else
	    state_stamps[i]->mirrored = 0;

	  state_stamps[i]->flipped = 0;

	  show_progress_bar();
	}
    }


  /* Load magic icons: */

  for (i = 0; i < NUM_MAGICS; i++)
    {
      img_magics[i] = loadimage(magic_img_fnames[i]);
      show_progress_bar();
    }


  /* Load shape icons: */

  for (i = 0; i < NUM_SHAPES; i++)
    {
      img_shapes[i] = loadimage(shape_img_fnames[i]);
      show_progress_bar();
    }


  /* Load tip tux images: */

  for (i = 0; i < NUM_TIP_TUX; i++)
    {
      img_tux[i] = loadimage(tux_img_fnames[i]);
      show_progress_bar();
    }
  
  
  /* Create toolbox and selector labels: */

  for (i = 0; i < NUM_TITLES; i++)
    {
      if (strlen(title_names[i]) > 0)
	{
	  if (need_own_font(language) && locale_font != NULL &&
		   strcmp(gettext(title_names[i]), title_names[i]) != 0)
	    {
	      tmp_surf = TTF_RenderUTF8_Blended(locale_font,
						textdir(gettext(title_names[i])), black);
	      img_title_names[i] = thumbnail(tmp_surf,
					     min(84, tmp_surf->w), tmp_surf->h, 0);
	      SDL_FreeSurface(tmp_surf);
	    }
	  else
	    {
	      upstr = uppercase(textdir(gettext(title_names[i])));
	      tmp_surf = TTF_RenderUTF8_Blended(large_font, upstr, black);
	      img_title_names[i] = thumbnail(tmp_surf,
					     min(84, tmp_surf->w), tmp_surf->h, 0);
	      SDL_FreeSurface(tmp_surf);
	      free(upstr);
	    }
	}
      else
	{
	  img_title_names[i] = NULL;
	}
    }



  /* Generate color selection buttons: */

#ifndef LOW_QUALITY_COLOR_SELECTOR

  /* Create appropriately-shaped buttons: */
  SDL_Surface *img1 = loadimage(DATA_PREFIX "brushes/round_36.png");
  SDL_Surface *img2 = thumbnail(img1,        (WINDOW_WIDTH - 96) / NUM_COLORS, 48, 0);
  tmp_btn_up        = thumbnail(img_btn_up,  (WINDOW_WIDTH - 96) / NUM_COLORS, 48, 0);
  tmp_btn_down      = thumbnail(img_btn_down,(WINDOW_WIDTH - 96) / NUM_COLORS, 48, 0);
  img_color_btn_off = thumbnail(img_btn_off, (WINDOW_WIDTH - 96) / NUM_COLORS, 48, 0);
  SDL_FreeSurface(img1);

  /* Create surfaces to draw them into: */

  for (i = 0; i < NUM_COLORS*2; i++)
    {

      img_color_btns[i] = SDL_CreateRGBSurface(screen->flags,
					       /* (WINDOW_WIDTH - 96) / NUM_COLORS, 48, */
					       tmp_btn_up->w, tmp_btn_up->h,
					       screen->format->BitsPerPixel,
					       screen->format->Rmask,
					       screen->format->Gmask,
					       screen->format->Bmask,
					       0);

      if (img_color_btns[i] == NULL)
	{
	  fprintf(stderr, "\nError: Can't build color button!\n"
		  "The Simple DirectMedia Layer error that occurred was:\n"
		  "%s\n\n", SDL_GetError());

	  cleanup();
	  exit(1);
	}

      SDL_LockSurface(img_color_btns[i]);
    }


  /* Generate the buttons based on the thumbnails: */

  SDL_LockSurface(tmp_btn_down);
  SDL_LockSurface(tmp_btn_up);

  for (y = 0; y < tmp_btn_up->h /* 48 */; y++)
    {
      for (x = 0; x < tmp_btn_up->w /* (WINDOW_WIDTH - 96) / NUM_COLORS */; x++)
	{
	  SDL_GetRGB(getpixel(tmp_btn_up, x, y), tmp_btn_up->format, &r, &g, &b);
	  double ru = sRGB_to_linear_table[r];
	  double gu = sRGB_to_linear_table[g];
	  double bu = sRGB_to_linear_table[b];
	  SDL_GetRGB(getpixel(tmp_btn_down, x, y), tmp_btn_down->format, &r, &g, &b);
	  double rd = sRGB_to_linear_table[r];
	  double gd = sRGB_to_linear_table[g];
	  double bd = sRGB_to_linear_table[b];
	  Uint8 a;
	  SDL_GetRGBA(getpixel(img2, x, y), img2->format, &r, &g, &b, &a);
	  double aa = a/255.0;

	  for (i = 0; i < NUM_COLORS; i++)
	    {
	      double rh = sRGB_to_linear_table[color_hexes[i][0]];
	      double gh = sRGB_to_linear_table[color_hexes[i][1]];
	      double bh = sRGB_to_linear_table[color_hexes[i][2]];
	      putpixel(img_color_btns[i], x, y,
		       SDL_MapRGB(img_color_btns[i]->format,
				  linear_to_sRGB(rh*aa + ru*(1.0-aa)),
				  linear_to_sRGB(gh*aa + gu*(1.0-aa)),
				  linear_to_sRGB(bh*aa + bu*(1.0-aa))));
	      putpixel(img_color_btns[i+NUM_COLORS], x, y,
		       SDL_MapRGB(img_color_btns[i+NUM_COLORS]->format,
				  linear_to_sRGB(rh*aa + rd*(1.0-aa)),
				  linear_to_sRGB(gh*aa + gd*(1.0-aa)),
				  linear_to_sRGB(bh*aa + bd*(1.0-aa))));
	    }
	}
    }

  for (i = 0; i < NUM_COLORS*2; i++)
    SDL_UnlockSurface(img_color_btns[i]);

  SDL_UnlockSurface(tmp_btn_up);
  SDL_UnlockSurface(tmp_btn_down);
  SDL_FreeSurface(tmp_btn_up);
  SDL_FreeSurface(tmp_btn_down);
  SDL_FreeSurface(img2);

#endif

  create_button_labels();


  /* Seed random-number generator: */

  srand(SDL_GetTicks());


  /* Enable Unicode support in SDL: */
  
  SDL_EnableUNICODE(1);


  /* Open demo recording or playback file: */

  if (recording)
    {
      demofi = fopen(playfile, "w");

      if (demofi == NULL)
	{
	  fprintf(stderr, "Error: Cannot create recording file: %s\n"
		  "%s\n\n",
		  playfile, strerror(errno));
	  exit(1);
	}
    }
  else if (playing)
    {
      demofi = fopen(playfile, "r");
    
      if (demofi == NULL)
	{
	  fprintf(stderr, "Error: Cannot open playback file: %s\n"
		  "%s\n\n",
		  playfile, strerror(errno));
	  exit(1);
	}
    }
  else
    {
      demofi = NULL;
    }
}


/* Render a button label using the appropriate string/font: */

static SDL_Surface * do_render_button_label(const char * const label)
{
  char * str;
  SDL_Surface * tmp_surf, * surf;
  SDL_Color black = {0, 0, 0, 0};

  if (need_own_font(language) && locale_font != NULL &&
	   strcmp(gettext(label), label) != 0)
    {
      tmp_surf = TTF_RenderUTF8_Blended(locale_font, textdir(gettext(label)), black);
      surf = thumbnail(tmp_surf, min(48, tmp_surf->w), tmp_surf->h, 0);
    }
  else
    {
      str = uppercase(textdir(gettext(label)));
      tmp_surf = TTF_RenderUTF8_Blended(small_font, str, black);
      surf = thumbnail(tmp_surf, min(48, tmp_surf->w), tmp_surf->h, 0);
      free(str);
      SDL_FreeSurface(tmp_surf);
    }

  return surf;
}

static void create_button_labels(void)
{
  int i;

  for (i = 0; i < NUM_TOOLS; i++)
    img_tool_names[i] = do_render_button_label(tool_names[i]);

  for (i = 0; i < NUM_MAGICS; i++)
    img_magic_names[i] = do_render_button_label(magic_names[i]);

  for (i = 0; i < NUM_SHAPES; i++)
    img_shape_names[i] = do_render_button_label(shape_names[i]);


  /* 'Open' label: */

  img_openlabels_open = do_render_button_label(textdir(gettext_noop("Open")));


  /* 'Erase' label: */

  img_openlabels_erase = do_render_button_label(textdir(gettext_noop("Erase")));


  /* 'Back' label: */

  img_openlabels_back = do_render_button_label(textdir(gettext_noop("Back")));
}


static void seticon(void)
{
  int masklen;
  Uint8 * mask;
  SDL_Surface * icon;

  /* Load icon into a surface: */

#ifndef WIN32
  icon = IMG_Load(DATA_PREFIX "images/icon.png");
#else
  icon = IMG_Load(DATA_PREFIX "images/icon32x32.png");
#endif

  if (icon == NULL)
    {
      fprintf(stderr,
	      "\nWarning: I could not load the icon image: %s\n"
	      "The Simple DirectMedia error that occurred was:\n"
	      "%s\n\n", DATA_PREFIX "images/icon.png", SDL_GetError());
      return;
    }


#ifndef WIN32
  /* Create mask: */
  masklen = (((icon -> w) + 7) / 8) * (icon -> h);
  mask = malloc(masklen * sizeof(Uint8));
  memset(mask, 0xFF, masklen);

  /* Set icon: */
  SDL_WM_SetIcon(icon, mask);

  /* Free icon surface & mask: */
  free(mask);
#else
  /* Set icon: */
  SDL_WM_SetIcon(icon, NULL);
#endif
  SDL_FreeSurface(icon);


  /* Grab keyboard and mouse, if requested: */

  if (grab_input)
    {
      debug("Grabbing input!");
      SDL_WM_GrabInput(SDL_GRAB_ON);
    }
}


/* Load a mouse pointer (cursor) shape: */

static SDL_Cursor * get_cursor(char * bits, char * mask_bits,
		        int width, int height, int x, int y)
{
  Uint8 b;
  Uint8 temp_bitmap[128], temp_bitmask[128];
  int i;


  if (((width + 7) / 8) * height > 128)
    {
      fprintf(stderr, "Error: Cursor is too large!\n");
      cleanup();
      exit(1);
    }

  for (i = 0; i < ((width + 7) / 8) * height; i++)
    {
      b = bits[i];

      temp_bitmap[i] = (((b & 0x01) << 7) |
			((b & 0x02) << 5) |
			((b & 0x04) << 3) |
			((b & 0x08) << 1) |
			((b & 0x10) >> 1) |
			((b & 0x20) >> 3) |
			((b & 0x40) >> 5) |
			((b & 0x80) >> 7));
    
      b = mask_bits[i];

      temp_bitmask[i] = (((b & 0x01) << 7) |
			 ((b & 0x02) << 5) |
			 ((b & 0x04) << 3) |
			 ((b & 0x08) << 1) |
			 ((b & 0x10) >> 1) |
			 ((b & 0x20) >> 3) |
			 ((b & 0x40) >> 5) |
			 ((b & 0x80) >> 7));
    }

  return(SDL_CreateCursor(temp_bitmap, temp_bitmask, width, height, x, y));
}


/* Load an image (with errors): */

static SDL_Surface * loadimage(const char * const fname)
{
  return(do_loadimage(fname, 1));
}


/* Load an image: */

static SDL_Surface * do_loadimage(const char * const fname, int abort_on_error)
{
  SDL_Surface * s, * disp_fmt_s;


  /* Load the image file: */

  s = IMG_Load(fname);
  if (s == NULL)
    {
      if (abort_on_error)
	{
	  fprintf(stderr,
		  "\nError: I couldn't load a graphics file:\n"
		  "%s\n"
		  "The Simple DirectMedia Layer error that occurred was:\n"
		  "%s\n\n", fname, SDL_GetError());

	  cleanup();
	  exit(1);
	}
      else
	{
	  return(NULL);
	}
    }


  /* Convert to the display format: */

  disp_fmt_s = SDL_DisplayFormatAlpha(s);
  if (disp_fmt_s == NULL)
    {
      if (abort_on_error)
	{
	  fprintf(stderr,
		  "\nError: I couldn't convert a graphics file:\n"
		  "%s\n"
		  "The Simple DirectMedia Layer error that occurred was:\n"
		  "%s\n\n", fname, SDL_GetError());

	  cleanup();
	  exit(1);
	}
      else
	{
	  SDL_FreeSurface(s);
	  return(NULL);
	}
    }


  /* Free the temp. surface & return the converted one: */

  SDL_FreeSurface(s);

  return(disp_fmt_s);
}


/* Draw the toolbar: */

static void draw_toolbar(void)
{
  int i;
  SDL_Rect dest;


  /* FIXME: A hack to make 'Print' button act just like 'New' button: */

  if (!disable_print)
    tool_avail[TOOL_PRINT] = tool_avail[TOOL_NEW];


  draw_image_title(TITLE_TOOLS, 0);

  for (i = 0; i < NUM_TOOLS + TOOLOFFSET; i++)
    {
      dest.x = ((i % 2) * 48);
      dest.y = ((i / 2) * 48) + 40;


      if (i < NUM_TOOLS)
	{
	  if (i == cur_tool)
	    {
	      SDL_BlitSurface(img_btn_down, NULL, screen, &dest);
	    }
	  else if (tool_avail[i])
	    {
	      SDL_BlitSurface(img_btn_up, NULL, screen, &dest);
	    }
	  else
	    {
	      SDL_BlitSurface(img_btn_off, NULL, screen, &dest);	
	    }

	  dest.x = ((i % 2) * 48) + 4;
	  dest.y = ((i / 2) * 48) + 40 + 4;

	  SDL_BlitSurface(img_tools[i], NULL, screen, &dest);


	  dest.x = ((i % 2) * 48) + 4 + (40 - img_tool_names[i]->w) / 2;
	  dest.y = ((i / 2) * 48) + 40 + 4 + (44 - img_tool_names[i]->h);

	  SDL_BlitSurface(img_tool_names[i], NULL, screen, &dest);
	}
      else
	{
	  SDL_BlitSurface(img_btn_off, NULL, screen, &dest);
	}
    }
}


/* Draw magic controls: */

static void draw_magic(void)
{
  int magic, i, max, off_y;
  SDL_Rect dest;


  /* FIXME: Should we worry about more than 14 magic effects? :^/ */


  draw_image_title(TITLE_MAGIC, WINDOW_WIDTH - 96);

  if (NUM_MAGICS > 14 + TOOLOFFSET)
    {
      off_y = 24;
      max = 12 + TOOLOFFSET;

      dest.x = WINDOW_WIDTH - 96;
      dest.y = 40;

      if (magic_scroll > 0)
	{
	  SDL_BlitSurface(img_scroll_up, NULL, screen, &dest);
	}
      else
	{
	  SDL_BlitSurface(img_scroll_up_off, NULL, screen, &dest);
	}

      dest.x = WINDOW_WIDTH - 96;
      dest.y = 40 + 24 + ((6 + TOOLOFFSET / 2) * 48);

      if (magic_scroll < NUM_MAGICS - 12 - TOOLOFFSET)
	{
	  SDL_BlitSurface(img_scroll_down, NULL, screen, &dest);
	}
      else
	{
	  SDL_BlitSurface(img_scroll_down_off, NULL, screen, &dest);
	}
    }
  else
    {
      off_y = 0;
      max = 14 + TOOLOFFSET;
    }

  
  for (magic = magic_scroll;
       magic < magic_scroll + max;
       magic++)
    {
      i = magic - magic_scroll;
      
      dest.x = ((i % 2) * 48) + (WINDOW_WIDTH - 96);
      dest.y = ((i / 2) * 48) + 40 + off_y;

      if (magic < NUM_MAGICS)
	{
	  if (magic == cur_magic)
	    {
	      SDL_BlitSurface(img_btn_down, NULL, screen, &dest);
	    }
	  else
	    {
	      SDL_BlitSurface(img_btn_up, NULL, screen, &dest);
	    }
	
	  dest.x = WINDOW_WIDTH - 96 + ((i % 2) * 48) + 4;
	  dest.y = ((i / 2) * 48) + 40 + 4 + off_y;

	  SDL_BlitSurface(img_magics[magic], NULL, screen, &dest);

	
	  dest.x = WINDOW_WIDTH - 96 + ((i % 2) * 48) + 4 +
	    (40 - img_magic_names[magic]->w) / 2;
	  dest.y = (((i / 2) * 48) + 40 + 4 +
		    (44 - img_magic_names[magic]->h) + off_y);

	  SDL_BlitSurface(img_magic_names[magic], NULL, screen, &dest);
	}
      else
	{
	  SDL_BlitSurface(img_btn_off, NULL, screen, &dest);	
	}
    }
}


/* Draw color selector: */

static int colors_state = COLORSEL_ENABLE | COLORSEL_CLOBBER;

static void draw_colors(int action)
{
  int i;
  SDL_Rect dest;
  static int old_color = -1;

  dest.x = 0;
  dest.y = 40 + ((NUM_TOOLS / 2) * 48) + HEIGHTOFFSET;

  int old_colors_state = colors_state;
  if (action==COLORSEL_CLOBBER)
    colors_state |= COLORSEL_CLOBBER;
  if (action==COLORSEL_REFRESH)
    colors_state &= ~COLORSEL_CLOBBER;
  if (action==COLORSEL_DISABLE)
    colors_state = COLORSEL_DISABLE;
  if (action==COLORSEL_ENABLE)
    colors_state = COLORSEL_ENABLE;

  colors_are_selectable = colors_state == COLORSEL_ENABLE;

  if (colors_state&COLORSEL_CLOBBER)
    return;
  if (cur_color==old_color && colors_state==old_colors_state)
    return;
  old_color = cur_color;

  if (colors_state == COLORSEL_ENABLE)
    {
      SDL_BlitSurface(img_title_large_on, NULL, screen, &dest);
  
      dest.x = 0;
      dest.y = 40 + ((NUM_TOOLS / 2) * 48) + HEIGHTOFFSET;
      SDL_BlitSurface(img_title_large_on, NULL, screen, &dest);
    
      dest.x = (96 - img_title_names[TITLE_COLORS]->w) / 2;
      dest.y = (40 + ((NUM_TOOLS / 2) * 48) + HEIGHTOFFSET + 
		(48 - img_title_names[TITLE_COLORS]->h) / 2);
      SDL_BlitSurface(img_title_names[TITLE_COLORS], NULL, screen, &dest);
    }
  else
    {
      SDL_BlitSurface(img_title_large_off, NULL, screen, &dest);
    }

  for (i = 0; i < NUM_COLORS; i++)
    {
      dest.x = (i * ((WINDOW_WIDTH - 96) / NUM_COLORS)) + 96;
      dest.y = 40 + ((NUM_TOOLS / 2) * 48) + HEIGHTOFFSET;
#ifndef LOW_QUALITY_COLOR_SELECTOR
      SDL_BlitSurface(
        (colors_state == COLORSEL_ENABLE)
          ? img_color_btns[i + (i==cur_color)*NUM_COLORS]
          : img_color_btn_off,
        NULL,
        screen,
        &dest
      );
#else
      dest.w = ((WINDOW_WIDTH - 96) / NUM_COLORS);
      dest.h = 48 + HEIGHTOFFSET;

      if (colors_state == COLORSEL_ENABLE)
	SDL_FillRect(screen, &dest,
		     SDL_MapRGB(screen->format,
				color_hexes[i][0],
				color_hexes[i][1],
				color_hexes[i][2]));
      else
	SDL_FillRect(screen, &dest,
		     SDL_MapRGB(screen->format, 240, 240, 240));
      if (i==cur_color && colors_state==COLORSEL_ENABLE)
	{
	  dest.x = (i * ((WINDOW_WIDTH - 96) / NUM_COLORS)) + 96;
	  dest.y = 44 + ((NUM_TOOLS / 2) * 48) + HEIGHTOFFSET;
	  SDL_BlitSurface(img_paintcan, NULL, screen, &dest);
	}
#endif

    }
//    SDL_UpdateRect(screen, 0, 0, screen->w, screen->h);
}


/* Draw brushes: */

static void draw_brushes(void)
{
  int i, off_y, max, brush;
  SDL_Rect dest;


  /* Draw the title: */

  draw_image_title(TITLE_BRUSHES, WINDOW_WIDTH - 96);


  /* Do we need scrollbars? */

  if (num_brushes > 14 + TOOLOFFSET)
    {
      off_y = 24;
      max = 12 + TOOLOFFSET;

      dest.x = WINDOW_WIDTH - 96;
      dest.y = 40;

      if (brush_scroll > 0)
	{
	  SDL_BlitSurface(img_scroll_up, NULL, screen, &dest);
	}
      else
	{
	  SDL_BlitSurface(img_scroll_up_off, NULL, screen, &dest);
	}

      dest.x = WINDOW_WIDTH - 96;
      dest.y = 40 + 24 + ((6 + TOOLOFFSET / 2) * 48);

      if (brush_scroll < num_brushes - 12 - TOOLOFFSET)
	{
	  SDL_BlitSurface(img_scroll_down, NULL, screen, &dest);
	}
      else
	{
	  SDL_BlitSurface(img_scroll_down_off, NULL, screen, &dest);
	}
    }
  else
    {
      off_y = 0;
      max = 14 + TOOLOFFSET;
    }


  /* Draw each of the shown brushes: */

  for (brush = brush_scroll;
       brush < brush_scroll + max;
       brush++)
    {
      i = brush - brush_scroll;


      dest.x = ((i % 2) * 48) + (WINDOW_WIDTH - 96);
      dest.y = ((i / 2) * 48) + 40 + off_y;

      if (brush == cur_brush)
	{
	  SDL_BlitSurface(img_btn_down, NULL, screen, &dest);
	}
      else if (brush < num_brushes)
	{
	  SDL_BlitSurface(img_btn_up, NULL, screen, &dest);
	}
      else
	{
	  SDL_BlitSurface(img_btn_off, NULL, screen, &dest);
	}

      if (brush < num_brushes)
	{
	  dest.x = ((i % 2) * 48) + (WINDOW_WIDTH - 96) +
	    ((48 - (img_brushes[brush]->w)) / 2);

	  /* FIXME: Shouldn't that be ->h??? */

	  dest.y = ((i / 2) * 48) + 40 + ((48 - (img_brushes[brush]->w)) / 2) +
	    off_y;

	  SDL_BlitSurface(img_brushes[brush], NULL, screen, &dest);
	}
    }
}


/* Draw fonts: */

static void draw_fonts(void)
{
  int i, off_y, max, font;
  SDL_Rect dest, src;
  SDL_Surface * tmp_surf;
  SDL_Color black = {0, 0, 0, 0};


  /* Draw the title: */

  draw_image_title(TITLE_LETTERS, WINDOW_WIDTH - 96);


  /* Do we need scrollbars? */

  if (num_fonts > 14 + TOOLOFFSET)
    {
      off_y = 24;
      max = 12 + TOOLOFFSET;

      dest.x = WINDOW_WIDTH - 96;
      dest.y = 40;

      if (font_scroll > 0)
	{
	  SDL_BlitSurface(img_scroll_up, NULL, screen, &dest);
	}
      else
	{
	  SDL_BlitSurface(img_scroll_up_off, NULL, screen, &dest);
	}

      dest.x = WINDOW_WIDTH - 96;
      dest.y = 40 + 24 + ((6 + TOOLOFFSET / 2) * 48);

      if (font_scroll < num_fonts - 12 - TOOLOFFSET)
	{
	  SDL_BlitSurface(img_scroll_down, NULL, screen, &dest);
	}
      else
	{
	  SDL_BlitSurface(img_scroll_down_off, NULL, screen, &dest);
	}
    }
  else
    {
      off_y = 0;
      max = 14 + TOOLOFFSET;
    }


  /* Draw each of the shown fonts: */

  for (font = font_scroll;
       font < font_scroll + max;
       font++)
    {
      i = font - font_scroll;


      dest.x = ((i % 2) * 48) + (WINDOW_WIDTH - 96);
      dest.y = ((i / 2) * 48) + 40 + off_y;

      if (font == cur_font)
	{
	  SDL_BlitSurface(img_btn_down, NULL, screen, &dest);
	}
      else if (font < num_fonts)
	{
	  SDL_BlitSurface(img_btn_up, NULL, screen, &dest);
	}
      else
	{
	  SDL_BlitSurface(img_btn_off, NULL, screen, &dest);
	}

    
      /* FIXME: We should render the font buttons once, at startup! */

      if (font < num_fonts)
	{
	  tmp_surf = TTF_RenderUTF8_Blended(fonts[font], "A", black);

	  src.x = (tmp_surf->w - 48) / 2;
	  src.y = (tmp_surf->h - 48) / 2;
	  src.w = 48;
	  src.h = 48;
      
	  if (src.x < 0)
	    src.x = 0;
	  if (src.y < 0)
	    src.y = 0;

	  dest.x = ((i % 2) * 48) + (WINDOW_WIDTH - 96);
	  if (src.w > tmp_surf->w)
	    {
	      src.w = tmp_surf->w;
	      dest.x = dest.x + ((48 - (tmp_surf->w)) / 2);
	    }

      
	  dest.y = ((i / 2) * 48) + 40 + off_y;
	  if (src.h > tmp_surf->h)
	    {
	      src.h = tmp_surf->h;
	      dest.y = dest.y + ((48 - (tmp_surf->h)) / 2);
	    }
      
	  SDL_BlitSurface(tmp_surf, &src, screen, &dest);
      
	  SDL_FreeSurface(tmp_surf);
	}
    }
}


/* Draw stamps: */

static void draw_stamps(void)
{
  int i, off_y, max, stamp, most;
  int base_x, base_y, xx, yy;
  SDL_Rect src, dest;
  SDL_Surface * img;


  /* Draw the title: */

  draw_image_title(TITLE_STAMPS, WINDOW_WIDTH - 96);


  /* How many can we show? */

  most = 10;
  if (disable_stamp_controls)
    most = 14;


  /* Do we need scrollbars? */

  if (num_stamps > most + TOOLOFFSET)
    {
      off_y = 24;
      max = (most - 2) + TOOLOFFSET;

      dest.x = WINDOW_WIDTH - 96;
      dest.y = 40;

      if (stamp_scroll > 0)
	{
	  SDL_BlitSurface(img_scroll_up, NULL, screen, &dest);
	}
      else
	{
	  SDL_BlitSurface(img_scroll_up_off, NULL, screen, &dest);
	}

    
      dest.x = WINDOW_WIDTH - 96;
      dest.y = 40 + 24 + ((6 + TOOLOFFSET / 2) * 48);

      if (!disable_stamp_controls)
	dest.y = dest.y - (48 * 2);

      if (stamp_scroll < num_stamps - (most - 2) - TOOLOFFSET)
	{
	  SDL_BlitSurface(img_scroll_down, NULL, screen, &dest);
	}
      else
	{
	  SDL_BlitSurface(img_scroll_down_off, NULL, screen, &dest);
	}
    }
  else
    {
      off_y = 0;
      max = most + TOOLOFFSET;
    }


  /* Draw each of the shown stamps: */

  for (stamp = stamp_scroll;
       stamp < stamp_scroll + max;
       stamp++)
    {
      i = stamp - stamp_scroll;


      dest.x = ((i % 2) * 48) + (WINDOW_WIDTH - 96);
      dest.y = ((i / 2) * 48) + 40 + off_y;

      if (stamp == cur_stamp)
	{
	  SDL_BlitSurface(img_btn_down, NULL, screen, &dest);
	}
      else if (stamp < num_stamps)
	{
	  SDL_BlitSurface(img_btn_up, NULL, screen, &dest);
	}
      else
	{
	  SDL_BlitSurface(img_btn_off, NULL, screen, &dest);
	}

      if (stamp < num_stamps)
	{
	  /* Draw the stamp itself: */

	  if (state_stamps[stamp]->mirrored &&
	      img_stamps_premirror[stamp] != NULL)
	    {
	      /* Use pre-drawn mirrored version! */

	      if (img_stamp_thumbs_premirror[stamp] != NULL)
		img = img_stamp_thumbs_premirror[stamp];
	      else
		img = img_stamps_premirror[stamp];
	    }
	  else
	    {
	      /* Use normal version: */
	      
	      if (img_stamp_thumbs[stamp] != NULL)
		img = img_stamp_thumbs[stamp];
	      else
		img = img_stamps[stamp];
	    }


	  /* Where to put it? */
      
	  base_x = ((i % 2) * 48) + (WINDOW_WIDTH - 96) +
	    ((48 - (img->w)) / 2);

	  base_y = ((i / 2) * 48) + 40 + ((48 - (img->h)) / 2) + off_y;


	  if (state_stamps[stamp]->mirrored &&
	      img_stamps_premirror[stamp] == NULL)
	    {
	      /* It's mirrored!: */
	      
	      if (state_stamps[stamp]->flipped)
		{
		  /* It's flipped, too!  Mirror AND flip! */
		
		  for (xx = 0; xx < img->w; xx++)
		    {
		      for (yy = 0; yy < img->h; yy++)
			{
			  src.x = xx;
			  src.y = yy;
			  src.w = 1;
			  src.h = 1;

			  dest.x = base_x + img->w - xx - 1;
			  dest.y = base_y + img->h - yy - 1;

			  SDL_BlitSurface(img, &src, screen, &dest);
			}
		    }
		}
	      else
		{
		  /* Not flipped; just simple mirror-image: */
	  
		  for (xx = 0; xx < img->w; xx++)
		    {
		      src.x = xx;
		      src.y = 0;
		      src.w = 1;
		      src.h = img->h;

		      dest.x = base_x + img->w - xx - 1;
		      dest.y = base_y;

		      SDL_BlitSurface(img, &src, screen, &dest);
		    }
		}
	    }
	  else
	    {
	      /* It's not mirrored: */

	      if (state_stamps[stamp]->flipped)
		{
		  /* Simple flip-image: */
	  
		  for (yy = 0; yy < img->h; yy++)
		    {
		      src.x = 0;
		      src.y = yy;
		      src.w = img->w;
		      src.h = 1;

		      dest.x = base_x;
		      dest.y = base_y + img->h - yy - 1;

		      SDL_BlitSurface(img, &src, screen, &dest);
		    }
		}
	      else
		{
		  /* No flip or mirror: just blit! */

		  dest.x = base_x;
		  dest.y = base_y;

		  SDL_BlitSurface(img, NULL, screen, &dest);
		}
	    }
	}
    }


  /* Draw stamp controls: */

  if (!disable_stamp_controls)
    {
      /* Show mirror button: */

      dest.x = WINDOW_WIDTH - 96;
      dest.y = 40 + ((5 + TOOLOFFSET / 2) * 48);

      if (inf_stamps[cur_stamp]->mirrorable)
	{
	  if (state_stamps[cur_stamp]->mirrored)
	    SDL_BlitSurface(img_btn_down, NULL, screen, &dest);
	  else
	    SDL_BlitSurface(img_btn_up, NULL, screen, &dest);
	}
      else
	{
	  SDL_BlitSurface(img_btn_off, NULL, screen, &dest);
	}
    
      dest.x = WINDOW_WIDTH - 96 + (48 - img_magics[MAGIC_MIRROR]->w) / 2;
      dest.y = (40 + ((5 + TOOLOFFSET / 2) * 48) +
		(48 - img_magics[MAGIC_MIRROR]->h) / 2);

      SDL_BlitSurface(img_magics[MAGIC_MIRROR], NULL, screen, &dest);

    
      /* Show flip button: */

      dest.x = WINDOW_WIDTH - 48;
      dest.y = 40 + ((5 + TOOLOFFSET / 2) * 48);

      if (inf_stamps[cur_stamp]->flipable)
	{
	  if (state_stamps[cur_stamp]->flipped)
	    SDL_BlitSurface(img_btn_down, NULL, screen, &dest);
	  else
	    SDL_BlitSurface(img_btn_up, NULL, screen, &dest);
	}
      else
	{
	  SDL_BlitSurface(img_btn_off, NULL, screen, &dest);
	}

      dest.x = WINDOW_WIDTH - 48 + (48 - img_magics[MAGIC_FLIP]->w) / 2;
      dest.y = (40 + ((5 + TOOLOFFSET / 2) * 48) +
		(48 - img_magics[MAGIC_FLIP]->h) / 2);
    
      SDL_BlitSurface(img_magics[MAGIC_FLIP], NULL, screen, &dest);

    
      /* Show shrink button: */

      dest.x = WINDOW_WIDTH - 96;
      dest.y = 40 + ((6 + TOOLOFFSET / 2) * 48);

      if (state_stamps[cur_stamp]->size > MIN_STAMP_SIZE)
	SDL_BlitSurface(img_btn_up, NULL, screen, &dest);
      else
	SDL_BlitSurface(img_btn_off, NULL, screen, &dest);
    
      dest.x = WINDOW_WIDTH - 96 + (48 - img_shrink->w) / 2;
      dest.y = (40 + ((6 + TOOLOFFSET / 2) * 48) +
		(48 - img_shrink->h) / 2);

      SDL_BlitSurface(img_shrink, NULL, screen, &dest);

    
      /* Show grow button: */

      dest.x = WINDOW_WIDTH - 48;
      dest.y = 40 + ((6 + TOOLOFFSET / 2) * 48);

      if (state_stamps[cur_stamp]->size < MAX_STAMP_SIZE)
	SDL_BlitSurface(img_btn_up, NULL, screen, &dest);
      else
	SDL_BlitSurface(img_btn_off, NULL, screen, &dest);

      dest.x = WINDOW_WIDTH - 48 + (48 - img_grow->w) / 2;
      dest.y = (40 + ((6 + TOOLOFFSET / 2) * 48) +
		(48 - img_grow->h) / 2);
    
      SDL_BlitSurface(img_grow, NULL, screen, &dest);
    }
}


/* Draw the shape selector: */

static void draw_shapes(void)
{
  int i;
  SDL_Rect dest;


  draw_image_title(TITLE_SHAPES, WINDOW_WIDTH - 96);

  for (i = 0; i < 14 + TOOLOFFSET; i++)
    {
      dest.x = ((i % 2) * 48) + WINDOW_WIDTH - 96;
      dest.y = ((i / 2) * 48) + 40;


      if (i == cur_shape)
	{
	  SDL_BlitSurface(img_btn_down, NULL, screen, &dest);
	}
      else if (i < NUM_SHAPES)
	{
	  SDL_BlitSurface(img_btn_up, NULL, screen, &dest);
	}
      else
	{
	  SDL_BlitSurface(img_btn_off, NULL, screen, &dest);
	}


      if (i < NUM_SHAPES)
	{
	  dest.x = ((i % 2) * 48) + 4 + WINDOW_WIDTH - 96;
	  dest.y = ((i / 2) * 48) + 40 + 4;
	
	  SDL_BlitSurface(img_shapes[i], NULL, screen, &dest);

	  dest.x = ((i % 2) * 48) + 4 + WINDOW_WIDTH - 96 +
	    (40 - img_shape_names[i]->w) / 2;
          dest.y = ((i / 2) * 48) + 40 + 4 + (44 - img_shape_names[i]->h);

          SDL_BlitSurface(img_shape_names[i], NULL, screen, &dest);
	}
    }
}


/* Draw the eraser selector: */

static void draw_erasers(void)
{
  int i, x, y, sz;
  SDL_Rect dest;


  draw_image_title(TITLE_ERASERS, WINDOW_WIDTH - 96);

  for (i = 0; i < 14 + TOOLOFFSET; i++)
    {
      dest.x = ((i % 2) * 48) + WINDOW_WIDTH - 96;
      dest.y = ((i / 2) * 48) + 40;


      if (i == cur_eraser)
	{
	  SDL_BlitSurface(img_btn_down, NULL, screen, &dest);
	}
      else if (i < NUM_ERASERS)
	{
	  SDL_BlitSurface(img_btn_up, NULL, screen, &dest);
	}
      else
	{
	  SDL_BlitSurface(img_btn_off, NULL, screen, &dest);
	}


      if (i < NUM_ERASERS)
	{
	  sz = (2 + ((NUM_ERASERS - 1 - i) * (38 / (NUM_ERASERS - 1))));

	  x = ((i % 2) * 48) + WINDOW_WIDTH - 96 + 24 - sz / 2;
	  y = ((i / 2) * 48) + 40 + 24 - sz / 2;

	  dest.x = x;
	  dest.y = y;
	  dest.w = sz;
	  dest.h = 2;

	  SDL_FillRect(screen, &dest,
		       SDL_MapRGB(screen->format, 0, 0, 0));

	  dest.x = x;
	  dest.y = y + sz - 2;
	  dest.w = sz;
	  dest.h = 2;

	  SDL_FillRect(screen, &dest,
		       SDL_MapRGB(screen->format, 0, 0, 0));

	  dest.x = x;
	  dest.y = y;
	  dest.w = 2;
	  dest.h = sz;

	  SDL_FillRect(screen, &dest,
		       SDL_MapRGB(screen->format, 0, 0, 0));
	  
	  dest.x = x + sz - 2;
	  dest.y = y;
	  dest.w = 2;
	  dest.h = sz;

	  SDL_FillRect(screen, &dest,
		       SDL_MapRGB(screen->format, 0, 0, 0));
	}
    }
}


/* Draw no selectables: */

static void draw_none(void)
{
  int i;
  SDL_Rect dest;

  dest.x = WINDOW_WIDTH - 96;
  dest.y = 0;
  SDL_BlitSurface(img_title_off, NULL, screen, &dest);

  for (i = 0; i < 14 + TOOLOFFSET; i++)
    {
      dest.x = ((i % 2) * 48) + WINDOW_WIDTH - 96;
      dest.y = ((i / 2) * 48) + 40;

      SDL_BlitSurface(img_btn_off, NULL, screen, &dest);
    }
}


/* Load an arbitrary set of images into an array (e.g., brushes or stamps) */

#ifndef NOSOUND
static void loadarbitrary(SDL_Surface * surfs[], SDL_Surface * altsurfs[],
		   char * descs[], info_type * infs[],
		   Mix_Chunk * sounds[],
		   int * count, int starting, int max,
		   const char * const dir, int fatal, int maxw, int maxh)
#else
static void loadarbitrary(SDL_Surface * surfs[], SDL_Surface * altsurfs[],
			char * descs[], info_type * infs[],
			int * count, int starting, int max,
			const char * const dir, int fatal, int maxw, int maxh)
#endif
{
  DIR * d;
  struct dirent * f;
  struct stat sbuf;
  char fname[512];
  int d_names_alloced;
  char * * d_names;
  int num_files, i;


  /* Make some space: */

  d_names_alloced = 32;
  d_names = (char * *) malloc(sizeof(char *) * d_names_alloced);
  if (d_names == NULL)
    {
      fprintf(stderr,
	      "\nError: I can't allocate memory for directory listing!\n"
	      "The system error that occurred was: %s\n",
	      strerror(errno));
      cleanup();
      exit(1);
    }


  *count = starting;

  /* Open the directory: */

  d = opendir(dir);
  if (d == NULL)
    {
      if (fatal)
	{
	  fprintf(stderr,
		  "\nError: I can't find a directory of images\n"
		  "%s\n"
		  "The system error that occurred was: %s\n",
		  dir, strerror(errno));

	  cleanup();
	  exit(1);
	}
      else
	{
	  return;
	}
    }


  /* Read directory for images: */

  num_files = 0;
  do
    {
      f = readdir(d);

      if (f != NULL)
	{
	  d_names[num_files] = strdup(f->d_name);
	  num_files++;

	  if (num_files >= d_names_alloced)
	    {
	      d_names_alloced = d_names_alloced + 32;

	      d_names = (char * *) realloc(d_names, sizeof(char *) * d_names_alloced);
	      if (d_names == NULL)
		{
		  fprintf(stderr,
			  "\nError: I can't reallocate memory for directory listing!\n"
			  "The system error that occurred was: %s\n",
			  strerror(errno));
		  cleanup();
		  exit(1);
		}
	    }
	}
    }
  while (f != NULL);

  closedir(d);


  qsort(d_names, num_files, sizeof(char *),
	(int(*)(const void *, const void *))compare_strings);


  /* Do something with each file (load if PNG, recurse if directory): */

  for (i = 0; i < num_files && *count < max; i++)
    {
      /* Ignore things starting with "." (e.g., "." and ".." dirs): */

      if (strstr(d_names[i], ".") != d_names[i])
	{
	  /* If it's a directory, recurse down into it: */

	  snprintf(fname, sizeof(fname), "%s/%s", dir, d_names[i]);
	  debug(fname);
	
	  stat(fname, &sbuf);
	
	  if (S_ISDIR(sbuf.st_mode))
	    {
	      debug("...is a directory");

#ifndef NOSOUND
	      loadarbitrary(surfs, altsurfs, descs, infs, sounds,
			    count, *count, max, fname,
			    fatal, maxw, maxh);
#else
	      loadarbitrary(surfs, altsurfs, descs, infs,
			    count, *count, max, fname,
			    fatal, maxw, maxh);
#endif
	    }
	  else if (strstr(d_names[i], ".png") != NULL &&
		   strstr(d_names[i], "_mirror.png") == NULL)
	    {
	      /* If it has ".png" in the filename, assume we can try to load it: */
	
	      surfs[*count] = loadimage(fname);

	      if ((surfs[*count]->w <= maxw &&
		   surfs[*count]->h <= maxh) ||
		  (maxw == -1 || maxh == -1))
		{
		  /* Check for a companion ".txt" file! */

		  if (descs != NULL)
		    descs[*count] = loaddesc(fname);

		  if (infs != NULL)
		    infs[*count] = loadinfo(fname);

		  if (altsurfs != NULL)
		    altsurfs[*count] = loadaltimage(fname);


#ifndef NOSOUND
		  if (use_sound)
		    {
		      if (sounds != NULL)
			sounds[*count] = loadsound(fname);
		    }
#endif


		  *count = *count + 1;
		}
	      else
		{
		  fprintf(stderr,
			  "\nWarning: Image too large (%d x %d - max: %d x %d)\n"
			  "%s\n\n",
			  surfs[*count]->w, surfs[*count]->h, maxw, maxh,
			  fname);
		}

	      show_progress_bar();
	    }
	}

      free(d_names[i]);
    }

  free(d_names);


  /* Give warning if too many files were found (e.g., some not loaded): */

  if (*count == max)
    {
      fprintf(stderr,
	      "\nWarning: Reached maximum images (%d) which can be stored in:\n"
	      "%s\n\n",
	      max, dir);
    }

  debug("loadarbitrary() ends...");
}


/* Create a thumbnail: */

static SDL_Surface * thumbnail(SDL_Surface * src, int max_x, int max_y,
			int keep_aspect)
{
  int x, y;
  float src_x, src_y, off_x, off_y;
  SDL_Surface * s;
  Uint32 amask, tr, tg, tb, ta;
  Uint8 r, g, b, a;
  float xscale, yscale;
  int tmp;


  /* Determine scale and centering offsets: */

  if (!keep_aspect)
    {
      yscale = (float) ((float) src->h / (float) max_y);
      xscale = (float) ((float) src->w / (float) max_x);

      off_x = 0;
      off_y = 0;
    }
  else
    {
      if (src->h > src->w)
	{
	  yscale = (float) ((float) src->h / (float) max_y);
	  xscale = yscale;

	  off_x = ((src->h - src->w) / xscale) / 2;
	  off_y = 0;
	}
      else
	{
	  xscale = (float) ((float) src->w / (float) max_x);
	  yscale = xscale;

	  off_x = 0;
	  off_y = ((src->w - src->h) / xscale) / 2;
	}
    }


  /* Create surface for thumbnail: */

  amask = ~(src->format->Rmask |
            src->format->Gmask |
            src->format->Bmask);
  
  s = SDL_CreateRGBSurface(src->flags, /* SDL_SWSURFACE, */
		           max_x, max_y,
			   src->format->BitsPerPixel,
			   src->format->Rmask,
			   src->format->Gmask,
			   src->format->Bmask,
			   src->format->Amask); /* amask); */


  if (s == NULL)
    {
      fprintf(stderr, "\nError: Can't build stamp thumbnails\n"
	      "The Simple DirectMedia Layer error that occurred was:\n"
	      "%s\n\n", SDL_GetError());

      cleanup();
      exit(1);
    }


  /* Create thumbnail itself: */

  SDL_LockSurface(src);
  SDL_LockSurface(s);

  for (y = 0; y < max_y; y++)
    {
      for (x = 0; x < max_x; x++)
	{
#ifndef LOW_QUALITY_THUMBNAILS
	  tr = 0;
	  tg = 0;
	  tb = 0;
	  ta = 0;

	  tmp = 0;

	  for (src_y = y * yscale; src_y < y * yscale + yscale &&
		 src_y < src->h; src_y++)
	    {
	      for (src_x = x * xscale; src_x < x * xscale + xscale &&
		     src_x < src->w; src_x++)
		{
		  SDL_GetRGBA(getpixel(src, src_x, src_y),
			      src->format,
			      &r, &g, &b, &a);

		  tr = tr + r;
		  tb = tb + b;
		  tg = tg + g;
		  ta = ta + a;

		  tmp++;
		}
	    }

	  if (tmp != 0)
	    {
	      tr = tr / tmp;
	      tb = tb / tmp;
	      tg = tg / tmp;
	      ta = ta / tmp;

	      putpixel(s, x + off_x, y + off_y, SDL_MapRGBA(s->format,
							    (Uint8) tr,
							    (Uint8) tg,
							    (Uint8) tb,
							    (Uint8) ta));
	    }
#else
	  src_x = x * xscale;
	  src_y = y * yscale;

	  putpixel(s, x + off_x, y + off_y, getpixel(src, src_x, src_y));
#endif
	}
    }

  SDL_UnlockSurface(s);
  SDL_UnlockSurface(src);

  return s;
}


/* Get a pixel: */

static Uint32 getpixel(SDL_Surface * surface, int x, int y)
{
  Uint8 * p;
  Uint32 pixel;

  // Always 4, except 3 when loading a saved image.
  int BytesPerPixel = surface->format->BytesPerPixel;

  /* get the X/Y values within the bounds of this surface */
  if (unlikely( (unsigned)x > (unsigned)surface->w - 1u ))
    x = (x<0) ? 0 : surface->w - 1;
  if (unlikely( (unsigned)y > (unsigned)surface->h - 1u ))
    y = (y<0) ? 0 : surface->h - 1;

  /* Set a pointer to the exact location in memory of the pixel
     in question: */

  p = (Uint8 *) (((Uint8 *)surface->pixels) +  /* Start at top of RAM */
    (y * surface->pitch) +  /* Go down Y lines */
    (x * BytesPerPixel));             /* Go in X pixels */


  /* Return the correctly-sized piece of data containing the
   * pixel's value (an 8-bit palette value, or a 16-, 24- or 32-bit
   * RGB value) */

  if (likely(BytesPerPixel == 4))
    return *(Uint32 *)p;  // 32-bit display

  if (BytesPerPixel == 1)         /* 8-bit display */
    pixel = *p;
  else if (BytesPerPixel == 2)    /* 16-bit display */
    pixel = *(Uint16 *)p;
  else /* if (BytesPerPixel == 3) */   /* 24-bit display */
    {
      /* Depending on the byte-order, it could be stored RGB or BGR! */

      if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
        pixel = p[0] << 16 | p[1] << 8 | p[2];
      else
        pixel = p[0] | p[1] << 8 | p[2] << 16;
    }

  return pixel;
}


/* Draw a single pixel into the surface: */

static void putpixel(SDL_Surface * surface, int x, int y, Uint32 pixel)
{
  Uint8 * p;
  // Always 4, except 3 when loading a saved image.
  int BytesPerPixel = surface->format->BytesPerPixel;

  /* Assuming the X/Y values are within the bounds of this surface... */

  if (likely( likely((unsigned)x<(unsigned)surface->w) && likely((unsigned)y<(unsigned)surface->h) ))
    {
      // Set a pointer to the exact location in memory of the pixel
      p = (Uint8 *) (((Uint8 *)surface->pixels) + /* Start: beginning of RAM */
		     (y * surface->pitch) +  /* Go down Y lines */
                     (x * BytesPerPixel));             /* Go in X pixels */


      /* Set the (correctly-sized) piece of data in the surface's RAM
       *          to the pixel value sent in: */

      if (likely(BytesPerPixel == 4))
        *(Uint32 *)p = pixel;  // 32-bit display
      else if (BytesPerPixel == 1)
        *p = pixel;
      else if (BytesPerPixel == 2)
        *(Uint16 *)p = pixel;
      else if (BytesPerPixel == 3)
        {
          if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
            {
              p[0] = (pixel >> 16) & 0xff;
              p[1] = (pixel >> 8) & 0xff;
              p[2] = pixel & 0xff;
            }
          else
            {
              p[0] = pixel & 0xff;
              p[1] = (pixel >> 8) & 0xff;
              p[2] = (pixel >> 16) & 0xff;
            }
        }
    }
}


/* Should really clip at the line level, but oh well... */

static void clipped_putpixel(SDL_Surface * dest, int x, int y, Uint32 c)
{
  if (x >= 96 && x < (WINDOW_WIDTH - 96) &&
      y >= 0 && y < (48 * 7 + 40 + HEIGHTOFFSET))
    {
      putpixel(dest, x, y, c);
    }
}



/* Show debugging stuff: */

static void debug(const char * const str)
{
#ifndef DEBUG
  (void)str;
#else
  fprintf(stderr, "DEBUG: %s\n", str);
  fflush(stderr);
#endif
}


/* Undo! */

static void do_undo(void)
{
  int wanna_update_toolbar;

  wanna_update_toolbar = 0;

  if (cur_undo != oldest_undo)
    {
      cur_undo--;

      if (cur_undo < 0)
	cur_undo = NUM_UNDO_BUFS - 1;

#ifdef DEBUG
      printf("BLITTING: %d\n", cur_undo);
#endif
      SDL_BlitSurface(undo_bufs[cur_undo], NULL, canvas, NULL);


      if (img_starter != NULL)
      {
        if (undo_starters[cur_undo] == UNDO_STARTER_MIRRORED)
        {
	  starter_mirrored = !starter_mirrored;
	  mirror_starter();
        }
        else if (undo_starters[cur_undo] == UNDO_STARTER_FLIPPED)
        {
	  starter_flipped = !starter_flipped;
	  flip_starter();
        }
      }

      update_canvas(0, 0, (WINDOW_WIDTH - 96), (48 * 7) + 40 + HEIGHTOFFSET);


      if (cur_undo == oldest_undo)
	{
	  tool_avail[TOOL_UNDO] = 0;
	  wanna_update_toolbar = 1;
	}

      if (tool_avail[TOOL_REDO] == 0)
	{
	  tool_avail[TOOL_REDO] = 1;
	  wanna_update_toolbar = 1;
	}

      if (wanna_update_toolbar)
	{
	  draw_toolbar();
	  SDL_UpdateRect(screen, 0, 0, 96, (48 * (7 + TOOLOFFSET / 2)) + 40);
	}

      been_saved = 0;
    }

#ifdef DEBUG
  printf("UNDO: Current=%d  Oldest=%d  Newest=%d\n",
	 cur_undo, oldest_undo, newest_undo);
#endif
}


/* Redo! */

static void do_redo(void)
{
  if (cur_undo != newest_undo)
    {
      if (img_starter != NULL)
      {
        if (undo_starters[cur_undo] == UNDO_STARTER_MIRRORED)
        {
	  starter_mirrored = !starter_mirrored;
	  mirror_starter();
        }
        else if (undo_starters[cur_undo] == UNDO_STARTER_FLIPPED)
        {
	  starter_flipped = !starter_flipped;
	  flip_starter();
        }
      }

      cur_undo = (cur_undo + 1) % NUM_UNDO_BUFS;

#ifdef DEBUG
      printf("BLITTING: %d\n", cur_undo);
#endif
      SDL_BlitSurface(undo_bufs[cur_undo], NULL, canvas, NULL);

      update_canvas(0, 0, (WINDOW_WIDTH - 96), (48 * 7) + 40 + HEIGHTOFFSET);

      been_saved = 0;
    }

#ifdef DEBUG
  printf("REDO: Current=%d  Oldest=%d  Newest=%d\n",
	 cur_undo, oldest_undo, newest_undo);
#endif


  if (((cur_undo + 1) % NUM_UNDO_BUFS) == newest_undo)
    {
      tool_avail[TOOL_REDO] = 0;
    }

  tool_avail[TOOL_UNDO] = 1;

  draw_toolbar();
  SDL_UpdateRect(screen, 0, 0, 96, (48 * (7 + TOOLOFFSET / 2)) + 40);
}


/* Create the current brush in the current color: */

static void render_brush(void)
{
  Uint32 amask;
  int x, y;
  Uint8 r, g, b, a;


  /* Kludge; not sure why cur_brush would become greater! */

  if (cur_brush >= num_brushes)
    cur_brush = 0;
  

  /* Free the old rendered brush (if any): */

  if (img_cur_brush != NULL)
    {
      SDL_FreeSurface(img_cur_brush);
    }


  /* Create a surface to render into: */

  amask = ~(img_brushes[cur_brush]->format->Rmask |
	    img_brushes[cur_brush]->format->Gmask |
	    img_brushes[cur_brush]->format->Bmask);

  img_cur_brush =
    SDL_CreateRGBSurface(SDL_SWSURFACE,
			 img_brushes[cur_brush]->w,
			 img_brushes[cur_brush]->h,
			 img_brushes[cur_brush]->format->BitsPerPixel,
			 img_brushes[cur_brush]->format->Rmask,
			 img_brushes[cur_brush]->format->Gmask,
			 img_brushes[cur_brush]->format->Bmask,
			 amask);

  if (img_cur_brush == NULL)
    {
      fprintf(stderr, "\nError: Can't render a brush!\n"
	      "The Simple DirectMedia Layer error that occurred was:\n"
	      "%s\n\n", SDL_GetError());

      cleanup();
      exit(1);
    }


  /* Render the new brush: */

  SDL_LockSurface(img_brushes[cur_brush]);
  SDL_LockSurface(img_cur_brush);

  for (y = 0; y < img_brushes[cur_brush]->h; y++)
    {
      for (x = 0; x < img_brushes[cur_brush]->w; x++)
	{
	  SDL_GetRGBA(getpixel(img_brushes[cur_brush], x, y),
		      img_brushes[cur_brush]->format,
		      &r, &g, &b, &a);
	
	  putpixel(img_cur_brush, x, y,
		   SDL_MapRGBA(img_cur_brush->format,
			       color_hexes[cur_color][0],
			       color_hexes[cur_color][1],
			       color_hexes[cur_color][2],
			       a));
	}
    }

  SDL_UnlockSurface(img_cur_brush);
  SDL_UnlockSurface(img_brushes[cur_brush]);

  brush_counter = 0;
}


/* Play a sound: */

static void playsound(int chan, int s, int override)
{
#ifndef NOSOUND
  if (use_sound)
    {
      if (override || !Mix_Playing(chan))
	Mix_PlayChannel(chan, sounds[s], 0);
    }
#endif
}


/* Draw a XOR line: */

static void line_xor(int x1, int y1, int x2, int y2)
{
  int dx, dy, y, num_drawn;
  float m, b;


  /* Kludgey, but it works: */
  
  //SDL_LockSurface(screen);

  dx = x2 - x1;
  dy = y2 - y1;

  num_drawn = 0;

  if (dx != 0)
    {
      m = ((float) dy) / ((float) dx);
      b = y1 - m * x1;

      if (x2 >= x1)
	dx = 1;
      else
	dx = -1;


      while (x1 != x2)
	{
	  y1 = m * x1 + b;
	  y2 = m * (x1 + dx) + b;

	  if (y1 > y2)
	    {
	      y = y1;
	      y1 = y2;
	      y2 = y;
	    }
	
	  for (y = y1; y <= y2; y++)
	    {
	      num_drawn++;
	      if (num_drawn < 10 || dont_do_xor == 0)
		{
		  clipped_putpixel(screen, x1 + 96, y,
				   0xFFFFFFFF - getpixel(screen, x1 + 96, y));
		}
	    }
	
	  x1 = x1 + dx;
	}
    }
  else
    {
      if (y1 > y2)
	{
          for (y = y1; y >= y2; y--)
	    {
	      num_drawn++;
	  
	      if (num_drawn < 10 || dont_do_xor == 0)
		{
		  clipped_putpixel(screen, x1 + 96, y,
				   0xFFFFFFFF - getpixel(screen, x1 + 96, y));
		}
	    }
	}
      else
        {
          for (y = y1; y <= y2; y++)
	    {
	      num_drawn++;
	  
	      if (num_drawn < 10 || dont_do_xor == 0)
		{
		  clipped_putpixel(screen, x1 + 96, y,
				   0xFFFFFFFF - getpixel(screen, x1 + 96, y));
		}
	    }
        }
    }

  //SDL_UnlockSurface(screen);
}


/* Draw a XOR rectangle: */

static void rect_xor(int x1, int y1, int x2, int y2)
{
  if (x1 < 0)
    x1 = 0;

  if (x2 < 0)
    x2 = 0;

  if (y1 < 0)
    y1 = 0;

  if (y2 < 0)
    y2 = 0;

  if (x1 >= (WINDOW_WIDTH - 96 - 96))
    x1 = (WINDOW_WIDTH - 96 - 96) - 1;

  if (x2 >= (WINDOW_WIDTH - 96 - 96))
    x2 = (WINDOW_WIDTH - 96 - 96) - 1;

  if (y1 >= (48 * 7) + 40 + HEIGHTOFFSET)
    y1 = (48 * 7) + 40 + HEIGHTOFFSET - 1;

  if (y2 >= (48 * 7) + 40 + HEIGHTOFFSET)
    y2 = (48 * 7) + 40 + HEIGHTOFFSET - 1;

  line_xor(x1, y1, x2, y1);
  line_xor(x2, y1, x2, y2);
  line_xor(x2, y2, x1, y2);
  line_xor(x1, y2, x1, y1);
}


/* Erase at the cursor! */

static void do_eraser(int x, int y)
{
  SDL_Rect dest;
  int sz;

  sz = (ERASER_MIN +
	((NUM_ERASERS - 1 - cur_eraser) *
	 ((ERASER_MAX - ERASER_MIN) / (NUM_ERASERS - 1))));

  dest.x = x - (sz / 2);
  dest.y = y - (sz / 2);
  dest.w = sz;
  dest.h = sz;

  if (img_starter_bkgd == NULL)
  {
    SDL_FillRect(canvas, &dest,
	         SDL_MapRGB(canvas->format, 255, 255, 255));
  }
  else
  {
    SDL_BlitSurface(img_starter_bkgd, &dest, canvas, &dest);
  }


#ifndef NOSOUND
  if (use_sound)
    {
      if (!Mix_Playing(0))
	{
	  eraser_sound = (eraser_sound + 1) % 2;
	
	  playsound(0, SND_ERASER1 + eraser_sound, 0);
	}
    }
#endif

  update_canvas(x - sz / 2, y - sz / 2, x + sz / 2, y + sz / 2);

  rect_xor(x - sz / 2, y - sz / 2,
           x + sz / 2, y + sz / 2);
}


/* Reset available tools (for new image / starting out): */

static void reset_avail_tools(void)
{
  int i;
  int disallow_print = disable_print;  /* set to 1 later if printer unavail */

  for (i = 0; i < NUM_TOOLS; i++)
    {
      tool_avail[i] = 1;
    }


  /* Unavailable at the beginning of a new canvas: */

  tool_avail[TOOL_UNDO] = 0;
  tool_avail[TOOL_REDO] = 0;
  tool_avail[TOOL_NEW] = 0;

  if (been_saved)
    tool_avail[TOOL_SAVE] = 0;


  /* Unavailable in rare circumstances: */

  if (num_stamps == 0)
    tool_avail[TOOL_STAMP] = 0;


  /* Disable quit? */

  if (disable_quit)
    tool_avail[TOOL_QUIT] = 0;


  /* Disable save? */

  if (disable_save)
    tool_avail[TOOL_SAVE] = 0;


#ifdef WIN32
  if(!IsPrinterAvailable()) disallow_print = 1;
#endif

#ifdef __BEOS__
  if(!IsPrinterAvailable()) disallow_print = disable_print = 1;
#endif


  /* Disable print? */

  if (disallow_print)
    tool_avail[TOOL_PRINT] = 0;
}


/* Save and disable available tools (for Open-Dialog) */

static void disable_avail_tools(void)
{
  int i;

  for (i = 0; i < NUM_TOOLS; i++)
    {
      tool_avail_bak[i] = tool_avail[i];
      tool_avail[i]=0;
    }
}

/* Restore and enable available tools (for End-Of-Open-Dialog) */

static void enable_avail_tools(void)
{
  int i;

  for (i = 0; i < NUM_TOOLS; i++)
    {
      tool_avail[i] = tool_avail_bak[i];
    }
}


/* Update a rect. based on two x/y coords (not necessarly in order): */

static void update_screen(int x1, int y1, int x2, int y2)
{
  int tmp;

  if (x1 > x2)
    {
      tmp = x1;
      x1 = x2;
      x2 = tmp;
    }

  if (y1 > y2)
    {
      tmp = y1;
      y1 = y2;
      y2 = tmp;
    }

  x1 = x1 - 1;
  x2 = x2 + 1;
  y1 = y1 - 1;
  y2 = y2 + 1;


  if (x1 < 0)
    x1 = 0;
  if (x2 < 0)
    x2 = 0;
  if (y1 < 0)
    y1 = 0;
  if (y2 < 0)
    y2 = 0;

  if (x1 >= WINDOW_WIDTH)
    x1 = WINDOW_WIDTH - 1;
  if (x2 >= WINDOW_WIDTH)
    x2 = WINDOW_WIDTH - 1;
  if (y1 >= WINDOW_HEIGHT)
    y1 = WINDOW_HEIGHT - 1;
  if (y2 >= WINDOW_HEIGHT)
    y2 = WINDOW_HEIGHT - 1;

  SDL_UpdateRect(screen, x1, y1, x2 - x1 + 1, y2 - y1 + 1);
}


/* For qsort() call in loadarbitrary()... */

static int compare_strings(char * * s1, char * * s2)
{
  return (strcmp(*s1, *s2));
}


/* For qsort() call in do_open()... */

static int compare_dirent2s(struct dirent2 * f1, struct dirent2 * f2)
{
#ifdef DEBUG
  printf("compare_dirents: %s\t%s\n", f1->f.d_name, f2->f.d_name);
#endif

  if (f1->place == f2->place)
    return (strcmp(f1->f.d_name, f2->f.d_name));
  else
    return (f1->place - f2->place);
}


/* Draw tux's text on the screen: */

static void draw_tux_text(int which_tux, const char * const str,
	 	          int want_right_to_left)
{
  SDL_Rect dest;
  SDL_Color black = {0, 0, 0, 0};


  /* Remove any text-changing timer if one is running: */

  control_drawtext_timer(0, "");


  /* Clear first: */

  dest.x = 0;
  dest.y = (48 * 7) + 40 + 48 + HEIGHTOFFSET;
  dest.w = WINDOW_WIDTH;
  dest.h = WINDOW_HEIGHT - ((48 * 7) + 40 + 48 + HEIGHTOFFSET);

  SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format, 255, 255, 255));


  /* Draw tux: */

  dest.x = 0;
  dest.y = WINDOW_HEIGHT - (img_tux[which_tux] -> h);

  if (dest.y < ((48 * 7) + 40 + 48 + HEIGHTOFFSET))
    dest.y = ((48 * 7) + 40 + 48 + HEIGHTOFFSET);

  SDL_BlitSurface(img_tux[which_tux], NULL, screen, &dest);


  wordwrap_text(str, black,
	        img_tux[which_tux] -> w + 5,
	        (48 * 7) + 40 + 48 + HEIGHTOFFSET,
	        WINDOW_WIDTH,
		want_right_to_left);


  /* Update the display: */

  SDL_UpdateRect(screen,
		 0, (48 * 7) + 40 + 48 + HEIGHTOFFSET,
		 WINDOW_WIDTH,
		 WINDOW_HEIGHT - ((48 * 7) + 40 + 48 + HEIGHTOFFSET));
}


static void wordwrap_text(const char * const str, SDL_Color color,
		   int left, int top, int right,
		   int want_right_to_left)
{
  int x, y, j;
  unsigned int i;
  char substr[512];
  unsigned char * locale_str;
  char * tstr;
  unsigned char utf8_char[5];
  int len;
  SDL_Surface * text;
  SDL_Rect dest, src;

  int utf8_str_len, last_text_height;
  unsigned char utf8_str[512];


  /* Cursor starting position: */
  
  x = left;
  y = top;
      
  last_text_height = 0;

  debug(str);
  debug(gettext(str));
  debug("...");

  if (strcmp(str, "") != 0)
    {
      if (want_right_to_left == 0)
	locale_str = strdup(gettext(str));
      else
	locale_str = strdup(textdir(gettext(str)));


      /* For each UTF8 character: */

      utf8_str_len = 0;
      utf8_str[0] = '\0';

      for (i = 0; i <= strlen(locale_str); i++)
	{
	  if (locale_str[i] < 128)
	    {
	      utf8_str[utf8_str_len++] = locale_str[i];
	      utf8_str[utf8_str_len] = '\0';


	      /* Space?  Blit the word! (Word-wrap if necessary) */

	      if (locale_str[i] == ' ' || locale_str[i] == '\0')
		{
		  text = TTF_RenderUTF8_Blended(locale_font, utf8_str, color);

		  if (!text) continue;  /* Didn't render anything... */

		  /* ----------- */
		  if (text->w > right - left)
		    {
		      /* Move left and down (if not already at left!) */

		      if (x > left)
			{
			  if (need_right_to_left(language) && want_right_to_left)
			    anti_carriage_return(left, right, top, top + text->h, y + text->h,
						 x - left);

			  x = left;
			  y = y + text->h;
			}


		      /* Junk the blitted word; it's too long! */

		      last_text_height = text->h;
		      SDL_FreeSurface(text);

	
		      /* For each UTF8 character: */

		      for (j = 0; j < utf8_str_len; j++)
			{
			  /* How many bytes does this character need? */

			  if (utf8_str[j] < 128)  /* 0xxx xxxx - 1 byte */
			    {
			      utf8_char[0] = utf8_str[j];
			      utf8_char[1] = '\0';
			    }
			  else if ((utf8_str[j] & 0xE0) == 0xC0)  /* 110x xxxx - 2 bytes */
			    {
			      utf8_char[0] = utf8_str[j];
			      utf8_char[1] = utf8_str[j + 1];
			      utf8_char[2] = '\0';
			      j = j + 1;
			    }
			  else if ((utf8_str[j] & 0xF0) == 0xE0)  /* 1110 xxxx - 3 bytes */
			    {
			      utf8_char[0] = utf8_str[j];
			      utf8_char[1] = utf8_str[j + 1];
			      utf8_char[2] = utf8_str[j + 2];
			      utf8_char[3] = '\0';
			      j = j + 2;
			    }
			  else  /* 1111 0xxx - 4 bytes */
			    {
			      utf8_char[0] = utf8_str[j];
			      utf8_char[1] = utf8_str[j + 1];
			      utf8_char[2] = utf8_str[j + 2];
			      utf8_char[3] = utf8_str[j + 3];
			      utf8_char[4] = '\0';
			      j = j + 3;
			    }
    

			  if (utf8_char[0] != '\0')
			    {
			      text = TTF_RenderUTF8_Blended(locale_font, utf8_char, color);
			      if (text != NULL)
				{
				  if (x + text->w > right)
				    {
				      if (need_right_to_left(language) && want_right_to_left)
					anti_carriage_return(left, right, top, top + text->h,
							     y + text->h, x - left);
	        
				      x = left;
				      y = y + text->h;
				    }

				  dest.x = x;

				  if (need_right_to_left(language) && want_right_to_left)
				    dest.y = top;
				  else
				    dest.y = y;

				  SDL_BlitSurface(text, NULL, screen, &dest);
      
				  last_text_height = text->h;

				  x = x + text->w;

				  SDL_FreeSurface(text);
				}
			    }
			}
		    }
		  else
		    {
		      /* Not too wide for one line... */
	      
		      if (x + text->w > right)
			{
			  /* This word needs to move down? */

			  if (need_right_to_left(language) && want_right_to_left)
			    anti_carriage_return(left, right, top, top + text->h, y + text->h,
						 x - left);
          
			  x = left;
			  y = y + text->h;
			}

		      dest.x = x;

		      if (need_right_to_left(language) && want_right_to_left)
			dest.y = top;
		      else
			dest.y = y;

		      SDL_BlitSurface(text, NULL, screen, &dest);

		      last_text_height = text->h;
		      x = x + text->w;

		      SDL_FreeSurface(text);
		    }
        
      
		  utf8_str_len = 0;
		  utf8_str[0] = '\0';
		}
	    }
	  else if ((locale_str[i] & 0xE0) == 0xC0)
	    {
	      utf8_str[utf8_str_len++] = locale_str[i];
	      utf8_str[utf8_str_len++] = locale_str[i + 1];
	      utf8_str[utf8_str_len] = '\0';
	      i++;
	    }
	  else if ((locale_str[i] & 0xF0) == 0xE0)
	    {
	      utf8_str[utf8_str_len++] = locale_str[i];
	      utf8_str[utf8_str_len++] = locale_str[i + 1];
	      utf8_str[utf8_str_len++] = locale_str[i + 2];
	      utf8_str[utf8_str_len] = '\0';
	      i = i + 2;
	    }
	  else
	    {
	      utf8_str[utf8_str_len++] = locale_str[i];
	      utf8_str[utf8_str_len++] = locale_str[i + 1];
	      utf8_str[utf8_str_len++] = locale_str[i + 2];
	      utf8_str[utf8_str_len++] = locale_str[i + 3];
	      utf8_str[utf8_str_len] = '\0';
	      i = i + 3;
	    }
	}
    
      free(locale_str);
    }
  else if (strlen(str) != 0)
    {
      /* Truncate if too big! (sorry!) */

      if (want_right_to_left == 0)
	tstr = strdup(uppercase(gettext(str)));
      else
	tstr = strdup(uppercase(textdir(gettext(str))));

      if (strlen(tstr) > sizeof(substr) - 1)
	tstr[sizeof(substr) - 1] = '\0';


      /* For each word... */

      for (i = 0; i < strlen(tstr); i++)
	{
	  /* Figure out the word... */

	  len = 0;

	  for (j = i; tstr[j] != ' ' && tstr[j] != '\0'; j++)
	    {
	      substr[len++] = tstr[j];
	    }

	  substr[len++] = ' ';
	  substr[len] = '\0';


	  /* Render the word for display... */

  
	  text = TTF_RenderUTF8_Blended(locale_font, substr, color);


	  /* If it won't fit on this line, move to the next! */

	  if (x + text->w > right)  /* Correct? */
	    {
	      if (need_right_to_left(language) && want_right_to_left)
		anti_carriage_return(left, right, top, top + text->h, y + text->h,
				     x - left);
        
	      x = left;
	      y = y + text->h;
	    }


	  /* Draw the word: */

	  dest.x = x;

	  if (need_right_to_left(language) && want_right_to_left)
	    dest.y = top;
	  else
	    dest.y = y;

	  SDL_BlitSurface(text, NULL, screen, &dest);


	  /* Move the cursor one word's worth: */

	  x = x + text->w;


	  /* Free the temp. surface: */

	  last_text_height = text->h;
	  SDL_FreeSurface(text);


	  /* Now on to the next word... */

	  i = j;
	}

      free(tstr);
    }


  /* Right-justify the final line of text, in right-to-left mode: */
  
  if (need_right_to_left(language) && want_right_to_left && last_text_height > 0)
    {
      src.x = left;
      src.y = top;
      src.w = x - left;
      src.h = last_text_height;

      dest.x = right - src.w;
      dest.y = top;

      SDL_BlitSurface(screen, &src, screen, &dest);

      dest.x = left;
      dest.y = top;
      dest.w = (right - left - src.w);
      dest.h = last_text_height;

      SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format, 255, 255, 255));
    }
}


/* Load a file's sound: */

#ifndef NOSOUND

static Mix_Chunk * loadsound(const char * const fname)
{
  char * snd_fname;
  char tmp_str[64];
  Mix_Chunk * tmp_snd;


  debug(fname);


  /* First, check for localized version of sound: */

  snd_fname = malloc(strlen(fname) + strlen(lang_prefixes[language]) + 2);

  strcpy(snd_fname, fname);
  snprintf(tmp_str, sizeof(tmp_str), "_%s.wav", lang_prefixes[language]);


  if (strstr(snd_fname, ".png") != NULL)
    {
      strcpy(strstr(snd_fname, ".png"), tmp_str);
      debug(snd_fname);

      tmp_snd = Mix_LoadWAV(snd_fname);

      if (tmp_snd == NULL)
	{
	  debug("...No local version of sound!");

	  /* Now, check for default sound: */

	  free(snd_fname);

	  snd_fname = strdup(fname);

	  if (strstr(snd_fname, ".png") != NULL)
	    {
	      strcpy(strstr(snd_fname, ".png"), ".wav");
	      debug(snd_fname);
	      tmp_snd = Mix_LoadWAV(snd_fname);
	      free(snd_fname);

	      if (tmp_snd == NULL)
		{
		  debug("...No default version of sound!");
		  return NULL;
		}

	      return (tmp_snd);
	    }
	  else
	    {
	      return NULL;
	    }
	}

      return (tmp_snd);
    }
  else
    {
      return NULL;
    }
}

#endif


/* Strip any trailing spaces: */

static void strip_trailing_whitespace( char *buf )
{
  unsigned i = strlen(buf);
  while(i--)
    {
      if(!isspace(buf[i])) break;
      buf[i] = '\0';
    }
}


/* Load a file's description: */

static char * loaddesc(const char * const fname)
{
  char * txt_fname;
  char buf[256], def_buf[256];
  int found, got_first;
  FILE * fi;


  txt_fname = strdup(fname);

  if (strstr(txt_fname, ".png") != NULL)
    {
      strcpy(strstr(txt_fname, ".png"), ".txt");

      fi = fopen(txt_fname, "r");

      if (fi == NULL)
	{
	  /*
	    fprintf(stderr, "\nWarning: Couldn't open a description file:\n");
	    perror(txt_fname);
	    fprintf(stderr, "\n");
	  */

	  free(txt_fname);

	  return NULL;
	}

      free(txt_fname);

      got_first = 0;
      found = 0;

      strcpy(def_buf, "");

      do
	{
	  fgets(buf, sizeof(buf), fi);

	  if (!feof(fi))
	    {
	      strip_trailing_whitespace(buf);


	      if (!got_first)
		{
		  /* First one is the default: */

		  strcpy(def_buf, buf);
		  got_first = 1;
		}


	      debug(buf);


	      /* See if it's the one for this locale... */
	
	      if (strstr(buf, lang_prefixes[language]) == buf)
		{

		  debug(buf + strlen(lang_prefixes[language]));
		  if (strstr(buf + strlen(lang_prefixes[language]), ".utf8=") ==
			   buf + strlen(lang_prefixes[language]))
		    {
		      found = 1;
		      
		      debug("...FOUND!");
		    }
		}
	    }
	}
      while (!feof(fi) && !found);

      fclose(fi);


      /* Return the string: */

      if (found)
	{
	  return(strdup(buf + (strlen(lang_prefixes[language])) + 6));
	}
      else
	{
	  /* No locale-specific translation; use the default (English) */

	  return(strdup(def_buf));
	}
    }
  else
    {
      return NULL;
    }
}


/* Load a file's info: */

static info_type * loadinfo(const char * const fname)
{
  char * dat_fname;
  char buf[256];
  info_type inf;
  info_type * inf_ret;
  FILE * fi;


  /* Clear info struct first: */

  inf.ratio = 1.0;
  inf.colorable = 0;
  inf.tintable = 0;
  inf.mirrorable = 1;
  inf.flipable = 1;
  inf.tinter = TINTER_NORMAL;

  /* Load info! */

  dat_fname = strdup(fname);

  if (strstr(dat_fname, ".png") != NULL)
    {
      strcpy(strstr(dat_fname, ".png"), ".dat");

      fi = fopen(dat_fname, "r");

      if (fi == NULL)
	{
	  /*
	    fprintf(stderr, "\nWarning: Couldn't open an info file:\n");
	    perror(txt_fname);
	    fprintf(stderr, "\n");
	  */

	  free(dat_fname);

	  return NULL;
	}

      free(dat_fname);


      do
	{
	  fgets(buf, sizeof(buf), fi);

	  if (!feof(fi))
	    {
	      strip_trailing_whitespace(buf);

	      if (strcmp(buf, "colorable") == 0)
		inf.colorable = 1;
	      else if (strcmp(buf, "tintable") == 0)
		inf.tintable = 1;
	      else if (!memcmp(buf, "scale", 5) && (isspace(buf[5]) || buf[5]=='='))
		{
		  double tmp, tmp2;
		  char *cp = buf+6;
		  while (isspace(*cp) || *cp=='=')
		    cp++;
		  if (strchr(cp,'%'))
		    {
		      tmp = strtod(cp,NULL) / 100.0;
		      if (tmp > 0.0001 && tmp < 10000.0)
		        inf.ratio = tmp;
		    }
		  else if (strchr(cp,'/'))
		    {
		      tmp = strtod(cp,&cp);
		      while(*cp && !isdigit(*cp))
		        cp++;
		      tmp2 = strtod(cp,NULL);
		      if (tmp>0.0001 && tmp<10000.0 && tmp2>0.0001 && tmp2<10000.0 && tmp/tmp2>0.0001 && tmp/tmp2<10000.0)
		        inf.ratio = tmp/tmp2;
		    }
		  else if (strchr(cp,':'))
		    {
		      tmp = strtod(cp,&cp);
		      while(*cp && !isdigit(*cp))
		        cp++;
		      tmp2 = strtod(cp,NULL);
		      if (tmp>0.0001 && tmp<10000.0 && tmp2>0.0001 && tmp2<10000.0 && tmp2/tmp>0.0001 && tmp2/tmp<10000.0)
		        inf.ratio = tmp2/tmp;
		    }
		  else
		    {
		      tmp = strtod(cp,NULL);
		      if (tmp > 0.0001 && tmp < 10000.0)
		        inf.ratio = 1.0 / tmp;
		    }
		}
	      else if (!memcmp(buf, "tinter", 6) && (isspace(buf[6]) || buf[6]=='='))
		{
		  char *cp = buf+7;
		  while (isspace(*cp) || *cp=='=')
		    cp++;
		  if (!strcmp(cp,"anyhue"))
		    {
		        inf.tinter = TINTER_ANYHUE;
		    }
		  else if (!strcmp(cp,"narrow"))
		    {
		        inf.tinter = TINTER_NARROW;
		    }
		  else if (!strcmp(cp,"normal"))
		    {
		        inf.tinter = TINTER_NORMAL;
		    }
		  else if (!strcmp(cp,"vector"))
		    {
		        inf.tinter = TINTER_VECTOR;
		    }
		  else
		    {
		        debug(cp);
		    }
		}
	      else if (strcmp(buf, "nomirror") == 0)
		inf.mirrorable = 0;
	      else if (strcmp(buf, "noflip") == 0)
		inf.flipable = 0;
	    }
	}
      while (!feof(fi));

      fclose(fi);


      /* Return the info: */
   
      inf_ret = malloc(sizeof(info_type));
      /* FIXME: Check for errors! */

      memcpy(inf_ret, &inf, sizeof(info_type));

      return(inf_ret);
    }
  else
    {
      return NULL;
    }
}


/* Load a file's alternative image: */

static SDL_Surface * loadaltimage(const char * const fname)
{
  char * alt_fname;
  SDL_Surface * s;
  

  s = NULL;


  alt_fname = (char *) malloc(sizeof(char) *
		              (strlen(fname) + strlen("_mirror") + 1));
  if (alt_fname != NULL)
    {
      strcpy(alt_fname, fname);

      if (strstr(alt_fname, ".png") != NULL)
	{
	  strcpy(strstr(alt_fname, ".png"), "_mirror.png");

	  s = do_loadimage(alt_fname, 0);
	}


      free(alt_fname);
    }

  return s;
}


/* Wait for a keypress or mouse click */

static void do_wait(void)
{
  SDL_Event event;
  int done, counter;

  done = 0;

  counter = 50;  /* About 5 seconds */

  do
    {
      while (mySDL_PollEvent(&event))
	{
	  if (event.type == SDL_QUIT)
	    {
	      done = 1;

	      /* FIXME: Handle SDL_Quit better */
	    }
	  else if (event.type == SDL_ACTIVEEVENT)
	    {
	      handle_active(&event);
	    }
	  else if (event.type == SDL_KEYDOWN)
	    {
	      done = 1;
	    }
	  else if (event.type == SDL_MOUSEBUTTONDOWN &&
		   event.button.button >= 1 &&
		   event.button.button <= 3)
	    {
	      done = 1;
	    }
	}

      counter--;
      SDL_Delay(100);
    }
  while (!done && counter > 0);
}


static int SDLCALL NondefectiveBlit(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect)
{
  int dstx = 0;
  int dsty = 0;
  int srcx = 0;
  int srcy = 0;
  int srcw = src->w;
  int srch = src->h;
  if(srcrect)
    {
      srcx = srcrect->x;
      srcy = srcrect->y;
      srcw = srcrect->w;
      srch = srcrect->h;
    }
  if(dstrect)
    {
      dstx = dstrect->x;
      dsty = dstrect->y;
    }
  if(dsty<0)
    {
      srcy += -dsty;
      srch -= -dsty;
      dsty = 0;
    }
  if(dstx<0)
    {
      srcx += -dstx;
      srcw -= -dstx;
      dstx = 0;
    }
  if(dstx+srcw > dst->w-1)
    {
      srcw -= (dstx+srcw) - (dst->w-1);
    }
  if(dsty+srch > dst->h-1)
    {
      srch -= (dsty+srch) - (dst->h-1);
    }
  if(srcw<1 || srch<1)
    return -1; /* no idea what to return if nothing done */
  while(srch--)
    {
      int i = srcw;
      while(i--)
        {
          putpixel(dst, i+dstx, srch+dsty, getpixel(src, i+srcx, srch+srcy));
        }
    }

  return(0);
}


// For the 3rd arg, pass either NondefectiveBlit or SDL_BlitSurface.
static void autoscale_copy_smear_free(SDL_Surface *src, SDL_Surface *dst, int SDLCALL (*blit)(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect))
{
  SDL_Surface *src1;
  SDL_Rect dest;
  // What to do when in 640x480 mode, and loading an
  // 800x600 (or larger) image!? Scale it. Starters must
  // be scaled too. Keep the pixels square though, filling
  // in the gaps via a smear.
  if(src->w != dst->w || src->h != dst->h)
    {
      if(src->w / (float)dst->w  >  src->h / (float)dst->h)
        src1 = thumbnail(src, dst->w, src->h*dst->w/src->w, 0);
      else
        src1 = thumbnail(src, src->w*dst->h/src->h, dst->h, 0);
      SDL_FreeSurface(src);
      src = src1;
    }

  dest.x = (dst->w - src->w) / 2;
  dest.y = (dst->h - src->h) / 2;
  blit(src, NULL, dst, &dest);

  if(src->w != dst->w)
    {
      // we know that the heights match, and src is narrower
      SDL_Rect sour;
      int i = (dst->w - src->w) / 2;
      sour.w = 1;
      sour.x = 0;
      sour.h = src->h;
      sour.y = 0;
      while(i-- > 0)
        {
          dest.x = i;
          blit(src, &sour, dst, &dest);
        }
      sour.x = src->w - 1;
      i = (dst->w - src->w) / 2 + src->w - 1;
      while(++i < dst->w)
        {
          dest.x = i;
          blit(src, &sour, dst, &dest);
        }
    }

  if(src->h != dst->h)
    {
      // we know that the widths match, and src is shorter
      SDL_Rect sour;
      int i = (dst->h - src->h) / 2;
      sour.w = src->w;
      sour.x = 0;
      sour.h = 1;
      sour.y = 0;
      while(i-- > 0)
        {
          dest.y = i;
          blit(src, &sour, dst, &dest);
        }
      sour.y = src->h - 1;
      i = (dst->h - src->h) / 2 + src->h - 1;
      while(++i < dst->h)
        {
          dest.y = i;
          blit(src, &sour, dst, &dest);
        }
    }

  SDL_FreeSurface(src);
}


static void load_starter_id(char * saved_id)
{
  char * rname;
  char fname[32];
  FILE * fi;
  
  snprintf(fname, sizeof(fname), "saved/%s.dat", saved_id);
  rname = get_fname(fname);

  starter_id[0] = '\0';

  fi = fopen(rname, "r");
  if (fi != NULL)
  {
    fgets(starter_id, sizeof(starter_id), fi);
    starter_id[strlen(starter_id) - 1] = '\0';

    fscanf(fi, "%d", &starter_mirrored);
    fscanf(fi, "%d", &starter_flipped);

    fclose(fi);
  }

  free(rname);
}



static void load_starter(char * img_id)
{
  char * dirname;
  char fname[256];
  SDL_Surface * tmp_surf;

  /* Determine path to starter files: */
  
  dirname = strdup(DATA_PREFIX "starters");

  /* Clear them to NULL first: */
  img_starter = NULL;
  img_starter_bkgd = NULL;

  /* Load the core image: */
  snprintf(fname, sizeof(fname), "%s/%s.png", dirname, img_id);
  tmp_surf = IMG_Load(fname);

  if (tmp_surf != NULL)
  {
    img_starter = SDL_DisplayFormatAlpha(tmp_surf);
    SDL_FreeSurface(tmp_surf);
  }

  if (img_starter != NULL &&
      (img_starter->w != canvas->w || img_starter->h != canvas->h))
  {
    tmp_surf = img_starter;

    img_starter = SDL_CreateRGBSurface(canvas->flags,
					    canvas->w, canvas->h,
					    tmp_surf->format->BitsPerPixel,
			     	            tmp_surf->format->Rmask,
			      	            tmp_surf->format->Gmask,
			      	            tmp_surf->format->Bmask,
					    tmp_surf->format->Amask);

    // 3rd arg ignored for RGBA surfaces
    SDL_SetAlpha(tmp_surf, SDL_RLEACCEL, SDL_ALPHA_OPAQUE);
    autoscale_copy_smear_free(tmp_surf,img_starter,NondefectiveBlit);
    SDL_SetAlpha(img_starter, SDL_RLEACCEL|SDL_SRCALPHA, SDL_ALPHA_OPAQUE);
  }

  /* Try to load the a background image: */

  /* (JPEG first) */
  snprintf(fname, sizeof(fname), "%s/%s-back.jpeg", dirname, img_id);
  tmp_surf = IMG_Load(fname);
 
  /* (Failed? Try PNG next) */
  if (tmp_surf == NULL)
  {
    snprintf(fname, sizeof(fname), "%s/%s-back.png", dirname, img_id);
    tmp_surf = IMG_Load(fname);
  }
  
  if (tmp_surf != NULL)
  {
    img_starter_bkgd = SDL_DisplayFormat(tmp_surf);
    SDL_FreeSurface(tmp_surf);
  }

  if (img_starter_bkgd != NULL &&
      (img_starter_bkgd->w != canvas->w || img_starter_bkgd->h != canvas->h))
  {
    tmp_surf = img_starter_bkgd;

    img_starter_bkgd = SDL_CreateRGBSurface(SDL_SWSURFACE,
					    canvas->w, canvas->h,
					    canvas->format->BitsPerPixel,
			     	            canvas->format->Rmask,
			      	            canvas->format->Gmask,
			      	            canvas->format->Bmask,
					    0);

    autoscale_copy_smear_free(tmp_surf,img_starter_bkgd,SDL_BlitSurface);
  }

  free(dirname);
}


/* Load current (if any) image: */

static void load_current(void)
{
  SDL_Surface * tmp;
  char * fname;
  char ftmp[1024];
  FILE * fi;

  /* Determine the current picture's ID: */

  fname = get_fname("current_id.txt");

  fi = fopen(fname, "r");
  if (fi == NULL)
    {
      fprintf(stderr,
	      "\nWarning: Couldn't determine the current image's ID\n"
	      "%s\n"
	      "The system error that occurred was:\n"
	      "%s\n\n", fname, strerror(errno));
      file_id[0] = '\0';
      starter_id[0] = '\0';
    }
  else
    {
      fgets(file_id, sizeof(file_id), fi);
      if (strlen(file_id) > 0)
	{
	  file_id[strlen(file_id) - 1] = '\0';
	}
      fclose(fi);
    }

  free(fname);


  /* Load that image: */

  if (file_id[0] != '\0')
    {
      snprintf(ftmp, sizeof(ftmp), "saved/%s%s", file_id, FNAME_EXTENSION);

      fname = get_fname(ftmp);

#ifdef SAVE_AS_BMP
      tmp = SDL_LoadBMP(fname);
#else
      tmp = IMG_Load(fname);
#endif

      if (tmp == NULL)
	{
	  fprintf(stderr,
		  "\nWarning: Couldn't load any current image.\n"
		  "%s\n"
		  "The Simple DirectMedia Layer error that occurred was:\n"
		  "%s\n\n", fname, SDL_GetError());

	  file_id[0] = '\0';
	  starter_id[0] = '\0';
	}
      else
	{
	  autoscale_copy_smear_free(tmp,canvas,SDL_BlitSurface);
	  load_starter_id(file_id);
	  load_starter(starter_id);

          if (starter_mirrored)
            mirror_starter();

          if (starter_flipped)
            flip_starter();

	  tool_avail[TOOL_NEW] = 1;
	}

      free(fname);
    }
}


/* Save the current image to disk: */

static void save_current(void)
{
  char * fname;
  int res;
  FILE * fi;


  fname = get_fname("");

  res = mkdir(fname, 0755);

  if (res != 0 && errno != EEXIST)
    {
      fprintf(stderr,
	      "\nError: Can't create user data directory:\n"
	      "%s\n"
	      "The error that occurred was:\n"
	      "%s\n\n", fname, strerror(errno));

      draw_tux_text(TUX_OOPS, strerror(errno), 0);
    }

  free(fname);


  fname = get_fname("current_id.txt");

  fi = fopen(fname, "w");
  if (fi == NULL)
    {
      fprintf(stderr,
	      "\nError: Can't keep track of current image.\n"
	      "%s\n"
	      "The error that occurred was:\n"
	      "%s\n\n", fname, strerror(errno));

      draw_tux_text(TUX_OOPS, strerror(errno), 0);
    }
  else
    {
      fprintf(fi, "%s\n", file_id);
      fclose(fi);
    }

  free(fname);
}


/* The filename for the current image: */

static char * get_fname(const char * const name)
{
  char f[512];
  const char * tux_settings_dir;


  /* Where is the user's data directory?
     This is where their saved files are stored,
     local fonts, brushes and stamps can be found,
     and where the "current_id.txt" file is saved */

#ifdef WIN32
  /* Windows predefines "savedir" as "userdata", though it may get
     overridden with "--savedir" option */

  snprintf(f, sizeof(f), "%s/%s", savedir, name);

#elif __BEOS__
  /* BeOS similarly predefines "savedir" as "./userdata"... */
 
  if (*name == '\0')
    strcpy(f, savedir);
  else
    snprintf(f, sizeof(f), "%s/%s", savedir, name);
#else
  /* On Mac, Linux and other Unixes, it's in a place under our home dir.: */
  
  
#ifdef __APPLE__
  /* Macintosh: It's under ~/Library/Application Support/TuxPaint */
  
  tux_settings_dir = "Library/Application Support/TuxPaint";
#else
  /* Linux & Unix: It's under ~/.tuxpaint */
  
  tux_settings_dir = ".tuxpaint";
#endif
 

  /* Put together home directory path + settings directory + filename... */
  
  if (savedir == NULL)
    {
      /* Save directory not overridden: */
	 
      if (getenv("HOME") != NULL)
	{
	  if (*name == '\0')
	    {
	      /* (Some mkdir()'s don't like trailing slashes) */

	      snprintf(f, sizeof(f), "%s/%s", getenv("HOME"), tux_settings_dir);
	    }
	  else
	    {
	      snprintf(f, sizeof(f), "%s/%s/%s",
		       getenv("HOME"), tux_settings_dir, name);
	    }
	}
      else
	{
	  /* WOAH!  Don't know where HOME directory is!  Last resort, use '.'! */

	  strcpy(f, name);
	}
    }
  else
    {
      /* User had overridden save directory with "--savedir" option: */

      if (*name != '\0')
	{
	  /* (Some mkdir()'s don't like trailing slashes) */

	  snprintf(f, sizeof(f), "%s/%s", savedir, name);
	}
      else
	{
	  snprintf(f, sizeof(f), "%s", savedir);
	}
    }
#endif

  return strdup(f);
}


/* Prompt the user with a yes/no question: */

static int do_prompt(const char * const text, const char * const btn_yes, const char * const btn_no)
{
  SDL_Event event;
  SDL_Rect dest;
  int done, ans, w;
  SDL_Color black = {0, 0, 0, 0};
  SDLKey key;
  SDLKey key_y, key_n;
  char keystr[200];
#ifndef NO_PROMPT_SHADOWS
  int i;
  SDL_Surface * alpha_surf;
#endif


  /* FIXME: Move elsewhere! Or not?! */

  strcpy(keystr, textdir(gettext("Yes")));
  key_y = tolower(keystr[0]);

  strcpy(keystr, textdir(gettext("No")));
  key_n = tolower(keystr[0]);


  do_setcursor(cursor_arrow);


  /* Move cursor automatically if in keymouse mode: */

  if (keymouse)
    {
      mouse_x = WINDOW_WIDTH / 2;
      mouse_y = WINDOW_HEIGHT / 2;
      SDL_WarpMouse(mouse_x, mouse_y);
    }


  /* Draw button box: */

  playsound(0, SND_PROMPT, 1);

  for (w = 0; w <= 96; w = w + 4)
    {
      dest.x = 160 + 96 - w + PROMPTOFFSETX;
      dest.y = 94 + 96 - w + PROMPTOFFSETY;
      dest.w = (320 - 96 * 2) + w * 2;
      dest.h = w * 2;
      SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format, 0, 0, 0));

      SDL_UpdateRect(screen, dest.x, dest.y, dest.w, dest.h);
      SDL_Delay(10);
    }
 

#ifndef NO_PROMPT_SHADOWS
  alpha_surf = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA,
		                    (320 - 96 * 2) + (w - 4) * 2,
				    (w - 4) * 2,
				    screen->format->BitsPerPixel,
				    screen->format->Rmask,
				    screen->format->Gmask,
				    screen->format->Bmask,
				    screen->format->Amask);

  if (alpha_surf != NULL)
    {
      SDL_FillRect(alpha_surf, NULL, SDL_MapRGB(alpha_surf->format, 0, 0, 0));
      SDL_SetAlpha(alpha_surf, SDL_SRCALPHA, 64);

      for (i = 8; i > 0; i = i - 2)
	{
	  dest.x = 160 + 96 - (w - 4) + i + PROMPTOFFSETX;
	  dest.y = 94 + 96 - (w - 4) + i + PROMPTOFFSETY;
	  dest.w = (320 - 96 * 2) + (w - 4) * 2;
	  dest.h = (w - 4) * 2;
      
	  SDL_BlitSurface(alpha_surf, NULL, screen, &dest);
	}
  
      SDL_FreeSurface(alpha_surf);
    }
#endif
  

  w = w - 6;

  dest.x = 160 + 96 - w + PROMPTOFFSETX;
  dest.y = 94 + 96 - w + PROMPTOFFSETY;
  dest.w = (320 - 96 * 2) + w * 2;
  dest.h = w * 2;
  SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format, 255, 255, 255));


  /* Draw the question: */

  wordwrap_text(text, black,
		166 + PROMPTOFFSETX, 100 + PROMPTOFFSETY, 475 + PROMPTOFFSETX,
		1);


  /* Draw yes button: */

  dest.x = 166 + PROMPTOFFSETX;
  dest.y = 178 + PROMPTOFFSETY;
  SDL_BlitSurface(img_yes, NULL, screen, &dest);


  /* (Bound to UTF8 domain, so always ask for UTF8 rendering!) */

  wordwrap_text(btn_yes, black, 166 + PROMPTOFFSETX + 48 + 4,
		183 + PROMPTOFFSETY, 475 + PROMPTOFFSETX, 1);


  /* Draw no button: */

  if (strlen(btn_no) != 0)
    {
      dest.x = 166 + PROMPTOFFSETX;
      dest.y = 230 + PROMPTOFFSETY;
      SDL_BlitSurface(img_no, NULL, screen, &dest);

      wordwrap_text(btn_no, black,
		    166 + PROMPTOFFSETX + 48 + 4, 235 + PROMPTOFFSETY,
		    475 + PROMPTOFFSETX, 1);
    }


  /* Draw Tux, waiting... */

  draw_tux_text(TUX_BORED, "", 0);

  SDL_Flip(screen);

  done = 0;
  ans = 0;

  do
    {
      mySDL_WaitEvent(&event);

      if (event.type == SDL_QUIT)
	{
	  ans = 0;
	  done = 1;
	}
      else if (event.type == SDL_ACTIVEEVENT)
	{
	  handle_active(&event);
	}
      else if (event.type == SDL_KEYUP)
	{
	  key = event.key.keysym.sym;

	  handle_keymouse(key, SDL_KEYUP);
	}
      else if (event.type == SDL_KEYDOWN)
	{
	  key = event.key.keysym.sym;

	  handle_keymouse(key, SDL_KEYDOWN);


	  /* FIXME: Should use SDLK_{c} instead of '{c}'?  How? */

	  if (key == key_y || key == SDLK_RETURN)
	    {
	      /* Y or ENTER - Yes! */
	
	      ans = 1;
	      done = 1;
	    }
	  else if (key == key_n || key == SDLK_ESCAPE)
	    {
	      /* N or ESCAPE - No! */

	      if (strlen(btn_no) != 0)
		{
		  ans = 0;
		  done = 1;
		}
	      else
		{
		  if (key == SDLK_ESCAPE)
		    {
		      /* ESCAPE also simply dismisses if there's no Yes/No
		         choice: */
	
		      ans = 1;
		      done = 1;
		    }
		}
	    }
	}
      else if (event.type == SDL_MOUSEBUTTONDOWN &&
	       event.button.button >= 1 &&
	       event.button.button <= 3)
	{
	  if (event.button.x >= 166 + PROMPTOFFSETX &&
	      event.button.x < 166 + PROMPTOFFSETX + 48)
	    {
	      if (event.button.y >= 178 + PROMPTOFFSETY &&
		  event.button.y < 178 + PROMPTOFFSETY + 48)
		{
		  ans = 1;
		  done = 1;
		}
	      else if (strlen(btn_no) != 0 &&
		       event.button.y >= 230 + PROMPTOFFSETY &&
		       event.button.y < 230 + PROMPTOFFSETY + 48)
		{
		  ans = 0;
		  done = 1;
		}
	    }
	}
      else if (event.type == SDL_MOUSEMOTION)
	{ 
	  if (event.button.x >= 166 + PROMPTOFFSETX &&
	      event.button.x < 166 + 48 + PROMPTOFFSETX &&
	      ((event.button.y >= 178 + PROMPTOFFSETY &&
		event.button.y < 178 + 48 + PROMPTOFFSETY) ||
	       (strlen(btn_no) != 0 &&
		event.button.y >= 230 && event.button.y < 230 + 48)))
	    {
	      do_setcursor(cursor_hand);
	    }
	  else
	    {
	      do_setcursor(cursor_arrow);
	    }
	}
    }
  while (!done);


  /* FIXME: Sound effect! */
  /* ... */


  /* Erase question prompt: */

  update_canvas(0, 0, WINDOW_WIDTH - 96 - 96, 48 * 7 + 40 + HEIGHTOFFSET);

  return ans;
}


/* Free memory and prepare to quit: */

static void cleanup(void)
{
  int i;

  for (i = 0; i < MAX_STAMPS; i++)
    {
      if (txt_stamps[i])
	{
	  free(txt_stamps[i]);
	  txt_stamps[i] = NULL;
	}

      if (inf_stamps[i])
	{
	  free(inf_stamps[i]);
	  inf_stamps[i] = NULL;
	}

      if (state_stamps[i])
	{
	  free(state_stamps[i]);
	  state_stamps[i] = NULL;
	}
    }

  free_surface_array( img_brushes, MAX_BRUSHES );
  free_surface_array( img_stamps, MAX_STAMPS );
  free_surface_array( img_stamps_premirror, MAX_STAMPS );
  free_surface_array( img_tools, NUM_TOOLS );
  free_surface_array( img_tool_names, NUM_TOOLS );
  free_surface_array( img_title_names, NUM_TITLES );
  free_surface_array( img_magics, NUM_MAGICS );
  free_surface_array( img_magic_names, NUM_MAGICS );
  free_surface_array( img_shapes, NUM_SHAPES );
  free_surface_array( img_shape_names, NUM_SHAPES );
  free_surface_array( img_tux, NUM_TIP_TUX );

  free_surface( &img_openlabels_open );
  free_surface( &img_openlabels_erase );
  free_surface( &img_openlabels_back );

  free_surface( &img_progress );

  free_surface( &img_yes );
  free_surface( &img_no );

  free_surface( &img_title_on );
  free_surface( &img_title_off );
  free_surface( &img_title_large_on );
  free_surface( &img_title_large_off );

  free_surface( &img_open );
  free_surface( &img_erase );
  free_surface( &img_back );

  free_surface( &img_btn_up );
  free_surface( &img_btn_down );
  free_surface( &img_btn_off );

  free_surface( &img_cursor_up );
  free_surface( &img_cursor_down );

  free_surface( &img_cursor_starter_up );
  free_surface( &img_cursor_starter_down );

  free_surface( &img_scroll_up );
  free_surface( &img_scroll_down );
  free_surface( &img_scroll_up_off );
  free_surface( &img_scroll_down_off );

  free_surface( &img_paintcan );

  free_surface( &img_sparkles );
  free_surface( &img_grass );

  free_surface_array( undo_bufs, NUM_UNDO_BUFS );
#ifndef LOW_QUALITY_COLOR_SELECTOR
  free_surface_array( img_color_btns, NUM_COLORS*2 );
#endif
  free_surface_array( img_stamp_thumbs, MAX_STAMPS );

  free_surface( &screen );
  free_surface( &img_starter );
  free_surface( &img_starter_bkgd );
  free_surface( &canvas );
  free_surface( &img_cur_brush );

  if (font != NULL)
    {
      TTF_CloseFont(font);
      font = NULL;
    }

  if (small_font != NULL)
    {
      TTF_CloseFont(small_font);
      small_font = NULL;
    }

  if (large_font != NULL)
    {
      TTF_CloseFont(large_font);
      large_font = NULL;
    }

  for (i = 0; i < MAX_FONTS; i++)
    {
      if (fonts[i])
	{
	  TTF_CloseFont(fonts[i]);
	  fonts[i] = NULL;
	}
    }

#ifndef NOSOUND
  if (use_sound)
    {
      for (i = 0; i < NUM_SOUNDS; i++)
	{
	  if (sounds[i])
	    {
	      Mix_FreeChunk(sounds[i]);
	      sounds[i] = NULL;
	    }
	}

      for (i = 0; i < num_stamps; i++)
	{
	  if (snd_stamps[i])
	    {
	      Mix_FreeChunk(snd_stamps[i]);
	      snd_stamps[i] = NULL;
	    }
	}

      Mix_CloseAudio();
    }
#endif


  free_cursor(&cursor_hand);
  free_cursor(&cursor_arrow);
  free_cursor(&cursor_watch);
  free_cursor(&cursor_up);
  free_cursor(&cursor_down);
  free_cursor(&cursor_tiny);
  free_cursor(&cursor_crosshair);
  free_cursor(&cursor_brush);
  free_cursor(&cursor_wand);
  free_cursor(&cursor_insertion);
  free_cursor(&cursor_rotate);
  

  /* (Just in case...) */
  
  SDL_WM_GrabInput(SDL_GRAB_OFF);


  /* Close recording or playback file: */

  if (demofi != NULL)
    {
      fclose(demofi);
    }


  /* If we're using a lockfile, we can 'clear' it when we quit
     (so Tux Paint can be launched again soon, if the user wants to!) */

  if (ok_to_use_lockfile)
  {
    char * lock_fname;
    time_t zero_time;
    FILE * fi;

    lock_fname = get_fname("lockfile.dat");

    zero_time = (time_t) 0;

    fi = fopen(lock_fname, "w");
    if (fi != NULL)
    {
      /* If we can write to it, do so! */

      fwrite(&zero_time, sizeof(time_t), 1, fi);
      fclose(fi);
    }
    else
    {
      fprintf(stderr,
 	      "\nWarning: I couldn't create the lockfile (%s)\n"
	      "The error that occurred was:\n"
	      "%s\n\n", lock_fname, strerror(errno));
    }

    free(lock_fname);
  }

  /* Close up! */

  TTF_Quit();
  SDL_Quit();

}


static void free_cursor(SDL_Cursor ** cursor)
{
  if (*cursor)
    {
      SDL_FreeCursor(*cursor);
      *cursor = NULL;
    }
}


static void free_surface(SDL_Surface **surface_array)
{
  if (*surface_array)
    {
      SDL_FreeSurface(*surface_array);
      *surface_array = NULL;
    }
}


static void free_surface_array(SDL_Surface *surface_array[], int count)
{
  int i;

  for (i = 0; i < count; ++i)
    {
      free_surface(&surface_array[i]);
    }
}


/* Update screen where shape is/was: */

// FIXME: unused
/*
static void update_shape(int cx, int ox1, int ox2, int cy, int oy1, int oy2, int fix)
{
  int rx, ry;

  rx = abs(ox1 - cx);
  if (abs(ox2 - cx) > rx)
    rx = abs(ox2 - cx);

  ry = abs(oy1 - cy);
  if (abs(oy2 - cy) > ry)
    ry = abs(oy2 - cy);

  if (fix)
    {
      if (ry > rx)
	rx = ry;
      else
	ry = rx;
    }

  SDL_UpdateRect(screen, max((cx - rx), 0) + 96, max(cy - ry, 0),
		 min((cx + rx) + 96, screen->w),
		 min(cy + ry, screen->h));
}
*/


/* Draw a shape! */

static void do_shape(int cx, int cy, int ox, int oy, int rotn, int use_brush)
{
  int side, angle_skip, init_ang, rx, ry, rmax, x1, y1, x2, y2, xp, yp,
    old_brush, step;
  float a1, a2, rotn_rad;
#ifdef SCAN_FILL
  point_type pts[1024];  /* Careful! */
  fpoint_type fpts_orig[1024], fpts_new[1024];
  int i;
  int num_pts;
#else
  int xx;
#endif


  /* Determine radius/shape of the shape to draw: */

  old_brush = 0;
  rx = ox - cx;
  ry = oy - cy;


  /* If the shape has a 1:1 ("locked") aspect ratio, use the larger radius: */

  if (shape_locked[cur_shape])
    {
      if (rx > ry)
	ry = rx;
      else
	rx = ry;
    }


  /* Is the shape tiny?  Make it SOME size, first! */

  if (rx < 15 && ry < 15)
  {
    rx = 15;
    ry = 15;
  }


  /* Render a default brush: */

  if (use_brush)
    {
      old_brush = cur_brush;
      cur_brush = 0;  /* Kludgy! */
      render_brush();
    }


  /* Draw the shape: */

  angle_skip = 360 / shape_sides[cur_shape];

  init_ang = shape_init_ang[cur_shape];


#ifdef SCAN_FILL
  num_pts = 0;
#endif


  step = 1;

  if (dont_do_xor && !use_brush)
    {
      /* If we're in light outline mode & not drawing the shape with the brush,
	 if it has lots of sides (like a circle), reduce the number of sides: */

      if (shape_sides[cur_shape] > 5)
	step = (shape_sides[cur_shape] / 8);
    }


  for (side = 0; side < shape_sides[cur_shape]; side = side + step)
    {
      a1 = (angle_skip * side + init_ang) * M_PI / 180;
      a2 = (angle_skip * (side + 1) + init_ang) * M_PI / 180;

      x1 = (int) (cos(a1) * rx);
      y1 = (int) (-sin(a1) * ry);

      x2 = (int) (cos(a2) * rx);
      y2 = (int) (-sin(a2) * ry);


      /* Rotate the line: */

      if (rotn != 0)
	{
	  rotn_rad = rotn * M_PI / 180;

	  xp = x1 * cos(rotn_rad) - y1 * sin(rotn_rad);
	  yp = x1 * sin(rotn_rad) + y1 * cos(rotn_rad);

	  x1 = xp;
	  y1 = yp;

	  xp = x2 * cos(rotn_rad) - y2 * sin(rotn_rad);
	  yp = x2 * sin(rotn_rad) + y2 * cos(rotn_rad);

	  x2 = xp;
	  y2 = yp;
	}


      /* Center the line around the center of the shape: */

      x1 = x1 + cx;
      y1 = y1 + cy;
      x2 = x2 + cx;
      y2 = y2 + cy;


      /* Draw: */

      if (!use_brush)
	{
	  /* (XOR) */

	  line_xor(x1, y1, x2, y2);
	}
      else
	{
	  /* Brush */

	  brush_draw(x1, y1, x2, y2, 0);
	}

#ifdef SCAN_FILL
      fpts_orig[num_pts].x = (float) x2;
      fpts_orig[num_pts].y = (float) y2;
      num_pts++;
#endif
    }


  if (use_brush && shape_filled[cur_shape])
    {
#ifdef SCAN_FILL
      /* FIXME: This is all broken!!! */

      num_pts = clip_polygon(num_pts, fpts_orig, fpts_new);

      for (i = 0; i < num_pts; i++)
	{
	  pts[i].x = (int) (fpts_new[i].x);
	  pts[i].y = (int) (fpts_new[i].y);
	}

      scan_fill(num_pts, pts);
#else
      /* FIXME: In the meantime, we'll do this lame radius-based fill: */

      for (xx = abs(rx); xx >= 0; xx--)
	{
	  for (side = 0; side < shape_sides[cur_shape]; side++)
	    {
	      a1 = (angle_skip * side + init_ang) * M_PI / 180;
	      a2 = (angle_skip * (side + 1) + init_ang) * M_PI / 180;

	      x1 = (int) (cos(a1) * xx);
	      y1 = (int) (-sin(a1) * ry);

	      x2 = (int) (cos(a2) * xx);
	      y2 = (int) (-sin(a2) * ry);


	      /* Rotate the line: */

	      if (rotn != 0)
		{
		  rotn_rad = rotn * M_PI / 180;

		  xp = x1 * cos(rotn_rad) - y1 * sin(rotn_rad);
		  yp = x1 * sin(rotn_rad) + y1 * cos(rotn_rad);

		  x1 = xp;
		  y1 = yp;

		  xp = x2 * cos(rotn_rad) - y2 * sin(rotn_rad);
		  yp = x2 * sin(rotn_rad) + y2 * cos(rotn_rad);

		  x2 = xp;
		  y2 = yp;
		}


	      /* Center the line around the center of the shape: */

	      x1 = x1 + cx;
	      y1 = y1 + cy;
	      x2 = x2 + cx;
	      y2 = y2 + cy;


	      /* Draw: */

	      brush_draw(x1, y1, x2, y2, 0);
	    }

	  if (xx % 10 == 0)
	    update_canvas(0, 0, WINDOW_WIDTH - 96, (48 * 7) + 40 + HEIGHTOFFSET);
	}
#endif
    }


  /* Update it! */

  if (use_brush)
    {
      if (abs(rx) > abs(ry))
	rmax = abs(rx) + 20;
      else
	rmax = abs(ry) + 20;

      update_canvas(cx - rmax, cy - rmax,
		    cx + rmax, cy + rmax);
    }


  /* Return to normal brush (for paint brush and line tools): */

  if (use_brush)
    {
      cur_brush = old_brush;
      render_brush();
    }
}


/* What angle is the mouse away from the center of a shape? */

static int rotation(int ctr_x, int ctr_y, int ox, int oy)
{
  return(atan2(oy - ctr_y, ox - ctr_x) * 180 / M_PI);
}


/* FIXME: Move elsewhere!!! */

#define PROMPT_SAVE_OVER_TXT gettext_noop("Save over the older version of this picture?")
#define PROMPT_SAVE_OVER_YES gettext_noop("Yes")
#define PROMPT_SAVE_OVER_NO  gettext_noop("No, save a new file")


/* Save the current image: */

static int do_save(void)
{
  int res;
  char * fname;
  char tmp[1024];
  SDL_Surface * thm;
#ifndef SAVE_AS_BMP
  FILE * fi;
#endif


  /* Was saving completely disabled? */

  if (disable_save)
    return 0;

  
  if (promptless_save == SAVE_OVER_NO)
    {
      /* Never save over - _always_ save a new file! */

      get_new_file_id();
    }
  else if (promptless_save == SAVE_OVER_PROMPT)
    {
      /* Saving the same picture? */

      if (file_id[0] != '\0')
	{
	  /* We sure we want to do that? */

	  if (do_prompt(PROMPT_SAVE_OVER_TXT,
			PROMPT_SAVE_OVER_YES,
			PROMPT_SAVE_OVER_NO) == 0)
	    {
	      /* No - Let's save a new picture! */

	      get_new_file_id();
	    }
	}
      else
	{
	  /* Saving a new picture: */
	
	  get_new_file_id();
	}
    }
  else if (promptless_save == SAVE_OVER_ALWAYS)
    {
      if (file_id[0] == '\0')
	get_new_file_id();
    }
  

  /* Make sure we have a ~/.tuxpaint directory: */

  show_progress_bar();
  do_setcursor(cursor_watch);

  fname = get_fname("");

  res = mkdir(fname, 0755);

  if (res != 0 && errno != EEXIST)
    {
      fprintf(stderr,
	      "\nError: Can't create user data directory:\n"
	      "%s\n"
	      "The error that occurred was:\n"
	      "%s\n\n", fname, strerror(errno));

      fprintf(stderr,
	      "Cannot save the any pictures! SORRY!\n\n");

      draw_tux_text(TUX_OOPS, SDL_GetError(), 0);

      free(fname);
      return 0;
    }
  free(fname);

  show_progress_bar();

  
  /* Make sure we have a ~/.tuxpaint/saved directory: */

  fname = get_fname("saved");

  res = mkdir(fname, 0755);

  if (res != 0 && errno != EEXIST)
    {
      fprintf(stderr,
	      "\nError: Can't create user data directory:\n"
	      "%s\n"
	      "The error that occurred was:\n"
	      "%s\n\n", fname, strerror(errno));

      fprintf(stderr,
	      "Cannot save any pictures! SORRY!\n\n");

      draw_tux_text(TUX_OOPS, SDL_GetError(), 0);

      free(fname);
      return 0;
    }
  free(fname);

  show_progress_bar();


  /* Make sure we have a ~/.tuxpaint/saved/.thumbs/ directory: */

  fname = get_fname("saved/.thumbs");

  res = mkdir(fname, 0755);

  if (res != 0 && errno != EEXIST)
    {
      fprintf(stderr,
	      "\nError: Can't create user data thumbnail directory:\n"
	      "%s\n"
	      "The error that occurred was:\n"
	      "%s\n\n", fname, strerror(errno));

      fprintf(stderr,
	      "Cannot save any pictures! SORRY!\n\n");

      draw_tux_text(TUX_OOPS, SDL_GetError(), 0);

      free(fname);
      return 0;
    }
  free(fname);

  show_progress_bar();

  
  /* Save the file: */

  snprintf(tmp, sizeof(tmp), "saved/%s%s", file_id, FNAME_EXTENSION);
  fname = get_fname(tmp);
  debug(fname);

#ifdef SAVE_AS_BMP
  if (SDL_SaveBMP(canvas, fname))
    {
      fprintf(stderr,
	      "\nError: Couldn't save the current image!\n"
	      "%s\n"
	      "The Simple DirectMedia Layer error that occurred was:\n"
	      "%s\n\n", fname, SDL_GetError());

      draw_tux_text(TUX_OOPS, SDL_GetError(), 0);

      free(fname);
      return 0;
    }
  else
    {
      /* Ta-Da! */

      playsound(0, SND_SAVE, 1);
      draw_tux_text(TUX_DEFAULT, tool_tips[TOOL_SAVE], 1);
    }
#else
  fi = fopen(fname, "wb");
  if (fi == NULL)
    {
      fprintf(stderr,
	      "\nError: Couldn't save the current image!\n"
	      "%s\n"
	      "The system error that occurred was:\n"
	      "%s\n\n",
	      fname, strerror(errno));

      draw_tux_text(TUX_OOPS, strerror(errno), 0);
    }
  else
    {
      if (!do_png_save(fi, fname, canvas))
	{
	  free(fname);
	  return 0;
	}
    }
#endif

  free(fname);
  
  show_progress_bar();
  
  
  /* Save thumbnail, too: */

  /* (Was thumbnail in old directory, rather than under .thumbs?) */
  
  snprintf(tmp, sizeof(tmp), "saved/%s-t%s", file_id, FNAME_EXTENSION);
  fname = get_fname(tmp);
  fi = fopen(fname, "r");
  if (fi != NULL)
    {
      fclose(fi);
    }
  else
    {
      /* No old thumbnail!  Save this image's thumbnail in the new place,
	 under ".thumbs" */
  
      snprintf(tmp, sizeof(tmp), "saved/.thumbs/%s-t%s", file_id, FNAME_EXTENSION);
      fname = get_fname(tmp);
    }
  
  debug(fname);

  thm = thumbnail(canvas, THUMB_W - 20, THUMB_H - 20, 0);
  
  fi = fopen(fname, "wb");
  if (fi == NULL)
    {
      fprintf(stderr, "\nError: Couldn't save thumbnail of image!\n"
	      "%s\n"
	      "The system error that occurred was:\n"
	      "%s\n\n",
	      fname, strerror(errno));
    }
  else
    {
      do_png_save(fi, fname, thm);
    }
  SDL_FreeSurface(thm);
  
  free(fname);


  /* Write 'starter' info, if any: */

  if (starter_id[0] != '\0')
  {
    snprintf(tmp, sizeof(tmp), "saved/%s.dat", file_id);
    fname = get_fname(tmp);
    fi = fopen(fname, "w");
    if (fi != NULL)
    {
      fprintf(fi, "%s\n", starter_id);
      fprintf(fi, "%d %d\n", starter_mirrored, starter_flipped);
      fclose(fi);
    }

    free(fname);
  }
  

  /* All happy! */

  playsound(0, SND_SAVE, 1);
  draw_tux_text(TUX_DEFAULT, tool_tips[TOOL_SAVE], 1);
  do_setcursor(cursor_arrow);

  return 1;
}


/* Actually save the PNG data to the file stream: */

static int do_png_save(FILE * fi, const char * const fname, SDL_Surface * surf)
{
  png_structp png_ptr;
  png_infop info_ptr;
  png_text text_ptr[4];
  unsigned char ** png_rows;
  Uint8 r, g, b;
  int x, y, count;
  
  
  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png_ptr == NULL)
    {
      fclose(fi);
      png_destroy_write_struct(&png_ptr, (png_infopp) NULL);

      fprintf(stderr, "\nError: Couldn't save the image!\n%s\n\n", fname);
      draw_tux_text(TUX_OOPS, strerror(errno), 0);
    }
  else
    {
      info_ptr = png_create_info_struct(png_ptr);
      if (info_ptr == NULL)
	{
	  fclose(fi);
	  png_destroy_write_struct(&png_ptr, (png_infopp) NULL);

	  fprintf(stderr, "\nError: Couldn't save the image!\n%s\n\n", fname);
	  draw_tux_text(TUX_OOPS, strerror(errno), 0);
	}
      else
	{
	  if (setjmp(png_jmpbuf(png_ptr)))
	    {
	      fclose(fi);
	      png_destroy_write_struct(&png_ptr, (png_infopp) NULL);

	      fprintf(stderr, "\nError: Couldn't save the image!\n%s\n\n", fname);
	      draw_tux_text(TUX_OOPS, strerror(errno), 0);
  
	      return 0;
	    }
	  else
	    {
	      png_init_io(png_ptr, fi);

	      info_ptr->width = surf->w;
	      info_ptr->height = surf->h;
	      info_ptr->bit_depth = 8;
	      info_ptr->color_type = PNG_COLOR_TYPE_RGB;
	      info_ptr->interlace_type = 1;
	      info_ptr->valid = 0;


	      /* Set headers */

	      count = 0;

	      /*
	      if (title != NULL && strlen(title) > 0)
	      {
	        text_ptr[count].key = "Title";
	        text_ptr[count].text = title;
	        text_ptr[count].compression = PNG_TEXT_COMPRESSION_NONE;
	        count++;
	      }
	      */

	      text_ptr[count].key = (png_charp) "Software";
	      text_ptr[count].text =
	        (png_charp) "Tux Paint " VER_VERSION " (" VER_DATE ")";
	      text_ptr[count].compression = PNG_TEXT_COMPRESSION_NONE;
	      count++;

	      
	      png_set_text(png_ptr, info_ptr, text_ptr, count);
			      
	      png_write_info(png_ptr, info_ptr);



	      /* Save the picture: */

	      png_rows = malloc(sizeof(char *) * surf->h);

	      for (y = 0; y < surf->h; y++)
		{
		  png_rows[y] = malloc(sizeof(char) * 3 * surf->w);

		  for (x = 0; x < surf->w; x++)
		    {
		      SDL_GetRGB(getpixel(surf, x, y), surf->format, &r, &g, &b);

		      png_rows[y][x * 3 + 0] = r;
		      png_rows[y][x * 3 + 1] = g;
		      png_rows[y][x * 3 + 2] = b;
		    }
		}
        
	      png_write_image(png_ptr, png_rows);

	      for (y = 0; y < surf->h; y++)
		free(png_rows[y]);

	      free(png_rows);
        

	      png_write_end(png_ptr, NULL);

	      png_destroy_write_struct(&png_ptr, &info_ptr);
	      fclose(fi);

	      return 1;
	    }
	}
    }

  return 0;
}


/* Pick a new file ID: */

static void get_new_file_id(void)
{
  time_t t;

  t = time(NULL);

  strftime(file_id, sizeof(file_id), "%Y%m%d%H%M%S", localtime(&t));
  debug(file_id);


  /* FIXME: Show thumbnail and prompt for title: */
}


/* Handle quitting (and prompting to save, if necessary!) */

static int do_quit(void)
{
  int done;

  done = do_prompt(PROMPT_QUIT_TXT,
	           PROMPT_QUIT_YES,
		   PROMPT_QUIT_NO);

  if (done && !been_saved && !disable_save)
    {
      if (do_prompt(PROMPT_QUIT_SAVE_TXT,
		    PROMPT_QUIT_SAVE_YES,
		    PROMPT_QUIT_SAVE_NO))
	{
	  if (do_save())
	    {
	      do_prompt(tool_tips[TOOL_SAVE],
			"OK",
			"");
	    }
	  else
	    {
	      /* Couldn't save!  Abort quit! */
	
	      done = 0;
	    }
	}
    }

  return(done);
}



/* Open a saved image: */

#define PLACE_STARTERS_DIR 0
#define PLACE_SAVED_DIR 1
#define NUM_PLACES_TO_LOOK 2


static int do_open(int want_new_tool)
{
  SDL_Surface * img, * img1, * img2;
  int things_alloced;
  SDL_Surface * * thumbs = NULL;
  DIR * d;
  struct dirent * f;
  struct dirent2 * fs;
  int place;
  char * dirname[NUM_PLACES_TO_LOOK];
  char * rfname;
  char * * d_names = NULL, * * d_exts = NULL;
  int * d_places;
  FILE * fi;
  char fname[1024];
  char * tmp_fname;
  int num_files, i, done, update_list, want_erase, cur, which,
    num_files_in_dirs, j, res;
  SDL_Rect dest;
  SDL_Event event;
  SDLKey key;
  Uint32 last_click_time;
  int last_click_which, last_click_button;
  int places_to_look;

  do_setcursor(cursor_watch);
  
  /* Allocate some space: */

  things_alloced = 32;
  
  fs = (struct dirent2 *) malloc(sizeof(struct dirent2) * things_alloced);
  
  num_files = 0;
  cur = 0;
  which = 0;
  num_files_in_dirs = 0;


  /* Open directories of images: */

  for (places_to_look = 0;
       places_to_look < NUM_PLACES_TO_LOOK;
       places_to_look++)
  {
    if (places_to_look == PLACE_STARTERS_DIR)
    {
      /* Check for coloring-book style 'starter' images first: */

      dirname[places_to_look] = strdup(DATA_PREFIX "starters");
    }
    else
    {
      /* Then check for saved-images: */

      dirname[places_to_look] = get_fname("saved");
    }
    

    /* Read directory of images and build thumbnails: */

    d = opendir(dirname[places_to_look]);
    
    if (d != NULL)
    {
      /* Gather list of files (for sorting): */
      
      do
	{
	  f = readdir(d);
	  
	  if (f != NULL)
	    {
	      memcpy(&(fs[num_files_in_dirs].f), f, sizeof(struct dirent));
	      fs[num_files_in_dirs].place = places_to_look;
	      
	      num_files_in_dirs++;
	      
	      if (num_files_in_dirs >= things_alloced)
		{
		  things_alloced = things_alloced + 32;
		  fs = (struct dirent2 *) realloc(fs,
				  		  sizeof(struct dirent2) *
						  things_alloced);
		}
	    }
	}
      while (f != NULL);
      
      closedir(d);
    }
  }


    /* (Re)allocate space for the information about these files: */

    thumbs = (SDL_Surface * *) malloc(sizeof(SDL_Surface *) *
		    			num_files_in_dirs);
    d_places = (int *) malloc(sizeof(int) * num_files_in_dirs);
    d_names = (char * *) malloc(sizeof(char *) * num_files_in_dirs);
    d_exts = (char * *) malloc(sizeof(char *) * num_files_in_dirs);


    /* Sort: */
      
    qsort(fs, num_files_in_dirs, sizeof(struct dirent2),
	  (int(*)(const void *, const void *))compare_dirent2s);
      
      
    /* Read directory of images and build thumbnails: */
      
    for (j = 0; j < num_files_in_dirs; j++)
    {
      f = &(fs[j].f);
      place = fs[j].place;

      show_progress_bar();
      
      if (f != NULL)
      {
        debug(f->d_name);
	      
        if (strstr(f->d_name, "-t.") == NULL &&
	    strstr(f->d_name, "-back.") == NULL)
	{
	  if (strstr(f->d_name, FNAME_EXTENSION) != NULL
#ifndef SAVE_AS_BMP
          /* Support legacy BMP files for load: */
		      
	      || strstr(f->d_name, ".bmp") != NULL
#endif
	      )
	  {
	    strcpy(fname, f->d_name);
	    if (strstr(fname, FNAME_EXTENSION) != NULL)
	    {
	      strcpy(strstr(fname, FNAME_EXTENSION), "");
	      d_exts[num_files] = strdup(FNAME_EXTENSION);
	    }
		      
#ifndef SAVE_AS_BMP
	    if (strstr(fname, ".bmp") != NULL)
	    {
	      strcpy(strstr(fname, ".bmp"), "");
	      d_exts[num_files] = strdup(".bmp");
	    }
#endif
 
	    d_names[num_files] = strdup(fname);
	    d_places[num_files] = place;


	    /* Is it the 'current' file we just loaded?
	       We'll make it the current selection! */
		 
	    if (strcmp(d_names[num_files], file_id) == 0)
	    {
	      which = num_files;
	      cur = (which / 4) * 4;

	      /* Center the cursor (useful for when the last item is
	         selected first!) */

	      if (cur - 8 >= 0)
		cur = cur - 8;
	      else if (cur - 4 >= 0)
		cur = cur - 4;
	    }
		     
    
	    /* Try to load thumbnail first: */

	    snprintf(fname, sizeof(fname), "%s/.thumbs/%s-t.png",
		     dirname[d_places[num_files]], d_names[num_files]);
	    debug(fname);
	    img = IMG_Load(fname);
		      
	    if (img == NULL)
	    {
	      /* No thumbnail in the new location ("saved/.thumbs"),
	         try the old locatin ("saved/"): */
	
	      snprintf(fname, sizeof(fname), "%s/%s-t.png",
		       dirname[d_places[num_files]],
		       d_names[num_files]);
              debug(fname);
			  
	      img = IMG_Load(fname);
	    }
	    
	    if (img != NULL)
	    {
	      /* Loaded the thumbnail from one or the other location */
	      show_progress_bar();

	      img1 = SDL_DisplayFormat(img);
	      SDL_FreeSurface(img);

	      // if too big, or too small in both dimensions, rescale it
	      // ( for now: using old thumbnail as source for high speed, low quality)
	      if (img1->w > THUMB_W-20 || img1->h > THUMB_H-20 || (img1->w < THUMB_W-20 && img1->h < THUMB_H-20) )
	        {
	          img2 = thumbnail(img1, THUMB_W - 20, THUMB_H - 20, 0);
	          SDL_FreeSurface(img1);
	          img1 = img2;
	        }

	      thumbs[num_files] = img1;
		      
	      if (thumbs[num_files] == NULL)
	      {
	        fprintf(stderr,
		        "\nError: Couldn't create a thumbnail of "
		        "saved image!\n"
		        "%s\n", fname);
	      }

	      num_files++;
	    }
	    else
	    {
	      /* No thumbnail - load original: */
	      /* (Make sure we have a .../saved/.thumbs/ directory:) */
			  
	      tmp_fname = get_fname("saved/.thumbs");
			  
	      res = mkdir(tmp_fname, 0755);
			  
	      if (res != 0 && errno != EEXIST)
	      {
	        fprintf(stderr,
		        "\nError: Can't create user data thumbnail directory:\n"
		        "%s\n"
		        "The error that occurred was:\n"
		        "%s\n\n", tmp_fname, strerror(errno));
	      }
			  
	      free(tmp_fname);


	      img = NULL;
	      
              if (d_places[num_files] == PLACE_STARTERS_DIR)
	      {
		/* Try to load a starter's background image, first!
		   If it exists, it should give a better idea of what the
		   starter looks like, compared to the overlay image... */

		/* (Try JPEG first) */
		snprintf(fname, sizeof(fname), "%s/%s-back.jpeg",
			 dirname[d_places[num_files]],
		         d_names[num_files]);

		img = IMG_Load(fname);


		if (img == NULL)
		{
		  /* (Try PNG next) */
		  snprintf(fname, sizeof(fname), "%s/%s-back.png",
			   dirname[d_places[num_files]],
		           d_names[num_files]);

		  img = IMG_Load(fname);
	        }
	      }

	      
	      if (img == NULL)
	      {
		/* Didn't load a starter background (or didn't try!),
		   try loading the actual image... */

	        snprintf(fname, sizeof(fname), "%s/%s",
		         dirname[d_places[num_files]], f->d_name);
			 debug(fname);
#ifdef SAVE_AS_BMP
	        img = SDL_LoadBMP(fname);
#else
	        img = IMG_Load(fname);
#endif
	      }

	      
	      show_progress_bar();
			  
	      if (img == NULL)
	      {
	        fprintf(stderr,
		        "\nWarning: I can't open one of the saved files!\n"
		        "%s\n"
		        "The Simple DirectMedia Layer error that "
		        "occurred was:\n"
		        "%s\n\n",
		        fname, SDL_GetError());
			      
		free(d_names[num_files]);
	        free(d_exts[num_files]);
	      }
	      else
	      {
	        /* Turn it into a thumbnail: */
			      
	        img1 = SDL_DisplayFormat(img);
	        img2 = thumbnail(img1, THUMB_W - 20, THUMB_H - 20, 0);
		SDL_FreeSurface(img1);
			      
		show_progress_bar();
			      
		thumbs[num_files] = SDL_DisplayFormat(img2);
		SDL_FreeSurface(img2);
		if (thumbs[num_files] == NULL)
		{
		  fprintf(stderr,
			  "\nError: Couldn't create a thumbnail of "
			  "saved image!\n"
			  "%s\n", fname);
		}
		 
		SDL_FreeSurface(img);
			      
		show_progress_bar();
			      
			      
		/* Let's save this thumbnail, so we don't have to
		   create it again next time 'Open' is called: */
		
		if (d_places[num_files] == PLACE_SAVED_DIR)
		{
		  debug("Saving thumbnail for this one!");

		  snprintf(fname, sizeof(fname), "%s/.thumbs/%s-t.png",
			   dirname[d_places[num_files]], d_names[num_files]);
			      
		  fi = fopen(fname, "wb");
		  if (fi == NULL)
		  {
		    fprintf(stderr,
			    "\nError: Couldn't save thumbnail of "
			    "saved image!\n"
			    "%s\n"
			    "The error that occurred was:\n"
			    "%s\n\n",
			    fname, strerror(errno));
		  }
		  else
		  {
		    do_png_save(fi, fname, thumbs[num_files]);
		  }
		 
		  show_progress_bar();
		}

		 
		num_files++;
	      }
	    }
	  }
	}
	else
	{
	  /* It was a thumbnail file ("...-t.png") or immutable scene starter's
	     overlay layer ("...-front.png") */
	}
      }
    }
      
      
      
#ifdef DEBUG
  printf("%d saved files were found!\n", num_files);
#endif


  
  if (num_files == 0)
  {
    do_prompt(PROMPT_OPEN_NOFILES_TXT, PROMPT_OPEN_NOFILES_YES, "");
  }
  else
  {
    /* Let user choose an image: */

    draw_tux_text(TUX_BORED,
  		textdir(gettext_noop("Choose the picture you want, "
  				     "then click “Open”.")), 1);

    /* NOTE: cur is now set above; if file_id'th file is found, it's
       set to that file's index; otherwise, we default to '0' */
    
    update_list = 1;
    want_erase = 0;

    done = 0;

    last_click_which = -1;
    last_click_time = 0;
    last_click_button = -1;


    do_setcursor(cursor_arrow);
  
  
    do
    {
      /* Update screen: */
	  
      if (update_list)
      {
        /* Erase: */
	      
	dest.x = 96;
	dest.y = 0;
	dest.w = WINDOW_WIDTH - 96 - 96;
	dest.h = 48 * 7 + 40 + HEIGHTOFFSET;
	      
	SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format,
					       255, 255, 255));
	      
	 
	/* Draw icons: */
	      
	for (i = cur; i < cur + 16 && i < num_files; i++)
	{
	  /* Draw cursor: */
	 
	  dest.x = THUMB_W * ((i - cur) % 4) + 96;
	  dest.y = THUMB_H * ((i - cur) / 4) + 24;
	 
	  if (d_places[i] == PLACE_SAVED_DIR)
	  {
	    if (i == which)
	    {
	      SDL_BlitSurface(img_cursor_down, NULL, screen, &dest);
	      debug(d_names[i]);
	    }
	    else
	      SDL_BlitSurface(img_cursor_up, NULL, screen, &dest);
	  }
	  else
	  {
	    if (i == which)
	    {
	      SDL_BlitSurface(img_cursor_starter_down, NULL, screen, &dest);
	      debug(d_names[i]);
	    }
	    else
	      SDL_BlitSurface(img_cursor_starter_up, NULL, screen, &dest);
	  }
		  
		  
		  
	  dest.x = THUMB_W * ((i - cur) % 4) + 96 + 10 +
		   (THUMB_W - 20 - thumbs[i]->w) / 2;
          dest.y = THUMB_H * ((i - cur) / 4) + 24 + 10 +
		   (THUMB_H - 20 - thumbs[i]->h) / 2;
		  
	  if (thumbs[i] != NULL)
	    SDL_BlitSurface(thumbs[i], NULL, screen, &dest);
	}
	      
	      
	/* Draw arrows: */
	      
	dest.x = (WINDOW_WIDTH - img_scroll_up->w) / 2;
	dest.y = 0;
	      
	if (cur > 0)
	  SDL_BlitSurface(img_scroll_up, NULL, screen, &dest);
	else
	  SDL_BlitSurface(img_scroll_up_off, NULL, screen, &dest);
	      
	dest.x = (WINDOW_WIDTH - img_scroll_up->w) / 2;
	dest.y = (48 * 7 + 40 + HEIGHTOFFSET) - 48;
	      
	if (cur < num_files - 16)
	  SDL_BlitSurface(img_scroll_down, NULL, screen, &dest);
	else
	  SDL_BlitSurface(img_scroll_down_off, NULL, screen, &dest);
	      
	      
	/* "Open" button: */
	      
	dest.x = 96;
	dest.y = (48 * 7 + 40 + HEIGHTOFFSET) - 48;
	SDL_BlitSurface(img_open, NULL, screen, &dest);
	     
	dest.x = 96 + (48 - img_openlabels_open->w) / 2;
	dest.y = (48 * 7 + 40 + HEIGHTOFFSET) - img_openlabels_open->h;
	SDL_BlitSurface(img_openlabels_open, NULL, screen, &dest);
	      
	      
	/* "Back" button: */
	      
	dest.x = WINDOW_WIDTH - 96 - 48;
	dest.y = (48 * 7 + 40 + HEIGHTOFFSET) - 48;
	SDL_BlitSurface(img_back, NULL, screen, &dest);
	      
	dest.x = WINDOW_WIDTH - 96 - 48 + (48 - img_openlabels_back->w) / 2;
	dest.y = (48 * 7 + 40 + HEIGHTOFFSET) - img_openlabels_back->h;
	SDL_BlitSurface(img_openlabels_back, NULL, screen, &dest);
	     
	      
	/* "Erase" button: */

	dest.x = WINDOW_WIDTH - 96 - 48 - 48;
	dest.y = (48 * 7 + 40 + HEIGHTOFFSET) - 48;
	
	if (d_places[which] != PLACE_STARTERS_DIR)
	  SDL_BlitSurface(img_erase, NULL, screen, &dest);
	else
	  SDL_BlitSurface(img_btn_off, NULL, screen, &dest);
	      
	dest.x = WINDOW_WIDTH - 96 - 48 - 48 + (48 - img_openlabels_erase->w) / 2;
	dest.y = (48 * 7 + 40 + HEIGHTOFFSET) - img_openlabels_erase->h;
	SDL_BlitSurface(img_openlabels_erase, NULL, screen, &dest);
	      
	      
	SDL_Flip(screen);
	      
	update_list = 0;
      }
	  
	  
      mySDL_WaitEvent(&event);
	  
      if (event.type == SDL_QUIT)
      {
	done = 1;

	/* FIXME: Handle SDL_Quit better */
      }
      else if (event.type == SDL_ACTIVEEVENT)
      {
	handle_active(&event);
      }
      else if (event.type == SDL_KEYUP)
      {
	key = event.key.keysym.sym;
	      
	handle_keymouse(key, SDL_KEYUP);
      }
      else if (event.type == SDL_KEYDOWN)
      {
	key = event.key.keysym.sym;
	      
	handle_keymouse(key, SDL_KEYDOWN);
	      
	if (key == SDLK_LEFT)
	{
	  if (which > 0)
	  {
	    which--;
		      
	    if (which < cur)
	      cur = cur - 4;
		      
	    update_list = 1;
	  }
	}
	else if (key == SDLK_RIGHT)
	{
	  if (which < num_files - 1)
	  {
	    which++;
		      
	    if (which >= cur + 16)
	      cur = cur + 4;
		      
	    update_list = 1;
	  }
	}
	else if (key == SDLK_UP)
	{
	  if (which >= 0)
	  {
	    which = which - 4;
		      
	    if (which < 0)
	      which = 0;
		      
	    if (which < cur)
	      cur = cur - 4;
		      
	    update_list = 1;
	  }
	}
        else if (key == SDLK_DOWN)
	{
	      if (which < num_files)
		{
		  which = which + 4;
	      
		  if (which >= num_files)
		    which = num_files - 1;
	      
		  if (which >= cur + 16)
		    cur = cur + 4;
	      
		  update_list = 1;
		}
	}
	else if (key == SDLK_RETURN || key == SDLK_SPACE)
	  {
	    /* Open */
	
	    done = 1;
	    playsound(1, SND_CLICK, 1);
	  }
	else if (key == SDLK_ESCAPE)
	  {
	    /* Go back: */
	
	    which = -1;
	    done = 1;
	    playsound(1, SND_CLICK, 1);
	  }
	  else if (key == SDLK_d &&
		   (event.key.keysym.mod & KMOD_CTRL ||
		    event.key.keysym.mod & KMOD_LCTRL ||
		    event.key.keysym.mod & KMOD_RCTRL) &&
		   d_places[which] != PLACE_STARTERS_DIR &&
		   !noshortcuts)
	    {
	      /* Delete! */
	  
	      want_erase = 1;
	    }
	}
     else if (event.type == SDL_MOUSEBUTTONDOWN &&
              event.button.button >= 1 &&
              event.button.button <= 3)
       {
		  if (event.button.x >= 96 && event.button.x < WINDOW_WIDTH - 96 &&
		      event.button.y >= 24 &&
		      event.button.y < (48 * 7 + 40 + HEIGHTOFFSET - 48))
		    {
		      /* Picked an icon! */
		  
		      which = ((event.button.x - 96) / (THUMB_W) +
			       (((event.button.y - 24) / THUMB_H) * 4)) + cur;
		  
		      if (which < num_files)
			{
			  playsound(1, SND_BLEEP, 1);
			  update_list = 1;
		      
		      
			  if (which == last_click_which &&
			      SDL_GetTicks() < last_click_time + 1000 &&
			      event.button.button == last_click_button)
			    {
			      /* Double-click! */
			  
			      done = 1;
			    }
		      
			  last_click_which = which;
			  last_click_time = SDL_GetTicks();
			  last_click_button = event.button.button;
			}
		    }
		  else if (event.button.x >= (WINDOW_WIDTH - img_scroll_up->w) / 2 &&
			   event.button.x <= (WINDOW_WIDTH + img_scroll_up->w) / 2)
		    {
		      if (event.button.y < 24)
			{
			  /* Up scroll button: */
		      
			  if (cur > 0)
			    {
			      cur = cur - 4;
			      update_list = 1;
			      playsound(1, SND_SCROLL, 1);

			      if (cur == 0)
				do_setcursor(cursor_arrow);
			    }
		      
			  if (which > cur + 16)
			    which = which - 4;
			}
		      else if (event.button.y >= (48 * 7 + 40 + HEIGHTOFFSET - 48) &&
			       event.button.y < (48 * 7 + 40 + HEIGHTOFFSET - 24))
			{
			  /* Down scroll button: */
		      
			  if (cur < num_files - 16)
			    {
			      cur = cur + 4;
			      update_list = 1;
			      playsound(1, SND_SCROLL, 1);

			      if (cur >= num_files - 16)
				do_setcursor(cursor_arrow);
			    }
		      
			  if (which < cur)
			    which = which + 4;
			}
		    }
		  else if (event.button.x >= 96 && event.button.x < 96 + 48 &&
			   event.button.y >= (48 * 7 + 40 + HEIGHTOFFSET) - 48 &&
			   event.button.y < (48 * 7 + 40 + HEIGHTOFFSET))
		    {
		      /* Open */
		  
		      done = 1;
		      playsound(1, SND_CLICK, 1);
		    }
		  else if (event.button.x >= (WINDOW_WIDTH - 96 - 48) &&
			   event.button.x < (WINDOW_WIDTH - 96) &&
			   event.button.y >= (48 * 7 + 40 + HEIGHTOFFSET) - 48 &&
			   event.button.y < (48 * 7 + 40 + HEIGHTOFFSET))
		    {
		      /* Back */
		  
		      which = -1;
		      done = 1;
		      playsound(1, SND_CLICK, 1);
		    }
		  else if (event.button.x >= (WINDOW_WIDTH - 96 - 48 - 48) &&
			   event.button.x < (WINDOW_WIDTH - 48 - 96) &&
			   event.button.y >= (48 * 7 + 40 + HEIGHTOFFSET) - 48 &&
			   event.button.y < (48 * 7 + 40 + HEIGHTOFFSET) &&
			   d_places[which] != PLACE_STARTERS_DIR)
		    {
		      /* Erase */
		  
             want_erase = 1;
           }
       }
     else if (event.type == SDL_MOUSEMOTION)
		{
		  /* Deal with mouse pointer shape! */

		  if (event.button.y < 24 &&
		      event.button.x >= (WINDOW_WIDTH - img_scroll_up->w) / 2 &&
		      event.button.x <= (WINDOW_WIDTH + img_scroll_up->w) / 2 &&
		      cur > 0)
		    {
		      /* Scroll up button: */

		      do_setcursor(cursor_up);
		    }
		  else if (event.button.y >= (48 * 7 + 40 + HEIGHTOFFSET - 48) &&
			   event.button.y < (48 * 7 + 40 + HEIGHTOFFSET - 24) &&
			   event.button.x >= (WINDOW_WIDTH - img_scroll_up->w) / 2 &&
			   event.button.x <= (WINDOW_WIDTH + img_scroll_up->w) / 2 &&
			   cur < num_files - 16)
		    {
		      /* Scroll down button: */

		      do_setcursor(cursor_down);
		    }
		  else if (((event.button.x >= 96 && event.button.x < 96 + 48) ||
			    (event.button.x >= (WINDOW_WIDTH - 96 - 48) &&
			     event.button.x < (WINDOW_WIDTH - 96)) ||
			    (event.button.x >= (WINDOW_WIDTH - 96 - 48 - 48) &&
			     event.button.x < (WINDOW_WIDTH - 48 - 96) &&
			     d_places[which] != PLACE_STARTERS_DIR)) &&
			   event.button.y >= (48 * 7 + 40 + HEIGHTOFFSET) - 48 &&
			   event.button.y < (48 * 7 + 40 + HEIGHTOFFSET))
		    {
		      /* One of the command buttons: */

		      do_setcursor(cursor_hand);
		    }
		  else if (event.button.x >= 96 && event.button.x < WINDOW_WIDTH - 96 &&
			   event.button.y > 24 &&
			   event.button.y < (48 * 7 + 40 + HEIGHTOFFSET) - 48 &&
			   ((((event.button.x - 96) / (THUMB_W) +
			      (((event.button.y - 24) / THUMB_H) * 4)) +
			     cur) < num_files))
		    {
		      /* One of the thumbnails: */
	
		      do_setcursor(cursor_hand);
		    }
		  else
		    {
		      /* Unclickable... */

		      do_setcursor(cursor_arrow);
		    }
		}

	  
	      if (want_erase)
		{
		  want_erase = 0;
	      
		  if (do_prompt(PROMPT_ERASE_TXT,
				PROMPT_ERASE_YES, PROMPT_ERASE_NO))
		    {
		      snprintf(fname, sizeof(fname), "saved/%s%s",
			       d_names[which], d_exts[which]);
		  
		      rfname = get_fname(fname);
		      debug(rfname);
		  
		      if (unlink(rfname) == 0)
			{
			  thumbs[which] = NULL;
			  update_list = 1;

		      
			  /* Delete the thumbnail, too: */
		      
			  snprintf(fname, sizeof(fname),
				   "saved/.thumbs/%s-t.png",
				   d_names[which]);
		      
			  free(rfname);
			  rfname = get_fname(fname);
			  debug(rfname);
			  
			  unlink(rfname);


			  /* Try deleting old-style thumbnail, too: */
		      
			  unlink(rfname);
			  snprintf(fname, sizeof(fname), "saved/%s-t.png",
				   d_names[which]);
		      
			  free(rfname);
			  rfname = get_fname(fname);
			  debug(rfname);
		      
			  unlink(rfname);
		      
		      
			  /* Delete .dat file, if any: */
		      
			  unlink(rfname);
			  snprintf(fname, sizeof(fname), "saved/%s.dat",
				   d_names[which]);
		      
			  free(rfname);
			  rfname = get_fname(fname);
			  debug(rfname);
		      
			  unlink(rfname);
		      
		      
			  /* Move all other files up a notch: */
		      
			  free(d_names[which]);
			  free(d_exts[which]);
			  free_surface(&thumbs[which]);
		      
			  for (i = which; i < num_files - 1; i++)
			    {
			      d_names[i] = d_names[i + 1];
			      d_exts[i] = d_exts[i + 1];
			      thumbs[i] = thumbs[i + 1];
			      d_places[i] = d_places[i + 1];
			    }
		      
			  num_files--;
		      
		      
			  /* Make sure the cursor doesn't go off the end! */
		      
			  if (which >= num_files)
			    which = num_files - 1;


			  /* Scroll up if the cursor goes off top of screen! */

			  if (which < cur && cur >= 4)
			    {
			      cur = cur - 4;
			      update_list = 1;
			    }
		      
		      
			  /* No files to open now? */
		      
			  if (which < 0)
			    {
			      do_prompt(PROMPT_OPEN_NOFILES_TXT,
					PROMPT_OPEN_NOFILES_YES, "");
			      done = 1;
			    }
			}
		      else
			{
			  perror(rfname);
		      
			  do_prompt("CAN'T", "OK", "");
			  update_list = 1;
			}
		  
		      free(rfname);
		    }
		  else
		    {
		      update_list = 1;
		    }
		}
	  
	    }
	  while (!done);
      
      
	  /* Load the chosen picture: */
      
	  if (which != -1)
	    {
	      /* Save old one first? */
	  
	      if (!been_saved && !disable_save)
		{
		  if (do_prompt(PROMPT_OPEN_SAVE_TXT,
				PROMPT_OPEN_SAVE_YES,
				PROMPT_OPEN_SAVE_NO))
		    {
		      do_save();
		    }
		}
	   

	      /* Figure out filename: */

	      snprintf(fname, sizeof(fname), "%s/%s%s",
		       dirname[d_places[which]],
		       d_names[which], d_exts[which]);

#ifdef SAVE_AS_BMP
	      img = SDL_LoadBMP(fname);
#else
	      img = IMG_Load(fname);
#endif
	  
	      if (img == NULL)
		{
		  fprintf(stderr,
			  "\nWarning: Couldn't load the saved image!\n"
			  "%s\n"
			  "The Simple DirectMedia Layer error that occurred "
			  "was:\n"
			  "%s\n\n", fname, SDL_GetError());
	      
		  do_prompt(PROMPT_OPEN_UNOPENABLE_TXT,
			    PROMPT_OPEN_UNOPENABLE_YES, "");
		}
	      else
		{
		  free_surface(&img_starter);
		  free_surface(&img_starter_bkgd);
		  starter_mirrored = 0;
		  starter_flipped = 0;

		  autoscale_copy_smear_free(img,canvas,SDL_BlitSurface);

		  cur_undo = 0;
		  oldest_undo = 0;
		  newest_undo = 0;

		  if (d_places[which] == PLACE_SAVED_DIR)
		  {
		    /* Saved image: */
			  
		    been_saved = 1;
	  
		    strcpy(file_id, d_names[which]);
		    starter_id[0] = '\0';

		    
		    /* See if this saved image was based on a 'starter' */

		    load_starter_id(d_names[which]);

		    if (starter_id[0] != '\0')
		    {
		      load_starter(starter_id);
		      
                      if (starter_mirrored)
                        mirror_starter();
         
		      if (starter_flipped)
			flip_starter();
		    }
		  }
		  else
		  {
		    /* Immutable 'starter' image;
		       we'll need to save a new image when saving...: */

		    been_saved = 1;

      		    file_id[0] = '\0';
		    strcpy(starter_id, d_names[which]);
		    load_starter(starter_id);

		    SDL_FillRect(canvas, NULL, SDL_MapRGB(canvas->format, 255, 255, 255));
		    SDL_BlitSurface(img_starter_bkgd, NULL, canvas, NULL);
		    SDL_BlitSurface(img_starter, NULL, canvas, NULL);
		  }
		  
		  
		  reset_avail_tools();
		  tool_avail[TOOL_NEW] = 1;
	      
		  tool_avail_bak[TOOL_UNDO] = 0;
		  tool_avail_bak[TOOL_REDO] = 0;

		  want_new_tool = 1;
		}
	    }
  
  
	  update_canvas(0, 0, WINDOW_WIDTH - 96 - 96, 48 * 7 + 40 + HEIGHTOFFSET);
	}


      /* Clean up: */

      free_surface_array(thumbs, num_files);

      free(thumbs);
  
      for (i = 0; i < num_files; i++)
	{
	  free(d_names[i]);
	  free(d_exts[i]);
	}

      for (i = 0; i < NUM_PLACES_TO_LOOK; i++)
	free(dirname[i]);

      free(d_names);
      free(d_exts);
      free(d_places);

  return(want_new_tool);
}

/* -------------- Poly Fill Stuff -------------- */

#ifdef SCANLINE_POLY_FILL

static void insert_edge(edge * list, edge * edg)
{
  edge * p, * q;

  debug("insert_edge()");

  q = list;

  p = q->next;
  while (p != NULL)
    {
      if (edg->x_intersect < p->x_intersect)
	{
	  p = NULL;
	}
      else
	{
	  q = p;
	  p = p->next;
	}
    }

  edg->next = q->next;
  q->next = edg;
}


static int y_next(int k, int cnt, point_type * pts)
{
  int j;

  debug("y_next()");

  if ((k + 1) > (cnt - 1))
    j = 0;
  else
    j = k + 1;

  while (pts[k].y == pts[j].y)
    {
      if ((j + 1) > (cnt - 1))
	j = 0;
      else
	j++;
    }

  return (pts[j].y);
}


static void make_edge_rec(point_type lower, point_type upper,
		   int y_comp, edge * edg, edge * edges[])
{
  debug("make_edge_rec()");

  edg->dx_per_scan = (float)((upper.x - lower.x) / (upper.y - lower.y));
  edg->x_intersect = lower.x;

  if (upper.y < y_comp)
    edg->y_upper = upper.y - 1;
  else
    edg->y_upper = upper.y;

  insert_edge(edges[lower.y], edg);
}


static void build_edge_list(int cnt, point_type * pts, edge * edges[])
{
  edge * edg;
  point_type v1, v2;
  int i, y_prev;

  debug("build_edge_list()");

  y_prev = pts[cnt - 2].y;

  v1.x = pts[cnt - 1].x;
  v1.y = pts[cnt - 1].y;

  for (i = 0; i < cnt; i++)
    {
      v2 = pts[i];
      if (v1.y != v2.y)
	{
	  edg = (edge *) malloc(sizeof(edge));
	  if (v1.y < v2.y)
	    make_edge_rec(v1, v2, y_next(i, cnt, pts), edg, edges);
	  else
	    make_edge_rec(v2, v1, y_prev, edg, edges);
	}

      y_prev = v1.y;
      v1 = v2;
    }
}


static void build_active_list(int scan, edge * active, edge * edges[])
{
  edge * p, * q;

  debug("build_active_list()");

  p = edges[scan]->next;
  while (p != NULL)
    {
      q = p->next;
      insert_edge(active, p);
      p = q;
    }
}


static void fill_scan(int scan, edge * active)
{
  edge * p1, * p2;
  int i;
  Uint32 color;


  debug("fill_scan()");

  color = SDL_MapRGB(canvas->format,
		     color_hexes[cur_color][0] / 2,
		     color_hexes[cur_color][1] / 2,
		     color_hexes[cur_color][2] / 2);

  SDL_LockSurface(canvas);

  p1 = active->next;
  while (p1 != NULL)
    {
      p2 = p1->next;

      for (i = p1->x_intersect; i < p2->x_intersect; i++)
	{
	  putpixel(canvas, i, scan, color);
	}

      p1 = p2->next;
    }

  SDL_UnlockSurface(canvas);
}


static void delete_after(edge * q)
{
  edge * p;

  debug("delete_after()");

  p = q->next;
  q->next = p->next;
  free(p);
}


static void update_active_list(int scan, edge * active)
{
  edge * q, * p;

  debug("update_active_list()");

  q = active;
  p = active->next;

  while (p != NULL)
    {
      if (scan >= p->y_upper)
	{
	  p = p->next;
	  delete_after(q);
	}
      else
	{
	  p->x_intersect = p->x_intersect + p->dx_per_scan;
	  q = p;
	  p = p->next;
	}
    }
}


static void resort_active_list(edge * active)
{
  edge * q, * p;

  debug("resort_active_list()");

  p = active->next;

  active->next = NULL;

  while (p != NULL)
    {
      q = p->next;
      insert_edge(active, p);
      p = q;
    }
}


static void scan_fill(int cnt, point_type * pts)
{
  /*  edge * edges[48 * 7 + 40 + HEIGHTOFFSET + 5], * active; */
  edge * * edges = alloca((48 * 7 + 40 + HEIGHTOFFSET + 5) * sizeof(edge*)),
    * active;
  int i, scan;

  debug("scan_fill()");


  /* Create empty edges: */

  for (i = 0; i < 48 * 7 + 40 + HEIGHTOFFSET + 5; i++)
    {
      edges[i] = (edge *) malloc(sizeof(edge));
      edges[i]->next = NULL;
    }


  /* Build edge list: */

  build_edge_list(cnt, pts, edges);


  /* Set active edge: */

  active = (edge *) malloc(sizeof(edge));
  active->next = NULL;


  /* Scan! */

  for (scan = 0; scan < 48 * 7 + 40 + HEIGHTOFFSET; scan++)
    {
      build_active_list(scan, active, edges);

      if (active->next)
	{
	  fill_scan(scan, active);

	  update_canvas(0, scan, WINDOW_WIDTH - 96, scan);
	  SDL_Flip(screen);
	  SDL_Delay(10);

	  update_active_list(scan, active);
	  resort_active_list(active);
	}
    }


  /* Free edge list: */

  debug("Freeing...");

  for (i = 0; i < 48 * 7 + 40 + HEIGHTOFFSET; i++)
    {
      free(edges[i]);
    }
}


/* ------------- Poly clipping stuff: -------------- */

static int inside(fpoint_type p, an_edge b)
{
  if (b == Left)
    {
      if (p.x < 0)
	return 0;
    }
  else if (b == Right)
    {
      if (p.x >= WINDOW_WIDTH - 96)
	return 0;
    }
  else if (b == Bottom)
    {
      if (p.y >= 48 * 7 + 40 + HEIGHTOFFSET)
	return 0;
    }
  else if (b == Top)
    {
      if (p.y < 0)
	return 0;
    }

  return 1;
}


static int cross(fpoint_type p1, fpoint_type p2, an_edge b)
{
  if (inside(p1, b) == inside(p2, b))
    return 0;
  else
    return 1;
}


static fpoint_type intersect(fpoint_type p1, fpoint_type p2, an_edge b)
{
  fpoint_type ipt;
  float m;

  if (p1.x != p2.x)
    m = (p1.y - p2.y) / (p1.x - p2.x);
  else
    m = 1.0;

  if (b == Left)
    {
      ipt.x = 0;
      ipt.y = p2.y + (-p2.x) * m;
    }
  else if (b == Right)
    {
      ipt.x = WINDOW_WIDTH - 96 - 1;
      ipt.y = p2.y + ((WINDOW_WIDTH - 96 - 1) - p2.x) * m;
    }
  else if (b == Top)
    {
      ipt.y = 0;
      if (p1.x != p2.x)
	ipt.x = p2.x + (-p2.y) / m;
      else
	ipt.x = p2.x;
    }
  else if (b == Bottom)
    {
      ipt.y = (48 * 7 + 40 + HEIGHTOFFSET) - 1;
      if (p1.x != p2.x)
	ipt.x = p2.x + (((48 * 7 + 40 + HEIGHTOFFSET) - 1) - p2.y) / m;
      else
	ipt.x = p2.x;
    }

  return(ipt);
}


static void clip_point(fpoint_type p, an_edge b, fpoint_type * pout, int * cnt,
		fpoint_type * first[], fpoint_type * s)
{
  fpoint_type ipt;

  if (first[b] == NULL)
    {
      first[b] = &p;
    }
  else
    {
      if (cross(p, s[b], b))
	{
	  ipt = intersect(p, s[b], b);

	  if (b < Top)  /* Should be NUM_EDGES? */
	    {
	      clip_point(ipt, b + 1, pout, cnt, first, s);
	    }
	  else
	    {
	      pout[*cnt] = ipt;
	      (*cnt)++;
	    }
	}
    }

  s[b] = p;


  if (inside(p, b))
    {
      if (b < Top)  /* Should be NUM_EDGES? */
	{
	  clip_point(p, b + 1, pout, cnt, first, s);
	}
      else
	{
	  pout[*cnt] = p;
	  (*cnt)++;
	}
    }
}


static void close_clip(fpoint_type * pout, int * cnt, fpoint_type * first[],
		fpoint_type * s)
{
  fpoint_type i;
  an_edge b;

  for (b = Left; b <= Top; b++)
    {
      if (cross(s[b], *first[b], b))
	{
	  i = intersect(s[b], *first[b], b);
	  if (b < Top)
	    {
	      clip_point(i, b + 1, pout, cnt, first, s);
	    }
	  else
	    {
	      pout[*cnt] = i;
	      (*cnt)++;
	    }
	}
    }
}


static int clip_polygon(int n, fpoint_type * pin, fpoint_type * pout)
{
  fpoint_type * first[NUM_EDGES] = {0, 0, 0, 0};
  fpoint_type s[NUM_EDGES];
  int i, cnt;

  cnt = 0;

  for (i = 0; i < n; i++)
    {
      clip_point(pin[i], Left, pout, &cnt, first, s);
    }

  close_clip(pout, &cnt, first, s);

  return(cnt);
}

#endif


/* Let sound effects (e.g., "Save" sfx) play out before quitting... */

static void wait_for_sfx(void)
{
#ifndef NOSOUND
  if (use_sound)
    {
      while (Mix_Playing(-1))
	SDL_Delay(10);
    }
#endif
}


/* Determine the current language/locale, and set the language string: */

static int current_language(void)
{
  char * loc;
#ifdef WIN32
  char str[128];
#endif
  int lang, i, found;


  /* Default... */

  lang = LANG_EN;


#ifndef WIN32
  loc = setlocale(LC_MESSAGES, NULL);
  if (loc != NULL)
    {
      if (strstr(loc, "LC_MESSAGES") != NULL)
	loc = getenv("LANG");
    }
#else
  loc = getenv("LANGUAGE");
  if (!loc)
    {
      loc = g_win32_getlocale();
      if (loc)
	{
	  snprintf(str, sizeof(str), "LANGUAGE=%s", loc);
	  putenv(str);
	}
    }
#endif

  debug(loc);

  if (loc != NULL)
    {
      /* Which, if any, of the locales is it? */

      found = 0;
      
      for (i = 0; i < NUM_LANGS && found == 0; i++)
	{
	  /* Case-insensitive */
	  /* (so that, e.g. "pt_BR" is recognized as "pt_br") */
      
	  if (strncasecmp(loc, lang_prefixes[i], strlen(lang_prefixes[i])) == 0)
	  /* if (strcasecmp(loc, lang_prefixes[i]) == 0) */
	    {
	      lang = i;
	      found = 1;
	    }
	}
    }

#ifdef DEBUG
  printf("lang=%d\n\n", lang);
  sleep(10);
#endif


  return lang;
}



////////////////////////////////////////////////////////////
// stamp outline
#ifndef LOW_QUALITY_STAMP_OUTLINE
/* XOR-based outline of rubber stamp shapes
   (unused if LOW_QUALITY_STAMP_OUTLINE is #defined) */

#if 1
#define STIPLE_W 5
#define STIPLE_H 5
static char stiple[] =
"84210"
"10842"
"42108"
"08421"
"21084"
;
#endif

#if 0
#define STIPLE_W 4
#define STIPLE_H 4
static char stiple[] =
"8000"
"0800"
"0008"
"0080"
;
#endif

#if 0
#define STIPLE_W 12
#define STIPLE_H 12
static char stiple[] =
"808808000000"
"800000080880"
"008088080000"
"808000000808"
"000080880800"
"088080000008"
"000000808808"
"080880800000"
"080000008088"
"000808808000"
"880800000080"
"000008088080"
;
#endif

static unsigned char *stamp_outline_data;
static int stamp_outline_w, stamp_outline_h;

static void update_stamp_xor(void)
{
  int xx, yy, rx, ry;
  Uint8 dummy;
  SDL_Surface * src;

  /* Use pre-mirrored stamp image, if applicable: */

  if (state_stamps[cur_stamp]->mirrored &&
      img_stamps_premirror[cur_stamp] != NULL)
    {
      src = img_stamps_premirror[cur_stamp];
    }
  else
    {
      src = img_stamps[cur_stamp];
    }

  // start by scaling
  src = thumbnail(src, CUR_STAMP_W, CUR_STAMP_H, 0);

  unsigned char *alphabits = calloc(src->w+4, src->h+4);

  SDL_LockSurface(src);
  for (yy = 0; yy < src->h; yy++)
    {
      /* Compensate for flip! */
      if (state_stamps[cur_stamp]->flipped)
	ry = src->h - 1 - yy;
      else
	ry = yy;
      for (xx = 0; xx < src->w; xx++)
	{
	  /* Compensate for mirror! */
	  if (state_stamps[cur_stamp]->mirrored &&
	      img_stamps_premirror[cur_stamp] == NULL)
	    {
	      rx = src->w - 1 - xx;
	    }
	  else
	    {
	      rx = xx;
	    }

	  SDL_GetRGBA(getpixel(src, rx, ry),
		      src->format, &dummy, &dummy, &dummy, alphabits + xx+2 + (yy+2)*(src->w+4));
	}
    }
  SDL_UnlockSurface(src);

  int new_w = src->w+4;
  int new_h = src->h+4;
  SDL_FreeSurface(src);
  unsigned char *outline   = calloc(new_w, new_h);

  for (yy = 1; yy < new_h-1; yy++)
    {
      for (xx = 1; xx < new_w-1; xx++)
	{
          unsigned char above = 0;
          unsigned char below = 0xff;
          unsigned char tmp;

	  tmp = alphabits[(xx-1) + (yy-1)*new_w];
	  above |= tmp;
	  below &= tmp;
	  tmp = alphabits[(xx+1) + (yy-1)*new_w];
	  above |= tmp;
	  below &= tmp;

	  tmp = alphabits[(xx+0) + (yy-1)*new_w];
	  above |= tmp;
	  below &= tmp;
	  tmp = alphabits[(xx+0) + (yy+0)*new_w];
	  above |= tmp;
	  below &= tmp;
	  tmp = alphabits[(xx+1) + (yy+0)*new_w];
	  above |= tmp;
	  below &= tmp;
	  tmp = alphabits[(xx-1) + (yy+0)*new_w];
	  above |= tmp;
	  below &= tmp;
	  tmp = alphabits[(xx+0) + (yy+1)*new_w];
	  above |= tmp;
	  below &= tmp;

	  tmp = alphabits[(xx-1) + (yy+1)*new_w];
	  above |= tmp;
	  below &= tmp;
	  tmp = alphabits[(xx+1) + (yy+1)*new_w];
	  above |= tmp;
	  below &= tmp;

	  outline[xx + yy*new_w] = (above^below)>>7;
	}
    }

    char *old_outline_data = stamp_outline_data;
    SDL_LockSurface(screen); // abuse this lock until I determine the correct need
    stamp_outline_data = outline;
    stamp_outline_w = new_w;
    stamp_outline_h = new_h;
    SDL_UnlockSurface(screen);
    if (old_outline_data)
      free(old_outline_data);
    free(alphabits);
}

static void stamp_xor(int x, int y)
{
  int xx, yy, sx, sy;

  SDL_LockSurface(screen);
  for (yy = 0; yy < stamp_outline_h; yy++)
    {
      for (xx = 0; xx < stamp_outline_w; xx++)
	{
          if(!stamp_outline_data[xx + yy*stamp_outline_w])
            continue;
          sx = 96 + x + xx - stamp_outline_w/2;
          sy =      y + yy - stamp_outline_h/2;
          if (stiple[sx%STIPLE_W + sy%STIPLE_H * STIPLE_W] != '8')
            continue;
          clipped_putpixel(screen, sx, sy, 0xFFFFFFFF - getpixel(screen, sx, sy));
	}
    }
  SDL_UnlockSurface(screen);
}

#endif
///////////////////////////////////////////////////

/* Returns whether a particular stamp can be colored: */

static int stamp_colorable(int stamp)
{
  if (inf_stamps[stamp] != NULL)
    {
      return inf_stamps[stamp]->colorable;
    }
  else
    {
      return 0;
    }
}


/* Returns whether a particular stamp can be tinted: */

static int stamp_tintable(int stamp)
{
  if (inf_stamps[stamp] != NULL)
    {
      return inf_stamps[stamp]->tintable;
    }
  else
    {
      return 0;
    }
}


static void rgbtohsv(Uint8 r8, Uint8 g8, Uint8 b8, float *h, float *s, float *v)
{
  float rgb_min, rgb_max, delta, r, g, b;
  
  r = (r8 / 255.0);
  g = (g8 / 255.0);
  b = (b8 / 255.0);
  
  rgb_min = min(r, min(g, b));
  rgb_max = max(r, max(g, b));
  *v = rgb_max;
  
  delta = rgb_max - rgb_min;

  if (rgb_max == 0)
    {
      /* Black */
      
      *s = 0;
      *h = -1;
    }
  else
    {
      *s = delta / rgb_max;
      
      if (r == rgb_max)
	*h = (g - b) / delta;
      else if (g == rgb_max)
	*h = 2 + (b - r) / delta;     /* between cyan & yellow */
      else
	*h = 4 + (r - g) / delta;     /* between magenta & cyan */
      
      *h = (*h * 60);                 /* degrees */
      
      if (*h < 0)
	*h = (*h + 360);
    }
}


static void hsvtorgb(float h, float s, float v, Uint8 *r8, Uint8 *g8, Uint8 *b8)
{
  int i;
  float f, p, q, t, r, g, b;
  
  if (s == 0)
    {
      /* Achromatic (grey) */
      
      r = v;
      g = v;
      b = v;
    }
  else
    {
      h = h / 60;
      i = floor(h);
      f = h - i;
      p = v * (1 - s);
      q = v * (1 - s * f);
      t = v * (1 - s * (1 - f));
      
      if (i == 0)
	{
	  r = v;
	  g = t;
	  b = p;
	}
      else if (i == 1)
	{
	  r = q;
	  g = v;
	  b = p;
	}
      else if (i == 2)
	{
	  r = p;
	  g = v;
	  b = t;
	}
      else if (i == 3)
	{
	  r = p;
	  g = q;
	  b = v;
	}
      else if (i == 4)
	{
	  r = t;
	  g = p;
	  b = v;
	}
      else
	{
	  r = v;
	  g = p;
	  b = q;
	}
    }

  
  *r8 = (Uint8) (r * 255);
  *g8 = (Uint8) (g * 255);
  *b8 = (Uint8) (b * 255);
}


static void show_progress_bar(void)
{
  SDL_Rect dest, src;
  int x;


  for (x = 0; x < WINDOW_WIDTH; x = x + 65)
    {
      src.x = 65 - (prog_bar_ctr % 65);
      src.y = 0;
      src.w = 65;
      src.h = 24;

      dest.x = x;
      dest.y = WINDOW_HEIGHT - 24;
  
      SDL_BlitSurface(img_progress, &src, screen, &dest);
    }

  prog_bar_ctr++;

  SDL_UpdateRect(screen, 0, WINDOW_HEIGHT - 24, WINDOW_WIDTH, 24);
}


static void do_print(void)
{
#if !defined(WIN32) && !defined(__BEOS__) && !defined(__APPLE__)
  /* Linux, Unix, etc. */

  FILE * pi;

  pi = popen(printcommand, "w");

  if (pi == NULL)
    {
      perror(printcommand);
    }
  else
    {
      if (do_png_save(pi, printcommand, canvas))
	do_prompt(PROMPT_PRINT_TXT, PROMPT_PRINT_YES, "");
    }
#else
#ifdef WIN32
  /* Win32 */

  char  f[512];
  int   show = (SDL_GetModState() & KMOD_ALT) && !fullscreen;

  snprintf(f, sizeof(f), "%s/%s", savedir, "print.cfg");

  {
    const char   *error = SurfacePrint(canvas, use_print_config?f:NULL, show);

    if ( error ) fprintf(stderr, "%s\n", error);
  }
#elif defined(__BEOS__)
  /* BeOS */
  
  SurfacePrint(canvas);
#elif defined(__APPLE__)
  /* Mac OS X */

  int show = (SDL_GetModState() & KMOD_ALT) && !fullscreen;

  const char* error = SurfacePrint (canvas, show);

  if (error)
    fprintf (stderr, "Cannot print: %s\n", error);
  else
    do_prompt (PROMPT_PRINT_TXT, PROMPT_PRINT_YES, "");
#endif
  
#endif
}


static void do_render_cur_text(int do_blit)
{
  int w, h;
  SDL_Color color = {color_hexes[cur_color][0],
	             color_hexes[cur_color][1],
      	             color_hexes[cur_color][2],
      	             0};
  SDL_Surface * tmp_surf;
  SDL_Rect dest, src;
  char * str;
    

  /* Keep cursor on the screen! */

  if (cursor_y > ((48 * 7 + 40 + HEIGHTOFFSET) -
		  TTF_FontHeight(fonts[cur_font])))
    {
      cursor_y = ((48 * 7 + 40 + HEIGHTOFFSET) -
		  TTF_FontHeight(fonts[cur_font]));
    }
  
  
  /* Render the text: */

  if (texttool_len > 0)
    {
      str = uppercase(texttool_str);
    
      tmp_surf = TTF_RenderUTF8_Blended(fonts[cur_font], str, color);

      w = tmp_surf->w;
      h = tmp_surf->h;
    
      cursor_textwidth = w;

      free(str);
    }
  else
    {
      /* FIXME: Do something different! */

      update_canvas(0, 0, WINDOW_WIDTH - 96, (48 * 7) + 40 + HEIGHTOFFSET);
      cursor_textwidth = 0;
      return;
    }


  if (!do_blit)
    {
      /* FIXME: Only delete what's changed! */

      update_canvas(0, 0, WINDOW_WIDTH - 96, (48 * 7) + 40 + HEIGHTOFFSET);


      /* Draw outline around text: */

      dest.x = cursor_x - 2 + 96;
      dest.y = cursor_y - 2;
      dest.w = w + 4;
      dest.h = h + 4;
    
      if (dest.x + dest.w > WINDOW_WIDTH - 96)
	dest.w = WINDOW_WIDTH - 96 - dest.x;
      if (dest.y + dest.h > (48 * 7 + 40 + HEIGHTOFFSET))
	dest.h = (48 * 7 + 40 + HEIGHTOFFSET) - dest.y;

      SDL_FillRect(screen, &dest,
		   SDL_MapRGB(canvas->format, 0, 0, 0));


      /* FIXME: This would be nice if it were alpha-blended: */

      dest.x = cursor_x + 96;
      dest.y = cursor_y;
      dest.w = w;
      dest.h = h;

      if (dest.x + dest.w > WINDOW_WIDTH - 96)
	dest.w = WINDOW_WIDTH - 96 - dest.x;
      if (dest.y + dest.h > (48 * 7 + 40 + HEIGHTOFFSET))
	dest.h = (48 * 7 + 40 + HEIGHTOFFSET) - dest.y;

      if ((color_hexes[cur_color][0] +
	   color_hexes[cur_color][1] +
	   color_hexes[cur_color][2]) >= 384)
	{
	  /* Grey background if blit is white!... */

	  SDL_FillRect(screen, &dest,
		       SDL_MapRGB(canvas->format, 64, 64, 64));
	}
      else
	{
	  /* White background, normally... */

	  SDL_FillRect(screen, &dest,
		       SDL_MapRGB(canvas->format, 255, 255, 255));
	}
    }
  

  /* Draw the text itself! */

  if (tmp_surf != NULL)
    {
      dest.x = cursor_x;
      dest.y = cursor_y;

      src.x = 0;
      src.y = 0;
      src.w = tmp_surf->w;
      src.h = tmp_surf->h;

      if (dest.x + src.w > WINDOW_WIDTH - 96 - 96)
	src.w = WINDOW_WIDTH - 96 - 96 - dest.x;
      if (dest.y + src.h > (48 * 7 + 40 + HEIGHTOFFSET))
	src.h = (48 * 7 + 40 + HEIGHTOFFSET) - dest.y;

      if (do_blit)
	{
	  SDL_BlitSurface(tmp_surf, &src, canvas, &dest);
	  update_canvas(dest.x, dest.y, dest.x + tmp_surf->w, dest.y + tmp_surf->h);
	}
      else
	{
	  dest.x = dest.x + 96;
	  SDL_BlitSurface(tmp_surf, &src, screen, &dest);
	}
    }
  
  
  /* FIXME: Only update what's changed! */
  
  SDL_Flip(screen);


  if (tmp_surf != NULL)
    SDL_FreeSurface(tmp_surf);
}


static void loadfonts(const char * const dir, int fatal)
{
  DIR * d;
  struct dirent * f;
  struct stat sbuf;
  char fname[512];
  int d_names_alloced;
  char * * d_names;
  int num_files, i;


  /* Open the directory: */

  d = opendir(dir);
  if (d == NULL)
    {
      if (fatal)
	{
	  fprintf(stderr,
		  "\nError: I can't find a directory of fonts\n"
		  "%s\n"
		  "The system error that occurred was: %s\n",
		  dir, strerror(errno));

	  cleanup();
	  exit(1);
	}
      else
	return;
    }


  /* Make some space: */

  d_names_alloced = 32;
  d_names = (char * *) malloc(sizeof(char *) * d_names_alloced);
  if (d_names == NULL)
    {
      fprintf(stderr,
	      "\nError: I can't allocate memory for directory listing!\n"
	      "The system error that occurred was: %s\n",
	      strerror(errno));
      cleanup();
      exit(1);
    }



  /* Read directory for images: */

  num_files = 0;
  do
    {
      f = readdir(d);

      if (f != NULL)
	{
	  d_names[num_files] = strdup(f->d_name);
	  num_files++;

	  if (num_files >= d_names_alloced)
	    {
	      d_names_alloced = d_names_alloced + 32;
	      d_names = (char * *) realloc(d_names, sizeof(char *) * d_names_alloced);

	      if (d_names == NULL)
		{
		  fprintf(stderr,
			  "\nError: I can't allocate memory for directory listing!\n"
			  "The system error that occurred was: %s\n",
			  strerror(errno));
		  cleanup();
		  exit(1);
		}
	    }
	}
    }
  while (f != NULL);

  closedir(d);


  qsort(d_names, num_files, sizeof(char *),
	(int(*)(const void *, const void *))compare_strings);


  /* Do something with each file (load TTFs): */

  for (i = 0; i < num_files && num_fonts + 3 < MAX_FONTS; i++)
    {
      /* Ignore things starting with "." (e.g., "." and ".." dirs): */

      if (strstr(d_names[i], ".") != d_names[i])
	{
	  /* If it's a directory, recurse down into it: */

	  snprintf(fname, sizeof(fname), "%s/%s", dir, d_names[i]);
	  debug(fname);
	
	  stat(fname, &sbuf);
	
	  if (strstr(d_names[i], ".ttf") != NULL)
	    {
	      /* If it has ".ttf" in the filename, assume we can try to load it: */
	
	      fonts[num_fonts++] = TTF_OpenFont(fname, 16);
	      fonts[num_fonts++] = TTF_OpenFont(fname, 24);
	      fonts[num_fonts++] = TTF_OpenFont(fname, 32);
	      fonts[num_fonts++] = TTF_OpenFont(fname, 48);
	  
	      show_progress_bar();
	    }
	}

      free(d_names[i]);
    }


  /* Give warning if too many files were found (e.g., some not loaded): */

  if (num_fonts == MAX_FONTS)
    {
      fprintf(stderr,
	      "\nWarning: Reached maximum fonts (%d) which can be loaded.\n\n",
	      MAX_FONTS);
    }
}


/* Return string as uppercase if that option is set: */

#ifdef OLD_UPPERCASE_CODE

static char * uppercase(char * str)
{
  char * ustr;
  int i;

  ustr = strdup(str);

  if (only_uppercase)
    {
      for (i = 0; i < strlen(ustr); i++)
	ustr[i] = toupper(ustr[i]);
    }
  
#ifdef DEBUG
  printf(" ORIGINAL: %s\n"
         "UPPERCASE: %s\n\n", str, ustr);
#endif

  return (ustr);
}

#else

static char * uppercase(char * str)
{
  unsigned int i, sz;
  wchar_t * dest;
  char * ustr;

  if (only_uppercase)
  {
    sz = sizeof(wchar_t) * (strlen(str) + 1);

    dest = (wchar_t *) malloc(sz);
    ustr = (char *) malloc(sizeof(char) * (strlen(str) + 1));

    if (dest != NULL)
    {
      mbstowcs(dest, str, sz);

      for (i = 0; i < strlen(str); i++)
      {
	dest[i] = towupper(dest[i]);
      }

      wcstombs(ustr, dest, sizeof(char) * (strlen(str) + 1));
      
      free(dest);
    }
  
    printf(" ORIGINAL: %s\n"
           "UPPERCASE: %s\n\n", str, ustr);
  }
  else
  {
    ustr = strdup(str);
  }

  return(ustr);
}

#endif


/* Return string in right-to-left mode, if necessary: */

static unsigned char * textdir(const unsigned char * const str)
{
  unsigned char * dstr;
  unsigned i, j;

#ifdef DEBUG
  printf("ORIG_DIR: %s\n", str);
#endif

  dstr = (unsigned char *) malloc((strlen(str) + 5) * sizeof(unsigned char));

  if (need_right_to_left(language))
    {
      dstr[strlen(str)] = '\0';

      for (i = 0; i < strlen(str); i++)
	{
	  j = (strlen(str) - i - 1);
      
	  if (str[i] < 128)  /* 0xxx xxxx - 1 byte */
	    {
	      dstr[j] = str[i];
	    }
	  else if ((str[i] & 0xE0) == 0xC0)  /* 110x xxxx - 2 bytes */
	    {
	      dstr[j - 1] = str[i + 0];
	      dstr[j - 0] = str[i + 1];
	      i = i + 1;
	    }
	  else if ((str[i] & 0xF0) == 0xE0)  /* 1110 xxxx - 3 bytes */
	    {
	      dstr[j - 2] = str[i + 0];
	      dstr[j - 1] = str[i + 1];
	      dstr[j - 0] = str[i + 2];
	      i = i + 2;
	    }
	  else  /* 1111 0xxx - 4 bytes */
	    {
	      dstr[j - 3] = str[i + 0];
	      dstr[j - 2] = str[i + 1];
	      dstr[j - 1] = str[i + 2];
	      dstr[j - 0] = str[i + 3];
	      i = i + 3;
	    }
	}
    }
  else
    {
      strcpy(dstr, str);
    }

#ifdef DEBUG
  printf("L2R_DIR: %s\n", dstr);
#endif

  return (dstr);
}


/* For flood fill... */

static int colors_close(Uint32 c1, Uint32 c2)
{
#ifdef LOW_QUALITY_FLOOD_FILL
  return (c1 == c2);
#else
  Uint8 r1, g1, b1,
    r2, g2, b2;

  if (c1 == c2)
    {
      /* Get it over with quick, if possible! */

      return 1;
    }
  else
    {
      SDL_GetRGB(c1, canvas->format, &r1, &g1, &b1);
      SDL_GetRGB(c2, canvas->format, &r2, &g2, &b2);

      if (abs(r1 - r2) <= 64 && 
	  abs(g1 - g2) <= 64 &&
	  abs(b1 - b2) <= 64)
	return 1;
      else
	return 0;
    }
#endif
}


/* Flood fill! */

static void do_flood_fill(int x, int y, Uint32 cur_colr, Uint32 old_colr)
{
  int fillL, fillR, i, in_line;
  static unsigned char prog_anim;


  if (cur_colr == old_colr ||
      colors_close(cur_colr, old_colr))
    return;

  
  fillL = x;
  fillR = x;
  
  prog_anim++;
  if ((prog_anim % 4) == 0)
    {
      show_progress_bar();
      playsound(0, SND_BUBBLE, 0);
    }


  /* Find left side, filling along the way */
  
  in_line = 1;
  
  while (in_line)
    {
      putpixel(canvas, fillL, y, cur_colr);
      fillL--;

      in_line = (fillL < 0) ? 0 : colors_close(getpixel(canvas, fillL, y),
					       old_colr);
    }
  
  fillL++;

  
  /* Find right side, filling along the way */

  in_line = 1;
  while (in_line)
    {
      putpixel(canvas, fillR, y, cur_colr);
      fillR++;
    
      in_line = (fillR >= canvas->w) ? 0 : colors_close(getpixel(canvas,
								 fillR, y),
							old_colr);
    }
  
  fillR--;

  
  /* Search top and bottom */
  
  for (i = fillL; i <= fillR; i++)
    {
      if (y > 0 && colors_close(getpixel(canvas, i, y - 1), old_colr))
	do_flood_fill(i, y - 1, cur_colr, old_colr);

      if (y < canvas->h && colors_close(getpixel(canvas, i, y + 1), old_colr))
	do_flood_fill(i, y + 1, cur_colr, old_colr);
    }
}


/* Scroll Timer */

static Uint32 scrolltimer_callback(Uint32 interval, void *param)
{
  SDL_PushEvent((SDL_Event*)param);

  return interval;
}


/* Controls the Text-Timer - interval == 0 removes the timer */

static void control_drawtext_timer(Uint32 interval, const char * const text)
{
  static int activated = 0;
  static SDL_TimerID TimerID = 0;
  static SDL_Event drawtext_event;

  
  /* Remove old timer if any is running */
  
  if (activated)
    {
      SDL_RemoveTimer(TimerID);
      activated = 0;
      TimerID = 0;
    }

  if (interval == 0)
    return;
  
  drawtext_event.type = SDL_USEREVENT;
  drawtext_event.user.code = USEREVENT_TEXT_UPDATE;
  drawtext_event.user.data1 = (void*) text;

  
  /* Add new timer */
  
  TimerID = SDL_AddTimer(interval, drawtext_callback, (void*) &drawtext_event);
  activated = 1;
}


/* Drawtext Timer */

static Uint32 drawtext_callback(Uint32 interval, void *param)
{
  (void)interval;
  SDL_PushEvent((SDL_Event*)param);

  return 0; /* Remove timer */
}


static void parse_options(FILE * fi)
{
  char str[256];
  
  do
    {
      fgets(str, sizeof(str), fi);

      strip_trailing_whitespace(str);

      if (!feof(fi))
	{
	  debug(str);
      
      
	  /* Should "lang=" and "locale=" be here as well???
	     Comments welcome ... bill@newbreedsoftware.com */


	  /* FIXME: This should be handled better! */
	  /* (e.g., complain on illegal lines, support comments, blanks, etc.) */

	  if (strcmp(str, "fullscreen=yes") == 0)
	    {
	      fullscreen = 1;
	    }
	  else if (strcmp(str, "fullscreen=no") == 0 ||
		   strcmp(str, "windowed=yes") == 0)
	    {
	      fullscreen = 0;
	    }
	  else if (strcmp(str, "nostampcontrols=yes") == 0)
	    {
	      disable_stamp_controls = 1;
	    }
	  else if (strcmp(str, "nostampcontrols=no") == 0 ||
		   strcmp(str, "stampcontrols=yes") == 0)
	    {
	      disable_stamp_controls = 0;
	    }
	  else if (strcmp(str, "mirrorstamps=yes") == 0)
	    {
	      mirrorstamps = 1;
	    }
	  else if (strcmp(str, "mirrorstamps=no") == 0 ||
		   strcmp(str, "dontmirrorstamps=yes") == 0)
	    {
	      mirrorstamps = 0;
	    }
	  else if (strcmp(str, "noshortcuts=yes") == 0)
	    {
	      noshortcuts = 1;
	    }
	  else if (strcmp(str, "noshortcuts=no") == 0 ||
		   strcmp(str, "shortcuts=yes") == 0)
	    {
	      noshortcuts = 0;
	    }
	  else if (!memcmp("windowsize=",str,11))
	    {
	      char *endp1;
	      char *endp2;
	      int w,h;
	      w = strtoul(str+11,  &endp1, 10);
	      h = strtoul(endp1+1, &endp2, 10);
	      // sanity check it
	      if(str+11==endp1 || endp1+1==endp2 || *endp1!='x' || *endp2 || w<500 || h<480 || h>w*3 || w>h*4)
		{
		  // Oddly, config files have no error checking.
		  //show_usage(stderr, (char *) getfilename(argv[0]));
		  //exit(1);
		}
	      else
	        {
		  WINDOW_WIDTH  = w;
		  WINDOW_HEIGHT = h;
		}
	    }
	  // to handle old config files
	  else if (strcmp(str, "800x600=yes") == 0 ||
		   strcmp(str, "windowsize=800x600") == 0)
	    {
	      WINDOW_WIDTH = 800;
	      WINDOW_HEIGHT = 600;
	    }
	  // also for old config files
	  else if (strcmp(str, "800x600=no") == 0 ||
		   strcmp(str, "640x480=yes") == 0 ||
		   strcmp(str, "windowsize=640x480") == 0)
	    {
	      WINDOW_WIDTH = 640;
	      WINDOW_HEIGHT = 480;
	    }
	  else if (strcmp(str, "nooutlines=yes") == 0)
	    {
	      dont_do_xor = 1;
	    }
	  else if (strcmp(str, "nooutlines=no") == 0 ||
		   strcmp(str, "outlines=yes") == 0)
	    {
	      dont_do_xor = 0;
	    }
	  else if (strcmp(str, "keyboard=yes") == 0)
	    {
	      keymouse = 1;
	    }
	  else if (strcmp(str, "keyboard=no") == 0 ||
		   strcmp(str, "mouse=yes") == 0)
	    {
	      keymouse = 0;
	    }
	  else if (strcmp(str, "nowheelmouse=yes") == 0)
	    {
	      wheely = 0;
	    }
	  else if (strcmp(str, "nowheelmouse=no") == 0 ||
		   strcmp(str, "wheelmouse=yes") == 0)
	    {
	      wheely = 1;
	    }
	  else if (strcmp(str, "grab=yes") == 0)
	    {
	      grab_input = 1;
	    }
	  else if (strcmp(str, "grab=no") == 0 ||
		   strcmp(str, "nograb=yes") == 0)
	    {
	      grab_input = 0;
	    }
	  else if (strcmp(str, "nofancycursors=yes") == 0)
	    {
	      no_fancy_cursors = 1;
	    }
	  else if (strcmp(str, "nofancycursors=no") == 0 ||
		   strcmp(str, "fancycursors=yes") == 0)
	    {
	      no_fancy_cursors = 0;
	    }
	  else if (strcmp(str, "uppercase=yes") == 0)
	    {
	      only_uppercase = 1;
	    }
	  else if (strcmp(str, "uppercase=no") == 0 ||
		   strcmp(str, "mixedcase=yes") == 0)
	    {
	      only_uppercase = 0;
	    }
	  else if (strcmp(str, "noquit=yes") == 0)
	    {
	      disable_quit = 1;
	    }
	  else if (strcmp(str, "noquit=no") == 0 ||
		   strcmp(str, "quit=yes") == 0)
	    {
	      disable_quit = 0;
	    }
	  else if (strcmp(str, "nosave=yes") == 0)
	    {
	      disable_save = 1;
	    }
	  else if (strcmp(str, "nosave=no") == 0 ||
		   strcmp(str, "save=yes") == 0)
	    {
	      disable_save = 0;
	    }
	  else if (strcmp(str, "noprint=yes") == 0)
	    {
	      disable_print = 1;
	    }
	  else if (strcmp(str, "noprint=no") == 0 ||
		   strcmp(str, "print=yes") == 0)
	    {
	      disable_print = 0;
	    }
	  else if (strcmp(str, "nostamps=yes") == 0)
	    {
	      dont_load_stamps = 1;
	    }
	  else if (strcmp(str, "nostamps=no") == 0 ||
		   strcmp(str, "stamps=yes") == 0)
	    {
	      dont_load_stamps = 0;
	    }
	  else if (strcmp(str, "nosound=yes") == 0)
	    {
	      use_sound = 0;
	    }
	  else if (strcmp(str, "nosound=no") == 0 ||
		   strcmp(str, "sound=yes") == 0)
	    {
	      use_sound = 1;
	    }
	  else if (strcmp(str, "simpleshapes=yes") == 0)
	    {
	      simple_shapes = 1;
	    }
	  else if (strcmp(str, "simpleshapes=no") == 0 ||
		   strcmp(str, "complexshapes=yes") == 0)
	    {
	      simple_shapes = 1;
	    }
	  else if (strstr(str, "lang=") == str)
	    {
	      langstr = strdup(str + 5);
#ifdef DEBUG
	      printf("langstr set to: %s\n", langstr);
#endif
	    }
	  else if (strstr(str, "printdelay=") == str)
	    {
	      sscanf(str + 11, "%d", &print_delay);
#ifdef DEBUG
	      printf("Print delay set to %d seconds\n", print_delay);
#endif
	    }
	  else if (strcmp(str, "printcfg=yes") == 0)
	    {
#ifndef WIN32
	      fprintf(stderr, "Note: printcfg option only applies to Windows!\n");
#endif
	      use_print_config = 1;
	    }
	  else if (strcmp(str, "printcfg=no") == 0 ||
		   strcmp(str, "noprintcfg=yes") == 0)
	    {
#ifndef WIN32
	      fprintf(stderr, "Note: printcfg option only applies to Windows!\n");
#endif
	      use_print_config = 0;
	    }
	  else if (strstr(str, "printcommand=") == str)
	    {
	      printcommand = strdup(str + 13);
	    }
	  else if (strcmp(str, "saveover=yes") == 0)
	    {
	      promptless_save = SAVE_OVER_ALWAYS;
	    }
	  else if (strcmp(str, "saveover=ask") == 0)
	    {
	      /* (Default) */

	      promptless_save = SAVE_OVER_PROMPT;
	    }
	  else if (strcmp(str, "saveover=new") == 0)
	    {
	      promptless_save = SAVE_OVER_NO;
	    }
	  else if (strstr(str, "savedir=") == str)
	    {
	      savedir = strdup(str + 8);
	      remove_slash(savedir);

#ifdef DEBUG
	      printf("savedir set to: %s\n", savedir);
#endif
	    }
	}
    }
  while (!feof(fi));
}


#ifdef DEBUG
static char * debug_gettext(const char * str)
{
  if (strcmp(str, dgettext(NULL, str)) == 0)
    {
      printf("NOTRANS: %s\n", str);
      fflush(stdout);
    }

  return(dgettext(NULL, str));
}
#endif


static void do_setcursor(SDL_Cursor * c)
{
  if (!no_fancy_cursors)
    SDL_SetCursor(c);
}


static const char * great_str(void)
{
  return(great_strs[rand() % (sizeof(great_strs) / sizeof(char *))]);
}


#ifdef DEBUG
static int charsize(char c)
{
  Uint16 str[2];
  int w, h;

  str[0] = c;
  str[1] = '\0';

  TTF_SizeUNICODE(fonts[cur_font], str, &w, &h);

  return w;
}
#endif

static void draw_image_title(int t, int x)
{
  SDL_Rect dest;

  dest.x = x;
  dest.y = 0;
  SDL_BlitSurface(img_title_on, NULL, screen, &dest);
  
  dest.x = x + (96 - img_title_names[t]->w) / 2;;
  dest.y = (40 - img_title_names[t]->h) / 2;
  SDL_BlitSurface(img_title_names[t], NULL, screen, &dest);
}



static int need_own_font(int l)
{
  int i, need;

  need = 0;

  for (i = 0; lang_use_own_font[i] != -1 && need == 0; i++)
    {
      if (lang_use_own_font[i] == l)
	{
	  need = 1;
	}
    }

  return need;
}


static int need_right_to_left(int l)
{
  int i, need;

  need = 0;

  for (i = 0; lang_use_right_to_left[i] != -1 && need == 0; i++)
    {
      if (lang_use_right_to_left[i] == l)
	{
	  need = 1;
	}
    }

  return need;
}



/* Handle keyboard events to control the mouse: */

static void handle_keymouse(SDLKey key, Uint8 updown)
{
  SDL_Event event;

  if (keymouse)
    {
      if (key == SDLK_LEFT)
	mousekey_left = updown;
      else if (key == SDLK_RIGHT)
	mousekey_right = updown;
      else if (key == SDLK_UP)
	mousekey_up = updown;
      else if (key == SDLK_DOWN)
	mousekey_down = updown;
      else if (key == SDLK_SPACE)
	{
	  if (updown == SDL_KEYDOWN)
	    event.type = SDL_MOUSEBUTTONDOWN;
	  else
	    event.type = SDL_MOUSEBUTTONUP;
      
	  event.button.x = mouse_x;
	  event.button.y = mouse_y;
	  event.button.button = 1;

	  SDL_PushEvent(&event);
	}

      if (mousekey_up == SDL_KEYDOWN && mouse_y > 0)
	mouse_y = mouse_y - 8;
      else if (mousekey_down == SDL_KEYDOWN && mouse_y < WINDOW_HEIGHT - 1)
	mouse_y = mouse_y + 8;
    
      if (mousekey_left == SDL_KEYDOWN && mouse_x > 0)
	mouse_x = mouse_x - 8;
      if (mousekey_right == SDL_KEYDOWN && mouse_x < WINDOW_WIDTH - 1)
	mouse_x = mouse_x + 8;

      SDL_WarpMouse(mouse_x, mouse_y);
    }
}


/* Unblank screen in fullscreen mode, if needed: */

static void handle_active( SDL_Event *event )
{
  if (event->active.state & SDL_APPACTIVE)
    {
      if (event->active.gain == 1 )
	{
	  if ( fullscreen )
	    SDL_Flip(screen);
	}
    }
}


/* removes a single '\' or '/' from end of path */

static char *remove_slash( char *path )
{
  int len = strlen(path);

  if (!len)
    return path;

  if (path[len-1] == '/' || path[len-1] == '\\')
    path[len-1] = 0;

  return path;
}


/* For right-to-left languages, when word-wrapping, we need to
   make sure the text doesn't end up going from bottom-to-top, too! */

static void anti_carriage_return(int left, int right, int cur_top, int new_top,
		          int cur_bot, int line_width)
{
  SDL_Rect src, dest;


  /* Move current set of text down one line (and right-justify it!): */
  
  src.x = left;
  src.y = cur_top;
  src.w = line_width;
  src.h = cur_bot - cur_top;

  dest.x = right - line_width;
  dest.y = new_top;

  SDL_BlitSurface(screen, &src, screen, &dest);


  /* Clear the top line for new text: */

  dest.x = left;
  dest.y = cur_top;
  dest.w = right - left;
  dest.h = new_top - cur_top;
  
  SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format, 255, 255, 255));
}


static int mySDL_WaitEvent(SDL_Event *event)
{
  int ret;

  
  if (playing)
    {
      if (!feof(demofi))
	{
	  ret = 1;
	  fread(event, sizeof(SDL_Event), 1, demofi);
	}
      else
	{
	  /* All done!  Back to normal! */

	  printf("(Done playing playback file '%s')\n", playfile);

	  ret = 0;
	  playing = 0;
	}
    }
  else
    {
      ret = SDL_WaitEvent(event);

      if (recording)
	{
	  fwrite(event, sizeof(SDL_Event), 1, demofi);
	}
    }

  return ret;
}


static int mySDL_PollEvent(SDL_Event *event)
{
  int ret;

  if (playing)
    {
      if (!feof(demofi))
	{
	  ret = 1;
	  fread(event, sizeof(SDL_Event), 1, demofi);
	}
      else
	{
	  /* All done!  Back to normal! */

	  printf("(Done playing playback file '%s')\n", playfile);

	  ret = 0;
	  playing = 0;
	}
    }
  else
    {
      ret = SDL_PollEvent(event);

      if (recording && ret > 0)
	{
	  fwrite(event, sizeof(SDL_Event), 1, demofi);
	}
    }

  return ret;
}


static TTF_Font *try_alternate_font(int language)
{
  char  str[128];
  char  prefix[64];
  char  *p;

  strcpy(prefix, lang_prefixes[language]);
  if ((p = strrchr(prefix, '_')) != NULL)
  {
    *p = 0;
    snprintf(str, sizeof(str), "%sfonts/locale/%s.ttf",
             DATA_PREFIX, prefix);

    return TTF_OpenFont(str, 18);
  }
  return NULL;
}


static SDL_Surface * duplicate_surface(SDL_Surface * orig)
{
/*
  Uint32 amask;

  amask = ~(orig->format->Rmask |
	    orig->format->Gmask |
	    orig->format->Bmask);
  
  return(SDL_CreateRGBSurface(SDL_SWSURFACE,
			      orig->w, orig->h,
			      orig->format->BitsPerPixel,
			      orig->format->Rmask,
			      orig->format->Gmask,
			      orig->format->Bmask,
			      amask));
*/

  return(SDL_DisplayFormatAlpha(orig));
}

static void mirror_starter(void)
{
  SDL_Surface * orig;
  int x;
  SDL_Rect src, dest;

  
  /* Mirror overlay: */

  orig = img_starter;
  img_starter = duplicate_surface(orig);

  if (img_starter != NULL)
  {
    for (x = 0; x < orig->w; x++)
    {
      src.x = x;
      src.y = 0;
      src.w = 1;
      src.h = orig->h;

      dest.x = orig->w - x - 1;
      dest.y = 0;

      SDL_BlitSurface(orig, &src, img_starter, &dest);
    }

    SDL_FreeSurface(orig);
  }
  else
  {
    img_starter = orig;
  }


  /* Mirror background: */

  if (img_starter_bkgd != NULL)
  {
    orig = img_starter_bkgd;
    img_starter_bkgd = duplicate_surface(orig);

    if (img_starter_bkgd != NULL)
    {
      for (x = 0; x < orig->w; x++)
      {
        src.x = x;
        src.y = 0;
        src.w = 1;
        src.h = orig->h;

        dest.x = orig->w - x - 1;
        dest.y = 0;

        SDL_BlitSurface(orig, &src, img_starter_bkgd, &dest);
      }

      SDL_FreeSurface(orig);
    }
    else
    {
      img_starter_bkgd = orig;
    }
  }
}


static void flip_starter(void)
{
  SDL_Surface * orig;
  int y;
  SDL_Rect src, dest;

  
  /* Flip overlay: */

  orig = img_starter;
  img_starter = duplicate_surface(orig);

  if (img_starter != NULL)
  {
    for (y = 0; y < orig->h; y++)
    {
      src.x = 0;
      src.y = y;
      src.w = orig->w;
      src.h = 1;

      dest.x = 0;
      dest.y = orig->h - y - 1;

      SDL_BlitSurface(orig, &src, img_starter, &dest);
    }

    SDL_FreeSurface(orig);
  }
  else
  {
    img_starter = orig;
  }


  /* Flip background: */

  if (img_starter_bkgd != NULL)
  {
    orig = img_starter_bkgd;
    img_starter_bkgd = duplicate_surface(orig);

    if (img_starter_bkgd != NULL)
    {
      for (y = 0; y < orig->h; y++)
      {
        src.x = 0;
        src.y = y;
        src.w = orig->w;
        src.h = 1;

        dest.x = 0;
        dest.y = orig->h - y - 1;

        SDL_BlitSurface(orig, &src, img_starter_bkgd, &dest);
      }

      SDL_FreeSurface(orig);
    }
    else
    {
      img_starter_bkgd = orig;
    }
  }
}

