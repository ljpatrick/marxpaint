/*
  tuxpaint.c
  
  Tux Paint - A simple drawing program for children.
  
  Copyright (c) 2003 by Bill Kendrick
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
  
  June 14, 2002 - December 20, 2003
*/


#define VER_VERSION     "0.9.13"
#define VER_DATE        "2003.12.20"


/* #define DEBUG */
/* #define LOW_QUALITY_THUMBNAILS */
/* #define LOW_QUALITY_COLOR_SELECTOR */
/* #define LOW_QUALITY_STAMP_OUTLINE */
/* #define LOW_QUALITY_FLOOD_FILL */
/* #define NO_PROMPT_SHADOWS */
/* #define USE_HWSURFACE */


#define USE_HQ4X


/* Disable fancy cursors in fullscreen mode, to avoid SDL bug: */
#define LARGE_CURSOR_FULLSCREEN_BUG

#define HEIGHTOFFSET (((WINDOW_HEIGHT - 480) / 48) * 48)
#define TOOLOFFSET (HEIGHTOFFSET / 48 * 2)
#define PROMPTOFFSETX (WINDOW_WIDTH - 640) / 2
#define PROMPTOFFSETY (HEIGHTOFFSET / 2)


#define MIN_STAMP_SIZE 25  /* Stamp can only shrink to 25% (1/4) of original */
#define MAX_STAMP_SIZE 400 /* Stamp can only grow to 400% (4 times) original */


// #define MAX_FILES 2048  /* Max. # of files in a dir. to worry about... */

#define REPEAT_SPEED 300  /* Initial repeat speed for scrollbars */
#define CURSOR_BLINK_SPEED 500  /* Initial repeat speed for cursor */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include "hqxx.h"
#include "hq3x.h"
#include "hq4x.h"

#include <locale.h>
#include <iconv.h>

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


#include "mouse/watch.xbm"
#include "mouse/watch-mask.xbm"

#include "mouse/hand.xbm"
#include "mouse/hand-mask.xbm"

#include "mouse/wand.xbm"
#include "mouse/wand-mask.xbm"

#include "mouse/insertion.xbm"
#include "mouse/insertion-mask.xbm"

#include "mouse/brush.xbm"
#include "mouse/brush-mask.xbm"

#include "mouse/crosshair.xbm"
#include "mouse/crosshair-mask.xbm"

#include "mouse/rotate.xbm"
#include "mouse/rotate-mask.xbm"

#include "mouse/up.xbm"
#include "mouse/up-mask.xbm"

#include "mouse/down.xbm"
#include "mouse/down-mask.xbm"

#include "mouse/tiny.xbm"
#include "mouse/tiny-mask.xbm"

#include "mouse/arrow.xbm"
#include "mouse/arrow-mask.xbm"



#ifdef WIN32
/*
  The SDL stderr redirection trick doesn't seem to work for perror().
  This does pretty much the same thing.
*/
void win32_perror(const char *str)
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
#define min(a,b) ((a < b) ? a : b)
#define max(a,b) ((a > b) ? a : b)
#endif

#define clamp(lo,value,hi)    (min(max(value,lo),hi))


#define RENDER_TEXT TTF_RenderUTF8_Blended


/* Possible languages: */

enum {
  LANG_CA,     /* Catalan */
  LANG_CS,     /* Czech */
  LANG_DA,     /* Danish */
  LANG_DE,     /* German */
  LANG_EL,     /* Greek */
  LANG_EN,     /* English (American) (DEFAULT) */
  LANG_EN_GB,  /* English (British) */
  LANG_ES,     /* Spanish */
  LANG_EU,     /* Basque */
  LANG_FI,     /* Finnish */
  LANG_FR,     /* French */
  LANG_HE,     /* Hebrew */
  LANG_HU,     /* Hungarian */
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
  LANG_PT,     /* Portuguese */
  LANG_RO,     /* Romanian */
  LANG_RU,     /* Russian */
  LANG_SK,     /* Slovak */
  LANG_SV,     /* Swedish */
  LANG_TA,     /* Tamil */
  LANG_TR,     /* Turkish */
  LANG_WA,     /* Walloon */
  LANG_ZH,     /* Chinese */
  NUM_LANGS
};

const char * lang_prefixes[NUM_LANGS] = {
  "ca",
  "cs",
  "da",
  "de",
  "el",
  "en",
  "en_gb",
  "es",
  "eu",
  "fi",
  "fr",
  "he",
  "hu",
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
  "pt",
  "ro",
  "ru",
  "sk",
  "sv",
  "ta",
  "tr",
  "wa",
  "zh"
};


/* List of languages where we should use Unicode font rendering: */

int lang_use_unicode[] = {
  -1
};

int lang_use_utf8[] = {
  LANG_EL,
  LANG_HE,
  LANG_JA,
  LANG_KO,
  /* LANG_LT, */
  /* LANG_PL, */
  /* LANG_RU, */
  LANG_TA,
  LANG_ZH,
  -1
};

int lang_use_right_to_left[] = {
  LANG_HE,
  -1
};


typedef struct info_type {
  int colorable;
  int tintable;
  int mirrorable;
  int flipable;
} info_type;


typedef struct state_type {
  int mirrored;
  int flipped;
  int size;
} state_type;


enum {
  SAVE_OVER_PROMPT,
  SAVE_OVER_ALWAYS,
  SAVE_OVER_NO
};


/* Globals: */

int use_sound, fullscreen, disable_quit, simple_shapes, language,
  disable_print, print_delay, only_uppercase, promptless_save, grab_input,
  wheely, no_fancy_cursors, keymouse, mouse_x, mouse_y,
  mousekey_up, mousekey_down, mousekey_left, mousekey_right,
  dont_do_xor, use_print_config, dont_load_stamps, noshortcuts,
  mirrorstamps, disable_stamp_controls;
int recording, playing;
char * playfile;
FILE * demofi;
int WINDOW_WIDTH, WINDOW_HEIGHT;
char * printcommand;
int prog_bar_ctr;
SDL_Surface * screen;

SDL_Surface * canvas;

#define NUM_UNDO_BUFS 20
SDL_Surface * undo_bufs[NUM_UNDO_BUFS];
int cur_undo, oldest_undo, newest_undo;

SDL_Surface * img_title, * img_progress;
SDL_Surface * img_btn_up, * img_btn_down, * img_btn_off;
SDL_Surface * img_yes, * img_no;
SDL_Surface * img_open, * img_erase, * img_back;
SDL_Surface * img_cursor_up, * img_cursor_down;
SDL_Surface * img_scroll_up, * img_scroll_down;
SDL_Surface * img_scroll_up_off, * img_scroll_down_off;
SDL_Surface * img_paintcan;
SDL_Surface * img_grow, * img_shrink;

SDL_Surface * img_sparkles;

SDL_Surface * img_title_on, * img_title_off,
  * img_title_large_on, * img_title_large_off;
SDL_Surface * img_title_names[NUM_TITLES];
SDL_Surface * img_tools[NUM_TOOLS], * img_tool_names[NUM_TOOLS];

#define MAX_STAMPS 256
#define MAX_BRUSHES 64
#define MAX_FONTS 64
int num_brushes, num_stamps;
SDL_Surface * img_brushes[MAX_BRUSHES];
SDL_Surface * img_stamps[MAX_STAMPS];
SDL_Surface * img_stamps_premirror[MAX_STAMPS];
char * txt_stamps[MAX_STAMPS];
info_type * inf_stamps[MAX_STAMPS];
state_type * state_stamps[MAX_STAMPS];
#ifndef NOSOUND
Mix_Chunk * snd_stamps[MAX_STAMPS];
#endif
SDL_Surface * img_stamp_thumbs[MAX_STAMPS],
  * img_stamp_thumbs_premirror[MAX_STAMPS];

SDL_Surface * img_shapes[NUM_SHAPES], * img_shape_names[NUM_SHAPES];
SDL_Surface * img_magics[NUM_MAGICS], * img_magic_names[NUM_MAGICS];
SDL_Surface * img_openlabels_open, * img_openlabels_erase,
  * img_openlabels_back;

SDL_Surface * img_tux[NUM_TIP_TUX];

#ifndef LOW_QUALITY_COLOR_SELECTOR
SDL_Surface * img_color_btns[NUM_COLORS];
#endif

SDL_Surface * img_cur_brush;
int brush_counter, rainbow_color;

TTF_Font * font, * small_font, * large_font, * locale_font;
TTF_Font * fonts[MAX_FONTS];
int num_fonts;

#ifndef NOSOUND
Mix_Chunk * sounds[NUM_SOUNDS];
#endif


SDL_Cursor * cursor_hand, * cursor_arrow, * cursor_watch,
  * cursor_up, * cursor_down, * cursor_tiny, * cursor_crosshair,
  * cursor_brush, * cursor_wand, * cursor_insertion, * cursor_rotate;


int cur_tool, cur_color, cur_brush, cur_stamp, cur_shape, cur_magic;
int cur_font, cursor_left, cursor_x, cursor_y, cursor_textwidth;
int colors_are_selectable;
int been_saved;
char file_id[32];
int brush_scroll, stamp_scroll, font_scroll;
int eraser_sound;

char texttool_str[256];
int texttool_len;

int tool_avail[NUM_TOOLS], tool_avail_bak[NUM_TOOLS];

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

SDL_Event scrolltimer_event;

char * langstr;
char * savedir;

int RGBtoYUV[65536];


/* Local function prototypes: */

void mainloop(void);
void brush_draw(int x1, int y1, int x2, int y2, int update);
void blit_brush(int x, int y);
void magic_draw(int x1, int y1, int x2, int y2);
void blit_magic(int x, int y, int x2, int y2);
void stamp_draw(int x, int y);
void rec_undo_buffer(void);
void update_canvas(int x1, int y1, int x2, int y2);
void show_usage(FILE * f, char * prg);
void show_lang_usage(FILE * f, char * prg);
void show_locale_usage(FILE * f, char * prg);
void setup(int argc, char * argv[]);
SDL_Cursor * get_cursor(char * bits, char * mask_bits,
		        int w, int h, int x, int y);
void seticon(void);
SDL_Surface * loadimage(char * fname);
SDL_Surface * do_loadimage(char * fname, int abort_on_error);
SDL_Surface * loadaltimage(char * fname);
void draw_toolbar(void);
void draw_magic(void);
void draw_colors(int enabled);
void draw_brushes(void);
void draw_stamps(void);
void draw_shapes(void);
void draw_fonts(void);
void draw_none(void);
#ifndef NOSOUND
void loadarbitrary(SDL_Surface * surfs[], SDL_Surface * altsurfs[],
		   char * descs[], info_type * infs[],
		   Mix_Chunk * sounds[], int * count, int starting, int max,
		   char * dir, int fatal, int maxw, int maxh);
#else
void loadarbitrary(SDL_Surface * surfs[], SDL_Surface * altsurfs[],
		   char * descs[], info_type * infs[],
		   int * count, int starting, int max,
		   char * dir, int fatal, int maxw, int maxh);
#endif
SDL_Surface * thumbnail(SDL_Surface * src, int max_x, int max_y,
			int keep_aspect);
Uint32 getpixel(SDL_Surface * surface, int x, int y);
void putpixel(SDL_Surface * surface, int x, int y, Uint32 pixel);
void debug(char * str);
void do_undo(void);
void do_redo(void);
void render_brush(void);
void playsound(int chan, int s, int override);
void line_xor(int x1, int y1, int x2, int y2);
void clipped_putpixel(SDL_Surface * dest, int x, int y, Uint32 c);
void rect_xor(int x1, int y1, int x2, int y2);
void stamp_xor(int x1, int y1);
void do_eraser(int x, int y);
void disable_avail_tools(void);
void enable_avail_tools(void);
void reset_avail_tools(void);
void update_screen(int x1, int y1, int x2, int y2);
Uint8 alpha(Uint8 c1, Uint8 c2, Uint8 a);
int compare_strings(char * * s1, char * * s2);
int compare_dirents(struct dirent * f1, struct dirent * f2);
void draw_tux_text(int which_tux, char * str, int want_utf8,
		   int force_locale_font, int want_right_to_left);
void wordwrap_text(TTF_Font * font, char * str, SDL_Color color,
		   int left, int top, int right, int want_utf8,
		   int force_locale_font, int want_right_to_left);
char * loaddesc(char * fname);
info_type * loadinfo(char * fname);
#ifndef NOSOUND
Mix_Chunk * loadsound(char * fname);
#endif
void do_wait(void);
void load_current(void);
void save_current(void);
char * get_fname(char * name);
int do_prompt(char * text, char * btn_yes, char * btn_no);
void cleanup(void);
void free_cursor(SDL_Cursor ** cursor);
void free_surface(SDL_Surface **surface_array);
void free_surface_array(SDL_Surface *surface_array[], int count);
void update_shape(int cx, int ox1, int ox2, int cy, int oy1, int oy2,
		  int fixed);
void do_shape(int cx, int cy, int ox, int oy, int rotn, int use_brush);
int rotation(int ctr_x, int ctr_y, int ox, int oy);
int do_save(void);
int do_png_save(FILE * fi, char * fname, SDL_Surface * surf);
void get_new_file_id(void);
int do_quit(void);
int do_open(int want_new_tool);
void scan_fill(int cnt, point_type * pts);
int clip_polygon(int n, fpoint_type * pin, fpoint_type * pout);
void wait_for_sfx(void);
int current_language(void);
int stamp_colorable(int stamp);
int stamp_tintable(int stamp);
void rgbtohsv(Uint8 r8, Uint8 g8, Uint8 b8, float *h, float *s, float *v);
void hsvtorgb(float h, float s, float v, Uint8 *r8, Uint8 *g8, Uint8 *b8);
void show_progress_bar(void);
void do_print(void);
void strip_trailing_whitespace(char * buf);
void do_render_cur_text(int do_blit);
void loadfonts(char * dir, int fatal);
char * uppercase(char * str);
unsigned char * textdir(unsigned char * str);
SDL_Surface * do_render_button_label(char * label);
static void create_button_labels(void);
int colors_close(Uint32 c1, Uint32 c2);
void do_flood_fill(int x, int y, Uint32 cur_colr, Uint32 old_colr);
static Uint32 scrolltimer_callback(Uint32 interval, void *param);
static Uint32 drawtext_callback(Uint32 interval, void *param);
void control_drawtext_timer(Uint32 interval, char* text);
void parse_options(FILE * fi);
char * debug_gettext(const char * str);
void do_setcursor(SDL_Cursor * c);
char * great_str(void);
int charsize(char c);
void draw_image_title(int t, int x);
int need_unicode(int l);
int need_utf8(int l);
int need_own_font(int l);
int want_own_font(int l);
int need_right_to_left(int l);
void handle_keymouse(SDLKey key, Uint8 updown);
void move_keymouse(void);
void handle_active(SDL_Event * event);
char * remove_slash(char * path);
unsigned char * utf8_decode(unsigned char * str);
unsigned char * unescape(char * str);

void convert_open(const char * from);
void convert_close();
char * convert2utf8(char c);
int converts();
int delete_utf8_char(char * utf8_str, int len);
void anti_carriage_return(int left, int right, int cur_top, int new_top,
		          int cur_bot, int line_width);
int mySDL_WaitEvent(SDL_Event *event);
int mySDL_PollEvent(SDL_Event *event);


#define MAX_UTF8_CHAR_LENGTH 6

#define USEREVENT_TEXT_UPDATE 1


/* --- MAIN --- */

int main(int argc, char * argv[])
{
  SDL_Surface * tmp_surf;
  SDL_Color black = {0, 0, 0};
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

  snprintf(tmp_str, sizeof(tmp_str), "%s - %s", VER_VERSION, VER_DATE);
  tmp_surf = TTF_RenderText_Blended(font, tmp_str, black);
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


  reset_avail_tools();


  /* Load current image (if any): */

  load_current();
  been_saved = 1;
  tool_avail[TOOL_SAVE] = 0;


  /* Draw the screen! */

  SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 255, 255, 255));

  draw_toolbar();
  draw_colors(1);
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

#define PROMPT_QUIT_SAVE_TXT gettext_noop("If you quit, you'll lose your picture! Save it?")
#define PROMPT_QUIT_SAVE_YES gettext_noop("Yes")
#define PROMPT_QUIT_SAVE_NO gettext_noop("No")

#define PROMPT_OPEN_SAVE_TXT gettext_noop("Save your picture first?")
#define PROMPT_OPEN_SAVE_YES gettext_noop("Yes")
#define PROMPT_OPEN_SAVE_NO gettext_noop("No")

#define PROMPT_OPEN_UNOPENABLE_TXT gettext_noop("Can't open that picture!")
#define PROMPT_OPEN_UNOPENABLE_YES gettext_noop("Okay")

#define PROMPT_NEW_TXT gettext_noop("Starting a new picture will erase the current one!")
#define PROMPT_NEW_YES gettext_noop("That's Ok")
#define PROMPT_NEW_NO gettext_noop("Never Mind!")

#define PROMPT_OPEN_NOFILES_TXT gettext_noop("There are no saved files!")
#define PROMPT_OPEN_NOFILES_YES gettext_noop("Okay")

#define PROMPT_PRINT_TXT gettext_noop("Your picture has been printed!")
#define PROMPT_PRINT_YES gettext_noop("Okay")

#define PROMPT_PRINT_TOO_SOON_TXT gettext_noop("You can't print yet!")
#define PROMPT_PRINT_TOO_SOON_YES gettext_noop("Okay")

#define PROMPT_ERASE_TXT gettext_noop("Erase this picture?")
#define PROMPT_ERASE_YES gettext_noop("Yes")
#define PROMPT_ERASE_NO gettext_noop("No")


enum {
  SHAPE_TOOL_MODE_STRETCH,
  SHAPE_TOOL_MODE_ROTATE,
  SHAPE_TOOL_MODE_DONE
};


/* --- MAIN LOOP! --- */

void mainloop(void)
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
		  draw_colors(0);
		  draw_none();
		  
		  tmp_int = do_open(tmp_int);
		  
		  enable_avail_tools();
		  tool_avail[TOOL_NEW] = tmp_int;
		  
		  draw_toolbar();
		  SDL_UpdateRect(screen,
				 0, 0,
				 96, (48 * (7 + TOOLOFFSET / 2)) + 40);
			  
		  if (cur_tool == TOOL_BRUSH ||
		      cur_tool == TOOL_LINES ||
		      cur_tool == TOOL_SHAPES ||
		      cur_tool == TOOL_TEXT)
		    {
		      draw_colors(1);
		    }
		  else if (cur_tool == TOOL_STAMP)
		    {
		      draw_colors(stamp_colorable(cur_stamp) ||
				  stamp_tintable(cur_stamp));
		    }
		  else if (cur_tool == TOOL_MAGIC &&
			   cur_magic == MAGIC_FILL)
		    {
		      draw_colors(1);
		    }

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
		  
		  draw_tux_text(TUX_GREAT, tool_tips[cur_tool], 0, 0, 1);


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
		      
		      playsound(1, SND_HARP, 1);
		    }
		  else
		    {
		      draw_tux_text(tool_tux[TUX_DEFAULT], TIP_NEW_ABORT,
				    0, 0, 1);
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
			      if (converts())
				{
				  texttool_len = delete_utf8_char(texttool_str, texttool_len);
				}
			      else
				{
				  texttool_len--;
				  texttool_str[texttool_len] = '\0';
				  playsound(0, SND_KEYCLICK, 1);
				}
	        
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
	  	     

			      if (converts())
				{
				  char * str = convert2utf8(key_unicode);
				  int i;
				  size_t len = strlen(str);

				  for (i = 0; i < len; i++)
				    {
				      texttool_str[texttool_len++] = str[i];
				    }

				  free(str);
				}
			      else
				{
				  texttool_str[texttool_len++] = key_unicode;
				}
		
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
				    0, 0, 1);
		      
		      
		      /* Draw items for this tool: */
		      
		      if (cur_tool == TOOL_BRUSH)
			{
			  draw_brushes();
			  draw_colors(1);
			}
		      else if (cur_tool == TOOL_STAMP)
			{
			  draw_stamps();
			  draw_colors(stamp_colorable(cur_stamp) ||
				      stamp_tintable(cur_stamp));
			}
		      else if (cur_tool == TOOL_LINES)
			{
			  draw_brushes();
			  draw_colors(1);
			}
		      else if (cur_tool == TOOL_SHAPES)
			{
			  draw_shapes();
			  draw_colors(1);
			  shape_tool_mode = SHAPE_TOOL_MODE_DONE;
			}
		      else if (cur_tool == TOOL_TEXT)
			{
			  draw_fonts();
			  draw_colors(1);
			}
		      else if (cur_tool == TOOL_ERASER)
			{
			  draw_none();
			  draw_colors(0);
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
		          draw_colors(0);
		          draw_none();
			  
			  tmp_int = do_open(tmp_int);
			  
			  enable_avail_tools();
			  tool_avail[TOOL_NEW] = tmp_int;
			  
			  cur_tool = old_tool;
			  draw_toolbar();
			  SDL_UpdateRect(screen,
					 0, 0,
					 96, (48 * (7 + TOOLOFFSET / 2)) + 40);
			  
			  draw_tux_text(TUX_GREAT, tool_tips[cur_tool], 0, 0, 1);

			  if (cur_tool == TOOL_BRUSH ||
			      cur_tool == TOOL_LINES ||
			      cur_tool == TOOL_SHAPES ||
			      cur_tool == TOOL_TEXT)
			    {
			      draw_colors(1);
			    }
			  else if (cur_tool == TOOL_STAMP)
			    {
			      draw_colors(stamp_colorable(cur_stamp) ||
					  stamp_tintable(cur_stamp));
			    }
		  	  else if (cur_tool == TOOL_MAGIC &&
				   cur_magic == MAGIC_FILL)
			    {
			      draw_colors(1);
			    }

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
			      
			      playsound(1, SND_HARP, 1);
			    }
			  else
			    {
			      draw_tux_text(tool_tux[TUX_DEFAULT],
					    TIP_NEW_ABORT, 0, 0, 1);
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
			      do_print();
			      
			      last_print_time = cur_time;
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
			  rainbow_color = 0;
			  draw_magic();

			  if (cur_magic == MAGIC_FILL)
			    draw_colors(1);
			  else
			    draw_colors(0);
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
				      state_stamps[cur_stamp]->size =
					state_stamps[cur_stamp]->size / 2;

				      if (state_stamps[cur_stamp]->size < MIN_STAMP_SIZE)
					state_stamps[cur_stamp]->size = MIN_STAMP_SIZE;
			    
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
				      state_stamps[cur_stamp]->size =
					state_stamps[cur_stamp]->size * 2;

				      if (state_stamps[cur_stamp]->size > MAX_STAMP_SIZE)
					state_stamps[cur_stamp]->size = MAX_STAMP_SIZE;
			    
				      playsound(0, SND_GROW, 0);
				      draw_stamps();
			
				      SDL_UpdateRect(screen,
						     WINDOW_WIDTH - 96, 0,
						     96, (48 * 7) + 40 + HEIGHTOFFSET);
				    }
				}
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
			  cur_stamp = cur_thing;
			  stamp_scroll = thing_scroll;
			  
			  if (do_draw)
			    draw_stamps();
			  
			  if (txt_stamps[cur_stamp] != NULL)
			    {
#ifdef DEBUG
			      printf("txt_stamps[cur_stamp] = %s\n",
				     txt_stamps[cur_stamp]);
#endif

			      if (txt_stamps[cur_stamp][0] == '=')
				{
				  /* FIXME: Stupid. Using '=' to denote UTF8 */

				  draw_tux_text(TUX_GREAT,
						txt_stamps[cur_stamp] + 1, 1, 0, 1);
				}
			      else
				{
				  draw_tux_text(TUX_GREAT,
						txt_stamps[cur_stamp], 0, 1, 0);
				}
			    }
			  else
			    draw_tux_text(TUX_GREAT, "", 0, 0, 0);
			  
			  
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
			  
			  draw_tux_text(TUX_GREAT, shape_tips[cur_shape],
					0, 0, 1);
			  
			  if (do_draw)
			    draw_shapes();
			}
		      else if (cur_tool == TOOL_MAGIC)
			{
			  if (cur_thing != cur_magic)
			    {
			      if (cur_thing == MAGIC_FILL)
				draw_colors(1);
			      else
				draw_colors(0);

			      SDL_UpdateRect(screen,
					     0, (48 * 7) + 40 + HEIGHTOFFSET,
					     WINDOW_WIDTH, 48);
			    }
			  
			  cur_magic = cur_thing;

			  draw_tux_text(TUX_GREAT, magic_tips[cur_magic],
					0, 0, 1);
			  
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
		       (cur_tool == TOOL_BRUSH || cur_tool == TOOL_LINES ||
			cur_tool == TOOL_SHAPES || cur_tool == TOOL_TEXT ||
			(cur_tool == TOOL_MAGIC && cur_magic == MAGIC_FILL) ||
			(cur_tool == TOOL_STAMP &&
			 (stamp_colorable(cur_stamp) ||
			  stamp_tintable(cur_stamp)))))
		{
		  /* Color! */
		  
		  which = ((event.button.x - 96) /
			   ((WINDOW_WIDTH - 96) / NUM_COLORS));
		  
		  if (which < NUM_COLORS)
		    {
		      cur_color = which;
		      playsound(1, SND_BUBBLE, 1);
		      draw_colors(1);
		      SDL_UpdateRect(screen,
				     0, (48 * 7) + 40 + HEIGHTOFFSET,
				     WINDOW_WIDTH, 48);
		      render_brush();
		      draw_tux_text(TUX_KISS, color_names[cur_color], 0, 0, 1);
		
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
		  
		  button_down = 1;
		  
		  old_x = event.button.x - 96;
		  old_y = event.button.y;
		  
		  if (been_saved)
		    {
		      been_saved = 0;
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
#ifdef LOW_QUALITY_STAMP_OUTLINE
		      rect_xor(old_x - ((img_stamps[cur_stamp]->w / 2) * (state_stamps[cur_stamp]->size) / 100),
			       old_y - ((img_stamps[cur_stamp]->h / 2) * (state_stamps[cur_stamp]->size) / 100),
			       old_x + ((img_stamps[cur_stamp]->w / 2) * (state_stamps[cur_stamp]->size) / 100),
			       old_y + ((img_stamps[cur_stamp]->h / 2) * (state_stamps[cur_stamp]->size) / 100));
#else
		      stamp_xor(old_x - (img_stamps[cur_stamp]->w / 2),
			        old_y - (img_stamps[cur_stamp]->h / 2));
#endif
		      playsound(1, SND_STAMP, 1);

		      draw_tux_text(TUX_GREAT, great_str(), 0, 0, 1);

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
		      draw_tux_text(TUX_BORED, TIP_LINE_START, 0, 0, 1);
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
			  draw_tux_text(TUX_BORED, TIP_SHAPE_START, 0, 0, 1);
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
			  draw_tux_text(TUX_GREAT, tool_tips[TOOL_SHAPES],
					0, 0, 1);
			}
		    }
		  else if (cur_tool == TOOL_MAGIC)
		    {
		      /* Start doing magic! */
		      
		      rec_undo_buffer();
		      
		      /* (Arbitrarily large, so we draw once now) */
		      brush_counter = 999;
		     
		      if (cur_magic != MAGIC_FILL)
			{
			  magic_draw(old_x, old_y, old_x, old_y);
			}
		      else
			{
			  do_flood_fill(old_x, old_y,
					SDL_MapRGB(canvas->format,
						   color_hexes[cur_color][0],
						   color_hexes[cur_color][1],
						   color_hexes[cur_color][2]),
					getpixel(canvas, old_x, old_y));

			  draw_tux_text(TUX_GREAT, magic_tips[MAGIC_FILL],
					0, 0, 1);
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
		      thing_scroll = 0;
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

		  if (do_draw)
		    draw_stamps();

		  if (txt_stamps[cur_stamp] != NULL)
		    {
		      if (txt_stamps[cur_stamp][0] == '=')
			{
			  /* FIXME: Stupid. Using '=' to denote UTF8 */

			  draw_tux_text(TUX_GREAT,
					txt_stamps[cur_stamp] + 1, 1, 0, 1);
			}
		      else
			{
			  draw_tux_text(TUX_GREAT, txt_stamps[cur_stamp],
					0, 1, 0);
			}
		    }
		  else
		    draw_tux_text(TUX_GREAT, "", 0, 0, 0);


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
                       
		  draw_tux_text(TUX_GREAT, shape_tips[cur_shape],
				0, 0, 1);

		  if (do_draw)
		    draw_shapes();
		}
	      else if (cur_tool == TOOL_MAGIC)
		{
		  if (cur_thing != cur_magic)
		    {
		      if (cur_thing == MAGIC_FILL)
			draw_colors(1);
		      else
			draw_colors(0);

		      SDL_UpdateRect(screen,
				     0, (48 * 7) + 40 + HEIGHTOFFSET,
				     WINDOW_WIDTH, 48);
		    }

		  cur_magic = cur_thing;

		  draw_tux_text(TUX_GREAT, magic_tips[cur_magic],
				0, 0, 1);

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
			  draw_tux_text(TUX_GREAT, (char *) event.user.data1 + 1,
					1, 0, 1);
			}
		      else
			{
			  draw_tux_text(TUX_GREAT, (char *) event.user.data1, 0, 1, 0);
			}
		    }
		  else
		    draw_tux_text(TUX_GREAT, "", 0, 0, 1);
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
		      draw_tux_text(TUX_GREAT, tool_tips[TOOL_LINES], 0, 0, 1);
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
			      draw_tux_text(TUX_BORED, TIP_SHAPE_NEXT, 0, 0, 1);
			      
			      
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
					    tool_tips[TOOL_SHAPES], 0, 0, 1);
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
		      thing_scroll = 0;
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
			  magic_draw(old_x, old_y, new_x, new_y);
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
		      w = 96;
		      h = 96;
		    }
		  
		  if (old_x >= 0 && old_x < WINDOW_WIDTH - 96 - 96 &&
		      old_y >= 0 && old_y < (48 * 7) + 40 + HEIGHTOFFSET)
		    {
		      if (cur_tool == TOOL_STAMP)
			{
#ifndef LOW_QUALITY_STAMP_OUTLINE
			  stamp_xor(old_x - w / 2, old_y - h / 2);
#else
			  rect_xor(old_x - ((img_stamps[cur_stamp]->w / 2) * (state_stamps[cur_stamp]->size) / 100),
				   old_y - ((img_stamps[cur_stamp]->h / 2) * (state_stamps[cur_stamp]->size) / 100),
				   old_x + ((img_stamps[cur_stamp]->w / 2) * (state_stamps[cur_stamp]->size) / 100),
				   old_y + ((img_stamps[cur_stamp]->h / 2) * (state_stamps[cur_stamp]->size) / 100));
#endif

			  update_screen(old_x - ((img_stamps[cur_stamp]->w / 2) * (state_stamps[cur_stamp]->size) / 100) + 96,
					old_y - ((img_stamps[cur_stamp]->h / 2) * (state_stamps[cur_stamp]->size) / 100),
					old_x + ((img_stamps[cur_stamp]->w / 2) * (state_stamps[cur_stamp]->size) / 100) + 96,
					old_y + ((img_stamps[cur_stamp]->h / 2) * (state_stamps[cur_stamp]->size) / 100));
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
#ifndef LOW_QUALITY_STAMP_OUTLINE
			  stamp_xor(new_x - w / 2, new_y - h / 2);
#else
			  rect_xor(new_x - ((img_stamps[cur_stamp]->w / 2) * (state_stamps[cur_stamp]->size) / 100),
				   new_y - ((img_stamps[cur_stamp]->h / 2) * (state_stamps[cur_stamp]->size) / 100),
				   new_x + ((img_stamps[cur_stamp]->w / 2) * (state_stamps[cur_stamp]->size) / 100),
				   new_y + ((img_stamps[cur_stamp]->h / 2) * (state_stamps[cur_stamp]->size) / 100));
#endif

			  update_screen(new_x - ((img_stamps[cur_stamp]->w / 2) * (state_stamps[cur_stamp]->size) / 100) + 96,
					new_y - ((img_stamps[cur_stamp]->h / 2) * (state_stamps[cur_stamp]->size) / 100),
					new_x + ((img_stamps[cur_stamp]->w / 2) * (state_stamps[cur_stamp]->size) / 100) + 96,
					new_y + ((img_stamps[cur_stamp]->h / 2) * (state_stamps[cur_stamp]->size) / 100));
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

void brush_draw(int x1, int y1, int x2, int y2, int update)
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

void blit_brush(int x, int y)
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


/* Draw using the current stamp: */

void stamp_draw(int x, int y)
{
  SDL_Rect src, dest;
  SDL_Surface * tmp_surf, * surf_ptr, * final_surf;
  Uint32 amask;
  Uint8 r, g, b, a;
  int xx, yy, dont_free_tmp_surf, base_x, base_y;
  float col_hue, col_sat, col_val,
    stamp_hue, stamp_sat, stamp_val;
  

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
	 they pick the color, or pick the stamp, like with brushes?
	 (Why? Because I'm LAZY! :^) ) */

	  
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
      /* Render the stamp in the chosen color: */

      /* FIXME: It sucks to render this EVERY TIME.  Why not just when
	 they pick the color, or pick the stamp, like with brushes?
	 (Why? Because I'm LAZY! :^) ) */

	  
      rgbtohsv(color_hexes[cur_color][0],
	       color_hexes[cur_color][1],
	       color_hexes[cur_color][2],
	       &col_hue, &col_sat, &col_val);
        

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
	
	      rgbtohsv(r, g, b, &stamp_hue, &stamp_sat, &stamp_val);	
	
	      if (stamp_sat > 0.25)
		hsvtorgb(col_hue, col_sat, stamp_val, &r, &g, &b);
	      //	else
	      //	  hsvtorgb(col_hue, col_sat, stamp_val, &r, &g, &b);
	
	      putpixel(tmp_surf, xx, yy,
		       SDL_MapRGBA(tmp_surf->format, r, g, b, a));
	    }
	}

      SDL_UnlockSurface(tmp_surf);
      SDL_UnlockSurface(surf_ptr);
    }
  else
    {
      /* No color change, just use it! */

      tmp_surf = surf_ptr;
    }

  
  /* Shrink or grow it! */

#ifdef USE_HQ4X
  if (state_stamps[cur_stamp]->size == 400)
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
      final_surf = thumbnail(tmp_surf,
			     (tmp_surf->w *
			      state_stamps[cur_stamp]->size) / 100,
			     (tmp_surf->h *
			      state_stamps[cur_stamp]->size) / 100,
			     0);
    }
  
  
  /* Where it will go? */
  
  base_x = x - (((tmp_surf->w * state_stamps[cur_stamp]->size) / 100) / 2);
  base_y = y - (((tmp_surf->h * state_stamps[cur_stamp]->size) / 100) / 2);
   

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

  update_canvas(x - (((tmp_surf->w * state_stamps[cur_stamp]->size) / 100) / 2),
		y - (((tmp_surf->h * state_stamps[cur_stamp]->size) / 100) / 2),
                x + (((tmp_surf->w * state_stamps[cur_stamp]->size) / 100) / 2),
		y + (((tmp_surf->h * state_stamps[cur_stamp]->size) / 100) / 2));
  
  /* Free the temporary surfaces */

  if (!dont_free_tmp_surf)
    SDL_FreeSurface(tmp_surf);
  
  SDL_FreeSurface(final_surf);
}


/* Draw using the current brush: */

void magic_draw(int x1, int y1, int x2, int y2)
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
		blit_magic(x1, y, x1, y - 1);
	    }
          else
	    {
	      for (y = y1; y <= y2; y++)
		blit_magic(x1, y, x1 + dx, y + 1);
	    }
	
	  x1 = x1 + dx;
	}
    }
  else
    {
      if (y1 > y2)
	{
	  for (y = y1; y >= y2; y--)
	    blit_magic(x1, y, x1, y - 1);
	}
      else
	{
	  for (y = y1; y <= y2; y++)
	    blit_magic(x1, y, x1, y + 1);
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
  else if (cur_magic == MAGIC_THICK)
    playsound(0, SND_THICK, 0);
  else if (cur_magic == MAGIC_THIN)
    playsound(0, SND_THIN, 0);
  else if (cur_magic == MAGIC_BLOCKS && ((rand() % 10) < 5))
    playsound(0, SND_BLOCKS, 0);
  else if (cur_magic == MAGIC_FADE)
    playsound(0, SND_FADE, 0);
  else if (cur_magic == MAGIC_RAINBOW)
    playsound(0, SND_RAINBOW, 0);


  /* FIXME: Arbitrary? */

  update_canvas(orig_x1 - 32, orig_y1 - 32,
		orig_x2 + 32, orig_y2 + 32);
}


/* Draw the current brush in the current color: */

void blit_magic(int x, int y, int x2, int y2)
{
  int xx, yy, w, h;
  Uint32 colr;
  Uint8 r, g, b,
    r1, g1, b1,
    r2, g2, b2,
    r3, g3, b3,
    r4, g4, b4;
  SDL_Surface * last;
  SDL_Rect src, dest;


  /* In case we need to use the current canvas (just saved to undo buf)... */

  if (cur_undo > 0)
    last = undo_bufs[cur_undo - 1];
  else
    last = undo_bufs[NUM_UNDO_BUFS - 1];


  brush_counter++;

  if (brush_counter >= 4)  /* FIXME: Arbitrary? */
    {
      brush_counter = 0;


      if (cur_magic == MAGIC_BLUR)
	{
	  /* FIXME: Circular "brush?" */

	  SDL_LockSurface(canvas);
	  for (yy = y - 16; yy < y + 16; yy = yy + 2)
	    {
	      for (xx = x - 16; xx < x + 16; xx = xx + 2)
		{
		  SDL_GetRGB(getpixel(canvas, clamp(0, xx, canvas->w - 1),
				      clamp(0, yy - 1, canvas->h - 1)),
			     canvas->format,
			     &r1, &g1, &b1);
	
		  SDL_GetRGB(getpixel(canvas, clamp(0, xx - 1, canvas->w - 1),
				      clamp(0, yy, canvas->h - 1)),
			     canvas->format,
			     &r2, &g2, &b2);
	
		  SDL_GetRGB(getpixel(canvas, clamp(0, xx + 1, canvas->w - 1),
				      clamp(0, yy, canvas->h - 1)),
			     canvas->format,
			     &r3, &g3, &b3);
	
		  SDL_GetRGB(getpixel(canvas, clamp(0, xx, canvas->w - 1),
				      clamp(0, yy + 1, canvas->h - 1)),
			     canvas->format,
			     &r4, &g4, &b4);
	

		  r = (r1 + r2 + r3 + r4) >> 2;
		  g = (g1 + g2 + g3 + g4) >> 2;
		  b = (b1 + b2 + b3 + b4) >> 2;

		  putpixel(canvas, xx, yy,
			   SDL_MapRGB(canvas->format, r, g, b));
		}
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
		  /* Get average color around here: */
		
		  SDL_GetRGB(getpixel(last, xx, yy), last->format,
			     &r1, &g1, &b1);
	
		  SDL_GetRGB(getpixel(last, xx + 2, yy), last->format,
			     &r2, &g2, &b2);
	
		  SDL_GetRGB(getpixel(last, xx, yy + 2), last->format,
			     &r3, &g3, &b3);
	
		  SDL_GetRGB(getpixel(last, xx + 2, yy + 2), last->format,
			     &r4, &g4, &b4);

		  r = (r1 + r2 + r3 + r4) / 4;
		  g = (g1 + g2 + g3 + g4) / 4;
		  b = (b1 + b2 + b3 + b4) / 4;

	
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
      else if (cur_magic == MAGIC_NEGATIVE)
	{
	  SDL_LockSurface(last);
	  SDL_LockSurface(canvas);

	  for (yy = y - 16; yy < y + 16; yy++)
	    {
	      for (xx = x - 16; xx < x + 16; xx++)
		{
		  /* Get average color around here: */
		
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
      else if (cur_magic == MAGIC_FADE)
	{
	  SDL_LockSurface(last);
	  SDL_LockSurface(canvas);

	  for (yy = y - 16; yy < y + 16; yy++)
	    {
	      for (xx = x - 16; xx < x + 16; xx++)
		{
		  /* Get average color around here: */
		
		  SDL_GetRGB(getpixel(last, xx, yy), last->format,
			     &r, &g, &b);
	
		  r = min(r + 48, 255);
		  g = min(g + 48, 255);
		  b = min(b + 48, 255);

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
      else if (cur_magic == MAGIC_FLIP)
	{
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
	}
      else if (cur_magic == MAGIC_MIRROR)
	{
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
	}
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

		  r = min(r, 255);
		  g = min(g, 255);
		  b = min(b, 255);
	
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
    }
}


/* Store canvas into undo buffer: */

void rec_undo_buffer(void)
{
  int wanna_update_toolbar;

  wanna_update_toolbar = 0;


  SDL_BlitSurface(canvas, NULL, undo_bufs[cur_undo], NULL);

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

void update_canvas(int x1, int y1, int x2, int y2)
{
  SDL_Rect dest;

  dest.x = 96;
  dest.y = 0;
  SDL_BlitSurface(canvas, NULL, screen, &dest);
  update_screen(x1 + 96, y1, x2 + 96, y2);
}


/* Show program version: */

void show_version(void)
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

void show_usage(FILE * f, char * prg)
{
  char * blank;
  int i;

  blank = strdup(prg);

  for (i = 0; i < strlen(blank); i++)
    blank[i] = ' ';

  fprintf(f,
	  "\n"
	  "Usage: %s {--usage | --help | --version | --copying}\n"
	  "\n"
	  "  %s [--windowed | --fullscreen]   [--640x480 | --800x600]\n"
	  "  %s [--sound | --nosound]         [--quit | --noquit]\n"
	  "  %s [--print | --noprint]         [--complexshapes | --simpleshapes]\n"
	  "  %s [--mixedcase | --uppercase]   [--fancycursors | --nofancycursors]\n"
	  "  %s [--mouse | --keyboard]        [--dontgrab | --grab]\n"
	  "  %s [--noshortcuts | --shortcuts] [--wheelmouse | --nowheelmouse]\n"
	  "  %s [--outlines | --nooutlines]   [--stamps | --nostamps]\n"
	  "  %s [--nostampcontrols | --stampcontrols]\n"
	  "  %s [--mirrorstamps | --dontmirrorstamps]\n"
	  "  %s [--wheelmouse | --nowheelmouse]\n"
	  "  %s [--saveoverask | --saveover | --saveovernew]\n"
	  "  %s [--savedir DIRECTORY]\n"
#ifdef WIN32
	  "  %s [--printcfg | --noprintcfg]\n"
#endif
	  "  %s [--printdelay=SECONDS]\n"
	  "  %s [--lang LANGUAGE | --locale LOCALE | --lang help]\n"
	  "  %s [--nosysconfig]\n"
	  /* "  %s [--record FILE | --playback FILE]\n" */
	  "\n",
	  prg, prg,
	  blank, blank, blank,
	  blank, blank, blank,
	  blank, blank, blank,
	  blank, blank, blank,
	  blank, blank);

  free(blank);
}


/* FIXME: All this should REALLY be array-based!!! */

/* Show available languages: */

void show_lang_usage(FILE * f, char * prg)
{
  fprintf(f,
	  "\n"
	  "Usage: %s [--lang LANGUAGE]\n"
	  "\n"
	  "LANGUAGE may be one of:\n"
	  "  english      american-english\n"
	  "  basque       euskara\n"
	  "  bokmal\n"
	  "  brazilian    brazilian-portuguese   portugues-brazilian\n"
	  "  british      british-english\n"
	  "  catalan      catala\n"
	  "  chinese\n"
	  "  czech        cesky\n"
	  "  danish       dansk\n"
	  "  dutch\n"
	  "  finnish      suomi\n"
	  "  french       francais\n"
	  "  german       deutsch\n"
	  "  greek\n"
	  "  hebrew\n"
	  "  hungarian    magyar\n"
	  "  icelandic    islenska\n"
	  "  indonesian   bahasa-indonesia\n"
	  "  italian      italiano\n"
	  "  japanese\n"
	  "  korean\n"
	  "  malay\n"
	  "  lithuanian   lietuviu\n"
	  "  norwegian    nynorsk                norsk\n"
	  "  polish       polski\n"
	  "  portuguese   portugues\n"
	  "  romanian\n"
	  "  russian      russkiy\n"
	  "  slovak\n"
	  "  spanish      espanol\n"
	  "  swedish      svenska\n"
	  "  tamil\n"
	  "  turkish\n"
	  "  walloon\n"
	  "\n",
	  prg);
}


/* FIXME: Add accented characters to the descriptions */

/* Show available locales: */

void show_locale_usage(FILE * f, char * prg)
{
  fprintf(f,
	  "\n"
	  "Usage: %s [--locale LOCALE]\n"
	  "\n"
	  "LOCALE may be one of:\n"
	  "  C       (English      American English)\n"
	  "  eu_ES   (Baque        Euskara)\n"
	  "  nb_NO   (Bokmal)\n"
	  "  pt_BR   (Brazilian    Brazilian Portuguese   Portugues Brazilian)\n"
	  "  en_GN   (British      British English)\n"
	  "  ca_ES   (Catalan      Catala)\n"
	  "  zh_CN   (Chinese)\n"
	  "  cs_CZ   (Czech        Cesky)\n"
	  "  da_DK   (Danish       Dansk)\n"
	  "          (Dutch)\n"
	  "  fi_FI   (Finnish      Suomi)\n"
	  "  fr_FR   (French       Francais)\n"
	  "  de_DE   (German       Deutsch)\n"
	  "  el_GR   (Greek)\n"
	  "  he_IL   (Hebrew)\n"
	  "  hu_HU   (Hungarian    Magyar)\n"
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
	  "  es_ES   (Spanish      Espanol)\n"
	  "  sv_SE   (Swedish      Svenska)\n"
	  "  tr_TR   (Turkish)\n"
	  "  wa_BE   (Walloon)\n"
	  "\n",
	  prg);
}


const char *getfilename(const char* path)
{
  char    *p;

  if ( (p = strrchr( path, '\\' )) != NULL )
    return p+1;
  if ( (p = strrchr( path, '/' )) != NULL )
    return p+1;
  return path;
}


/* Setup: */

void setup(int argc, char * argv[])
{
  int i, ok_to_use_sysconfig;
  char str[128];
  char * upstr;
  SDL_Color black = {0, 0, 0, 0};
  char * homedirdir;
  FILE * fi;
  SDL_Surface * tmp_surf;
  SDL_Rect dest;
#ifndef LOW_QUALITY_COLOR_SELECTOR
  int x, y;
  SDL_Surface * tmp_btn;
  Uint8 r, g, b, a;
#endif


#ifdef __BEOS__
  /* if run from gui, like OpenTracker in BeOS or Explorer in Windows,
     find path from which binary was run and change dir to it
     so all files will be local :) */

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
      else if (strcmp(argv[i], "--800x600") == 0)
	{
	  WINDOW_WIDTH = 800;
	  WINDOW_HEIGHT = 600;
	}
      else if (strcmp(argv[i], "--640x480") == 0)
	{
	  WINDOW_WIDTH = 640;
	  WINDOW_HEIGHT = 480;
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
	  putenv("LANG=C");
	  putenv("LC_ALL=C");
	}
      else if (strcmp(langstr, "catalan") == 0 ||
	       strcmp(langstr, "catala") == 0)
	{
	  putenv("LANG=ca_ES");
	  putenv("LC_ALL=ca_ES");
	}
      else if (strcmp(langstr, "czech") == 0 ||
	       strcmp(langstr, "cesky") == 0)
	{
	  putenv("LANG=cs_CZ");
	  putenv("LC_ALL=cs_CZ");
	}
      else if (strcmp(langstr, "danish") == 0 ||
	       strcmp(langstr, "dansk") == 0)
	{
	  putenv("LANG=da_DK");
	  putenv("LC_ALL=da_DK");
	}
      else if (strcmp(langstr, "german") == 0 ||
	       strcmp(langstr, "deutsch") == 0)
	{
	  putenv("LANG=de_DE@euro");
	  putenv("LC_ALL=de_DE@euro");
	}
      else if (strcmp(langstr, "greek") == 0)
	{
	  putenv("LANG=el_GR.UTF8");
	  putenv("LC_ALL=el_GR.UTF8");
	}
      else if (strcmp(langstr, "british-english") == 0 ||
	       strcmp(langstr, "british") == 0)
	{
	  putenv("LANG=en_GB");
	  putenv("LC_ALL=en_GB");
	}
      else if (strcmp(langstr, "spanish") == 0 ||
	       strcmp(langstr, "espanol") == 0)
	{
	  putenv("LANG=es_ES@euro");
	  putenv("LC_ALL=es_ES@euro");
	}
      else if (strcmp(langstr, "finnish") == 0 ||
	       strcmp(langstr, "suomi") == 0)
	{
	  putenv("LANG=fi_FI@euro");
	  putenv("LC_ALL=fi_FI@euro");
	}
      else if (strcmp(langstr, "french") == 0 ||
	       strcmp(langstr, "francais") == 0)
	{
	  putenv("LANG=fr_FR@euro");
	  putenv("LC_ALL=fr_FR@euro");
	}
      else if (strcmp(langstr, "hebrew") == 0)
	{
	  putenv("LANG=he_IL");
	  putenv("LC_ALL=he_IL");
	}
      else if (strcmp(langstr, "hungarian") == 0 ||
	       strcmp(langstr, "magyar") == 0)
	{
	  putenv("LANG=hu_HU");
	  putenv("LC_ALL=hu_HU");
	}
      else if (strcmp(langstr, "indonesian") == 0 ||
	       strcmp(langstr, "bahasa-indonesia") == 0)
	{
	  putenv("LANG=id_ID");
	  putenv("LC_ALL=id_ID");
	}
      else if (strcmp(langstr, "icelandic") == 0 ||
	       strcmp(langstr, "islenska") == 0)
	{
	  putenv("LANG=is_IS");
	  putenv("LC_ALL=is_IS");
	}
      else if (strcmp(langstr, "italian") == 0 ||
	       strcmp(langstr, "italiano") == 0)
	{
	  putenv("LANG=it_IT@euro");
	  putenv("LC_ALL=it_IT@euro");
	}
      else if (strcmp(langstr, "japanese") == 0)
	{
	  putenv("LANG=ja_JP.UTF-8");
	  putenv("LC_ALL=ja_JP.UTF-8");
	}
      else if (strcmp(langstr, "bokmal") == 0)
	{
	  putenv("LANG=nb_NN");
	  putenv("LC_ALL=nb_NN");
	}
      else if (strcmp(langstr, "basque") == 0 || strcmp(langstr, "euskara") == 0)
	{
	  putenv("LANG=eu_ES");
	  putenv("LC_ALL=eu_ES");
	}
      else if (strcmp(langstr, "korean") == 0)
	{
	  putenv("LANG=ko_KR.UTF-8");
	  putenv("LC_ALL=ko_KR.UTF-8");
	}
      else if (strcmp(langstr, "tamil") == 0)
	{
	  putenv("LANG=ta_IN.UTF-8");
	  putenv("LC_ALL=ta_IN.UTF-8");
	}
      else if (strcmp(langstr, "lithuanian") == 0 ||
	       strcmp(langstr, "lietuviu") == 0)
	{
	  putenv("LANG=lt_LT");
	  putenv("LC_ALL=lt_LT");
	}
      else if (strcmp(langstr, "malay") == 0)
	{
	  putenv("LANG=ms_MY");
	  putenv("LC_ALL=ms_MY");
	}
      else if (strcmp(langstr, "dutch") == 0)
	{
	  putenv("LANG=nl_NL@euro");
	  putenv("LC_ALL=nl_NL@euro");
	}
      else if (strcmp(langstr, "norwegian") == 0 ||
	       strcmp(langstr, "nynorsk") == 0 ||
	       strcmp(langstr, "norsk") == 0)
	{
	  putenv("LANG=nn_NO");
	  putenv("LC_ALL=nn_NO");
	}
      else if (strcmp(langstr, "polish") == 0 ||
	       strcmp(langstr, "polski") == 0)
	{
	  putenv("LANG=pl_PL");
	  putenv("LC_ALL=pl_PL");
	}
      else if (strcmp(langstr, "brazilian-portuguese") == 0 ||
	       strcmp(langstr, "portugues-brazilian") == 0 ||
	       strcmp(langstr, "brazilian") == 0)
	{
	  putenv("LANG=pt_BR");
	  putenv("LC_ALL=pt_BR");
	}
      else if (strcmp(langstr, "portuguese") == 0 ||
	       strcmp(langstr, "portugues") == 0)
	{
	  putenv("LANG=pt_PT@euro");
	  putenv("LC_ALL=pt_PT@euro");
	}
      else if (strcmp(langstr, "romanian") == 0)
	{
	  putenv("LANG=ro_RO");
	  putenv("LC_ALL=ro_RO");
	}
      else if (strcmp(langstr, "russian") == 0 ||
	       strcmp(langstr, "russkiy") == 0)
	{
	  putenv("LANG=ru_RU.UTF-8");
	  putenv("LC_ALL=ru_RU.UTF-8");
	}
      else if (strcmp(langstr, "slovak") == 0)
	{
	  putenv("LANG=sk_SK");
	  putenv("LC_ALL=sk_SK");
	}
      else if (strcmp(langstr, "swedish") == 0 ||
	       strcmp(langstr, "svenska") == 0)
	{
	  putenv("LANG=sv_SE@euro");
	  putenv("LC_ALL=sv_SE@euro");
	}
      else if (strcmp(langstr, "turkish") == 0)
	{
	  putenv("LANG=tr_TR@euro");
	  putenv("LC_ALL=tr_TR@euro");
	}
      else if (strcmp(langstr, "walloon") == 0)
	{
	  putenv("LANG=wa_BE@euro");
	  putenv("LC_ALL=wa_BE@euro");
	}
      else if (strcmp(langstr, "chinese") == 0)
	{
	  putenv("LANG=zh_CN.UTF-8");
	  putenv("LC_ALL=zh_CN.UTF-8");
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
  bind_textdomain_codeset("tuxpaint", "UTF8");

  textdomain("tuxpaint");

  language = current_language();

  if (language == LANG_JA)
    {
      putenv("OUTPUT_CHARSET=ja_JP.UTF-8");
    }
  else if (language == LANG_HE)
    {
      putenv("OUTPUT_CHARSET=he_IL");
      convert_open("ISO8859-8");
    }
  else if (language == LANG_PL)
    {
      putenv("OUTPUT_CHARSET=pl_PL.UTF-8");
      convert_open("ISO8859-2");
    }
  else if (language == LANG_LT)
    {
      putenv("OUTPUT_CHARSET=lt_LT.UTF-8");
      convert_open("ISO8859-13");
    }
  else if (language == LANG_TA)
    {
      putenv("OUTPUT_CHARSET=ta_IN.UTF-8");
      convert_open("ISO8859-1");
    }


#ifdef DEBUG
  printf("DEBUG: Language is %s (%d)\n", lang_prefixes[language], language);
#endif

#ifndef WIN32
  putenv("SDL_VIDEO_X11_WMCLASS=TuxPaint.TuxPaint");
#endif
  


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
	  if (Mix_OpenAudio(44100, AUDIO_S16, 2, 256) < 0)
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


  /* Open Window: */

  if (fullscreen)
    {
#ifdef USE_HWSURFACE
      screen = SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT,
				16, SDL_FULLSCREEN | SDL_HWSURFACE);
#else
      screen = SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT,
				16, SDL_FULLSCREEN | SDL_SWSURFACE);
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
      screen = SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT, 16, SDL_HWSURFACE);
#else
      screen = SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT, 16, SDL_SWSURFACE);
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

  cursor_hand = get_cursor(hand_bits, hand_mask_bits,
			   hand_width, hand_height,
			   12, 1);
  
  cursor_wand = get_cursor(wand_bits, wand_mask_bits,
		           wand_width, wand_height,
		           4, 4);
  
  cursor_insertion = get_cursor(insertion_bits, insertion_mask_bits,
		                insertion_width, insertion_height,
		                7, 4);
  
  cursor_brush = get_cursor(brush_bits, brush_mask_bits,
			    brush_width, brush_height,
			    4, 28);
  
  cursor_crosshair = get_cursor(crosshair_bits, crosshair_mask_bits,
				crosshair_width, crosshair_height,
				15, 15);
  
  cursor_rotate = get_cursor(rotate_bits, rotate_mask_bits,
		             rotate_width, rotate_height,
		             15, 15);
  
  cursor_watch = get_cursor(watch_bits, watch_mask_bits,
			    watch_width, watch_height,
			    14, 14);
  
  cursor_arrow = get_cursor(arrow_bits, arrow_mask_bits,
			    arrow_width, arrow_height,
			    0, 0);

  cursor_up = get_cursor(up_bits, up_mask_bits,
			 up_width, up_height,
			 15, 1);

  cursor_down = get_cursor(down_bits, down_mask_bits,
			   down_width, down_height,
			   15, 30);

  cursor_tiny = get_cursor(tiny_bits, tiny_mask_bits,
			   tiny_width, tiny_height,
			   3, 3);

  do_setcursor(cursor_watch);
  


  /* Set window icon and caption: */

  seticon();
  SDL_WM_SetCaption("Tux Paint", "Tux Paint");


  /* Create drawing canvas: */

  canvas = SDL_CreateRGBSurface(screen->flags,
				WINDOW_WIDTH - (96 * 2),
				(48 * 7) + 40 + HEIGHTOFFSET,
				screen->format->BitsPerPixel,
				screen->format->Rmask,
				screen->format->Gmask,
				screen->format->Bmask,
				0);


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
  
  
  /* Init high quality scaling stuff: */
  
  InitLUTs(RGBtoYUV);


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


  /* FIXME: Hmm... how should we REALLY deal with this? */
#ifdef SVGA
  img_cursor_up = loadimage(DATA_PREFIX "images/ui/cursor_up_large.png");
  img_cursor_down = loadimage(DATA_PREFIX "images/ui/cursor_down_large.png");
#else
  img_cursor_up = loadimage(DATA_PREFIX "images/ui/cursor_up.png");
  img_cursor_down = loadimage(DATA_PREFIX "images/ui/cursor_down.png");
#endif

  img_scroll_up = loadimage(DATA_PREFIX "images/ui/scroll_up.png");
  img_scroll_down = loadimage(DATA_PREFIX "images/ui/scroll_down.png");

  img_scroll_up_off = loadimage(DATA_PREFIX "images/ui/scroll_up_off.png");
  img_scroll_down_off = loadimage(DATA_PREFIX "images/ui/scroll_down_off.png");

  img_paintcan = loadimage(DATA_PREFIX "images/ui/paintcan.png");

  show_progress_bar();

  img_sparkles = loadimage(DATA_PREFIX "images/ui/sparkles.png");


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
	  fprintf(stderr,
		  "\nWarning: Can't load font for this locale:\n"
		  "%s\n"
		  "The Simple DirectMedia Layer error that occurred was:\n"
		  "%s\n\n"
		  "Will use default (American English) instead.\n\n",
		  str, SDL_GetError());


	  /* Revert to default: */
      
	  putenv("LANG=C");
	  putenv("OUTPUT_CHARSET=C");
	  setlocale(LC_ALL, "C");

	  bindtextdomain("tuxpaint", LOCALEDIR);
	  textdomain("tuxpaint");
	  language = current_language();
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


      if (num_stamps == 0)
	{
	  fprintf(stderr,
		  "\nWarning: No stamps found in " DATA_PREFIX "stamps/\n"
		  "or %s\n\n", homedirdir);
	}

      free(homedirdir);


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
	    }
      

	  /* If Tux Paint is in mirror-image-by-default mode, mirror, if we can: */
      
	  if (mirrorstamps && inf_stamps[i]->mirrorable)
	    state_stamps[i]->mirrored = 1;
	  else
	    state_stamps[i]->mirrored = 0;

	  state_stamps[i]->flipped = 0;
	  state_stamps[i]->size = 100;

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
	  if (need_unicode(language) && locale_font != NULL &&
	      strcmp(gettext(title_names[i]), title_names[i]) != 0)
	    {
	      tmp_surf = TTF_RenderUNICODE_Blended(locale_font,
						   (Uint16 *) textdir(gettext(title_names[i])),
						   black);
	      img_title_names[i] = thumbnail(tmp_surf,
					     min(84, tmp_surf->w), tmp_surf->h, 0);
	      SDL_FreeSurface(tmp_surf);
	    }
	  else if (need_utf8(language) && locale_font != NULL &&
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
	      tmp_surf = RENDER_TEXT(large_font, upstr, black);
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

  tmp_btn = thumbnail(img_btn_down, (WINDOW_WIDTH - 96) / NUM_COLORS, 48, 0);


  /* Create surfaces to draw them into: */

  for (i = 0; i < NUM_COLORS; i++)
    {

      img_color_btns[i] = SDL_CreateRGBSurface(screen->flags,
					       /* (WINDOW_WIDTH - 96) / NUM_COLORS, 48, */
					       tmp_btn->w, tmp_btn->h,
					       screen->format->BitsPerPixel,
					       screen->format->Rmask,
					       screen->format->Gmask,
					       screen->format->Bmask,
					       0);

      if (img_color_btns[i] == NULL)
	{
	  fprintf(stderr, "\nError: Can't build color button! (%d of %d)\n"
		  "The Simple DirectMedia Layer error that occurred was:\n"
		  "%s\n\n", i + 1, NUM_COLORS, SDL_GetError());

	  cleanup();
	  exit(1);
	}

      SDL_LockSurface(img_color_btns[i]);
    }


  /* Generate the buttons based on the thumbnails: */

  SDL_LockSurface(tmp_btn);

  for (y = 0; y < tmp_btn->h /* 48 */; y++)
    {
      for (x = 0; x < tmp_btn->w /* (WINDOW_WIDTH - 96) / NUM_COLORS */; x++)
	{
	  SDL_GetRGB(getpixel(tmp_btn, x, y), tmp_btn->format,
		     &r, &g, &b);
	  a = 255 - ((r + g + b) / 3);

	  for (i = 0; i < NUM_COLORS; i++)
	    {
	      putpixel(img_color_btns[i], x, y,
		       SDL_MapRGB(img_color_btns[i]->format,
				  alpha(color_hexes[i][0], 255, a),
				  alpha(color_hexes[i][1], 255, a),
				  alpha(color_hexes[i][2], 255, a)));
	    }
	}
    }

  for (i = 0; i < NUM_COLORS; i++)
    SDL_UnlockSurface(img_color_btns[i]);

  SDL_UnlockSurface(tmp_btn);
  SDL_FreeSurface(tmp_btn);

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

SDL_Surface * do_render_button_label(char * label)
{
  char * str;
  SDL_Surface * tmp_surf, * surf;
  SDL_Color black = {0, 0, 0, 0};

  if (need_unicode(language) && locale_font != NULL &&
      strcmp(gettext(label), label) != 0)
    {
      tmp_surf = TTF_RenderUNICODE_Blended(locale_font, (Uint16 *) textdir(gettext(label)),
					   black);
      surf = thumbnail(tmp_surf, min(48, tmp_surf->w), tmp_surf->h, 0);
    }
  else if (need_utf8(language) && locale_font != NULL &&
	   strcmp(gettext(label), label) != 0)
    {
      tmp_surf = TTF_RenderUTF8_Blended(locale_font, textdir(gettext(label)), black);
      surf = thumbnail(tmp_surf, min(48, tmp_surf->w), tmp_surf->h, 0);
    }
  else
    {
      str = uppercase(textdir(gettext(label)));
      tmp_surf = RENDER_TEXT(small_font, str, black);
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


void seticon(void)
{
  int masklen;
  Uint8 * mask;
  SDL_Surface * icon;


  /* Load icon into a surface: */

#ifndef WIN32
  icon = IMG_Load(DATA_PREFIX "images/icon.png");
#else
  icon = IMG_Load(DATA_PREFIX "images/icon-win32.png");
#endif

  if (icon == NULL)
    {
      fprintf(stderr,
	      "\nWarning: I could not load the icon image: %s\n"
	      "The Simple DirectMedia error that occurred was:\n"
	      "%s\n\n", DATA_PREFIX "images/icon.png", SDL_GetError());
      return;
    }


  /* Create mask: */

  masklen = (((icon -> w) + 7) / 8) * (icon -> h);
  mask = malloc(masklen * sizeof(Uint8));
  memset(mask, 0xFF, masklen);


  /* Set icon: */

  SDL_WM_SetIcon(icon, mask);


  /* Free icon surface & mask: */

  free(mask);
  SDL_FreeSurface(icon);


  /* Grab keyboard and mouse, if requested: */

  if (grab_input)
    {
      debug("Grabbing input!");
      SDL_WM_GrabInput(SDL_GRAB_ON);
    }
}


/* Load a mouse pointer (cursor) shape: */

SDL_Cursor * get_cursor(char * bits, char * mask_bits,
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

SDL_Surface * loadimage(char * fname)
{
  return(do_loadimage(fname, 1));
}


/* Load an image: */

SDL_Surface * do_loadimage(char * fname, int abort_on_error)
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

void draw_toolbar(void)
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

void draw_magic(void)
{
  int i;
  SDL_Rect dest;


  /* FIXME: Should we worry about more than 14 magic effects? :^/ */


  draw_image_title(TITLE_MAGIC, WINDOW_WIDTH - 96);

  for (i = 0; i < 14 + TOOLOFFSET; i++)
    {
      dest.x = WINDOW_WIDTH - 96 + ((i % 2) * 48);
      dest.y = ((i / 2) * 48) + 40;


      if (i < NUM_MAGICS)
	{
	  if (i == cur_magic)
	    {
	      SDL_BlitSurface(img_btn_down, NULL, screen, &dest);
	    }
	  else
	    {
	      SDL_BlitSurface(img_btn_up, NULL, screen, &dest);
	    }
	
	  dest.x = WINDOW_WIDTH - 96 + ((i % 2) * 48) + 4;
	  dest.y = ((i / 2) * 48) + 40 + 4;

	  SDL_BlitSurface(img_magics[i], NULL, screen, &dest);

	
	  dest.x = WINDOW_WIDTH - 96 + ((i % 2) * 48) + 4 +
	    (40 - img_magic_names[i]->w) / 2;
	  dest.y = ((i / 2) * 48) + 40 + 4 + (44 - img_magic_names[i]->h);

	  SDL_BlitSurface(img_magic_names[i], NULL, screen, &dest);
	}
      else
	{
	  SDL_BlitSurface(img_btn_off, NULL, screen, &dest);	
	}
    }
}


/* Draw color selector: */

void draw_colors(int enabled)
{
  int i;
  SDL_Rect dest;

  dest.x = 0;
  dest.y = 40 + ((NUM_TOOLS / 2) * 48) + HEIGHTOFFSET;

  if (enabled)
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
#ifndef LOW_QUALITY_COLOR_SELECTOR
      dest.x = (i * ((WINDOW_WIDTH - 96) / NUM_COLORS)) + 96;
      dest.y = 40 + ((NUM_TOOLS / 2) * 48) + HEIGHTOFFSET;

      if (enabled)
	SDL_BlitSurface(img_color_btns[i], NULL, screen, &dest);
      else
	SDL_BlitSurface(img_color_btns[COLOR_WHITE], NULL, screen, &dest);
#else
      dest.x = (i * ((WINDOW_WIDTH - 96) / NUM_COLORS)) + 96;
      dest.y = 40 + ((NUM_TOOLS / 2) * 48) + HEIGHTOFFSET;
      dest.w = ((WINDOW_WIDTH - 96) / NUM_COLORS);
      dest.h = 48 + HEIGHTOFFSET;

      if (enabled)
	SDL_FillRect(screen, &dest,
		     SDL_MapRGB(screen->format,
				color_hexes[i][0],
				color_hexes[i][1],
				color_hexes[i][2]));
      else
	SDL_FillRect(screen, &dest,
		     SDL_MapRGB(screen->format, 240, 240, 240));
#endif

      if (i == cur_color && enabled)
	{
	  dest.x = (i * ((WINDOW_WIDTH - 96) / NUM_COLORS)) + 96;
	  dest.y = 40 + ((NUM_TOOLS / 2) * 48) + HEIGHTOFFSET;

	  SDL_BlitSurface(img_paintcan, NULL, screen, &dest);
	}
    }


  /* Keep track of this globally, so the cursor shape will act right */

  colors_are_selectable = enabled;
}


/* Draw brushes: */

void draw_brushes(void)
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

void draw_fonts(void)
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
	  tmp_surf = RENDER_TEXT(fonts[font], "A", black);

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

void draw_stamps(void)
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

void draw_shapes(void)
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




/* Draw no selectables: */

void draw_none(void)
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
void loadarbitrary(SDL_Surface * surfs[], SDL_Surface * altsurfs[],
		   char * descs[], info_type * infs[],
		   Mix_Chunk * sounds[],
		   int * count, int starting, int max,
		   char * dir, int fatal, int maxw, int maxh)
#else
     void loadarbitrary(SDL_Surface * surfs[], SDL_Surface * altsurfs[],
			char * descs[], info_type * infs[],
			int * count, int starting, int max,
			char * dir, int fatal, int maxw, int maxh)
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

SDL_Surface * thumbnail(SDL_Surface * src, int max_x, int max_y,
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
  
  s = SDL_CreateRGBSurface(SDL_SWSURFACE,
		           max_x, max_y,
			   src->format->BitsPerPixel,
			   src->format->Rmask,
			   src->format->Gmask,
			   src->format->Bmask,
			   amask);


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

Uint32 getpixel(SDL_Surface * surface, int x, int y)
{
  int bpp;
  Uint8 * p;
  Uint32 pixel;

  pixel = 0;


  /* Assuming the X/Y values are within the bounds of this surface... */

  if (x >= 0 && y >= 0 && x < surface -> w && y < surface -> h)
    {
      /* SDL_LockSurface(surface); */


      /* Determine bytes-per-pixel for the surface in question: */

      bpp = surface->format->BytesPerPixel;


      /* Set a pointer to the exact location in memory of the pixel
         in question: */

      p = (Uint8 *) (((Uint8 *)surface->pixels) +  /* Start at top of RAM */
		     (y * surface->pitch) +  /* Go down Y lines */
		     (x * bpp));             /* Go in X pixels */


      /* Return the correctly-sized piece of data containing the
       * pixel's value (an 8-bit palette value, or a 16-, 24- or 32-bit
       * RGB value) */

      if (bpp == 1)         /* 8-bit display */
        pixel = *p;
      else if (bpp == 2)    /* 16-bit display */
        pixel = *(Uint16 *)p;
      else if (bpp == 3)    /* 24-bit display */
        {
          /* Depending on the byte-order, it could be stored RGB or BGR! */

          if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
            pixel = p[0] << 16 | p[1] << 8 | p[2];
          else
            pixel = p[0] | p[1] << 8 | p[2] << 16;
        }
      else if (bpp == 4)    /* 32-bit display */
        pixel = *(Uint32 *)p;

      /* SDL_UnlockSurface(surface); */
    }

  return pixel;
}


/* Draw a single pixel into the surface: */

void putpixel(SDL_Surface * surface, int x, int y, Uint32 pixel)
{
  int bpp;
  Uint8 * p;


  /* Assuming the X/Y values are within the bounds of this surface... */

  if (x >= 0 && y >= 0 && x < surface->w && y < surface->h)
    {
      /* SDL_LockSurface(surface); */


      /* Determine bytes-per-pixel for the surface in question: */

      bpp = surface->format->BytesPerPixel;


      /* Set a pointer to the exact location in memory of the pixel
       *          in question: */

      p = (Uint8 *) (((Uint8 *)surface->pixels) + /* Start: beginning of RAM */
		     (y * surface->pitch) +  /* Go down Y lines */
                     (x * bpp));             /* Go in X pixels */


      /* Set the (correctly-sized) piece of data in the surface's RAM
       *          to the pixel value sent in: */

      if (bpp == 1)
        *p = pixel;
      else if (bpp == 2)
        *(Uint16 *)p = pixel;
      else if (bpp == 3)
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
      else if (bpp == 4)
        {
          *(Uint32 *)p = pixel;
        }

      /* SDL_UnlockSurface(surface); */
    }
}


/* Show debugging stuff: */

void debug(char * str)
{
#ifdef DEBUG
  fprintf(stderr, "DEBUG: %s\n", str);
  fflush(stderr);
#endif
}


/* Undo! */

void do_undo(void)
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

void do_redo(void)
{
  if (cur_undo != newest_undo)
    {
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

void render_brush(void)
{
  Uint32 amask;
  int x, y;
  Uint8 r, g, b, a;


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

void playsound(int chan, int s, int override)
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

void line_xor(int x1, int y1, int x2, int y2)
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


/* Should really clip at the line level, but oh well... */

void clipped_putpixel(SDL_Surface * dest, int x, int y, Uint32 c)
{
  if (x >= 96 && x < (WINDOW_WIDTH - 96) &&
      y >= 0 && y < (48 * 7 + 40 + HEIGHTOFFSET))
    {
      putpixel(dest, x, y, c);
    }
}


/* Draw a XOR rectangle: */

void rect_xor(int x1, int y1, int x2, int y2)
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

void do_eraser(int x, int y)
{
  SDL_Rect dest;

  dest.x = x - 48;
  dest.y = y - 48;
  dest.w = 96;
  dest.h = 96;

  SDL_FillRect(canvas, &dest,
	       SDL_MapRGB(canvas->format, 255, 255, 255));


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

  update_canvas(x - 48, y - 48, x + 48, y + 48);

  rect_xor(x - 48, y - 48,
           x + 48, y + 48);
}


/* Reset available tools (for new image / starting out): */

void reset_avail_tools(void)
{
  int i;

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


#ifdef WIN32
  disable_print = !IsPrinterAvailable();
#endif

#ifdef __BEOS__
  disable_print = !IsPrinterAvailable();
#endif

  
  /* Disable print? */

  if (disable_print)
    tool_avail[TOOL_PRINT] = 0;
}


/* Save and disable available tools (for Open-Dialog) */

void disable_avail_tools(void)
{
  int i;

  for (i = 0; i < NUM_TOOLS; i++)
    {
      tool_avail_bak[i] = tool_avail[i];
      tool_avail[i]=0;
    }
}

/* Restore and enable available tools (for End-Of-Open-Dialog) */

void enable_avail_tools(void)
{
  int i;

  for (i = 0; i < NUM_TOOLS; i++)
    {
      tool_avail[i] = tool_avail_bak[i];
    }
}


/* Update a rect. based on two x/y coords (not necessarly in order): */

void update_screen(int x1, int y1, int x2, int y2)
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


/* Build a color based on two colors and an alpha... */

Uint8 alpha(Uint8 c1, Uint8 c2, Uint8 a)
{
  Uint16 c, nc1, nc2, na;

  na = a;
  nc1 = c1;
  nc2 = c2;

  if (nc1 > 200)
    nc1 = 200;

  c = ((nc1 * na) / 255 + (nc2 * (255 - na)) / 255);

  return (Uint8) c;
}


/* For qsort() call in loadarbitrary()... */

int compare_strings(char * * s1, char * * s2)
{
  return (strcmp(*s1, *s2));
}


/* For qsort() call in do_open()... */

int compare_dirents(struct dirent * f1, struct dirent * f2)
{
#ifdef DEBUG
  printf("compare_dirents: %s\t%s\n", f1->d_name, f2->d_name);
#endif

  return (strcmp(f1->d_name, f2->d_name));
}


/* Draw tux's text on the screen: */

void draw_tux_text(int which_tux, char * str, int want_utf8,
	 	   int force_locale_font, int want_right_to_left)
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


  wordwrap_text(font, str, black,
	        img_tux[which_tux] -> w + 5,
	        (48 * 7) + 40 + 48 + HEIGHTOFFSET + 5,
	        WINDOW_WIDTH, want_utf8, force_locale_font,
		want_right_to_left);


  /* Update the display: */

  SDL_UpdateRect(screen,
		 0, (48 * 7) + 40 + 48 + HEIGHTOFFSET,
		 WINDOW_WIDTH,
		 WINDOW_HEIGHT - ((48 * 7) + 40 + 48 + HEIGHTOFFSET));
}


void wordwrap_text(TTF_Font * font, char * str, SDL_Color color,
		   int left, int top, int right, int want_utf8,
		   int force_locale_font, int want_right_to_left)
{
  int x, y, i, j;
  char substr[512];
  unsigned char * locale_str;
  char * tstr;
  Uint16 unicode_char[2];
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

  if (strcmp(str, "") != 0 &&
      (want_utf8 ||
       (need_utf8(language) && strcmp(gettext(str), str) != 0)))
    {
      if (want_utf8 || want_right_to_left == 0)
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
				  SDL_FreeSurface(text);

				  x = x + text->w;
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
		      SDL_FreeSurface(text);
		      x = x + text->w;
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
  else if (need_unicode(language) && locale_font != NULL &&
	   strcmp(gettext(str), str) != 0 && strcmp(str, "") != 0)
    {
      if (want_right_to_left == 0)
	locale_str = strdup(gettext(str));
      else
	locale_str = strdup(textdir(gettext(str)));
    
    
      /* For each pair of bytes... */
    
      for (i = 0; i < strlen(locale_str); i = i + 2)
	{
	  /* FIXME: Is this endian-safe? */
	    
	  unicode_char[0] = (locale_str[i] << 8) + (locale_str[i + 1]);
	  unicode_char[1] = 0;
      
	  text = TTF_RenderUNICODE_Blended(locale_font, unicode_char, color);


	  /* Wrap, if needed: */
      
	  if (x + text->w > right)
	    {
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

	  x = x + text->w;

	  last_text_height = text->h;
	  SDL_FreeSurface(text);
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

	  //if (force_locale_font && locale_font != NULL)
	  //  text = RENDER_TEXT(locale_font, substr, color);
	  //else
	  text = RENDER_TEXT(font, substr, color);


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

Mix_Chunk * loadsound(char * fname)
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

void strip_trailing_whitespace( char *buf )
{
  while (buf[strlen(buf) - 1] == ' ' ||
         buf[strlen(buf) - 1] == '\r' ||
         buf[strlen(buf) - 1] == '\n')
    {
      buf[strlen(buf) - 1] = '\0';
    }
}


/* Load a file's description: */

char * loaddesc(char * fname)
{
  char * txt_fname;
  char buf[256], def_buf[256];
  int found, got_first, in_utf8, in_html_escape;
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
      in_utf8 = 0;
      in_html_escape = 0;

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

		  if (buf[strlen(lang_prefixes[language])] == '=')
		    {
		      found = 1;
		      in_utf8 = 0;
		    }
		  else if (strstr(buf + strlen(lang_prefixes[language]), ".utf8=") ==
			   buf + strlen(lang_prefixes[language]))
		    {
		      found = 1;
		      in_utf8 = 1;

		      debug("...IS UTF-8!");
		    }
		  else if (strstr(buf + strlen(lang_prefixes[language]), ".esc=") ==
			   buf + strlen(lang_prefixes[language]))
		    {
		      found = 1;
		      in_html_escape = 1;

		      debug("...IS HTML ESCAPED!");
		    }
		}
	    }
	}
      while (!feof(fi) && !found);

      fclose(fi);


      /* Return the string: */

      if (found)
	{
	  if (in_utf8 == 1)
	    {
	      /* UTF format!  Decode! */

	      /* FIXME: Stupid... Using '=' at beginning to mark as a UTF8 string */

	      return(strdup(buf + (strlen(lang_prefixes[language])) + 5));
	    }
	  else if (in_html_escape == 1)
	    {
	      /* HTML escape-code style!  Unescape! */
	      
	      return(unescape(buf + (strlen(lang_prefixes[language])) + 5));
	    }
	  else
	    {
	      /* Plain old ASCII... just copy it: */

	      return(strdup(buf + (strlen(lang_prefixes[language])) + 1));
	    }
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

info_type * loadinfo(char * fname)
{
  char * dat_fname;
  char buf[256];
  info_type inf;
  info_type * inf_ret;
  FILE * fi;


  /* Clear info struct first: */

  inf.colorable = 0;
  inf.tintable = 0;
  inf.mirrorable = 1;
  inf.flipable = 1;


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

SDL_Surface * loadaltimage(char * fname)
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

void do_wait(void)
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


/* Load current (if any) image: */

void load_current(void)
{
  SDL_Surface * tmp;
  char * fname;
  char ftmp[1024];
  FILE * fi;
  SDL_Rect dest;


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
	}
      else
	{
	  SDL_FillRect(canvas, NULL, SDL_MapRGB(canvas->format, 255, 255, 255));

	  dest.x = (canvas->w - tmp->w) / 2;
	  dest.y = (canvas->h - tmp->h) / 2;
	  SDL_BlitSurface(tmp, NULL, canvas, &dest);

	  SDL_FreeSurface(tmp);

	  tool_avail[TOOL_NEW] = 1;
	}

      free(fname);
    }
}


/* Save the current image to disk: */

void save_current(void)
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

      draw_tux_text(TUX_OOPS, strerror(errno), 0, 0, 0);
    }

  free(fname);


  fname = get_fname("current_id.txt");

  fi = fopen(fname, "w");
  if (fi == NULL)
    {
      fprintf(stderr,
	      "\nError: Can't keep track of current image.\n"
	      "%s\n"
	      "The error that occred was:\n"
	      "%s\n\n", fname, strerror(errno));

      draw_tux_text(TUX_OOPS, strerror(errno), 0, 0, 0);
    }
  else
    {
      fprintf(fi, "%s\n", file_id);
      fclose(fi);
    }

  free(fname);
}


/* The filename for the current image: */

char * get_fname(char * name)
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
  /* Macintosh: It's under ~/Library/Preferences/tuxpaint */
  
  tux_settings_dir = "Library/Preferences/tuxpaint";
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

      if (*name == '\0')
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

int do_prompt(char * text, char * btn_yes, char * btn_no)
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

  wordwrap_text(font, text, black,
		166 + PROMPTOFFSETX, 100 + PROMPTOFFSETY, 475 + PROMPTOFFSETX, 0, 0, 1);


  /* Draw yes button: */

  dest.x = 166 + PROMPTOFFSETX;
  dest.y = 178 + PROMPTOFFSETY;
  SDL_BlitSurface(img_yes, NULL, screen, &dest);

  wordwrap_text(font, btn_yes, black, 166 + PROMPTOFFSETX + 48 + 4,
		183 + PROMPTOFFSETY, 475 + PROMPTOFFSETX, 0, 0, 1);


  /* Draw no button: */

  if (strlen(btn_no) != 0)
    {
      dest.x = 166 + PROMPTOFFSETX;
      dest.y = 230 + PROMPTOFFSETY;
      SDL_BlitSurface(img_no, NULL, screen, &dest);

      wordwrap_text(font, btn_no, black,
		    166 + PROMPTOFFSETX + 48 + 4, 235 + PROMPTOFFSETY,
		    475 + PROMPTOFFSETX, 0, 0, 1);
    }


  /* Draw Tux, waiting... */

  draw_tux_text(TUX_BORED, "", 0, 0, 0);

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
		      /* ESCAPE also simply dismisses if there's no Yes/No choice: */
	
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

void cleanup(void)
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

  free_surface( &img_scroll_up );
  free_surface( &img_scroll_down );
  free_surface( &img_scroll_up_off );
  free_surface( &img_scroll_down_off );

  free_surface( &img_paintcan );

  free_surface( &img_sparkles );

  free_surface_array( undo_bufs, NUM_UNDO_BUFS );
#ifndef LOW_QUALITY_COLOR_SELECTOR
  free_surface_array( img_color_btns, NUM_COLORS );
#endif
  free_surface_array( img_stamp_thumbs, MAX_STAMPS );

  free_surface( &screen );
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


  /* Close up! */

  TTF_Quit();
  SDL_Quit();

  convert_close();
}


void free_cursor(SDL_Cursor ** cursor)
{
  if (*cursor)
    {
      SDL_FreeCursor(*cursor);
      *cursor = NULL;
    }
}


void free_surface(SDL_Surface **surface_array)
{
  if (*surface_array)
    {
      SDL_FreeSurface(*surface_array);
      *surface_array = NULL;
    }
}


void free_surface_array(SDL_Surface *surface_array[], int count)
{
  int i;

  for (i = 0; i < count; ++i)
    {
      free_surface(&surface_array[i]);
    }
}


/* Update screen where shape is/was: */

void update_shape(int cx, int ox1, int ox2, int cy, int oy1, int oy2, int fix)
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


/* Draw a shape! */

void do_shape(int cx, int cy, int ox, int oy, int rotn, int use_brush)
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

int rotation(int ctr_x, int ctr_y, int ox, int oy)
{
  return(atan2(oy - ctr_y, ox - ctr_x) * 180 / M_PI);
}


/* FIXME: Move elsewhere!!! */

#define PROMPT_SAVE_OVER_TXT gettext_noop("Save over the older version of this drawing?")
#define PROMPT_SAVE_OVER_YES gettext_noop("Yes")
#define PROMPT_SAVE_OVER_NO  gettext_noop("No, save a new file")


/* Save the current image: */

int do_save(void)
{
  int res;
  char * fname;
  char tmp[1024];
  SDL_Surface * thm;
#ifndef SAVE_AS_BMP
  FILE * fi;
#endif


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

      draw_tux_text(TUX_OOPS, SDL_GetError(), 0, 0, 0);

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

      draw_tux_text(TUX_OOPS, SDL_GetError(), 0, 0, 0);

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

      draw_tux_text(TUX_OOPS, SDL_GetError(), 0, 0, 0);

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

      draw_tux_text(TUX_OOPS, SDL_GetError(), 0, 0, 0);

      free(fname);
      return 0;
    }
  else
    {
      /* Ta-Da! */

      playsound(0, SND_SAVE, 1);
      draw_tux_text(TUX_DEFAULT, tool_tips[TOOL_SAVE], 0, 0, 1);
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

      draw_tux_text(TUX_OOPS, strerror(errno), 0, 0, 0);
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
  

  /* All happy! */

  playsound(0, SND_SAVE, 1);
  draw_tux_text(TUX_DEFAULT, tool_tips[TOOL_SAVE], 0, 0, 1);
  do_setcursor(cursor_arrow);

  return 1;
}


/* Actually save the PNG data to the file stream: */

int do_png_save(FILE * fi, char * fname, SDL_Surface * surf)
{
  png_structp png_ptr;
  png_infop info_ptr;
  unsigned char ** png_rows;
  Uint8 r, g, b;
  int x, y;
  
  
  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png_ptr == NULL)
    {
      fclose(fi);
      png_destroy_write_struct(&png_ptr, (png_infopp) NULL);

      fprintf(stderr, "\nError: Couldn't save the image!\n%s\n\n", fname);
      draw_tux_text(TUX_OOPS, strerror(errno), 0, 0, 0);
    }
  else
    {
      info_ptr = png_create_info_struct(png_ptr);
      if (info_ptr == NULL)
	{
	  fclose(fi);
	  png_destroy_write_struct(&png_ptr, (png_infopp) NULL);

	  fprintf(stderr, "\nError: Couldn't save the image!\n%s\n\n", fname);
	  draw_tux_text(TUX_OOPS, strerror(errno), 0, 0, 0);
	}
      else
	{
	  if (setjmp(png_jmpbuf(png_ptr)))
	    {
	      fclose(fi);
	      png_destroy_write_struct(&png_ptr, (png_infopp) NULL);

	      fprintf(stderr, "\nError: Couldn't save the image!\n%s\n\n", fname);
	      draw_tux_text(TUX_OOPS, strerror(errno), 0, 0, 0);
  
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

void get_new_file_id(void)
{
  time_t t;

  t = time(NULL);

  strftime(file_id, sizeof(file_id), "%Y%m%d%H%M%S", localtime(&t));
  debug(file_id);


  /* FIXME: Show thumbnail and prompt for title: */
}


/* Handle quitting (and prompting to save, if necessary!) */

int do_quit(void)
{
  int done;

  done = do_prompt(PROMPT_QUIT_TXT,
	           PROMPT_QUIT_YES,
		   PROMPT_QUIT_NO);

  if (done && !been_saved)
    {
      if (do_prompt(PROMPT_QUIT_SAVE_TXT,
		    PROMPT_QUIT_SAVE_YES,
		    PROMPT_QUIT_SAVE_NO))
	{
	  if (do_save())
	    {
	      do_prompt(tool_tips[TOOL_SAVE],
			"Okay",
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

int do_open(int want_new_tool)
{
  SDL_Surface * img, * img1, * img2;
  int things_alloced;
  SDL_Surface * * thumbs = NULL;
  DIR * d;
  struct dirent * f;
#ifndef __BEOS__
  struct dirent * fs;
#endif
  char * dirname, * rfname;
  char * * d_names = NULL, * * d_exts = NULL;
  FILE * fi;
  char fname[1024];
  char * tmp_fname;
  int num_files, i, done, update_list, want_erase, cur, which,
    num_files_in_dir, j, res;
  SDL_Rect dest;
  SDL_Event event;
  SDLKey key;
  Uint32 last_click_time;
  int last_click_which, last_click_button;
#ifdef __BEOS__
  char * dot = NULL;
#endif



  do_setcursor(cursor_watch);


  /* Open directory of images: */

  dirname = get_fname("saved");

  d = opendir(dirname);
  if (d == NULL)
    {
      fprintf(stderr,
	      "\nWarning: There's no directory of saved images\n"
	      "%s\n"
	      "The system error that occurred was: %s\n",
	      dirname, strerror(errno));
    }


  /* Allocate some space: */

  things_alloced = 32;
#ifndef __BEOS__
  fs = (struct dirent *) malloc(sizeof(struct dirent) * things_alloced);
#else
  thumbs = (SDL_Surface * *) malloc(sizeof(SDL_Surface *) * things_alloced);
  d_names = (char * *) malloc(sizeof(char *) * things_alloced);
  d_exts = (char * *) malloc(sizeof(char *) * things_alloced);
#endif


  /* Read directory of images and build thumbnails: */

  num_files = 0;
  
  if (d != NULL)
    {
      /* Gather list of files (for sorting): */

#ifdef __BEOS__
      num_files_in_dir = 0;
      do
	{
	  f = readdir(d);
	  
	  if (f && (dot = strstr(f->d_name, FNAME_EXTENSION)) != NULL)
	    {
	      if( strstr(f->d_name, "-t") == NULL)
		{
		  d_exts[num_files_in_dir] = strdup(dot);
		  *dot = 0;
		  d_names[num_files_in_dir] = strdup(f->d_name);
		  
		  
		  /* Try to load thumbnail first: */
		  
		  snprintf(fname, sizeof(fname), "%s/%s-t%s",
			   dirname, d_names[num_files], FNAME_EXTENSION);
		  
		  img = IMG_Load(fname);
		  if (img != NULL)
		    {
		      show_progress_bar();
		      thumbs[num_files] = SDL_DisplayFormat(img);
		      SDL_FreeSurface(img);
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
	
		      snprintf(fname, sizeof(fname), "%s/%s%s",
			       dirname, d_names[num_files], FNAME_EXTENSION);
		      img = IMG_Load(fname);
		      show_progress_bar();
		      if (img != NULL)
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

	      
			  /* Let's save this thumbnail, so we don't have to create it
			     again next time 'Open' is called: */
	      
			  debug("Saving thumbnail for this one!");
			  snprintf(fname, sizeof(fname), "%s/%s-t%s",
				   dirname, d_names[num_files], FNAME_EXTENSION);
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
	        
			      /* NOTE: fi is closed there so no need to fclose it here */
			    }

			  show_progress_bar();
			  num_files++;
			}
		    }

		  *dot = '.';
		  num_files_in_dir++;


		  if (num_files_in_dir > things_alloced)
		    {
		      things_alloced = things_alloced + 32;
	    
		      thumbs = (SDL_Surface * *)
			realloc(thumbs, sizeof(SDL_Surface *) * things_alloced);
	    
		      d_names = (char * *)
			realloc(d_names, sizeof(char *) * things_alloced);

		      d_exts = (char * *)
			realloc(d_exts, sizeof(char *) * things_alloced);
		    }
		}
	    }
	}
      while (f != NULL && num_files_in_dir < MAX_FILES);

      closedir(d);
  
#else
      
      num_files_in_dir = 0;
      do
	{
	  f = readdir(d);
	  
	  if (f != NULL)
	    {
	      memcpy(&(fs[num_files_in_dir]), f, sizeof(struct dirent));
	      num_files_in_dir++;
	      
	      if (num_files_in_dir >= things_alloced)
		{
		  things_alloced = things_alloced + 32;
		  fs = (struct dirent *) realloc(fs, sizeof(struct dirent) * things_alloced);
		}
	    }
	}
      while (f != NULL);
      
      
      thumbs = (SDL_Surface * *) malloc(sizeof(SDL_Surface *) * num_files_in_dir);
      d_names = (char * *) malloc(sizeof(char *) * num_files_in_dir);
      d_exts = (char * *) malloc(sizeof(char *) * num_files_in_dir);
      
      
      
      closedir(d);
#endif
      
      
      /* Sort: */
      
      qsort(fs, num_files_in_dir, sizeof(struct dirent),
	    (int(*)(const void *, const void *))compare_dirents);
      
      
      /* Read directory of images and build thumbnails: */
      
      for (j = 0; j < num_files_in_dir; j++)
	{
	  f = &(fs[j]);

	  show_progress_bar();
      
	  if (f != NULL)
	    {
	      debug(f->d_name);
	      
	      if (strstr(f->d_name, "-t") == NULL)
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
		      
		      
		      /* Try to load thumbnail first: */
		      
		      snprintf(fname, sizeof(fname), "%s/.thumbs/%s-t.png",
			       dirname, d_names[num_files]);
		      debug(fname);
		      img = IMG_Load(fname);
		      
		      if (img == NULL)
			{
			  /* No thumbnail in the new location ("saved/.thumbs"),
			     try the old locatin ("saved/"): */
			  
			  snprintf(fname, sizeof(fname), "%s/%s-t.png", dirname,
				   d_names[num_files]);
			  debug(fname);
			  
			  img = IMG_Load(fname);
			}
		      
		      if (img != NULL)
			{
			  show_progress_bar();
			  
			  thumbs[num_files] = SDL_DisplayFormat(img);
			  SDL_FreeSurface(img);
		      
			  if (thumbs[num_files] == NULL)
			    {
			      fprintf(stderr,
				      "\nError: Couldn't create a thumbnail of "
				      "saved image!\n"
				      "%s\n", fname);
			    }
			  
			  num_files++;
			}
		      
		      if (img == NULL)
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
			  
			  
			  
			  snprintf(fname, sizeof(fname), "%s/%s",
				   dirname, f->d_name);
			  debug(fname);
#ifdef SAVE_AS_BMP
			  img = SDL_LoadBMP(fname);
#else
			  img = IMG_Load(fname);
#endif
		    
			  show_progress_bar();
			  
			  if (img == NULL)
			    {
			      fprintf(stderr,
				      "\nWarning: I can't open one of the "
				      "saved files!\n"
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
			      
			      debug("Saving thumbnail for this one!");
			      
			      snprintf(fname, sizeof(fname), "%s/.thumbs/%s-t.png",
				       dirname, d_names[num_files]);
			      
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
			      
			      
			      num_files++;
			    }
			}
		    }
		}
	      else
		{
		  /* It was a thumbnail file ("...-t.png") */
		}
	    }
	}
      
      free(dirname);
      
      
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
					     "then click 'Open'")), 0, 0, 1);
      
	  cur = 0;
	  update_list = 1;
	  want_erase = 0;
      
	  done = 0;
	  which = 0;
      
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
		  
		      if (i == which)
			{
			  SDL_BlitSurface(img_cursor_down, NULL, screen, &dest);
			  debug(d_names[i]);
			}
		      else
			SDL_BlitSurface(img_cursor_up, NULL, screen, &dest);
		  
		  
		  
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
		  SDL_BlitSurface(img_erase, NULL, screen, &dest);
	      
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
			   event.button.y < (48 * 7 + 40 + HEIGHTOFFSET))
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
			     event.button.x < (WINDOW_WIDTH - 48 - 96))) &&
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
		      
			  snprintf(fname, sizeof(fname), "saved/%s-t.png",
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
	  
	      if (!been_saved)
		{
		  if (do_prompt(PROMPT_OPEN_SAVE_TXT,
				PROMPT_OPEN_SAVE_YES,
				PROMPT_OPEN_SAVE_NO))
		    {
		      do_save();
		    }
		}
	    
	      snprintf(fname, sizeof(fname), "saved/%s%s",
		       d_names[which], d_exts[which]);
	  
	      rfname = get_fname(fname);
	  
#ifdef SAVE_AS_BMP
	      img = SDL_LoadBMP(rfname);
#else
	      img = IMG_Load(rfname);
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
		  SDL_FillRect(canvas, NULL,
			       SDL_MapRGB(canvas->format, 255, 255, 255));

		  /* FIXME: What to do when in 640x480 mode, and loading an
		     800x600 image!? */

		  dest.x = (canvas->w - img->w) / 2;
		  dest.y = (canvas->h - img->h) / 2;
	      
		  SDL_BlitSurface(img, NULL, canvas, &dest);
		  SDL_FreeSurface(img);
	      
		  cur_undo = 0;
		  oldest_undo = 0;
		  newest_undo = 0;
	      
		  been_saved = 1;
		  reset_avail_tools();
		  tool_avail[TOOL_NEW] = 1;
	      
		  tool_avail_bak[TOOL_UNDO] = 0;
		  tool_avail_bak[TOOL_REDO] = 0;
	  
		  strcpy(file_id, d_names[which]);

		  want_new_tool = 1;
		}
      
	      free(rfname);
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

      free(d_names);
      free(d_exts);
    }

  return(want_new_tool);
}


/* -------------- Poly Fill Stuff -------------- */

#ifdef SCANLINE_POLY_FILL

void insert_edge(edge * list, edge * edg)
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


int y_next(int k, int cnt, point_type * pts)
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


void make_edge_rec(point_type lower, point_type upper,
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


void build_edge_list(int cnt, point_type * pts, edge * edges[])
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


void build_active_list(int scan, edge * active, edge * edges[])
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


void fill_scan(int scan, edge * active)
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


void delete_after(edge * q)
{
  edge * p;

  debug("delete_after()");

  p = q->next;
  q->next = p->next;
  free(p);
}


void update_active_list(int scan, edge * active)
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


void resort_active_list(edge * active)
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


void scan_fill(int cnt, point_type * pts)
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

int inside(fpoint_type p, an_edge b)
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


int cross(fpoint_type p1, fpoint_type p2, an_edge b)
{
  if (inside(p1, b) == inside(p2, b))
    return 0;
  else
    return 1;
}


fpoint_type intersect(fpoint_type p1, fpoint_type p2, an_edge b)
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


void clip_point(fpoint_type p, an_edge b, fpoint_type * pout, int * cnt,
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


void close_clip(fpoint_type * pout, int * cnt, fpoint_type * first[],
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


int clip_polygon(int n, fpoint_type * pin, fpoint_type * pout)
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

void wait_for_sfx(void)
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

int current_language(void)
{
  char * loc;
#ifdef WIN32
  char str[128];
#endif
  int lang, i;


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
  loc = getenv("LANG");
  if (!loc)
    {
      loc = g_win32_getlocale();
      if (loc)
	{
	  snprintf(str, sizeof(str), "LANG=%s", loc);
	  putenv(str);
	}
    }
#endif

  debug(loc);

  if (loc != NULL)
    {
      /* Which, if any, of the locales is it? */

      for (i = 0; i < NUM_LANGS; i++)
	{
	  /* Case-insensitive */
	  /* (so that, e.g. "pt_BR" is recognized as "pt_br") */
      
	  if (strncasecmp(loc, lang_prefixes[i], strlen(lang_prefixes[i])) == 0)
	    {
	      lang = i;
	    }
	}
    }

#ifdef DEBUG
  printf("lang=%d\n\n", lang);
#endif

  return lang;
}


/* XOR-based outline of rubber stamp shapes
   (unused if LOW_QUALITY_STAMP_OUTLINE is #defined) */

void stamp_xor(int x, int y)
{
  int xx, yy, rx, ry, sx, sy;
  Uint8 r, g, b, a, olda, abovea;
  SDL_Surface * surf_ptr;


  /* Use pre-mirrored stamp image, if applicable: */

  if (state_stamps[cur_stamp]->mirrored &&
      img_stamps_premirror[cur_stamp] != NULL)
    {
      surf_ptr = img_stamps_premirror[cur_stamp];
    }
  else
    {
      surf_ptr = img_stamps[cur_stamp];
    }

  
  SDL_LockSurface(surf_ptr);
  SDL_LockSurface(screen);
  

  for (yy = (y % 2) + 1; yy < surf_ptr->h; yy = yy + 2)
    {
      olda = 0;


      /* Compensate for flip! */

      if (state_stamps[cur_stamp]->flipped)
	ry = surf_ptr->h - 1 - yy;
      else
	ry = yy;


      for (xx = (x % 2); xx < surf_ptr->w; xx = xx + 2)
	{
	  /* Compensate for mirror! */

	  if (state_stamps[cur_stamp]->mirrored &&
	      img_stamps_premirror[cur_stamp] == NULL)
	    {
	      rx = img_stamps[cur_stamp]->w - 1 - xx;
	    }
	  else
	    {
	      rx = xx;
	    }

	  SDL_GetRGBA(getpixel(surf_ptr, rx, ry),
		      surf_ptr->format, &r, &g, &b, &a);
      
	  SDL_GetRGBA(getpixel(surf_ptr, rx, ry - 1),
		      surf_ptr->format, &r, &g, &b, &abovea);

	  if ((a < 128 && olda >= 128) ||
	      (a >= 128 && olda < 128) ||
	      (a < 128 && abovea >= 128) ||
	      (a >= 128 && abovea < 128))
	    {
	      sx = x + 96 + (((xx - (img_stamps[cur_stamp]->w / 2)) * state_stamps[cur_stamp]->size) / 100) + (img_stamps[cur_stamp]->w / 2);
	      sy = y + (((yy - (img_stamps[cur_stamp]->h / 2)) * state_stamps[cur_stamp]->size) / 100) + (img_stamps[cur_stamp]->h / 2);
	
	      clipped_putpixel(screen, sx, sy,
			       0xFFFFFFFF - getpixel(screen, sx, sy));
	    }

	  olda = a;
	}
    }

  SDL_UnlockSurface(screen);
  SDL_UnlockSurface(surf_ptr);
}


/* Returns whether a particular stamp can be colored: */

int stamp_colorable(int stamp)
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

int stamp_tintable(int stamp)
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


void rgbtohsv(Uint8 r8, Uint8 g8, Uint8 b8, float *h, float *s, float *v)
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


void hsvtorgb(float h, float s, float v, Uint8 *r8, Uint8 *g8, Uint8 *b8)
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


void show_progress_bar(void)
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


void do_print(void)
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


void do_render_cur_text(int do_blit)
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
    
      tmp_surf = RENDER_TEXT(fonts[cur_font], str, color);

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


void loadfonts(char * dir, int fatal)
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

char * uppercase(char * str)
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


/* Return string in right-to-left mode, if necessary: */

unsigned char * textdir(unsigned char * str)
{
  unsigned char * dstr;
  int i, j;

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

int colors_close(Uint32 c1, Uint32 c2)
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

void do_flood_fill(int x, int y, Uint32 cur_colr, Uint32 old_colr)
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

void control_drawtext_timer(Uint32 interval, char* text)
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
  SDL_PushEvent((SDL_Event*)param);

  return 0; /* Remove timer */
}


void parse_options(FILE * fi)
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
	  else if (strcmp(str, "800x600=yes") == 0)
	    {
	      WINDOW_WIDTH = 800;
	      WINDOW_HEIGHT = 600;
	    }
	  else if (strcmp(str, "800x600=no") == 0 ||
		   strcmp(str, "640x480=yes") == 0)
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


char * debug_gettext(const char * str)
{
  if (strcmp(str, dgettext(NULL, str)) == 0)
    {
      printf("NOTRANS: %s\n", str);
      fflush(stdout);
    }

  return(dgettext(NULL, str));
}


void do_setcursor(SDL_Cursor * c)
{
  if (!no_fancy_cursors)
    SDL_SetCursor(c);
}


char * great_str(void)
{
  return(great_strs[rand() % (sizeof(great_strs) / sizeof(char *))]);
}


int charsize(char c)
{
  Uint16 str[2];
  int w, h;

  str[0] = c;
  str[1] = '\0';

  TTF_SizeUNICODE(fonts[cur_font], str, &w, &h);

  return w;
}


void draw_image_title(int t, int x)
{
  SDL_Rect dest;

  dest.x = x;
  dest.y = 0;
  SDL_BlitSurface(img_title_on, NULL, screen, &dest);
  
  dest.x = x + (96 - img_title_names[t]->w) / 2;;
  dest.y = (40 - img_title_names[t]->h) / 2;
  SDL_BlitSurface(img_title_names[t], NULL, screen, &dest);
}


int need_unicode(int l)
{
  int i, need;

  need = 0;

  for (i = 0; lang_use_unicode[i] != -1 && need == 0; i++)
    {
      if (lang_use_unicode[i] == l)
	{
	  need = 1;
	}
    }

  return need;
}


int need_utf8(int l)
{
  int i, need;

  need = 0;

  for (i = 0; lang_use_utf8[i] != -1 && need == 0; i++)
    {
      if (lang_use_utf8[i] == l)
	{
	  need = 1;
	}
    }

  return need;
}


int need_own_font(int l)
{
  if (need_utf8(l) || need_unicode(l))
    return 1;
  else
    return 0;
}


int need_right_to_left(int l)
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

void handle_keymouse(SDLKey key, Uint8 updown)
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

void handle_active( SDL_Event *event )
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

char *remove_slash( char *path )
{
  int len = strlen(path);

  if (!len)
    return path;

  if (path[len-1] == '/' || path[len-1] == '\\')
    path[len-1] = 0;

  return path;
}


/* Decode a UTF8 string */

unsigned char * utf8_decode(unsigned char * str)
{
  int i;
  unsigned char utf8_char[4];
  unsigned char utf8_str[1024];
  

  utf8_str[0] = '\0';
  
  for (i = 0; i < strlen(str); i++)
    {
      /* How many bytes does this character need? */

      if (str[i] < 128)  /* 0xxx xxxx - 1 byte */
	{
	  utf8_char[0] = str[i];
	  utf8_char[1] = '\0';
	}
      else if ((str[i] & 0xE0) == 0xC0)  /* 110x xxxx - 2 bytes */
	{
	  utf8_char[0] = str[i];
	  utf8_char[1] = str[i + 1];
	  utf8_char[2] = '\0';
	  i = i + 1;
	}
      else if ((str[i] & 0xF0) == 0xE0)  /* 1110 xxxx - 3 bytes */
	{
	  utf8_char[0] = str[i];
	  utf8_char[1] = str[i + 1];
	  utf8_char[2] = str[i + 2];
	  utf8_char[3] = '\0';
	  i = i + 2;
	}
      else  /* 1111 0xxx - 4 bytes */
	{
	  utf8_char[0] = str[i];
	  utf8_char[1] = str[i + 1];
	  utf8_char[2] = str[i + 2];
	  utf8_char[3] = str[i + 3];
	  utf8_char[4] = '\0';
	  i = i + 3;
	}

      strcat(utf8_str, utf8_char);
    }

  return(strdup(utf8_str));
}


/* Escape codes for HTML-style escaping of stamp description strings: */

typedef struct escape_string_type {
  char * str;
  unsigned int chr;
} escape_string_type;


/* Thanks to http://bushong.net/dawn/links/htmlCodes.shtml
   for the list of escape codes */

escape_string_type escape_strings[] = {
  {"quot",   '\"'},
  {"amp",    '&'},
  {"lt",     '<'},
  {"gt",     '>'},
  {"iexcl",  161},
  {"cent",   162},
  {"pound",  163},
  {"curren", 164},
  {"yen",    165},
  {"brvbar", 166},
  {"sect",   167},
  {"uml",    168},
  {"copy",   169},
  {"ordf",   170},
  {"laquo",  171},
  {"not",    172},
  {"shy",    173},
  {"reg",    174},
  {"macr",   175},
  {"deg",    176},
  {"plusmn", 177},
  {"sup2",   178},
  {"sup3",   179},
  {"acute",  180},
  {"micro",  181},
  {"para",   182},
  {"middot", 183},
  {"cedil",  184},
  {"sup1",   185},
  {"ordm",   186},
  {"raquo",  187},
  {"frac14", 188},
  {"frac12", 189},
  {"frac34", 190},
  {"iquest", 191},
  {"Agrave", 192},
  {"Aacute", 193},
  {"Acirc",  194},
  {"Atilde", 195},
  {"Auml",   196},
  {"Aring",  197},
  {"AElig",  198},
  {"Ccedil", 199},
  {"Egrave", 200},
  {"Eacute", 201},
  {"Ecirc",  202},
  {"Euml",   203},
  {"Igrave", 204},
  {"Iacute", 205},
  {"Icirc",  206},
  {"Iuml",   207},
  {"ETH",    208},
  {"Ntilde", 209},
  {"Ograve", 210},
  {"Oacute", 211},
  {"Ocirc",  212},
  {"Otilde", 213},
  {"Ouml",   214},
  {"times",  215},
  {"Oslash", 216},
  {"Ugrave", 217},
  {"Uacute", 218},
  {"Ucirc",  219},
  {"Uuml",   220},
  {"Yacute", 221},
  {"THORN",  222},
  {"szlig",  223},
  {"agrave", 224},
  {"aacute", 225},
  {"acirc",  226},
  {"atilde", 227},
  {"auml",   228},
  {"aring",  229},
  {"aelig",  230},
  {"ccedil", 231},
  {"egrave", 232},
  {"eacute", 233},
  {"ecirc",  234},
  {"euml",   235},
  {"igrave", 236},
  {"iacute", 237},
  {"icirc",  238},
  {"iuml",   239},
  {"eth",    240},
  {"ntilde", 241},
  {"ograve", 242},
  {"oacute", 243},
  {"ocirc",  244},
  {"otilde", 245},
  {"ouml",   246},
  {"divide", 247},
  {"oslash", 248},
  {"ugrave", 249},
  {"uacute", 250},
  {"ucirc",  251},
  {"uuml",   252},
  {"yacute", 253},
  {"thorn",  254},
  {"yuml",   255},
  {"OElig",  338},
  {"oelig",  339},
  {"Scaron", 352},
  {"scaron", 353},
  {"Yuml",   376},
  {"circ",   710},
  {"tilde",  732},
  {"ensp",   8194},
  {"emsp",   8195},
  {"thinsp", 8201},
  {"zwnj",   8204},
  {"zwj",    8205},
  {"lrm",    8206},
  {"rlm",    8207},
  {"ndash",  8211},
  {"mdash",  8212},
  {"lsquo",  8216},
  {"rsquo",  8217},
  {"sbquo",  8218},
  {"ldquo",  8220},
  {"rdquo",  8221},
  {"bdquo",  8222},
  {"dagger", 8224},
  {"Dagger", 8225},
  {"permil", 8240},
  {"lsaquo", 8249},
  {"rsaquo", 8250},
  {"euro",   8364},
  {NULL, 0}
};


/* Unescape an HTML-escaped-style string (e.g., convert "&ntilde;" and such) */

unsigned char * unescape(char * str)
{
  int i, j, len, esclen, inside_escape;

#ifndef WIN32
  char outstr[strlen(str + 1)], escapestr[strlen(str + 1)];
#else
  char *outstr = alloca( strlen(str + 1) );
  char *escapestr = alloca( strlen(str + 1) );
#endif

  inside_escape = 0;
  len = 0;
  esclen = 0;
 

  /* For each character in the input string: */

  for (i = 0; i < strlen(str); i++)
    {
      if (str[i] == '&')
	{
	  /* Starting an escape character! */

	  inside_escape = 1;
	  esclen = 0;
	}
      else
	{
	  if (inside_escape == 0)
	    {
	      /* Not within an escaped character... simply append this literally: */
	
	      outstr[len++] = str[i];
	    }
	  else
	    {
	      /* Within an escaped character! */
	
	      if (str[i] == ';')
		{
		  /* We're ending it! */

		  inside_escape = 0;

		  escapestr[esclen] = '\0';

	  
		  /* What string was it!? */

		  for (j = 0; escape_strings[j].str != NULL; j++)
		    {
		      if (strcmp(escape_strings[j].str, escapestr) == 0)
			{
			  if (escape_strings[j].chr <= 255)
			    {
			      outstr[len++] = escape_strings[j].chr;
			    }
			  else
			    {
			      /* Needs to be represented by two bytes: */

			      /* FIXME: How should this be handled!? */

			      outstr[len++] =
				((((escape_strings[j].chr & 0xFF00) >> 8) & 0x1F) | 0xC0);
			      outstr[len++] = ((escape_strings[j].chr & 0x00FF) >> 0);
			    }
			}
		    }
		}
	      else
		{
		  /* Collect the escaped character: */

		  escapestr[esclen++] = str[i];
		}
	    }
	}
    }

  outstr[len] = '\0';


  return(strdup(outstr));
}


static iconv_t cd = (iconv_t)(-1);

int converts()
{
  if ( cd == (iconv_t)(-1) )
    return 0;
  else
    return 1;
}

void convert_open(const char *from)
{
  cd = iconv_open ("UTF-8", from);
  if (cd == (iconv_t)(-1))
    {
      /* FIXME: Error! */
    }
}

void convert_close()
{
  if (cd != (iconv_t)(-1))
    iconv_close(cd);
}

char * convert2utf8(char c)
{
  char inbuf[1];
  char outbuf[4];

  char *inptr;
  char *outptr;
  size_t inbytes_left, outbytes_left;
  int count;

  inbuf[0]=c;
  memset(outbuf, 0, 4);
  inbytes_left = 1;
  outbytes_left = 4;
  inptr = inbuf;
  outptr = (char *) outbuf;

  count = iconv (cd, &inptr, &inbytes_left, &outptr, &outbytes_left);
  /*
    if (count < 0)
    {
    printf ("Error!\n");
    }
  */

  return strdup(outbuf);
}


/* in:
   char *utf8_str - buffer containing a well-formed utf8 string
   int len        - length of the above string in bytes; len >= 1
   out:
   int result     - length of the above string in bytes after removing
   last utf8 char
*/

int delete_utf8_char(char *utf8_str, int len)
{
  /* from man utf-8:
     The first byte of a multi-byte sequence which represents a single
     non-ASCII UCS character is always in the range 0xc0 to 0xfd and indi-
     cates how long this multi-byte sequence is.  All further bytes in a
     multi-byte sequence are in the range 0x80 to 0xbf.
  */
     
  unsigned char *current_char_ptr = utf8_str + len - 1;
  unsigned char current_char = *current_char_ptr;

  while ( (current_char >= 0x80) && (current_char <= 0xbf) )
    {
      /* part of the utf8 multibyte char but not the first byte */

      len--;
      current_char_ptr--;
      current_char = *current_char_ptr;
    }

  /* we have 1 char to remove */
  *current_char_ptr = '\0';
  len--;

  return len;
}


/* For right-to-left languages, when word-wrapping, we need to
   make sure the text doesn't end up going from bottom-to-top, too! */

void anti_carriage_return(int left, int right, int cur_top, int new_top,
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


int mySDL_WaitEvent(SDL_Event *event)
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


int mySDL_PollEvent(SDL_Event *event)
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

