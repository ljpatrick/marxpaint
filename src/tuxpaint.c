/*
  tuxpaint.c
  
  Tux Paint - A simple drawing program for children.
  
  Copyright (c) 2005 by Bill Kendrick and others; see AUTHORS.txt
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
  (See COPYING.txt)
  
  June 14, 2002 - January 16, 2005
*/


#define VER_VERSION     "0.9.15"
#define VER_DATE        "2005-01-16"


/* Color depth for Tux Paint to run in, and store canvases in: */

//#define VIDEO_BPP 15 // saves memory
//#define VIDEO_BPP 16 // causes discoloration
//#define VIDEO_BPP 24 // compromise
#define VIDEO_BPP 32 // might be fastest, if conversion funcs removed


// plan to rip this out as soon as it is considered stable
//#define THREADED_FONTS
#define FORKED_FONTS

/* Method for printing images: */

#define PRINTMETHOD_PS             /* Direct to PostScript */
//#define PRINTMETHOD_PNM_PS       /* Output PNM, assuming it gets printed */
//#define PRINTMETHOD_PNG_PNM_PS   /* Output PNG, assuming it gets printed */


/* Default print command, depending on the print method: */

#ifdef PRINTMETHOD_PNG_PNM_PS
#define PRINTCOMMAND "pngtopnm | pnmtops | lpr"
#elif defined(PRINTMETHOD_PNM_PS)
#define PRINTCOMMAND "pnmtops | lpr"
#elif defined(PRINTMETHOD_PS)
#define PRINTCOMMAND "lpr"
#else
#error No print method defined!
#endif


/* Compile-time options: */

/* #define DEBUG */
/* #define DEBUG_MALLOC */
/* #define LOW_QUALITY_THUMBNAILS */
/* #define LOW_QUALITY_COLOR_SELECTOR */
/* #define LOW_QUALITY_STAMP_OUTLINE */
/* #define LOW_QUALITY_FLOOD_FILL */
/* #define NO_PROMPT_SHADOWS */
/* #define USE_HWSURFACE */

/* Disable fancy cursors in fullscreen mode, to avoid SDL bug: */
#define LARGE_CURSOR_FULLSCREEN_BUG

// control the color selector
#define COLORSEL_DISABLE 0  // disable and draw the (greyed out) colors
#define COLORSEL_ENABLE  1  // enable and draw the colors
#define COLORSEL_CLOBBER 2  // colors get scribbled over
#define COLORSEL_REFRESH 4  // redraw the colors, either on or off

static unsigned draw_colors(unsigned action);

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


/* Macros: */

#define HARD_MIN_STAMP_SIZE 0  // bottom of scaletable
#define HARD_MAX_STAMP_SIZE (sizeof scaletable / sizeof scaletable[0] - 1)

#define MIN_STAMP_SIZE (state_stamps[cur_stamp]->min)
#define MAX_STAMP_SIZE (state_stamps[cur_stamp]->max)

// to scale some offset, in pixels, like the current stamp is scaled
#define SCALE_LIKE_STAMP(x) ( ((x) * scaletable[state_stamps[cur_stamp]->size].numer + scaletable[state_stamps[cur_stamp]->size].denom-1) / scaletable[state_stamps[cur_stamp]->size].denom )
// pixel dimensions of the current stamp, as scaled
#define CUR_STAMP_W SCALE_LIKE_STAMP(img_stamps[cur_stamp]->w)
#define CUR_STAMP_H SCALE_LIKE_STAMP(img_stamps[cur_stamp]->h)

///////////////////////////////////////////////////////////////////////////////


#define REPEAT_SPEED 300  /* Initial repeat speed for scrollbars */
#define CURSOR_BLINK_SPEED 500  /* Initial repeat speed for cursor */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

// math.h makes y1 an obscure function!
#define y1 evil_y1
#include <math.h>
#undef y1

#include <locale.h>

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

#include <libintl.h>
#ifndef gettext_noop
#define gettext_noop(String) String
#endif


#ifdef DEBUG
#define gettext(String) debug_gettext(String)
#endif


#ifndef M_PI
#define M_PI 3.14159265358979323846
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
#include "wrapperdata.h"
extern WrapperData macosx;
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
#ifndef _SDL_IMAGE_H
#ifndef _IMG_h
#error "---------------------------------------------------"
#error "If you installed SDL_image from a package, be sure"
#error "to get the development package, as well!"
#error "(e.g., 'libsdl-image1.2-devel.rpm')"
#error "---------------------------------------------------"
#endif
#endif

#include "SDL_ttf.h"
#ifndef _SDL_TTF_h
#ifndef _SDLttf_h
#error "---------------------------------------------------"
#error "If you installed SDL_ttf from a package, be sure"
#error "to get the development package, as well!"
#error "(e.g., 'libsdl-ttf1.2-devel.rpm')"
#error "---------------------------------------------------"
#endif
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

#if defined(THREADED_FONTS) || defined(THREADED_STAMPS)
#include "SDL_thread.h"
#include "SDL_mutex.h"
#else
#define SDL_CreateThread(fn,vp) (void*)(long)(fn(vp))
#define SDL_WaitThread(tid,rcp) do{(void)tid;(void)rcp;}while(0)
#define SDL_Thread int
#define SDL_mutex int
#define SDL_CreateMutex() 0  // creates in released state
#define SDL_DestroyMutex(lock)
#define SDL_mutexP(lock)  // take lock
#define SDL_mutexV(lock)  // release lock
#endif

static SDL_Thread *font_thread;
static volatile long font_thread_done;
static void run_font_scanner(void);
static int font_scanner_pid;
static int font_socket_fd;

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

#ifdef DEBUG_MALLOC
#include "malloc.c"
#endif

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

static const char *getfilename(const char* path)
{
  char    *p;

  if ( (p = strrchr( path, '\\' )) != NULL )
    return p+1;
  if ( (p = strrchr( path, '/' )) != NULL )
    return p+1;
  return path;
}


///////////////////////////////////////////////////////////////////
// Language stuff

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
  LANG_GA,     /* Gaelic */
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
  LANG_SW,     /* Swahili */
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
  "ga",
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
  "sw",
  "ta",
  "tr",
  "vi",
  "wa",
  "zh_cn",
  "zh_tw",
};


// languages which don't use the default font
static int lang_use_own_font[] = {
//  LANG_EL,
//  LANG_HE,
  LANG_HI,
//  LANG_JA,
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


static int search_int_array(int l, int *array)
{
  int i;

  for (i = 0; array[i] != -1; i++)
    {
      if (array[i] == l)
        return 1;
    }

  return 0;
}

static char * langstr;

static void set_langstr(const char *s)
{
  if (langstr)
    free(langstr);
  langstr = strdup(s);
}

static int need_own_font;
static int need_right_to_left;
static const char * lang_prefix;

/* Determine the current language/locale, and set the language string: */

static void set_current_language(void)
{
  char * loc;
#ifdef WIN32
  char str[128];
#endif
  int lang, i, found;

  bindtextdomain("tuxpaint", LOCALEDIR);
  /* Old version of glibc does not have bind_textdomain_codeset() */
#if defined __GLIBC__ && __GLIBC__ == 2 && __GLIBC_MINOR__ >=2 || __GLIBC__ > 2
  bind_textdomain_codeset("tuxpaint", "UTF-8");
#endif
  textdomain("tuxpaint");

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
	  putenv(strdup(str));
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
	  // Case-insensitive (both "pt_BR" and "pt_br" work, etc.)
      	  if (strncasecmp(loc, lang_prefixes[i], strlen(lang_prefixes[i])) == 0)
	    {
	      lang = i;
	      found = 1;
	    }
	}
    }

  lang_prefix = lang_prefixes[lang];
  need_own_font = search_int_array(lang,lang_use_own_font);
  need_right_to_left = search_int_array(lang,lang_use_right_to_left);

#ifdef DEBUG
  printf("DEBUG: Language is %s (%d)\n", lang_prefix, lang);
#endif

}

/* FIXME: All this should REALLY be array-based!!! */
/* Show available languages: */
static void show_lang_usage(FILE * f, const char * const prg)
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
/* ga */     "  gaelic       gaidhlig\n"
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
/* sw */     "  swahili\n"
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
static void show_locale_usage(FILE * f, const char * const prg)
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
	  "  ga_IE   (Gaelic       Gaidhlig)\n"
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
	  "  sw_TZ   (Swahili)\n"
	  "  sv_SE   (Swedish      Svenska)\n"
	  "  tr_TR   (Turkish)\n"
	  "  vi_VN   (Vietnamese)\n"
	  "  wa_BE   (Walloon)\n"
	  "  cy_GB   (Welsh        Cymraeg)\n"
	  "\n",
	  prg);
}


typedef struct language_to_locale_struct {
  const char *language;
  const char *locale;
} language_to_locale_struct;

static const language_to_locale_struct language_to_locale_array[] = {
{"english",              "C"},
{"american-english",     "C"},
{"croatian",             "hr_HR"},
{"hrvatski",             "hr_HR"},
{"catalan",              "ca_ES"},
{"catala",               "ca_ES"},
{"belarusian",           "be_BY"},
{"bielaruskaja",         "be_BY"},
{"czech",                "cs_CZ"},
{"cesky",                "cs_CZ"},
{"danish",               "da_DK"},
{"dansk",                "da_DK"},
{"german",               "de_DE"},
{"deutsch",              "de_DE"},
{"greek",                "el_GR"},
{"british-english",      "en_GB"},
{"british",              "en_GB"},
{"spanish",              "es_ES"},
{"espanol",              "es_ES"},
{"finnish",              "fi_FI"},
{"suomi",                "fi_FI"},
{"french",               "fr_FR"},
{"francais",             "fr_FR"},
{"gaelic",               "ga_IE"},
{"gaidhlig",             "ga_IE"},
{"galician",             "gl_ES"},
{"galego",               "gl_ES"},
{"hebrew",               "he_IL"},
{"hindi",                "hi_IN"},
{"hungarian",            "hu_HU"},
{"magyar",               "hu_HU"},
{"indonesian",           "id_ID"},
{"bahasa-indonesia",     "id_ID"},
{"icelandic",            "is_IS"},
{"islenska",             "is_IS"},
{"italian",              "it_IT"},
{"italiano",             "it_IT"},
{"japanese",             "ja_JP"},
{"vietnamese",           "vi_VN"},
{"afrikaans",            "af_ZA"},
{"albanian",             "sq_AL"},
{"breton",               "br_FR"},
{"brezhoneg",            "br_FR"},
{"bulgarian",            "bg_BG"},
{"welsh",                "cy_GB"},
{"cymraeg",              "cy_GB"},
{"bokmal",               "nb_NO"},
{"basque",               "eu_ES"},
{"euskara",              "eu_ES"},
{"korean",               "ko_KR"},
{"klingon",              "tlh"},
{"tlhIngan",             "tlh"},
{"tlhingan",             "tlh"},
{"tamil",                "ta_IN"},
{"lithuanian",           "lt_LT"},
{"lietuviu",             "lt_LT"},
{"malay",                "ms_MY"},
{"dutch",                "nl_NL"},
{"nederlands",           "nl_NL"},
{"norwegian",            "nn_NO"},
{"nynorsk",              "nn_NO"},
{"norsk",                "nn_NO"},
{"polish",               "pl_PL"},
{"polski",               "pl_PL"},
{"brazilian-portuguese", "pt_BR"},
{"portugues-brazilian",  "pt_BR"},
{"brazilian",            "pt_BR"},
{"portuguese",           "pt_PT"},
{"portugues",            "pt_PT"},
{"romanian",             "ro_RO"},
{"russian",              "ru_RU"},
{"russkiy",              "ru_RU"},
{"slovak",               "sk_SK"},
{"slovenian",            "sl_SI"},
{"slovensko",            "sl_SI"},
{"serbian",              "sr_YU"},
{"swedish",              "sv_SE"},
{"svenska",              "sv_SE"},
{"swahili",              "sw_TZ"},
{"turkish",              "tr_TR"},
{"walloon",              "wa_BE"},
{"walon",                "wa_BE"},
{"chinese",              "zh_CN"},
{"simplified-chinese",   "zh_CN"},
{"traditional-chinese",  "zh_TW"},
};


static void setup_language(const char * const prg)
{

// Justify this or delete it. It seems to make Tux Paint
// violate the behavior documented by "man 7 locale".
#if 0
  if (langstr == NULL && getenv("LANG"))
    {
      if(!strncasecmp(getenv("LANG"), "lt_LT", 5))
        set_langstr("lithuanian");
      if(!strncasecmp(getenv("LANG"), "pl_PL", 5))
        set_langstr("polish");
    }
#endif

  if (langstr != NULL)
    {
      int i = sizeof language_to_locale_array / sizeof language_to_locale_array[0];
      const char *locale = NULL;
      while(i--)
        {
          if (strcmp(langstr, language_to_locale_array[i].language))
            continue;
          locale = language_to_locale_array[i].locale;
          break;
        }

      if (!locale)
        {
          if (strcmp(langstr, "help") == 0 || strcmp(langstr, "list") == 0)
            {
              show_lang_usage(stdout, prg);
              //free(langstr);  // pointless
              exit(0);
            }
          else
            {
              fprintf(stderr, "%s is an invalid language\n", langstr);
              show_lang_usage(stderr, prg);
              //free(langstr);  // pointless
              exit(1);
            }
        }

      if (locale[0]=='C' && !locale[1])
        {
          putenv((char *) "LANGUAGE=C");
          putenv((char *) "LC_ALL=C");
        }
      else
        {
          char *s;
          ssize_t len;
          len = strlen("LANGUAGE=.UTF-8")+strlen(locale)+1;
          s = malloc(len);
          snprintf(s, len, "LANGUAGE=%s.UTF-8", locale);
          putenv(s);
          len = strlen("LC_ALL=.UTF-8")+strlen(locale)+1;
          s = malloc(len);
          snprintf(s, len, "LC_ALL=%s.UTF-8", locale);
          putenv(s);
        }
    
      setlocale(LC_ALL, "");
      free(langstr);
    }
 
  set_current_language();
}


// handle --locale arg
static void do_locale_option(const char * const arg)
{
  int len = strlen(arg) + 6;
  char *str = malloc(len);
  snprintf(str, len, "LANG=%s", arg);
  putenv(str);
  // We leak "str" because it can not be freed. It is now part
  // of the environment. If it were local, the environment would
  // get corrupted.
  setlocale(LC_ALL, ""); /* use arg ? */
}


static TTF_Font *try_alternate_font(int size)
{
  char  str[128];
  char  prefix[64];
  char  *p;

  strcpy(prefix, lang_prefix);
  if ((p = strrchr(prefix, '_')) != NULL)
  {
    *p = 0;
    snprintf(str, sizeof(str), "%sfonts/locale/%s.ttf",
             DATA_PREFIX, prefix);

    return TTF_OpenFont(str, size);
  }
  return NULL;
}


static TTF_Font *load_locale_font(TTF_Font *fallback, int size)
{
  TTF_Font *ret = NULL;
  if (need_own_font)
    {
      char str[128];
      snprintf(str, sizeof(str), "%sfonts/locale/%s.ttf",
	       DATA_PREFIX, lang_prefix);

      ret = TTF_OpenFont(str, size);

      if (ret == NULL)
      {
          ret = try_alternate_font(size);
          if (ret == NULL)
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

	      set_current_language();
	  }
      }
    }
  return ret ? ret : fallback;
}

///////////////////////////////////////////////////////////////////
// sizing

// The old Tux Paint:
// 640x480 screen
// 448x376 canvas
//  40x96  titles near the top
//  48x48  button tiles
//  ??x56  tux area
//  room for 2x7 button tile grids

typedef struct {
  Uint8 rows, cols;
} grid_dims;

//static SDL_Rect r_screen; // was 640x480 @ 0,0  -- but this isn't so useful
static SDL_Rect r_canvas; // was 448x376 @ 96,0
static SDL_Rect r_tools; // was 96x336 @ 0,40
static SDL_Rect r_toolopt; // was 96x336 @ 544,40
static SDL_Rect r_colors; // was 544x48 @ 96,376
static SDL_Rect r_ttools; // was 96x40 @ 0,0  (title for tools, "Tools")
static SDL_Rect r_tcolors; // was 96x48 @ 0,376 (title for colors, "Colors")
static SDL_Rect r_ttoolopt; // was 96x40 @ 544,0 (title for tool options)
static SDL_Rect r_tuxarea; // was 640x56

static int button_w;  // was 48
static int button_h;  // was 48

static int color_button_w;  // was 32
static int color_button_h;  // was 48

// Define button grid dimensions. (in button units)
// These are the maximum slots -- some may be unused.
static grid_dims gd_tools;   // was 2x7
static grid_dims gd_toolopt; // was 2x7
//static grid_dims gd_open;    // was 4x4
static grid_dims gd_colors;   // was 17x1

#define HEIGHTOFFSET (((WINDOW_HEIGHT - 480) / 48) * 48)
#define TOOLOFFSET (HEIGHTOFFSET / 48 * 2)
#define PROMPTOFFSETX (WINDOW_WIDTH - 640) / 2
#define PROMPTOFFSETY (HEIGHTOFFSET / 2)

#define THUMB_W ((WINDOW_WIDTH - 96 - 96) / 4)
#define THUMB_H (((48 * 7 + 40 + HEIGHTOFFSET) - 72) / 4)

static int WINDOW_WIDTH, WINDOW_HEIGHT;


static void setup_normal_screen_layout(void)
{
  button_w = 48;
  button_h = 48;

  gd_toolopt.cols = 2;
  gd_tools.cols = 2;

  r_ttools.x = 0;
  r_ttools.y = 0;
  r_ttools.w = gd_tools.cols * button_w;
  r_ttools.h = 40;

  r_ttoolopt.w = gd_toolopt.cols * button_w;
  r_ttoolopt.h = 40;
  r_ttoolopt.x = WINDOW_WIDTH - r_ttoolopt.w;
  r_ttoolopt.y = 0;

  gd_colors.rows = 1;
  gd_colors.cols = (NUM_COLORS + gd_colors.rows - 1) / gd_colors.rows;

  r_colors.h = 48;
  color_button_h = r_colors.h / gd_colors.rows;
  r_tcolors.h = r_colors.h;

  r_tcolors.x = 0;
  r_tcolors.w = gd_tools.cols * button_w;
  r_colors.x = r_tcolors.w;
  r_colors.w = WINDOW_WIDTH - r_tcolors.w;

  color_button_w = r_colors.w / gd_colors.cols;

  // This would make it contain _just_ the color spots,
  // without any leftover bit on the end. Hmmm...
  // r_colors.w = color_button_w * gd_colors.cols;

  r_canvas.x = gd_tools.cols * button_w;
  r_canvas.y = 0;
  r_canvas.w = WINDOW_WIDTH - (gd_tools.cols+gd_toolopt.cols) * button_w;

  r_tuxarea.x = 0;
  r_tuxarea.w = WINDOW_WIDTH;

  // need 56 minimum for the Tux area
  int buttons_tall = (WINDOW_HEIGHT -  r_ttoolopt.h - 56 - r_colors.h) / button_h;
  gd_tools.rows = buttons_tall;
  gd_toolopt.rows = buttons_tall;

  r_canvas.h = r_ttoolopt.h + buttons_tall * button_h;
  
  r_colors.y = r_canvas.h + r_canvas.y;
  r_tcolors.y = r_canvas.h + r_canvas.y;

  r_tuxarea.y = r_colors.y + r_colors.h;
  r_tuxarea.h = WINDOW_HEIGHT - r_tuxarea.y;

  r_tools.x = 0;
  r_tools.y = r_ttools.h + r_ttools.y;
  r_tools.w = gd_tools.cols * button_w;
  r_tools.h = gd_tools.rows * button_h;

  r_toolopt.w = gd_toolopt.cols * button_w;
  r_toolopt.h = gd_toolopt.rows * button_h;
  r_toolopt.x = WINDOW_WIDTH - r_ttoolopt.w;
  r_toolopt.y = r_ttoolopt.h + r_ttoolopt.y;

  // TODO: dialog boxes

}

#ifdef DEBUG
static void debug_rect(SDL_Rect *r, char *name)
{
  printf("%-12s %dx%d @ %d,%d\n", name, r->w, r->h, r->x, r->y);
}
#define DR(x) debug_rect(&x, #x)

static void debug_dims(grid_dims *g, char *name)
{
  printf("%-12s %dx%d\n", name, g->cols, g->rows);
}
#define DD(x) debug_dims(&x, #x)

static void print_layout(void)
{
  printf("\n--- layout ---\n");
  DR(r_canvas);
  DR(r_tools);
  DR(r_toolopt);
  DR(r_colors);
  DR(r_ttools);
  DR(r_tcolors);
  DR(r_ttoolopt);
  DR(r_tuxarea);
  DD(gd_tools);
  DD(gd_toolopt);
  DD(gd_colors);
  printf("buttons are %dx%d\n", button_w,button_h);
  printf("color buttons are %dx%d\n", color_button_w,color_button_h);
}
#undef DD
#undef DR
#endif

static void setup_screen_layout(void)
{
  // can do right-to-left, colors at the top, extra tool option columns, etc.
  setup_normal_screen_layout();
#ifdef DEBUG
  print_layout();
#endif
}

static SDL_Surface * screen;
static SDL_Surface * canvas;
static SDL_Surface * img_starter, * img_starter_bkgd;

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

  SDL_UpdateRect(screen, x1, y1, x2-x1+1, y2-y1+1);
}


static void update_screen_rect(SDL_Rect *r)
{
  SDL_UpdateRect(screen, r->x, r->y, r->w, r->h);
}

static int hit_test(const SDL_Rect * const r, unsigned x, unsigned y)
{
  // note the use of unsigned math: no need to check for negative
  return   x - r->x < r->w   &&   y - r->y < r->h  ;
}

#define HIT(r) hit_test(&(r), event.button.x, event.button.y)


/* "#if"ing out, since unused; bjk 2005.01.09 */

#if 0

// x,y are pixel-wise screen-relative (mouse location), not grid-wise
// w,h are the size of a grid item
// Return the grid box.
// NOTE: grid items must fill full SDL_Rect width exactly
static int grid_hit_wh(const SDL_Rect * const r, unsigned x, unsigned y, unsigned w, unsigned h)
{
  return   (x - r->x) / w  +  (y - r->y) / h * (r->w/w) ;
}

// test an SDL_Rect r containing an array of WxH items for a grid location
#define GRIDHIT_WH(r,W,H) grid_hit_wh(&(r), event.button.x, event.button.y, W,H)

#endif

// test an SDL_Rect r containing an array of SDL_Surface surf for a grid location
#define GRIDHIT_SURF(r,surf) grid_hit_wh(&(r), event.button.x, event.button.y, (surf)->w, (surf)->h)

// x,y are pixel-wise screen-relative (mouse location), not grid-wise
// Return the grid box.
// NOTE: returns -1 if hit is below or to the right of the grid
static int grid_hit_gd(const SDL_Rect * const r, unsigned x, unsigned y, grid_dims *gd)
{
  unsigned item_w = r->w / gd->cols;
  unsigned item_h = r->h / gd->rows;
  unsigned col = (x - r->x) / item_w;
  unsigned row = (y - r->y) / item_h;
#ifdef DEBUG
  printf("%d,%d resolves to %d,%d in a %dx%d grid, index is %d\n", x,y, col,row, gd->cols,gd->rows, col+row*gd->cols);
#endif
  if (col >= gd->cols || row >= gd->rows)
    return -1;
  return col + row * gd->cols;
}

// test an SDL_Rect r for a grid location, based on a grid_dims gd
#define GRIDHIT_GD(r,gd) grid_hit_gd(&(r), event.button.x, event.button.y, &(gd))

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

  SDL_BlitSurface(canvas, NULL, screen, &r_canvas);
  update_screen(x1 + 96, y1, x2 + 96, y2);
}


///////////////////////////////////////////////////////////////////

/* Globals: */

static int use_sound, fullscreen, disable_quit, simple_shapes,
  disable_print, print_delay, only_uppercase, promptless_save, grab_input,
  wheely, no_fancy_cursors, keymouse, mouse_x, mouse_y,
  mousekey_up, mousekey_down, mousekey_left, mousekey_right,
  dont_do_xor, use_print_config, dont_load_stamps, noshortcuts,
  no_system_fonts, no_button_distinction,
  mirrorstamps, disable_stamp_controls, disable_save, ok_to_use_lockfile;
static int starter_mirrored, starter_flipped;
static int recording, playing;
static char * playfile;
static FILE * demofi;
static const char * printcommand = PRINTCOMMAND;
static int prog_bar_ctr;

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
static SDL_Surface * img_black, * img_grey;
static SDL_Surface * img_yes, * img_no;
static SDL_Surface * img_open, * img_erase, * img_back, * img_trash;
static SDL_Surface * img_printer, * img_printer_wait, * img_save_over;
static SDL_Surface * img_popup_arrow;
static SDL_Surface * img_cursor_up, * img_cursor_down;
static SDL_Surface * img_cursor_starter_up, * img_cursor_starter_down;
static SDL_Surface * img_scroll_up, * img_scroll_down;
static SDL_Surface * img_scroll_up_off, * img_scroll_down_off;
static SDL_Surface * img_grow, * img_shrink;
static SDL_Surface * img_bold, * img_italic;

static SDL_Surface * img_sparkles;
static SDL_Surface * img_grass;

static SDL_Surface * img_title_on, * img_title_off,
  * img_title_large_on, * img_title_large_off;
static SDL_Surface * img_title_names[NUM_TITLES];
static SDL_Surface * img_tools[NUM_TOOLS], * img_tool_names[NUM_TOOLS];

static SDL_Surface * thumbnail(SDL_Surface * src, int max_x, int max_y,
			int keep_aspect);

//////////////////////////////////////////////////////////////////////
// font stuff

// example from a Debian box with MS fonts:
// start with 232 files
// remove "Cursor", "Webdings", "Dingbats", "Standard Symbols L"
// split "Condensed" faces out into own family
// group by family
// end up with 34 user choices

static int text_state;
// nice progression (alternating 33% and 25%) 9 12 18 24 36 48 72 96 144 192
// commonly hinted sizes seem to be: 9, 10, 12, 14, 18, 20 (less so), 24
// reasonable: 9,12,18... and 10,14,18...
static int text_sizes[] = {9, 12, 18, 24, 36, 48,
	                   56, 64, 96, 112, 128, 160};  // point sizes
#define MIN_TEXT_SIZE 0u
#define MAX_TEXT_SIZE (sizeof text_sizes / sizeof text_sizes[0] - 1)
static unsigned text_size = 4;   // initial text size


// for sorting through the font files at startup
typedef struct style_info {
  char *filename;
  char *directory;
  char *family;    // name like "FooCorp Thunderstruck"
  char *style;     // junk like "Oblique Demi-Bold"
  int italic;
  int boldness;
  int score;
  int truetype; // Is it? (TrueType gets priority)
} style_info;

// user's notion of a font
typedef struct family_info {
  char *directory;
  char *family;
  char *filename[4];
  TTF_Font *handle;
  int score;
} family_info;

static TTF_Font * medium_font, * small_font, * large_font, * locale_font;

static family_info **user_font_families;
static int num_font_families;
static int num_font_families_max;

static style_info **user_font_styles;
static int num_font_styles;
static int num_font_styles_max;

static void receive_some_font_info(void);

static TTF_Font *getfonthandle(int desire)
{
  family_info *fi = user_font_families[desire];
  if(fi->handle)
    return fi->handle;
  int missing = 0;

  char *name = fi->filename[text_state];
  if(!name)
    {
      name = fi->filename[text_state ^ TTF_STYLE_ITALIC];
      missing = text_state & TTF_STYLE_ITALIC;
    }
  if(!name)
    {
      name = fi->filename[text_state ^ TTF_STYLE_BOLD];
      missing = text_state & TTF_STYLE_BOLD;
    }
  if(!name)
    {
      name = fi->filename[text_state ^ (TTF_STYLE_ITALIC|TTF_STYLE_BOLD)];
      missing = text_state & (TTF_STYLE_ITALIC|TTF_STYLE_BOLD);
    }
  char *pathname = alloca(strlen(fi->directory) + 1 + strlen(name) + 1);
  sprintf(pathname, "%s/%s", fi->directory, name);
  
  fi->handle = TTF_OpenFont(pathname,text_sizes[text_size]);
  // if the font doesn't load, we die -- it did load OK before though
  TTF_SetFontStyle(fi->handle,missing);
  return fi->handle;
}


static SDL_Surface *render_text(TTF_Font *restrict font, const char *restrict str, SDL_Color color)
{
  SDL_Surface *ret;
  ret = TTF_RenderUTF8_Blended(font, str, color);
  if(ret)
    return ret;
  // Sometimes a font will be missing a character we need. Sometimes the library
  // will substitute a rectangle without telling us. Sometimes it returns NULL.
  // Probably we should use FreeType directly. For now though...
  int height = TTF_FontHeight(font);
  if(height<2)
    height = 2;
  return thumbnail(img_title_large_off, height*strlen(str)/2, height, 0);
}


// void qsort(void *base, size_t nmemb, size_t size,
// int(*compar)(const void *, const void *));


// For qsort() and other use, to see if font files are groupable
static int compar_fontgroup(const void *v1, const void *v2)
{
  const style_info *s1 = *(style_info**)v1;
  const style_info *s2 = *(style_info**)v2;
  int rc;

  rc = strcmp(s1->directory,s2->directory);
  if(rc)
    return rc;

  rc = s1->truetype - s2->truetype;
  if(rc)
    return rc;

  return strcmp(s1->family,s2->family);
}


// For qsort() and other use, to see if font files are duplicates
static int compar_fontkiller(const void *v1, const void *v2)
{
  const family_info *f1 = *(family_info**)v1;
  const family_info *f2 = *(family_info**)v2;
  int rc;

  rc = strcmp(f1->family,f2->family);
  if(rc)
    return rc;

  return f1->score - f2->score;
}


// For qsort() and other use, to order the worst ones last
static int compar_fontscore(const void *v1, const void *v2)
{
  const family_info *f1 = *(family_info**)v1;
  const family_info *f2 = *(family_info**)v2;

  return f2->score - f1->score;
}

// Font style names are a mess that we must try to make
// sense of. For example...
//
// Cooper: Light, Medium, Light Bold, Black
// HoeflerText: (nil), Black
static void parse_font_style(style_info *si)
{
  int have_light = 0;
  int have_demi = 0;
  int have_bold = 0;
  int have_medium = 0;
  int have_black = 0;
  int have_heavy = 0;

  si->italic = 0;

  int stumped = 0;
  char *sp = si->style;

  while(*sp)
    {
      if(*sp==' ')
        {
          sp++;
          continue;
        }
      if(!strncasecmp(sp,"Bold",strlen("Bold")))
        {
          sp += strlen("Bold");
          have_bold = 1;
          continue;
        }
      if(!strncasecmp(sp,"Regular",strlen("Regular")))
        {
          sp += strlen("Regular");
          continue;
        }
      if(!strncasecmp(sp,"Italic",strlen("Italic")))
        {
          sp += strlen("Italic");
          si->italic = 1;
          continue;
        }
      if(!strncasecmp(sp,"Oblique",strlen("Oblique")))
        {
          sp += strlen("Oblique");
          si->italic = 1;
          continue;
        }
      // move " Condensed" from style to family
      if(!strncasecmp(sp,"Condensed",strlen("Condensed")))
        {
          sp += strlen("Condensed");
          size_t len = strlen(si->family);
          char *name = malloc(len + strlen(" Condensed") + 1);
          memcpy(name,si->family,len);
          strcpy(name+len," Condensed");
          free(si->family);
          si->family = name;
          continue;
        }
      if(!strncasecmp(sp,"Light",strlen("Light")))
        {
          sp += strlen("Light");
          have_light = 1;
          continue;
        }
      if(!strncasecmp(sp,"Medium",strlen("Medium")))
        {
          sp += strlen("Medium");
          have_medium = 1;
          continue;
        }
      if(!strncasecmp(sp,"Demi",strlen("Demi")))
        {
          sp += strlen("Demi");
          have_demi = 1;
          continue;
        }
      if(!strncasecmp(sp,"Heavy",strlen("Heavy")))
        {
          sp += strlen("Heavy");
          have_heavy = 1;
          continue;
        }
      if(!strncasecmp(sp,"Normal",strlen("Normal")))
        {
          sp += strlen("Normal");
          continue;
        }
      if(!strncasecmp(sp,"Black",strlen("Black")))
        {
          sp += strlen("Black");
          have_black = 1;
          continue;
        }
      if(!strncasecmp(sp,"Roman",strlen("Roman")))
        {
          sp += strlen("Roman");
          continue;
        }
      if(!strncasecmp(sp,"Book",strlen("Book")))
        {
          sp += strlen("Book");
          continue;
        }
      if(!strncasecmp(sp,"Chancery",strlen("Chancery")))
        {
          sp += strlen("Chancery");
          si->italic = 1;
          continue;
        }
      if(!strncasecmp(sp,"Thin",strlen("Thin")))
        {
          sp += strlen("Thin");
          have_light = 1;
          continue;
        }
      if(!strncmp(sp,"LR",strlen("LR")))
        {
          sp += strlen("LR");
          continue;
        }

      if(!stumped)
        {
          stumped=1;
#if 0
// THREADED_FONTS
          printf("Font style parser stumped by \"%s\".\n", si->style);
#endif
        }
      sp++; // bad: an unknown character
    }

  if (have_demi || have_medium)
    si->boldness = 2;
  else if (have_bold || have_black || have_heavy) // TODO: split these
    si->boldness = 3;
  else if (have_light)
    si->boldness = 0;
  else
    si->boldness = 1;

  // we'll count both TrueType and OpenType
  si->truetype = !!strstr(si->filename,".ttf") || !!strstr(si->filename,".otf");
}

static void groupfonts_range(style_info **base, int count)
{
  int boldcounts[4] = {0,0,0,0};
  int boldmap[4] = {-1,-1,-1,-1};
  int i;

#if 0
// THREADED_FONTS
if(count<1 || count>4)
{
printf("\n::::::: %d styles in %s:\n",count, base[0]->family);
i = count;
while(i--)
{
printf("               %s\n", base[i]->style);
}
}
#endif

  i = count;
  while(i--)
    boldcounts[base[i]->boldness]++;

  int boldmax = base[0]->boldness;
  int boldmin = base[0]->boldness;
  int bolduse = 0;

  i = 4;
  while(i--)
    {
      if(!boldcounts[i])
        continue;
      if(i>boldmax)
        boldmax = i;
      if(i<boldmin)
        boldmin = i;
      bolduse++;
    }
  if(likely(bolduse<=2))
    {
      // in case they are same, we want non-bold,
      // so that setting goes second
      boldmap[boldmax] = 1;
      boldmap[boldmin] = 0;
    }
  else if(count==3)
    {
      boldmap[boldmax] = 1;
      boldmap[boldmin] = 0;
      int boldmid = boldcounts[boldmin+1] ? boldmin+1 : boldmin+2;
      int zmin = 0, zmid = 0, zmax = 0;
      i = 3;
      while(i--){
        if(base[i]->boldness==boldmin)
          zmin = base[i]->italic;
        if(base[i]->boldness==boldmid)
          zmid = base[i]->italic;
        if(base[i]->boldness==boldmax)
          zmax = base[i]->italic;
      }
      if(zmin!=zmid)
        boldmap[boldmid] = 0;
      else if(zmid!=zmax)
        boldmap[boldmid] = 1;
      else if(boldmin==0 && boldmid==1)
        {
          boldmap[0] = -1;
          boldmap[1] = 0;
        }
    }
  else
    {
      // 3 or 4 boldness levels, 4 or more styles!
      // This is going to be random hacks and hopes.

      // bold is bold
      boldmap[3] = 1;
      int claimed_bold = boldcounts[3];

      // norm is norm
      boldmap[1] = 0;
      int claimed_norm = boldcounts[1];

      // classify demi-bold or medium
      if(claimed_bold<2)
        {
          boldmap[2] = 1;
          claimed_bold += boldcounts[2];
        }
      else if(claimed_norm<2)
        {
          boldmap[2] = 0;
          claimed_norm += boldcounts[2];
        }

      // classify lightface
      if(claimed_norm<2)
        {
          boldmap[0] = 0;
          //claimed_norm += boldcounts[0];
        }
    }

  if (num_font_families==num_font_families_max)
    {
      num_font_families_max = num_font_families_max * 5 / 4 + 30;
      user_font_families = realloc(user_font_families, num_font_families_max * sizeof *user_font_families); 
    }

  family_info *fi = calloc(1, sizeof *fi);
  user_font_families[num_font_families++] = fi;
  fi->directory = strdup(base[0]->directory);
  fi->family    = strdup(base[0]->family);
  fi->score     = base[0]->truetype + base[0]->score;
  i = count;
  while(i--)
    {
      int b = boldmap[base[i]->boldness];
      if(b==-1)
        {
#if 0
// THREADED_FONTS
          printf("too many boldness levels, discarding: %s, %s\n", base[i]->family, base[i]->style);
#endif
          continue;
        }
      int spot = b ? TTF_STYLE_BOLD : 0;
      spot += base[i]->italic ? TTF_STYLE_ITALIC : 0;
      if(fi->filename[spot])
        {
#if 0
// THREADED_FONTS
          printf("duplicates, discarding: %s, %s\n", base[i]->family, base[i]->style);
          printf("b %d, spot %d\n", b, spot);
          printf("occupancy %p %p %p %p\n", fi->filename[0], fi->filename[1], fi->filename[2], fi->filename[3]);
#endif
          continue;
        }
      fi->filename[spot] = strdup(base[i]->filename);
      fi->score += 2;
    }
  if(!fi->filename[0] && !fi->filename[1])
    {
      fi->filename[0] = fi->filename[2];
      fi->filename[2] = NULL;
      fi->filename[1] = fi->filename[3];
      fi->filename[3] = NULL;
    }
  if(!fi->filename[0] && !fi->filename[2])
    {
      fi->filename[0] = fi->filename[1];
      fi->filename[1] = NULL;
      fi->filename[2] = fi->filename[3];
      fi->filename[3] = NULL;
    }
  if(!fi->filename[0])
    {
      fi->filename[0] = strdup(fi->filename[TTF_STYLE_BOLD]);
    }
}


static void dupe_markdown_range(family_info **base, int count)
{
  int bestscore = -999;
  int bestslot = 0;
  int i = count;
  while(i--)
    {
      int score = base[i]->score;
      if(score<=bestscore)
        continue;
      bestscore = score;
      bestslot = i;
    }
  i = count;
  while(i--)
    {
      if (i==bestslot)
        continue;
      base[i]->score = -999;
    }
}


static void groupfonts(void)
{
  int i = num_font_styles;
  while(i--)
    parse_font_style(user_font_styles[i]);
  qsort(user_font_styles, num_font_styles, sizeof user_font_styles[0], compar_fontgroup);
  int low  = 0;
  for(;;)
    {
      if(low >= num_font_styles)
        break;
      int high = low;
      for(;;)
        {
          if(++high >= num_font_styles)
            break;
          if(compar_fontgroup(user_font_styles+low, user_font_styles+high))
            break;
        }
      groupfonts_range(user_font_styles+low, high-low);
      low = high;
    }

  i = num_font_styles;
  while(i--)
    {
      free(user_font_styles[i]->filename);
      free(user_font_styles[i]->directory);
      free(user_font_styles[i]->family);
      free(user_font_styles[i]->style);
      free(user_font_styles[i]);
    }
  free(user_font_styles);
  user_font_styles = NULL; // just to catch bugs

  qsort(user_font_families, num_font_families, sizeof user_font_families[0], compar_fontkiller);
  low = 0;
  for(;;)
    {
      if(low >= num_font_families)
        break;
      int high = low;
      for(;;)
        {
          if(++high >= num_font_families)
            break;
          if(strcmp(user_font_families[low]->family,user_font_families[high]->family))
            break;
        }
      dupe_markdown_range(user_font_families+low,high-low);
      low = high;
    }
  qsort(user_font_families, num_font_families, sizeof user_font_families[0], compar_fontscore);
  if(user_font_families[0]->score < 0)
    printf("sorted the wrong way, or all fonts were crap\n");
#if 0
// THREADED_FONTS
  printf("Trim starting with %d families\n", num_font_families);
#endif
  while(num_font_families>1 && user_font_families[num_font_families-1]->score < 0)
    {
      i = --num_font_families;
      free(user_font_families[i]->directory);
      free(user_font_families[i]->family);
      char ** cpp = user_font_families[i]->filename;
      if(cpp[0])
        free(cpp[0]);
      if(cpp[1])
        free(cpp[1]);
      if(cpp[2])
        free(cpp[2]);
      if(cpp[3])
        free(cpp[3]);
      free(user_font_families[i]);
      user_font_families[i] = NULL;
    }
#if 0
// THREADED_FONTS
  printf("Trim ending with %d families\n", num_font_families);
#endif
}

////////////////////////////////////////////////////////////////////

static int num_brushes, num_brushes_max;
static SDL_Surface **img_brushes;

#define MAX_STAMPS 512
static int num_stamps;
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

static SDL_Surface * img_mouse, * img_mouse_click;

#ifdef LOW_QUALITY_COLOR_SELECTOR
static SDL_Surface * img_paintcan;
#else
static SDL_Surface * img_color_btns[NUM_COLORS*2];
static SDL_Surface * img_color_btn_off;
#endif

static int colors_are_selectable;

static SDL_Surface * img_cur_brush;
static int brush_counter, rainbow_color;

#ifndef NOSOUND
static Mix_Chunk * sounds[NUM_SOUNDS];
#endif

#define NUM_ERASERS 6 /* How many sizes of erasers (from ERASER_MIN to _MAX) */
#define ERASER_MIN 13 
#define ERASER_MAX 128 


static SDL_Cursor * cursor_hand, * cursor_arrow, * cursor_watch,
  * cursor_up, * cursor_down, * cursor_tiny, * cursor_crosshair,
  * cursor_brush, * cursor_wand, * cursor_insertion, * cursor_rotate;


static unsigned cur_color;
static int cur_tool, cur_brush, cur_stamp, cur_shape, cur_magic;
static int cur_font, cur_eraser;
static int cursor_left, cursor_x, cursor_y, cursor_textwidth; // canvas-relative
static int been_saved;
static char file_id[32];
static char starter_id[32];
static int brush_scroll, stamp_scroll, font_scroll, magic_scroll;
static int eraser_scroll, shape_scroll; // dummy variables for now

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

int non_left_click_count = 0;


static char * savedir;

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
static void show_usage(FILE * f, char * prg);
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

static void putpixel8(SDL_Surface * surface, int x, int y, Uint32 pixel);
static void putpixel16(SDL_Surface * surface, int x, int y, Uint32 pixel);
static void putpixel24(SDL_Surface * surface, int x, int y, Uint32 pixel);
static void putpixel32(SDL_Surface * surface, int x, int y, Uint32 pixel);

static void (*putpixels[])(SDL_Surface *, int, int, Uint32) = {
  putpixel8, putpixel8, putpixel16, putpixel24, putpixel32 };

static Uint32 getpixel8(SDL_Surface * surface, int x, int y);
static Uint32 getpixel16(SDL_Surface * surface, int x, int y);
static Uint32 getpixel24(SDL_Surface * surface, int x, int y);
static Uint32 getpixel32(SDL_Surface * surface, int x, int y);

static Uint32 (*getpixels[])(SDL_Surface *, int, int) = {
  getpixel8, getpixel8, getpixel16, getpixel24, getpixel32 };

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
static void do_wait(int counter);
static void load_current(void);
static void save_current(void);
static char * get_fname(const char * const name);
static int do_prompt_image_flash(const char * const text, const char * const btn_yes, const char * const btn_no, SDL_Surface * img1, SDL_Surface * img2, SDL_Surface * img3, int animate);
static int do_prompt_image(const char * const text, const char * const btn_yes, const char * const btn_no, SDL_Surface * img1, SDL_Surface * img2, SDL_Surface * img3);
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
static void wait_for_sfx(void);
static int stamp_colorable(int stamp);
static int stamp_tintable(int stamp);
static void rgbtohsv(Uint8 r8, Uint8 g8, Uint8 b8, float *h, float *s, float *v);
static void hsvtorgb(float h, float s, float v, Uint8 *r8, Uint8 *g8, Uint8 *b8);

static void show_progress_bar(void);
static int progress_bar_disabled;

static void do_print(void);
static void strip_trailing_whitespace(char * buf);
static void do_render_cur_text(int do_blit);
static void loadfonts(const char * const dir);
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
static void draw_image_title(int t, SDL_Rect dest);
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
static void mirror_starter(void);
static void flip_starter(void);
int valid_click(Uint8 button);

#ifdef DEBUG
static char * debug_gettext(const char * str);
static int charsize(char c);
#endif


#define MAX_UTF8_CHAR_LENGTH 6

#define USEREVENT_TEXT_UPDATE 1

static int bypass_splash_wait;

// Wait for a keypress or mouse click
// counter is in 1/10 second units
static void do_wait(int counter)
{
  SDL_Event event;
  int done;

  if(bypass_splash_wait)
    return;

  done = 0;

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
		   valid_click(event.button.button))
	    {
	      done = 1;
	    }
	}

      counter--;
      SDL_Delay(100);
    }
  while (!done && counter > 0);
}


// This lets us exit quickly; perhaps the system is swapping to death
// or the user started Tux Paint by accident. It also lets the user
// more easily bypass the splash screen wait.
static void eat_sdl_events(void)
{
  SDL_Event event;
  while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_QUIT)
        exit(0);  // can't safely use do_quit during start-up
      else if (event.type == SDL_ACTIVEEVENT)
        handle_active(&event);
      else if (event.type == SDL_KEYDOWN)
        {
          SDLKey key  = event.key.keysym.sym;
          SDLMod ctrl = event.key.keysym.mod & (KMOD_CTRL|KMOD_LCTRL|KMOD_RCTRL);  
          SDLMod alt  = event.key.keysym.mod & KMOD_ALT;
          if (key==SDLK_ESCAPE || (key==SDLK_c && ctrl) || (key==SDLK_F4 && alt))
            exit(0);
          else                                                              
            bypass_splash_wait = 1;
        }
      else if (event.type == SDL_MOUSEBUTTONDOWN)
        bypass_splash_wait = 1;
    }
}


static void show_progress_bar(void)
{
  SDL_Rect dest, src;
  int x;
  static Uint32 oldtime;
  Uint32 newtime;

  if(progress_bar_disabled)
    return;

  newtime = SDL_GetTicks();
  if(newtime > oldtime+15)  // trying not to eat some serious CPU time!
    {
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
  oldtime = newtime;

  eat_sdl_events();
}



#ifdef __powerpc__
// Ticks at 1/4  the memory bus clock (24.907667 MHz on Albert's Mac Cube)
// This is good for 80-second diff or 160-second total.
#define CLOCK_ASM(tbl) asm volatile("mftb %0" : "=r" (tbl))
#define CLOCK_TYPE unsigned long
#ifndef CLOCK_SPEED
#warning Benchmark times are based on a 99.63 MHz memory bus.
#define CLOCK_SPEED 24907667.0
#endif
#endif

#ifdef __i386__
#define CLOCK_ASM(tbl) asm volatile("rdtsc" : "=A" (tbl))
#define CLOCK_TYPE unsigned long long
#ifndef CLOCK_SPEED
#warning Benchmark times are based on a 450 MHz CPU.
#define CLOCK_SPEED 450000000.0
#endif
#endif

#ifndef CLOCK_ASM
#warning No idea how to read CPU cycles for you, sorry.
#define CLOCK_ASM(tbl)
#define CLOCK_TYPE unsigned long
#define CLOCK_SPEED 1000000000.0
#endif



/* --- MAIN --- */

int main(int argc, char * argv[])
{
#ifdef FORKED_FONTS
  run_font_scanner();
#endif

  /* Set up locale support */
  setlocale(LC_ALL, "");

  CLOCK_TYPE time1;
  CLOCK_ASM(time1);

  /* Set up! */
  setup(argc, argv);

#if 0
  while(!font_thread_done)
    {
      // FIXME: should respond to quit events
      // FIXME: should have a read-depends memory barrier around here
      show_progress_bar();
      SDL_Delay(20);
    }
  SDL_WaitThread(font_thread, NULL);
#endif

  CLOCK_TYPE time2;
  CLOCK_ASM(time2);

  printf("Start-up time: %.3f\n", (double)(time2-time1)/CLOCK_SPEED);

  // Let the user know we're (nearly) ready now

  SDL_Rect dest;
  SDL_Rect src;
  dest.x = 0;
  dest.y = WINDOW_HEIGHT - img_progress->h;
  dest.h = img_progress->h;
  dest.w = WINDOW_WIDTH;
  SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format, 255, 255, 255));
  src.h = img_progress->h;
  src.w = img_title->w;
  src.x = 0;
  src.y = img_title->h - img_progress->h;
  dest.x = (WINDOW_WIDTH - img_title->w) / 2;
  SDL_BlitSurface(img_title, &src, screen, &dest);
  SDL_FreeSurface(img_title);

  dest.x = 0;
  dest.w = WINDOW_WIDTH;  // SDL mangles this! So, do repairs.
  update_screen_rect(&dest);

  do_setcursor(cursor_arrow);
  playsound(0, SND_HARP, 1);
  do_wait(50);  // about 5 seconds


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

  draw_tux_text(tool_tux[cur_tool], tool_tips[cur_tool], 1);

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

#define PROMPT_TIP_LEFTCLICK_TXT gettext_noop("Remember to use the left mouse button!")
#define PROMPT_TIP_LEFTCLICK_YES gettext_noop("OK")


enum {
  SHAPE_TOOL_MODE_STRETCH,
  SHAPE_TOOL_MODE_ROTATE,
  SHAPE_TOOL_MODE_DONE
};


/* --- MAIN LOOP! --- */

static void mainloop(void)
{
  int done, which, button_down, old_x, old_y, new_x, new_y,
    line_start_x, line_start_y, shape_tool_mode,
    shape_ctr_x, shape_ctr_y, shape_outer_x, shape_outer_y;
  int num_things;
  int *thing_scroll;
  int cur_thing, do_draw, old_tool,
    tmp_int, max;
  int cur_time, last_print_time, scrolling, ignoring_motion;
  SDL_TimerID scrolltimer;
  SDL_Event event;
  SDLKey key, key_down;
  Uint16 key_unicode;
  SDLMod mod;
  Uint32 last_cursor_blink, cur_cursor_blink,
    pre_event_time, current_event_time;


  num_things = num_brushes;
  thing_scroll = &brush_scroll;
  cur_thing = 0;
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

	      if (key == SDLK_ESCAPE && !disable_quit)
		{
		  done = do_quit();
		}
	      else if (key == SDLK_ESCAPE &&
		       (mod & KMOD_SHIFT) && (mod & KMOD_CTRL))
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
		      update_screen_rect(&r_tools);
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
		      update_screen_rect(&r_tools);
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
		  update_screen_rect(&r_tools);
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
		  update_screen_rect(&r_tools);
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
		  update_screen_rect(&r_tools);
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
			      playsound(0, SND_KEYCLICK, 0);
	        
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
			  int font_height = TTF_FontHeight(getfonthandle(cur_font));

			  cursor_x = cursor_left;
			  cursor_y = min(cursor_y+font_height, canvas->h-font_height);
	    
			  playsound(0, SND_RETURN, 1);
			}
		      else if (key_down == SDLK_TAB)
		        {
			  if (texttool_len > 0)
			    {
			      rec_undo_buffer();
			      do_render_cur_text(1);
			      cursor_x = min(cursor_x + cursor_textwidth,
					     canvas->w);
			      texttool_len = 0;
			      cursor_textwidth = 0;
			    }
		        }
		      else if (isprint(key_unicode))
			{
			  if (texttool_len < sizeof(texttool_str) - MAX_UTF8_CHAR_LENGTH)
			    {
			      int old_cursor_textwidth = cursor_textwidth;
#ifdef DEBUG
			      printf("    key = %c\n"
				     "unicode = %c (%d)\n\n",
				     key_down, key_unicode, key_unicode);
#endif
	  	     
			        texttool_str[texttool_len++] = key_unicode;
		
			      texttool_str[texttool_len] = '\0';
			      do_render_cur_text(0);


			      if (cursor_x + old_cursor_textwidth <= canvas->w - 50 &&
				  cursor_x + cursor_textwidth > canvas->w - 50)
			      {
				playsound(0, SND_KEYCLICKRING, 1);
			      }
			      else
			      {
				playsound(0, SND_KEYCLICK, 0);
			      }
			    }
			}
		    }
		}
	    }
	  else if (event.type == SDL_MOUSEBUTTONDOWN &&
		   valid_click(event.button.button))
	    {
	      if (HIT(r_tools))
		{
		  /* A tool on the left has been pressed! */
		  which = GRIDHIT_GD(r_tools,gd_tools);
		  
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
		      update_screen_rect(&r_tools);
		      
		      playsound(1, SND_CLICK, 0);

                      // FIXME: this "if" is just plain gross
		      if(cur_tool != TOOL_TEXT)
		        draw_tux_text(tool_tux[cur_tool], tool_tips[cur_tool], 1);
		      
		      /* Draw items for this tool: */
		      
		      if (cur_tool == TOOL_BRUSH)
			{
			  cur_thing = cur_brush;
			  num_things = num_brushes;
			  thing_scroll = &brush_scroll;
			  draw_brushes();
			  draw_colors(COLORSEL_ENABLE);
			}
		      else if (cur_tool == TOOL_STAMP)
			{
			  cur_thing = cur_stamp;
			  num_things = num_stamps;
			  thing_scroll = &stamp_scroll;
			  draw_stamps();
			  draw_colors(stamp_colorable(cur_stamp) ||
				      stamp_tintable(cur_stamp));
			  update_stamp_xor();
			}
		      else if (cur_tool == TOOL_LINES)
			{
			  cur_thing = cur_brush;
			  num_things = num_brushes;
			  thing_scroll = &brush_scroll;
			  draw_brushes();
			  draw_colors(COLORSEL_ENABLE);
			}
		      else if (cur_tool == TOOL_SHAPES)
			{
			  cur_thing = cur_shape;
			  num_things = NUM_SHAPES;
			  thing_scroll = &shape_scroll;
			  draw_shapes();
			  draw_colors(COLORSEL_ENABLE);
			  shape_tool_mode = SHAPE_TOOL_MODE_DONE;
			}
		      else if (cur_tool == TOOL_TEXT)
			{
			  if(!font_thread_done)
			    {
                              draw_colors(COLORSEL_DISABLE);
                              draw_none();
                              update_screen_rect(&r_toolopt);
                              update_screen_rect(&r_ttoolopt);
                              draw_tux_text(TUX_WAIT, "This is a slow computer with lots of fonts...", 1);
#ifdef FORKED_FONTS
                              receive_some_font_info();
#else
                              while(!font_thread_done)
                                {
                                  // FIXME: should respond to quit events
                                  // FIXME: should have a read-depends memory barrier around here
                                  show_progress_bar();
                                  SDL_Delay(20);
                                }
                              // FIXME: should kill this in any case
                              SDL_WaitThread(font_thread, NULL);
#endif
                            }
		          draw_tux_text(tool_tux[cur_tool], tool_tips[cur_tool], 1);
			  cur_thing = cur_font;
			  num_things = num_font_families;
			  thing_scroll = &font_scroll;
			  draw_fonts();
			  draw_colors(COLORSEL_ENABLE);
			}
		      else if (cur_tool == TOOL_MAGIC)
			{
			  cur_thing = cur_magic;
			  num_things = NUM_MAGICS;
			  thing_scroll = &magic_scroll;
			  rainbow_color = 0;
			  draw_magic();
			  draw_colors(magic_colors[cur_magic]);
			}
		      else if (cur_tool == TOOL_ERASER)
			{
			  cur_thing = cur_eraser;
			  num_things = NUM_ERASERS;
			  thing_scroll = &eraser_scroll;
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
			  update_screen_rect(&r_tools);
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
			  update_screen_rect(&r_tools);
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
			  update_screen_rect(&r_tools);
			  
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
			  update_screen_rect(&r_tools);
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
			  update_screen_rect(&r_tools);
			}
		      else if (cur_tool == TOOL_PRINT)
			{
			  cur_time = SDL_GetTicks() / 1000;
			  
#ifdef DEBUG
			  printf("Current time = %d\n", cur_time);
#endif
			  
			  if (cur_time >= last_print_time + print_delay)
			    {
		              if (do_prompt_image(PROMPT_PRINT_NOW_TXT,
				            PROMPT_PRINT_NOW_YES,
				            PROMPT_PRINT_NOW_NO,
					    img_printer, NULL, NULL))
			      {
			        do_print();
			      
			        last_print_time = cur_time;
			      }
			    }
			  else
			    {
			      do_prompt_image(PROMPT_PRINT_TOO_SOON_TXT,
					PROMPT_PRINT_TOO_SOON_YES,
					"",
					img_printer_wait, NULL, NULL);
			    }
			  
			  cur_tool = old_tool;
			  draw_toolbar();
			  update_screen_rect(&r_tools);
			}
		      else if (cur_tool == TOOL_QUIT)
			{
			  done = do_quit();
			  cur_tool = old_tool;
			  draw_toolbar();
			  update_screen_rect(&r_tools);
			}
		      update_screen_rect(&r_toolopt);
		      update_screen_rect(&r_ttoolopt);
		    }
		}
	      else if (HIT(r_toolopt))
		{
		  // Options on the right
                  // WARNING: this must be kept in sync with the mouse-move
                  // code (for cursor changes) and mouse-scroll code.
		  
		  if (cur_tool == TOOL_BRUSH || cur_tool == TOOL_STAMP ||
		      cur_tool == TOOL_SHAPES || cur_tool == TOOL_LINES ||
		      cur_tool == TOOL_MAGIC || cur_tool == TOOL_TEXT ||
		      cur_tool == TOOL_ERASER)
		    {
		      int old_thing;
		      grid_dims gd_controls = {0,0}; // might become 2-by-2
		      grid_dims gd_items = {2,2}; // generally becoming 2-by-whatever

		      /* Note set of things we're dealing with */
		      /* (stamps, brushes, etc.) */
		      
		      if (cur_tool == TOOL_STAMP)
			{
			  if(!disable_stamp_controls)
			    gd_controls = (grid_dims){2,2};
			}
		      else if (cur_tool == TOOL_TEXT)
			{
			  if(!disable_stamp_controls)
			    gd_controls = (grid_dims){2,2};
			}

		      // number of whole or partial rows that will be needed
		      // (can make this per-tool if variable columns needed)
	              int num_rows_needed = (num_things+gd_items.cols-1)/gd_items.cols;

		      do_draw = 0;

		      SDL_Rect r_controls;
		      r_controls.w = r_toolopt.w;
		      r_controls.h = gd_controls.rows * button_h;
		      r_controls.x = r_toolopt.x;
		      r_controls.y = r_toolopt.y + r_toolopt.h - r_controls.h;
		      
		      SDL_Rect r_notcontrols;
		      r_notcontrols.w = r_toolopt.w;
		      r_notcontrols.h = r_toolopt.h - r_controls.h;
		      r_notcontrols.x = r_toolopt.x;
		      r_notcontrols.y = r_toolopt.y;
		      
		      SDL_Rect r_items = r_notcontrols;
		      if(num_rows_needed * button_h > r_items.h)
		        {
		          // too many; we'll need scroll buttons
		          r_items.h -= button_h;
		          r_items.y += button_h/2;
		        }
	              gd_items.rows = r_items.h / button_h;

	              int toolopt_changed = 0;

		      if(HIT(r_items))
		        {
		          which = GRIDHIT_GD(r_items,gd_items) + *thing_scroll;
                          
                          if (which < num_things)
                            {
                              toolopt_changed = 1;
#ifndef NOSOUND
                              if (cur_tool != TOOL_STAMP || snd_stamps[which] == NULL)
                                { 
                                  playsound(1, SND_BLEEP, 0);
                                }
#endif
                              old_thing = cur_thing;
                              cur_thing = which;
                              do_draw = 1;
                            }
		        }
		      else if(HIT(r_controls))
		        {
		          which = GRIDHIT_GD(r_controls,gd_controls);
		          if (cur_tool == TOOL_STAMP)
		            {
                              /* Stamp controls! */
                              int control_sound = -1;
                              if (which&2)
                                {
                                  /* One of the bottom buttons: */
                                  if (which&1)
                                    {
                                      /* Bottom right button: Grow: */
                                      if (state_stamps[cur_stamp]->size < MAX_STAMP_SIZE)
                                        {
                                          state_stamps[cur_stamp]->size++;
                                          control_sound = SND_GROW;
                                        }
                                    }
                                  else
                                    {
                                      /* Bottom left button: Shrink: */
                                      if (state_stamps[cur_stamp]->size > MIN_STAMP_SIZE)
                                        {
                                          state_stamps[cur_stamp]->size--;
                                          control_sound = SND_SHRINK;
                                        }
                                    }
                                }
                              else
                                {
                                  /* One of the top buttons: */
                                  if (which&1)
                                    {
                                      /* Top right button: Flip: */
                                      if (inf_stamps[cur_stamp]->flipable)
                                        {
                                          state_stamps[cur_stamp]->flipped =
                                            !state_stamps[cur_stamp]->flipped;
                                          control_sound = SND_FLIP;
                                        }
                                    }
                                  else
                                    {
                                      /* Top left button: Mirror: */
                                      if (inf_stamps[cur_stamp]->mirrorable)
                                        {
                                          state_stamps[cur_stamp]->mirrored =
                                            !state_stamps[cur_stamp]->mirrored;
                                          control_sound = SND_MIRROR;
                                        }
                                    }
                                }
                              if (control_sound != -1)
                                {
                                  playsound(0, control_sound, 0);
                                  draw_stamps();
                                  update_screen_rect(&r_toolopt);
                                  update_stamp_xor();
                                }
		            }
		          else  // not TOOL_STAMP, so must be TOOL_TEXT
		            {
                              /* Text controls! */
                              int control_sound = -1;
                              if (which&2)
                                {
                                  /* One of the bottom buttons: */
                                  if (which&1)
                                    {
                                      /* Bottom right button: Grow: */
                                      if (text_size < MAX_TEXT_SIZE)
                                        {
                                          text_size++;
                                          control_sound = SND_GROW;
				          toolopt_changed = 1;
                                        }
                                    }
                                  else
                                    {
                                      /* Bottom left button: Shrink: */
                                      if (text_size > MIN_TEXT_SIZE)
                                        {
                                          text_size--;
                                          control_sound = SND_SHRINK;
				          toolopt_changed = 1;
                                        }
                                    }
                                }
                              else
                                {
                                  /* One of the top buttons: */
                                  if (which&1)
                                    {
                                      /* Top right button: Italic: */
                                      if (text_state & TTF_STYLE_ITALIC)
                                        {
                                          text_state &= ~TTF_STYLE_ITALIC;
                                          control_sound = SND_ITALIC_ON;
                                        }
                                      else
                                        {
                                          text_state |= TTF_STYLE_ITALIC;
                                          control_sound = SND_ITALIC_OFF;
                                        }
                                    }
                                  else
                                    {
                                      /* Top left button: Bold: */
                                      if (text_state & TTF_STYLE_BOLD)
                                        {
                                          text_state &= ~TTF_STYLE_BOLD;
                                          control_sound = SND_THIN;
                                        }
                                      else
                                        {
                                          text_state |= TTF_STYLE_BOLD;
                                          control_sound = SND_THICK;
                                        }
                                    }
				  toolopt_changed = 1;
                                }


                              if (control_sound != -1)
                                {
                                  playsound(0, control_sound, 0);


				  if (cur_tool == TOOL_TEXT) // Huh? It had better be!
				  {
                                    // need to invalidate all the cached user fonts, causing reload on demand
                                    int i;
                                    for (i = 0; i < num_font_families; i++)
                                      {
                                        if (user_font_families[i] && user_font_families[i]->handle)
                                          {
                                            TTF_CloseFont(user_font_families[i]->handle);
                                            user_font_families[i]->handle = NULL;
                                          }
                                      }
                                    // FIXME: is setting do_draw enough?
                                    draw_fonts();
                                    update_screen_rect(&r_toolopt);
				  }
                                }
                            }
		        }
		      else
		        {
		          // scroll button
		          int is_upper = event.button.y < r_toolopt.y + button_h/2;
                          if (
                               (is_upper && *thing_scroll > 0)  // upper arrow
                               ||
                               (!is_upper && *thing_scroll/gd_items.cols < num_rows_needed-gd_items.rows) // lower arrow
                             )
                            {
                              *thing_scroll += is_upper ? -gd_items.cols : gd_items.cols;
                              do_draw = 1;
                              playsound(1, SND_SCROLL, 1);
                              if (!scrolling)
                                {
                                  memcpy(&scrolltimer_event, &event, sizeof(SDL_Event));
                                  /* FIXME: Make delay value changable: */
                                  scrolltimer = SDL_AddTimer(REPEAT_SPEED, scrolltimer_callback, (void*) &scrolltimer_event);
                                  scrolling = 1;
                                }
                              else
                                {
                                  SDL_RemoveTimer(scrolltimer);
                                  scrolltimer = SDL_AddTimer(REPEAT_SPEED / 3, scrolltimer_callback, (void*) &scrolltimer_event);
                                }
                              if (*thing_scroll == 0)
                                {
                                  do_setcursor(cursor_arrow);
                                  if (scrolling)
                                    {
                                      SDL_RemoveTimer(scrolltimer);
                                      scrolling = 0;
                                    }
                                }
                            }
		        }
                      
                      
                      /* Assign the change(s), if any / redraw, if needed: */
                      
                      if (cur_tool == TOOL_BRUSH || cur_tool == TOOL_LINES)
                        {
                          cur_brush = cur_thing;
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
                          
                          char font_tux_text[512];
                          snprintf(font_tux_text, sizeof font_tux_text, "%s (%s).", TTF_FontFaceFamilyName(getfonthandle(cur_font)), TTF_FontFaceStyleName(getfonthandle(cur_font)));
//                          printf("font change:%s\n", font_tux_text);
                          draw_tux_text(TUX_GREAT, font_tux_text, 1);
                          
                          if (do_draw)
                            draw_fonts();
                          
			  
			  // Only rerender when picking a different font
			  if (toolopt_changed)
                            do_render_cur_text(0);
                        }
                      else if (cur_tool == TOOL_STAMP)
                        {
#ifndef NOSOUND
                          if (toolopt_changed)
                            {
                              // Only play when picking a different stamp
                              if (snd_stamps[cur_thing] != NULL)
                                Mix_PlayChannel(2, snd_stamps[cur_thing], 0);
                            }
#endif

                          if (cur_thing != cur_stamp)
                          {
                            cur_stamp = cur_thing;
                            update_stamp_xor();
                          }

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
                          draw_colors(stamp_colorable(cur_stamp) || stamp_tintable(cur_stamp));
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
                            }
                          
                          draw_tux_text(TUX_GREAT, magic_tips[cur_magic], 1);
                          
                          if (do_draw)
                            draw_magic();
                        }
                      
                      /* Update the screen: */
                      if (do_draw)
                        update_screen_rect(&r_toolopt);
                    }
                }
	      else if (HIT(r_colors) && colors_are_selectable)
		{
		  /* Color! */
		  which = GRIDHIT_GD(r_colors,gd_colors);
		  
		  if (which < NUM_COLORS)
		    {
		      cur_color = which;
		      playsound(1, SND_BUBBLE, 1);
		      draw_colors(COLORSEL_REFRESH);
		      render_brush();
		      draw_tux_text(TUX_KISS, color_names[cur_color], 1);
		
		      if (cur_tool == TOOL_TEXT)
		        do_render_cur_text(0);
		    }
		}
	      else if (HIT(r_canvas))
		{
		  /* Draw something! */
		  
		  old_x = event.button.x - r_canvas.x;
		  old_y = event.button.y - r_canvas.y;
		  
		  if (been_saved)
		    {
		      been_saved = 0;

		      if (!disable_save)
		        tool_avail[TOOL_SAVE] = 1;

		      draw_toolbar();
		      update_screen_rect(&r_tools);
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
					    event.button.x - r_canvas.x,
					    event.button.y - r_canvas.y),
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
					getpixels[canvas->format->BytesPerPixel](canvas, old_x, old_y));

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
		      update_screen_rect(&r_tools);
		    }
		}
	    }
	  else if (event.type == SDL_MOUSEBUTTONDOWN &&
		   wheely &&
		   event.button.button >= 4 &&
		   event.button.button <= 5)
            {
              // Scroll wheel code.
              // WARNING: this must be kept in sync with the mouse-move
              // code (for cursor changes) and mouse-click code.

              if (cur_tool == TOOL_BRUSH || cur_tool == TOOL_STAMP ||
                  cur_tool == TOOL_SHAPES || cur_tool == TOOL_LINES ||
                  cur_tool == TOOL_MAGIC || cur_tool == TOOL_TEXT ||
                  cur_tool == TOOL_ERASER)
                {
                  grid_dims gd_controls = {0,0}; // might become 2-by-2
                  grid_dims gd_items = {2,2}; // generally becoming 2-by-whatever

                  /* Note set of things we're dealing with */
                  /* (stamps, brushes, etc.) */
                  
                  if (cur_tool == TOOL_STAMP)
                    {
                      if(!disable_stamp_controls)
                        gd_controls = (grid_dims){2,2};
                    }
                  else if (cur_tool == TOOL_TEXT)
                    {
                      if(!disable_stamp_controls)
                        gd_controls = (grid_dims){2,2};
                    }

                  // number of whole or partial rows that will be needed
                  // (can make this per-tool if variable columns needed)
                  int num_rows_needed = (num_things+gd_items.cols-1)/gd_items.cols;

                  do_draw = 0;

                  SDL_Rect r_controls;
                  r_controls.w = r_toolopt.w;
                  r_controls.h = gd_controls.rows * button_h;
                  r_controls.x = r_toolopt.x;
                  r_controls.y = r_toolopt.y + r_toolopt.h - r_controls.h;
                  
                  SDL_Rect r_notcontrols;
                  r_notcontrols.w = r_toolopt.w;
                  r_notcontrols.h = r_toolopt.h - r_controls.h;
                  r_notcontrols.x = r_toolopt.x;
                  r_notcontrols.y = r_toolopt.y;
                  
                  SDL_Rect r_items = r_notcontrols;
                  if(num_rows_needed * button_h > r_items.h)
                    {
                      // too many; we'll need scroll buttons
                      r_items.h -= button_h;
                      r_items.y += button_h/2;
                    }
                  gd_items.rows = r_items.h / button_h;

                  if(0)
                    {
                    }
                  else
                    {
                      // scroll button
                      int is_upper = (event.button.button == 4);
                      if (
                           (is_upper && *thing_scroll > 0)  // upper arrow
                           ||
                           (!is_upper && *thing_scroll/gd_items.cols < num_rows_needed-gd_items.rows) // lower arrow
                         )
                        {
                          *thing_scroll += is_upper ? -gd_items.cols : gd_items.cols;
                          do_draw = 1;
                          playsound(1, SND_SCROLL, 1);
#if 0
                          if (!scrolling)
                            {
                              memcpy(&scrolltimer_event, &event, sizeof(SDL_Event));
                              /* FIXME: Make delay value changable: */
                              scrolltimer = SDL_AddTimer(REPEAT_SPEED, scrolltimer_callback, (void*) &scrolltimer_event);
                              scrolling = 1;
                            }
                          else
                            {
                              SDL_RemoveTimer(scrolltimer);
                              scrolltimer = SDL_AddTimer(REPEAT_SPEED / 3, scrolltimer_callback, (void*) &scrolltimer_event);
                            }
#endif
                          if (*thing_scroll == 0)
                            {
                              do_setcursor(cursor_arrow);
#if 0
                              if (scrolling)
                                {
                                  SDL_RemoveTimer(scrolltimer);
                                  scrolling = 0;
                                }
#endif
                            }
                        }
                    }
                  
                  
                  /* Assign the change(s), if any / redraw, if needed: */
                  
                  if (cur_tool == TOOL_BRUSH || cur_tool == TOOL_LINES)
                    {
                      if (do_draw)
                        draw_brushes();
                    }
                  else if (cur_tool == TOOL_ERASER)
                    {
                      if (do_draw)
                        draw_erasers();
                    }
                  else if (cur_tool == TOOL_TEXT)
                    {
                      if (do_draw)
                        draw_fonts();
                    }
                  else if (cur_tool == TOOL_STAMP)
                    {
                      if (do_draw)
                        draw_stamps();
                    }
                  else if (cur_tool == TOOL_SHAPES)
                    {
                      if (do_draw)
                        draw_shapes();
                    }
                  else if (cur_tool == TOOL_MAGIC)
                    {
                      if (do_draw)
                        draw_magic();
                    }
                  
                  /* Update the screen: */
                  if (do_draw)
                    update_screen_rect(&r_toolopt);
                }
            }
	  else if (event.type == SDL_MOUSEBUTTONDOWN &&
		   event.button.button >= 2 &&
		   event.button.button <= 3)
	    {
	      /* They're using the middle or right mouse buttons! */

	      non_left_click_count++;


	      if (non_left_click_count == 10 ||
		  non_left_click_count == 20 ||
		  (non_left_click_count % 50) == 0)
	      {
		/* Pop up an informative animation: */

		do_prompt_image_flash(PROMPT_TIP_LEFTCLICK_TXT,
				      PROMPT_TIP_LEFTCLICK_YES,
				      "",
			              img_mouse, img_mouse_click, NULL, 1);
	      }
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
				 event.button.x-r_canvas.x, event.button.y-r_canvas.y, 1);
		      brush_draw(event.button.x-r_canvas.x, event.button.y-r_canvas.y,
				 event.button.x-r_canvas.x, event.button.y-r_canvas.y, 1);
		      
		      playsound(1, SND_LINE_END, 1);
		      draw_tux_text(TUX_GREAT, tool_tips[TOOL_LINES], 1);
		    }
		  else if (cur_tool == TOOL_SHAPES)
		    {
		      if (shape_tool_mode == SHAPE_TOOL_MODE_STRETCH)
			{
			  /* Now we can rotate the shape... */
			  
			  shape_outer_x = event.button.x-r_canvas.x;
			  shape_outer_y = event.button.y-r_canvas.y;
			  
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
	      new_x = event.button.x-r_canvas.x;
	      new_y = event.button.y-r_canvas.y;
	      
	      
	      /* FIXME: Is doing this every event too intensive? */
	      /* Should I check current cursor first? */
	      
	      if (HIT(r_tools))
		{
		  /* Tools: */
		  
		  if (tool_avail[((event.button.x - r_tools.x) / button_w) +
				 ((event.button.y - r_tools.y) / button_h) * gd_tools.cols])
		    {
		      do_setcursor(cursor_hand);
		    }
		  else
		    {
		      do_setcursor(cursor_arrow);
		    }
		}
	      else if (HIT(r_colors))
		{
		  /* Color picker: */
		 
	  	  if (colors_are_selectable)
		    do_setcursor(cursor_hand);
		  else
		    do_setcursor(cursor_arrow);
		}
	      else if (HIT(r_toolopt))
		{
		  // mouse cursor code
                  // WARNING: this must be kept in sync with the mouse-click
                  // and mouse-click code. (it isn't, currently!)
		  
		  /* Note set of things we're dealing with */
		  /* (stamps, brushes, etc.) */
		  
		  if (cur_tool == TOOL_STAMP)
		    {
		    }
		  else if (cur_tool == TOOL_TEXT)
		    {
		    }

		  max = 14;
		  if (cur_tool == TOOL_STAMP && !disable_stamp_controls)
		    max = 10;
		  if (cur_tool == TOOL_TEXT && !disable_stamp_controls)
		    max = 10;
		  
		  
		  if (num_things > max + TOOLOFFSET)
		    {
		      /* Are there scroll buttons? */
		      
		      if (event.button.y < 40 + 24)
			{
			  /* Up button; is it available? */
			  
			  if (*thing_scroll > 0)
			    do_setcursor(cursor_up);
			  else
			    do_setcursor(cursor_arrow);
			}
		      else if (event.button.y > (48 * ((max - 2) / 2 + TOOLOFFSET / 2)) + 40 + 24 &&
		               event.button.y <= (48 * ((max - 2) / 2 + TOOLOFFSET / 2)) + 40 + 24 + 24)
			{
			  /* Down button; is it available? */
			  
			  if (*thing_scroll < num_things - (max - 2))
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
	      else if (HIT(r_canvas))
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
		      
		      update_screen(line_start_x + r_canvas.x, line_start_y + r_canvas.y,
				    old_x + r_canvas.x, old_y + r_canvas.y);
		      update_screen(line_start_x + r_canvas.x, line_start_y + r_canvas.y,
				    new_x + r_canvas.x, new_y + r_canvas.y);
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
                  int w, h;
		  /* Moving: Draw XOR where stamp/eraser will apply: */
		  
		  if (cur_tool == TOOL_STAMP)
		    {
		      w = img_stamps[cur_stamp]->w;
		      h = img_stamps[cur_stamp]->h;
		    }
		  else
		    {
		      w = (ERASER_MIN +
			   ((NUM_ERASERS - cur_eraser - 1) *
			    ((ERASER_MAX - ERASER_MIN) / (NUM_ERASERS - 1))));

		      h = w;
		    }
		  
		  if (old_x >= 0 && old_x < r_canvas.w &&
		      old_y >= 0 && old_y < r_canvas.h)
		    {
		      if (cur_tool == TOOL_STAMP)
			{
			  stamp_xor(old_x, old_y);

			  update_screen(old_x - (CUR_STAMP_W+1)/2 + r_canvas.x,
					old_y - (CUR_STAMP_H+1)/2 + r_canvas.y,
					old_x + (CUR_STAMP_W+1)/2 + r_canvas.x,
					old_y + (CUR_STAMP_H+1)/2 + r_canvas.y);
			}
		      else
			{
			  rect_xor(old_x - w / 2, old_y - h / 2,
				   old_x + w / 2, old_y + h / 2);

			  update_screen(old_x - w / 2 + r_canvas.x, old_y - h / 2 + r_canvas.y,
					old_x + w / 2 + r_canvas.x, old_y + h / 2 + r_canvas.y);
			}
		    }
		  
		  if (new_x >= 0 && new_x < r_canvas.w &&
		      new_y >= 0 && new_y < r_canvas.h)
		    {
		      if (cur_tool == TOOL_STAMP)
			{
			  stamp_xor(new_x, new_y);

			  update_screen(old_x - (CUR_STAMP_W+1)/2 + r_canvas.x,
					old_y - (CUR_STAMP_H+1)/2 + r_canvas.y,
					old_x + (CUR_STAMP_W+1)/2 + r_canvas.x,
					old_y + (CUR_STAMP_H+1)/2 + r_canvas.y);
			}
		      else
			{
			  rect_xor(new_x - w / 2, new_y - h / 2,
				   new_x + w / 2, new_y + h / 2);

			  update_screen(new_x - w / 2 + r_canvas.x, new_y - h / 2 + r_canvas.y,
					new_x + w / 2 + r_canvas.x, new_y + h / 2 + r_canvas.y);
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
		   cursor_y + TTF_FontHeight(getfonthandle(cur_font)));

          update_screen(cursor_x + r_canvas.x + cursor_textwidth,
                        cursor_y + r_canvas.y,
			cursor_x + r_canvas.x + cursor_textwidth,
			cursor_y + r_canvas.y + TTF_FontHeight(getfonthandle(cur_font)));
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
  Uint32 (*getpixel)(SDL_Surface *, int, int) = getpixels[in->format->BytesPerPixel];
  

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
  void (*putpixel)(SDL_Surface *, int, int, Uint32) = putpixels[out->format->BytesPerPixel];

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
  Uint32 (*getpixel)(SDL_Surface *, int, int) = getpixels[in->format->BytesPerPixel];
  void (*putpixel)(SDL_Surface *, int, int, Uint32) = putpixels[out->format->BytesPerPixel];

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
  Uint32 (*getpixel)(SDL_Surface *, int, int);
  void (*putpixel)(SDL_Surface *, int, int, Uint32);
  

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
  
  getpixel = getpixels[surf_ptr->format->BytesPerPixel];


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

  if (tmp_surf != NULL)
    putpixel = putpixels[tmp_surf->format->BytesPerPixel];
  else
    putpixel = NULL;


  /* Alter the stamp's color, if needed: */

  if (stamp_colorable(cur_stamp) && tmp_surf != NULL)
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

  final_surf = thumbnail(tmp_surf, CUR_STAMP_W, CUR_STAMP_H, 0);
  
  
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
     Grass (mower?) */


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
  double ran_r = rand()/(double)RAND_MAX;
  double ran_g = rand()/(double)RAND_MAX;
  double base_r = sRGB_to_linear_table[color_hexes[cur_color][0]]*1.5 + sRGB_to_linear_table[127]*5.0 + ran_r;
  double base_g = sRGB_to_linear_table[color_hexes[cur_color][1]]*1.5 + sRGB_to_linear_table[76] *5.0 + ran_g;
  double base_b = sRGB_to_linear_table[color_hexes[cur_color][2]]*1.5 + sRGB_to_linear_table[73] *5.0 + (ran_r+ran_g*2.0)/3.0;

  Uint8 r = linear_to_sRGB(base_r/7.5);
  Uint8 g = linear_to_sRGB(base_g/7.5);
  Uint8 b = linear_to_sRGB(base_b/7.5);


  SDL_FillRect(canvas, &dest, SDL_MapRGB(canvas->format, r, g, b));

// This is better for debugging brick layout:
//  SDL_FillRect(canvas, &dest, SDL_MapRGB(canvas->format, rand()&255, rand()&255, rand()&255));
  
  
  /* Note: We only play the brick sound when we actually DRAW a brick: */

  playsound(0, SND_BRICK, 1);
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
  Uint32 (*getpixel_canvas)(SDL_Surface *, int, int) = getpixels[canvas->format->BytesPerPixel];
  void (*putpixel)(SDL_Surface *, int, int, Uint32) = putpixels[canvas->format->BytesPerPixel];


  /* In case we need to use the current canvas (just saved to undo buf)... */

  if (cur_undo > 0)
    undo_ctr = cur_undo - 1;
  else
    undo_ctr = NUM_UNDO_BUFS - 1;

  last = undo_bufs[undo_ctr];
  Uint32 (*getpixel_last)(SDL_Surface *, int, int) = getpixels[last->format->BytesPerPixel];


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

	      SDL_GetRGB(getpixel_canvas(canvas, x+ix-16, y+iy-16), last->format, &r, &g, &b);
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
		      p_tmp = getpixel_last(last, xx+(i>>2), yy+(i&3));
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
	  int nominal_length;
	  int brick_x, brick_y;

	  int vertical_joint   = 2; // between a brick and the one above/below
	  int horizontal_joint = 2; // between a brick and the one to the side
	  int nominal_width    = 18;
	  int nominal_height   = 12; // 11 to 14, for joints of 2

#if 0
	  if (cur_magic == MAGIC_SMALLBRICK)
	    {
	      vertical_joint   = 1; // between a brick and the one above/below
	      horizontal_joint = 1; // between a brick and the one to the side
	      nominal_width    = 9;
	      nominal_height   = 6; // 11 to 14, for joints of 2
	    }
#endif
#if 0
	  if (cur_magic == MAGIC_LARGEBRICK)
	    {
	      vertical_joint   = 3; // between a brick and the one above/below
	      horizontal_joint = 3; // between a brick and the one to the side
	      nominal_width    = 27;
	      nominal_height   = 18; // 11 to 14, for joints of 2
	    }
#endif
#if 1
	  if (cur_magic == MAGIC_LARGEBRICK)
	    {
	      vertical_joint   = 4; // between a brick and the one above/below
	      horizontal_joint = 4; // between a brick and the one to the side
	      nominal_width    = 36;
	      nominal_height   = 24; // 11 to 14, for joints of 2
	    }
#endif

          nominal_length   = 2 * nominal_width;
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

          if ( (unsigned)x < (unsigned)canvas->w && (unsigned)y < (unsigned)canvas->h && !*mybrick)
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

	      SDL_GetRGB(getpixel_canvas(canvas, x+ix-16, y+iy-16), last->format, &r, &g, &b);
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
		  SDL_GetRGB(getpixel_last(last, xx, yy), last->format,
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
		
		  SDL_GetRGB(getpixel_last(last, xx, yy), last->format,
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
		
		  SDL_GetRGB(getpixel_last(last, xx, yy), last->format,
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
		
		  SDL_GetRGB(getpixel_last(last, xx, yy), last->format,
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
	
		  colr = getpixel_last(last, clamp(0, xx, canvas->w-1),
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

	      Uint32 (*getpixel_grass)(SDL_Surface *, int, int) = getpixels[img_grass->format->BytesPerPixel];
	      
	      for (yy = 0; yy < ah; yy++)
	        {
	          for (xx = 0; xx < 64; xx++)
		    {
		      SDL_GetRGBA(getpixel_grass(img_grass, xx+src.x, yy+src.y), img_grass->format,
			         &r, &g, &b, &a);

                      double rd = sRGB_to_linear_table[r]*8.0 + tmp_red;
                      rd = rd * (a/255.0) / 11.0;
                      double gd = sRGB_to_linear_table[g]*8.0 + tmp_green;
                      gd = gd * (a/255.0) / 11.0;
                      double bd = sRGB_to_linear_table[b]*8.0 + tmp_blue;
                      bd = bd * (a/255.0) / 11.0;

		      SDL_GetRGB(getpixel_canvas(canvas, xx+dest.x, yy+dest.y), canvas->format,
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
      update_screen_rect(&r_tools);
    }
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
	  "  %s [--nosysfonts | --sysfonts]\n"
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
	  blank, blank);

  free(blank);
}


// The original Tux Paint canvas was 448x376. The canvas can be
// other sizes now, but many old stamps are sized for the small
// canvas. So, with larger canvases, we must choose a good scale
// factor to compensate. As the canvas size grows, the user will
// want a balance of "more stamps on the screen" and "stamps not
// getting tiny". This will calculate the needed scale factor.
static unsigned compute_default_scale_factor(double ratio)
{
  double old_diag = sqrt(448*448+376*376);
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


// backdoor into qsort operations, so we don't have to do work again
static int was_bad_font;

// see if two font surfaces are the same
static int do_surfcmp(const SDL_Surface *const *const v1, const SDL_Surface *const *const v2)
{
  const SDL_Surface *const s1 = *v1;
  const SDL_Surface *const s2 = *v2;
  if(s1==s2)
    {
      printf("WTF?\n");
      return 0;
    }
  if(!s1 || !s2 || !s1->w || !s2->w || !s1->h || !s2->h || !s1->format || !s2->format)
    {
      was_bad_font = 1;
      return 0;
    }
  if(s1->format->BytesPerPixel != s2->format->BytesPerPixel)
    {
      // something really strange and bad happened
      was_bad_font = 1;
      return s1->format->BytesPerPixel - s2->format->BytesPerPixel;
    }

  if(s1->w != s2->w)
    return s1->w - s2->w;
  if(s1->h != s2->h)
    return s1->h - s2->h;

  const char *const c1 = (const char *const)s1->pixels;
  const char *const c2 = (const char *const)s2->pixels;
  int width = s1->format->BytesPerPixel * s1->w;
  if(width==s1->pitch)
    return memcmp(c1,c2,width*s1->h);
  int cmp = 0;
  int i = s1->h;
  while(i--)
    {
      cmp = memcmp(c1 + i*s1->pitch, c2 + i*s2->pitch, width);
      if(cmp)
        break;
    }
  return cmp;
}

// see if two font surfaces are the same
static int surfcmp(const void *s1, const void *s2)
{
  int diff = do_surfcmp(s1, s2);
  if(!diff)
    was_bad_font = 1;
  return diff;
}

// check if the characters will render distinctly
static int charset_works(TTF_Font *font, const char *s)
{
  SDL_Color black = {0, 0, 0, 0};
  SDL_Surface **surfs = malloc(strlen(s) * sizeof surfs[0]);
  unsigned count = 0;
  int ret = 0;
  while(*s)
    {
      char c[8];
      unsigned offset = 0;
      do
        c[offset++] = *s++;
        while((*s & 0xc0u) == 0x80u); // assume safe input
      c[offset++] = '\0';
      SDL_Surface *tmp_surf = TTF_RenderUTF8_Blended(font, c, black);
      if(!tmp_surf)
        {
#if 0
// THREADED_FONTS
          printf("could not render \"%s\" font\n", TTF_FontFaceFamilyName(font));
#endif
          goto out;
        }
      surfs[count++] = tmp_surf;
    }
  was_bad_font = 0;
  qsort(surfs, count, sizeof surfs[0], surfcmp);
  ret = !was_bad_font;
out:
  while(count--)
    SDL_FreeSurface(surfs[count]);
  free(surfs);
  return ret;
}


/////////////////////////////// directory tree walking /////////////////////

#define TP_FTW_UNKNOWN 1
#define TP_FTW_DIRECTORY 2
#define TP_FTW_NORMAL 0

#define TP_FTW_PATHSIZE 400

typedef struct tp_ftw_str {
  char *str;
  unsigned char len;
//  unsigned char is_rsrc;
} tp_ftw_str;

// For qsort()
static int compare_ftw_str(const void *v1, const void *v2)
{
  const char *restrict const s1 = ((tp_ftw_str*)v1)->str;
  const char *restrict const s2 = ((tp_ftw_str*)v2)->str;
  return -strcmp(s1, s2);
}

static void tp_ftw(char *restrict const dir, unsigned dirlen, int rsrc,
  void (*fn)(const char *restrict const dir, unsigned dirlen, tp_ftw_str *files, unsigned count)
  )
{
  dir[dirlen++] = '/';
  dir[dirlen] = '\0';
//printf("processing directory %s %d\n", dir, dirlen);
  /* Open the directory: */
  DIR *d = opendir(dir);
  if (!d)
    return;

  unsigned num_file_names = 0;
  unsigned max_file_names = 0;
  tp_ftw_str *file_names = NULL;
  unsigned num_dir_names = 0;
  unsigned max_dir_names = 0;
  tp_ftw_str *dir_names = NULL;

  for(;;)
    {
      struct dirent * f = readdir(d);
      if(!f)
        break;
      if(f->d_name[0]=='.')
        continue;
      int filetype = TP_FTW_UNKNOWN;
// Linux and BSD can often provide file type info w/o the stat() call
#ifdef DT_UNKNOWN
      switch(f->d_type)
        {
          default:
            continue;
          case DT_REG:
            if(!rsrc)  // if maybe opening resource files, need st_size
              filetype = TP_FTW_NORMAL;
            break;
          case DT_DIR:
            filetype = TP_FTW_DIRECTORY;
            break;
          case DT_UNKNOWN:
          case DT_LNK:
            ;
        }
#else
#warning Failed to see DT_UNKNOWN
#endif

#if defined(_DIRENT_HAVE_D_NAMLEN) || defined(__APPLE__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__FreeBSD__)
      int d_namlen = f->d_namlen;
#else
      int d_namlen = strlen(f->d_name);
#endif
      int add_rsrc = 0;

      if(filetype == TP_FTW_UNKNOWN)
        {
          struct stat sbuf;
          memcpy(dir+dirlen, f->d_name, d_namlen+1);
          if(stat(dir, &sbuf))
            continue;  // oh well... try the next one
          if(S_ISDIR(sbuf.st_mode))
            filetype = TP_FTW_DIRECTORY;
          else if(S_ISREG(sbuf.st_mode))
            {
              filetype = TP_FTW_NORMAL;
              if(rsrc && !sbuf.st_size)
                add_rsrc = 5;  // 5 is length of "/rsrc"
            }
          else
            continue;  // was a device file or somesuch
        }
      if(filetype==TP_FTW_NORMAL)
        {
          if(num_file_names==max_file_names)
            {
              max_file_names = max_file_names * 5 / 4 + 30;
              file_names = realloc(file_names, max_file_names * sizeof *file_names);
            }
          char *cp = malloc(d_namlen + add_rsrc + 1);
          memcpy(cp, f->d_name, d_namlen);
          if(add_rsrc)
            memcpy(cp+d_namlen, "/rsrc", 6);
          else
            cp[d_namlen] = '\0';
          file_names[num_file_names].str = cp;
          file_names[num_file_names].len = d_namlen;
          num_file_names++;
        }
      if(filetype==TP_FTW_DIRECTORY)
        {
          if(num_dir_names==max_dir_names)
            {
              max_dir_names = max_dir_names * 5 / 4 + 3;
              dir_names = realloc(dir_names, max_dir_names * sizeof *dir_names);
            }
          char *cp = malloc(d_namlen + 1);
          memcpy(cp, f->d_name, d_namlen + 1);
          dir_names[num_dir_names].str = cp;
          dir_names[num_dir_names].len = d_namlen;
          num_dir_names++;
        }
    }

  closedir(d);
  show_progress_bar();
  dir[dirlen] = '\0';   // repair it (clobbered for stat() call above)

  if(file_names)
    {
// let callee sort and keep the string
#if 0
      qsort(file_names, num_file_names, sizeof *file_names, compare_ftw_str);
      while(num_file_names--)
        {
          free(file_names[num_file_names].str);
        }
      free(file_names);
#else
      fn(dir, dirlen, file_names, num_file_names);
#endif
    }

  if(dir_names)
    {
      while(num_dir_names--)
        {
          memcpy(dir+dirlen, dir_names[num_dir_names].str, dir_names[num_dir_names].len+1);
          tp_ftw(dir, dirlen+dir_names[num_dir_names].len, rsrc, fn);
          free(dir_names[num_dir_names].str);
        }
      free(dir_names);
    }
}


///////////////// directory walking callers and callbacks //////////////////

static void loadfont_callback(const char *restrict const dir, unsigned dirlen, tp_ftw_str *files, unsigned i)
{
  while(i--)
    {
      show_progress_bar();
      int loadable = 0;
      const char *restrict const cp = strchr(files[i].str, '.');
      if(cp)
        {
          // need gcc 3.4 for the restrict in this location
          const char * /*restrict*/ const suffixes[] = {"ttc", "dfont", "pfa", "pfb", "otf", "ttf",};
          int j = sizeof suffixes / sizeof suffixes[0];
          while(j--)
            {
              // only check part, because of potential .gz or .bz2 suffix
              if(!strncasecmp(cp+1,suffixes[j],strlen(suffixes[j])))
                {
                  loadable = 1;
                  break;
                }
            }
        }
      if(!loadable)
        {
          if(strstr(files[i].str, "/rsrc"))
            loadable = 1;
        }
      // Loadable: TrueType (.ttf), OpenType (.otf), Type1 (.pfa and .pfb),
      // and various useless bitmap fonts. Compressed files (with .gz or .bz2)
      // should also work. A *.dfont is pretty much a Mac resource fork in a normal
      // file, and may load with some library versions.
      if (loadable)
        {
          char fname[512];
          snprintf(fname, sizeof fname, "%s/%s", dir, files[i].str);
//printf("Loading font: %s\n", fname);
          TTF_Font *font = TTF_OpenFont(fname, text_sizes[text_size]);
          if(font)
            {
              const char *restrict const family = TTF_FontFaceFamilyName(font);
              const char *restrict const style = TTF_FontFaceStyleName(font);
              int numfaces = TTF_FontFaces(font);
              if (numfaces != 1)
                printf("Found %d faces in %s, %s, %s\n", numfaces, files[i].str, family, style);
              if(charset_works(font, gettext("jq")) && charset_works(font, gettext("JQ")))
                {
                  if (num_font_styles==num_font_styles_max)
                    {
                      num_font_styles_max = num_font_styles_max * 5 / 4 + 30;
                      user_font_styles = realloc(user_font_styles, num_font_styles_max * sizeof *user_font_styles);
                    }
                  user_font_styles[num_font_styles] = malloc(sizeof *user_font_styles[num_font_styles]);
                  user_font_styles[num_font_styles]->directory = strdup(dir);
                  user_font_styles[num_font_styles]->filename = files[i].str; // steal it (mark NULL below)
                  user_font_styles[num_font_styles]->family = strdup(family);
                  user_font_styles[num_font_styles]->style = strdup(style);
                  user_font_styles[num_font_styles]->score  = charset_works(font, gettext("oO"));
                  user_font_styles[num_font_styles]->score += charset_works(font, gettext("`\%_@$~#{}<>^&*"));
                  user_font_styles[num_font_styles]->score += charset_works(font, gettext(",.?!"));
                  user_font_styles[num_font_styles]->score += charset_works(font, gettext("017"));
                  user_font_styles[num_font_styles]->score += charset_works(font, gettext("O0"));
                  user_font_styles[num_font_styles]->score += charset_works(font, gettext("1Il|"));
                  num_font_styles++;
//printf("Accepted: %s, %s, %s\n", files[i].str, family, style);
                  files[i].str = NULL;  // so free() won't crash -- we stole the memory
                }
              else
                {
#if 0
// THREADED_FONTS
                  printf("Font missing critical chars: %s, %s, %s\n", files[i].str, family, style);
#endif
                }
              TTF_CloseFont(font);
            }
          else
            {
#if 0
// THREADED_FONTS
              printf("could not open %s\n", files[i].str);
#endif
            }
        }
      free(files[i].str);
    }
  free(files);
}



static void loadfonts(const char * const dir)
{
  char buf[TP_FTW_PATHSIZE];
  unsigned dirlen = strlen(dir);
  memcpy(buf,dir,dirlen);
  tp_ftw(buf, dirlen, 1, loadfont_callback);
}  


static void loadbrush_callback(const char *restrict const dir, unsigned dirlen, tp_ftw_str *files, unsigned i)
{
  qsort(files, i, sizeof *files, compare_ftw_str);
  while(i--)
    {
      show_progress_bar();
      if (strstr(files[i].str, ".png"))
        {
          char fname[512];
          snprintf(fname, sizeof fname, "%s/%s", dir, files[i].str);
          if (num_brushes==num_brushes_max)
            {
              num_brushes_max = num_brushes_max * 5 / 4 + 4;
              img_brushes = realloc(img_brushes, num_brushes_max * sizeof *img_brushes);
            }
          img_brushes[num_brushes] = loadimage(fname);
          num_brushes++;
        }
      free(files[i].str);
    }
  free(files);
}



static void load_brush_dir(const char * const dir)
{
  char buf[TP_FTW_PATHSIZE];
  unsigned dirlen = strlen(dir);
  memcpy(buf,dir,dirlen);
  tp_ftw(buf, dirlen, 0, loadbrush_callback);
}


static unsigned default_stamp_size;

static void loadstamp_finisher(int i)
{
  state_stamps[i] = malloc(sizeof(state_type));

  if (img_stamps[i]->w > 40 || img_stamps[i]->h > 40)
    img_stamp_thumbs[i] = thumbnail(img_stamps[i], 40, 40, 1);
  else
    img_stamp_thumbs[i] = NULL;

  if (img_stamps_premirror[i])
    {
      /* Also thumbnail the pre-drawn mirror version, if any: */
      if (img_stamps_premirror[i]->w > 40 || img_stamps_premirror[i]->h > 40)
        img_stamp_thumbs_premirror[i] = thumbnail(img_stamps_premirror[i], 40, 40, 1);
      else
        img_stamp_thumbs_premirror[i] = NULL;
    }

  /* If Tux Paint is in mirror-image-by-default mode, mirror, if we can: */

  if (mirrorstamps && inf_stamps[i]->mirrorable)
    state_stamps[i]->mirrored = 1;
  else
    state_stamps[i]->mirrored = 0;

  state_stamps[i]->flipped = 0;

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


static void loadstamp_callback(const char *restrict const dir, unsigned dirlen, tp_ftw_str *files, unsigned i)
{
  qsort(files, i, sizeof *files, compare_ftw_str);
  while(i--)
    {
      show_progress_bar();

      if (strstr(files[i].str, ".png") && !strstr(files[i].str, "_mirror.png"))
        {
          char fname[512];
          snprintf(fname, sizeof fname, "%s/%s", dir, files[i].str);
          txt_stamps[num_stamps] = loaddesc(fname);
          inf_stamps[num_stamps] = loadinfo(fname);

          img_stamps[num_stamps] = NULL;
          if(!mirrorstamps || !disable_stamp_controls || !inf_stamps[num_stamps]->mirrorable)
            img_stamps[num_stamps] = loadimage(fname);

          img_stamps_premirror[num_stamps] = NULL;
          if((mirrorstamps || !disable_stamp_controls) && inf_stamps[num_stamps]->mirrorable)
            img_stamps_premirror[num_stamps] = loadaltimage(fname);

          if(img_stamps[num_stamps] || img_stamps_premirror[num_stamps])
            {
              // we have a stamp; finalize it
#ifndef NOSOUND
              if (use_sound)
                snd_stamps[num_stamps] = loadsound(fname);
#endif
              loadstamp_finisher(num_stamps);
              num_stamps++;       // FIXME: no limit and no resizing right now...
            }
          else
            {
              // we have a failure, abort mission
              free(txt_stamps[num_stamps]);
              free(inf_stamps[num_stamps]);
              free(img_stamps[num_stamps]);
              free(img_stamps_premirror[num_stamps]);
            }
        }
      free(files[i].str);
    }
  free(files);
}



static void load_stamp_dir(const char * const dir)
{
  char buf[TP_FTW_PATHSIZE];
  unsigned dirlen = strlen(dir);
  memcpy(buf,dir,dirlen);
  tp_ftw(buf, dirlen, 0, loadstamp_callback);
}


static void load_stamps(void)
{
  default_stamp_size = compute_default_scale_factor(1.0);

  char * homedirdir = get_fname("stamps");

  load_stamp_dir(homedirdir);
  load_stamp_dir(DATA_PREFIX "stamps");
#ifdef __APPLE__
  load_stamp_dir("/Library/Application Support/TuxPaint/stamps");
#endif

  if (num_stamps == 0)
    {
      fprintf(stderr,
              "\nWarning: No stamps found in " DATA_PREFIX "stamps/\n"
              "or %s\n\n", homedirdir);
    }

  free(homedirdir);
}



static int load_user_fonts(void *vp)
{
  (void)vp; // junk passed by threading library

  loadfonts(DATA_PREFIX "fonts");

  if (!no_system_fonts)
  {
#ifdef WIN32
    loadfonts("%SystemRoot%\\Fonts");
#elif defined(__BEOS__)
    loadfonts("/boot/home/config/font/ttffonts");
    loadfonts("/usr/share/fonts");
    loadfonts("/usr/X11R6/lib/X11/fonts");
#elif defined(__APPLE__)
    loadfonts("/System/Library/Fonts");
    loadfonts("/Library/Fonts");
    loadfonts(macosx.fontsPath);
    loadfonts("/usr/share/fonts");
    loadfonts("/usr/X11R6/lib/X11/fonts");
#elif defined(__sun__)
    loadfonts("/usr/openwin/lib/X11/fonts");
    loadfonts("/usr/share/fonts");
    loadfonts("/usr/X11R6/lib/X11/fonts");
#else
    loadfonts("/usr/share/feh/fonts");
    loadfonts("/usr/share/fonts");
    loadfonts("/usr/X11R6/lib/X11/fonts");
    loadfonts("/usr/share/texmf/fonts");
    loadfonts("/usr/share/grace/fonts/type1");
    loadfonts("/usr/share/hatman/fonts");
    loadfonts("/usr/share/icewm/themes/jim-mac");
    loadfonts("/usr/share/vlc/skins2/fonts");
    loadfonts("/usr/share/xplanet/fonts");
#endif
  }

  char *homedirdir = get_fname("fonts");
  loadfonts(homedirdir);
  free(homedirdir);

  groupfonts();

  font_thread_done = 1;
  // FIXME: need a memory barrier here
  return 0; // useless, wanted by threading library
}


#ifdef FORKED_FONTS

#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <sys/wait.h>

#ifdef _POSIX_PRIORITY_SCHEDULING
#include <sched.h>
#else
#define sched_yield()
#endif

#ifdef __linux__
#include <sys/prctl.h>
#else
#define prctl(o,a1)
#define PR_SET_PDEATHSIG 0
#endif

static void reliable_write(int fd, const void *buf, size_t count)
{
  do
    {
      ssize_t rc = write(fd,buf,count);
      if(rc==-1)
        {
          switch(errno)
            {
              default:
                return;
              case EAGAIN:
              case ENOSPC:
                ; // satisfy a C syntax abomination
                struct pollfd p = (struct pollfd){fd, POLLOUT, 0};
                poll(&p, 1, -1);  // try not to burn CPU time
                // FALL THROUGH
              case EINTR:
                continue;
            }
        }
      buf += rc;
      count -= rc;
    } while(count);
}


static void reliable_read(int fd, void *buf, size_t count)
{
  do
    {
      ssize_t rc = read(fd,buf,count);
      if(rc==-1)
        {
          switch(errno)
            {
              default:
                return;
              case EAGAIN:
                ; // satisfy a C syntax abomination
                struct pollfd p = (struct pollfd){fd, POLLIN, 0};
                poll(&p, 1, -1);  // try not to burn CPU time
                // FALL THROUGH
              case EINTR:
                continue;
            }
        }
      if(rc==0)
        break;  // EOF. Better not happen before the end!
      buf += rc;
      count -= rc;
    } while(count);
}


static void run_font_scanner(void)
{
  int sv[2];
  if(socketpair(AF_UNIX, SOCK_STREAM, 0, sv))
    exit(42);
  font_scanner_pid = fork();
  if(font_scanner_pid)
    {
      // parent (or error -- but we're screwed in that case)
      font_socket_fd = sv[0];
      close(sv[1]);
      return;
    }
  nice(42);       // be nice, letting the main thread get the CPU
  sched_yield();     // try to let the parent run right now
  prctl(PR_SET_PDEATHSIG, 9);    // get killed if parent exits
  font_socket_fd = sv[1];
  close(sv[0]);
  progress_bar_disabled = 1;
  reliable_read(font_socket_fd, &no_system_fonts, sizeof no_system_fonts);
  sched_yield();     // try to let the parent run right now
  SDL_Init(SDL_INIT_NOPARACHUTE);
  TTF_Init();
  load_user_fonts(NULL);
  int size = 0;
  int i = num_font_families;
  while(i--)
    {
      char *s;
      s = user_font_families[i]->directory;
      if(s) size += strlen(s);
      s = user_font_families[i]->family;
      if(s) size += strlen(s);
      s = user_font_families[i]->filename[0];
      if(s) size += strlen(s);
      s = user_font_families[i]->filename[1];
      if(s) size += strlen(s);
      s = user_font_families[i]->filename[2];
      if(s) size += strlen(s);
      s = user_font_families[i]->filename[3];
      if(s) size += strlen(s);
      size += 6;  // for '\0' on each of the above
    }
  size += 2;  // for 2-byte font count
  char *buf = malloc(size);
  char *walk = buf;
  printf("Sending %u bytes with %u families.\n", size, num_font_families);
  *walk++ = num_font_families & 0xffu;
  *walk++ = num_font_families >> 8u;
  i = num_font_families;
  while(i--)
    {
      int len;
      char *s;

      s = user_font_families[i]->directory;
      if(s)
        {
          len = strlen(s);
          memcpy(walk, s, len);
          walk += len;
        }
      *walk++ = '\0';
      
      s = user_font_families[i]->family;
      if(s)
        {
          len = strlen(s);
          memcpy(walk, s, len);
          walk += len;
        }
      *walk++ = '\0';
      
      s = user_font_families[i]->filename[0];
      if(s)
        {
          len = strlen(s);
          memcpy(walk, s, len);
          walk += len;
        }
      *walk++ = '\0';
      
      s = user_font_families[i]->filename[1];
      if(s)
        {
          len = strlen(s);
          memcpy(walk, s, len);
          walk += len;
        }
      *walk++ = '\0';
      
      s = user_font_families[i]->filename[2];
      if(s)
        {
          len = strlen(s);
          memcpy(walk, s, len);
          walk += len;
        }
      *walk++ = '\0';
      
      s = user_font_families[i]->filename[3];
      if(s)
        {
          len = strlen(s);
          memcpy(walk, s, len);
          walk += len;
        }
      *walk++ = '\0';
    }
  reliable_write(font_socket_fd, buf, size);
  exit(0);
}

static void receive_some_font_info(void)
{
  char *buf = NULL;
  unsigned buf_size = 0;
  unsigned buf_fill = 0;
  fcntl(font_socket_fd, F_SETFL, O_NONBLOCK);
  for(;;)
    {
      if(buf_size <= buf_fill*9/8+128)
        {
          buf_size = buf_size*5/4+256;
          buf = realloc(buf, buf_size);
        }
      ssize_t rc = read(font_socket_fd, buf+buf_fill, buf_size-buf_fill);
//printf("read: fd=%d buf_fill=%u buf_size=%u rc=%ld\n", font_socket_fd, buf_fill, buf_size, rc);
      if(rc==-1)
        {
          switch(errno)
            {
              default:
                return;
              case EAGAIN:
                ; // satisfy a C syntax abomination
                struct pollfd p = (struct pollfd){font_socket_fd, POLLIN, 0};
                show_progress_bar();
                poll(&p, 1, 29);  // try not to burn CPU time
                continue;
              case EINTR:
                continue;
            }
        }
      buf_fill += rc;
      if(!rc)
        break;
    }
  close(font_socket_fd);
  show_progress_bar();
  unsigned char *walk = buf;
  num_font_families = *walk++;
  num_font_families += *walk++ << 8u;
  printf("Got %u bytes with %u families.\n", buf_fill, num_font_families);
  user_font_families = malloc(num_font_families * sizeof *user_font_families);
  family_info *fip = malloc(num_font_families * sizeof **user_font_families);
  unsigned i = num_font_families;
  while(i--)
    {
      user_font_families[i] = fip+i;
      unsigned len;

      len = strlen(walk);
      user_font_families[i]->directory = len ? walk : NULL;
      walk += len + 1;

      len = strlen(walk);
      user_font_families[i]->family = len ? walk : NULL;
      walk += len + 1;

      len = strlen(walk);
      user_font_families[i]->filename[0] = len ? walk : NULL;
      walk += len + 1;

      len = strlen(walk);
      user_font_families[i]->filename[1] = len ? walk : NULL;
      walk += len + 1;

      len = strlen(walk);
      user_font_families[i]->filename[2] = len ? walk : NULL;
      walk += len + 1;

      len = strlen(walk);
      user_font_families[i]->filename[3] = len ? walk : NULL;
      walk += len + 1;

      user_font_families[i]->handle = NULL;
      
      // score left uninitialized
    }
  waitpid(font_scanner_pid,NULL,0);
  font_thread_done = 1;
}
#endif

////////////////////////////////////////////////////////////////////////////////
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
  no_button_distinction = 0;
  grab_input = 0;
  no_fancy_cursors = 0;
  simple_shapes = 0;
  only_uppercase = 0;
  promptless_save = SAVE_OVER_PROMPT;
  disable_quit = 0;
  disable_save = 0;
  disable_print = 0;
  dont_load_stamps = 0;
  no_system_fonts = 0;
  print_delay = 0;
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
      else if (strcmp(argv[i], "--nosysfonts") == 0)
	{
	  no_system_fonts = 1;
	}
      else if (strcmp(argv[i], "--nobuttondistinction") == 0)
	{
	  no_button_distinction = 1;
	}
      else if (strcmp(argv[i], "--sysfonts") == 0)
	{
	  no_system_fonts = 0;
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
	      do_locale_option(argv[++i]);
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
	  set_langstr(argv[i] + 7);
	}
      else if (strcmp(argv[i], "--lang") == 0 || strcmp(argv[i], "-l") == 0)
	{
	  if (i < argc - 1)
	    {
	      set_langstr(argv[i + 1]);
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


  setup_language(getfilename(argv[0]));


#ifndef WIN32
  putenv((char *) "SDL_VIDEO_X11_WMCLASS=TuxPaint.TuxPaint");
#endif
  
  /* Test for lockfile, if we're using one: */

  if (ok_to_use_lockfile)
  {
    char * lock_fname;
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


  Uint32 init_flags = SDL_INIT_VIDEO|SDL_INIT_TIMER;
  if(use_sound)
    init_flags |= SDL_INIT_AUDIO;
  if(!fullscreen)
    init_flags |= SDL_INIT_NOPARACHUTE;  // allow debugger to catch crash

  /* Init SDL */
  if (SDL_Init(init_flags) < 0)
    {
#ifndef NOSOUND
      use_sound = 0;
      init_flags &= ~SDL_INIT_AUDIO;
      char *olderr = strdup(SDL_GetError());
      if (SDL_Init(init_flags) >= 0)
        {
          // worked, w/o sound
	  fprintf(stderr,
		  "\nWarning: I could not initialize audio!\n"
		  "The Simple DirectMedia Layer error that occurred was:\n"
		  "%s\n\n", olderr);
          free(olderr);
        }
      else
#endif
        {
          fprintf(stderr,
	          "\nError: I could not initialize video and/or the timer!\n"
	          "The Simple DirectMedia Layer error that occurred was:\n"
	          "%s\n\n", SDL_GetError());
          exit(1);
        }
    }

#ifndef NOSOUND
  // need Mix_OpenAudio(44100, AUDIO_S16, 2, 2048) for WIN32 ?
  if (use_sound && Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 1024) < 0)
    {
      fprintf(stderr,
	"\nWarning: I could not set up audio for 44100 Hz "
	"16-bit stereo.\n"
	"The Simple DirectMedia Layer error that occurred was:\n"
	"%s\n\n", SDL_GetError());
      use_sound = 0;
    }

  i = NUM_SOUNDS;
  while(use_sound && i--)
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
#endif


  /* Set-up Key-Repeat: */
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,
		      SDL_DEFAULT_REPEAT_INTERVAL);
 
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



  setup_screen_layout();

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


  ////////// quickly: title image, version, progress bar, and watch cursor

  img_title = loadimage(DATA_PREFIX "images/title.png");
  img_progress = loadimage(DATA_PREFIX "images/ui/progress.png");

  SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 255, 255, 255));

  dest.x = (WINDOW_WIDTH - img_title->w) / 2;
  dest.y = (WINDOW_HEIGHT - img_title->h);

  SDL_BlitSurface(img_title, NULL, screen, &dest);

  prog_bar_ctr = 0;
  show_progress_bar();

  SDL_Flip(screen);

  medium_font = TTF_OpenFont(DATA_PREFIX "fonts/default_font.ttf",
		      18 - (only_uppercase * 3));

  if (medium_font == NULL)
    {
      fprintf(stderr,
	      "\nError: Can't load font file: "
	      DATA_PREFIX "fonts/default_font.ttf\n"
	      "The Simple DirectMedia Layer error that occurred was:\n"
	      "%s\n\n", SDL_GetError());

      cleanup();
      exit(1);
    }

  char tmp_str[128];
  snprintf(tmp_str, sizeof(tmp_str), "%s – %s", VER_VERSION, VER_DATE);
  tmp_surf = render_text(medium_font, tmp_str, black);
  dest.x = 20 + (WINDOW_WIDTH - img_title->w) / 2;
  dest.y = WINDOW_HEIGHT - 60;
  SDL_BlitSurface(tmp_surf, NULL, screen, &dest);
  SDL_FreeSurface(tmp_surf);
  SDL_Flip(screen);


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

  // this one first, because we need it yesterday
  cursor_watch = get_cursor(watch_bits, watch_mask_bits,
			    watch_width, watch_height,
			    14 / scale, 14 / scale);

  do_setcursor(cursor_watch);
  show_progress_bar();

#ifdef FORKED_FONTS
  reliable_write(font_socket_fd, &no_system_fonts, sizeof no_system_fonts);
#else
  font_thread = SDL_CreateThread(load_user_fonts, NULL);
#endif

  // continuing on with the rest of the cursors...

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

  img_black = SDL_CreateRGBSurface(SDL_SRCALPHA|SDL_SWSURFACE,
					       img_btn_off->w, img_btn_off->h,
					       img_btn_off->format->BitsPerPixel,
					       img_btn_off->format->Rmask,
					       img_btn_off->format->Gmask,
					       img_btn_off->format->Bmask,
					       img_btn_off->format->Amask);
  SDL_FillRect(img_black, NULL, SDL_MapRGBA(screen->format, 0, 0, 0, 255));

  img_grey = SDL_CreateRGBSurface(SDL_SRCALPHA|SDL_SWSURFACE,
					       img_btn_off->w, img_btn_off->h,
					       img_btn_off->format->BitsPerPixel,
					       img_btn_off->format->Rmask,
					       img_btn_off->format->Gmask,
					       img_btn_off->format->Bmask,
					       img_btn_off->format->Amask);
  SDL_FillRect(img_grey, NULL, SDL_MapRGBA(screen->format, 0x88, 0x88, 0x88, 255));

  show_progress_bar();

  img_yes = loadimage(DATA_PREFIX "images/ui/yes.png");
  img_no = loadimage(DATA_PREFIX "images/ui/no.png");

  img_open = loadimage(DATA_PREFIX "images/ui/open.png");
  img_erase = loadimage(DATA_PREFIX "images/ui/erase.png");
  img_back = loadimage(DATA_PREFIX "images/ui/back.png");
  img_trash = loadimage(DATA_PREFIX "images/ui/trash.png");
  
  img_popup_arrow = loadimage(DATA_PREFIX "images/ui/popup_arrow.png");
  
  img_printer = loadimage(DATA_PREFIX "images/ui/printer.png");
  img_printer_wait = loadimage(DATA_PREFIX "images/ui/printer_wait.png");

  img_save_over = loadimage(DATA_PREFIX "images/ui/save_over.png");

  img_grow = loadimage(DATA_PREFIX "images/ui/grow.png");
  img_shrink = loadimage(DATA_PREFIX "images/ui/shrink.png");
  
  img_bold = loadimage(DATA_PREFIX "images/ui/bold.png");
  img_italic = loadimage(DATA_PREFIX "images/ui/italic.png");

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

#ifdef LOW_QUALITY_COLOR_SELECTOR
  img_paintcan = loadimage(DATA_PREFIX "images/ui/paintcan.png");
#endif

  show_progress_bar();

  img_sparkles = loadimage(DATA_PREFIX "images/ui/sparkles.png");
  img_grass    = loadimage(DATA_PREFIX "images/ui/grass.png");


  /* Load brushes: */
  load_brush_dir(DATA_PREFIX "brushes");
  homedirdir = get_fname("brushes");
  load_brush_dir(homedirdir);

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


  locale_font = load_locale_font(medium_font,18);

#if 0
  // put elsewhere for THREADED_FONTS
  /* Load user fonts, for the text tool */
  load_user_fonts();
#endif

  if (!dont_load_stamps)
    load_stamps();


  /* Load magic icons: */
  for (i = 0; i < NUM_MAGICS; i++)
    img_magics[i] = loadimage(magic_img_fnames[i]);

  show_progress_bar();

  /* Load shape icons: */
  for (i = 0; i < NUM_SHAPES; i++)
    img_shapes[i] = loadimage(shape_img_fnames[i]);

  show_progress_bar();

  /* Load tip tux images: */
  for (i = 0; i < NUM_TIP_TUX; i++)
    img_tux[i] = loadimage(tux_img_fnames[i]);

  show_progress_bar();

  img_mouse = loadimage(DATA_PREFIX "images/ui/mouse.png");
  img_mouse_click = loadimage(DATA_PREFIX "images/ui/mouse_click.png");

  show_progress_bar();
  
  /* Create toolbox and selector labels: */

  for (i = 0; i < NUM_TITLES; i++)
    {
      if (strlen(title_names[i]) > 0)
	{
	  TTF_Font * myfont = large_font;
	  if (need_own_font && strcmp(gettext(title_names[i]), title_names[i]))
	    myfont = locale_font;
          char *td_str = textdir(gettext(title_names[i]));
          upstr = uppercase(td_str);
          free(td_str);
          tmp_surf = render_text(myfont, upstr, black);
          free(upstr);
	  img_title_names[i] = thumbnail(tmp_surf, min(84, tmp_surf->w), tmp_surf->h, 0);
	  SDL_FreeSurface(tmp_surf);
	}
      else
	{
	  img_title_names[i] = NULL;
	}
    }



  /* Generate color selection buttons: */

#ifndef LOW_QUALITY_COLOR_SELECTOR

  /* Create appropriately-shaped buttons: */
  SDL_Surface *img1 = loadimage(DATA_PREFIX "images/ui/paintwell.png");
  SDL_Surface *img2 = thumbnail(img1,        color_button_w, color_button_h, 0);
  tmp_btn_up        = thumbnail(img_btn_up,  color_button_w, color_button_h, 0);
  tmp_btn_down      = thumbnail(img_btn_down,color_button_w, color_button_h, 0);
  img_color_btn_off = thumbnail(img_btn_off, color_button_w, color_button_h, 0);
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

  Uint32 (*getpixel_tmp_btn_up)(SDL_Surface *, int, int) = getpixels[tmp_btn_up->format->BytesPerPixel];
  Uint32 (*getpixel_tmp_btn_down)(SDL_Surface *, int, int) = getpixels[tmp_btn_down->format->BytesPerPixel];
  Uint32 (*getpixel_img2)(SDL_Surface *, int, int) = getpixels[img2->format->BytesPerPixel];
  

  for (y = 0; y < tmp_btn_up->h /* 48 */; y++)
    {
      for (x = 0; x < tmp_btn_up->w /* (WINDOW_WIDTH - 96) / NUM_COLORS */; x++)
	{
	  SDL_GetRGB(getpixel_tmp_btn_up(tmp_btn_up, x, y), tmp_btn_up->format, &r, &g, &b);
	  double ru = sRGB_to_linear_table[r];
	  double gu = sRGB_to_linear_table[g];
	  double bu = sRGB_to_linear_table[b];
	  SDL_GetRGB(getpixel_tmp_btn_down(tmp_btn_down, x, y), tmp_btn_down->format, &r, &g, &b);
	  double rd = sRGB_to_linear_table[r];
	  double gd = sRGB_to_linear_table[g];
	  double bd = sRGB_to_linear_table[b];
	  Uint8 a;
	  SDL_GetRGBA(getpixel_img2(img2, x, y), img2->format, &r, &g, &b, &a);
	  double aa = a/255.0;

	  for (i = 0; i < NUM_COLORS; i++)
	    {
	      double rh = sRGB_to_linear_table[color_hexes[i][0]];
	      double gh = sRGB_to_linear_table[color_hexes[i][1]];
	      double bh = sRGB_to_linear_table[color_hexes[i][2]];
	      putpixels[img_color_btns[i]->format->BytesPerPixel]
		       (img_color_btns[i], x, y,
		       SDL_MapRGB(img_color_btns[i]->format,
				  linear_to_sRGB(rh*aa + ru*(1.0-aa)),
				  linear_to_sRGB(gh*aa + gu*(1.0-aa)),
				  linear_to_sRGB(bh*aa + bu*(1.0-aa))));
	      putpixels[img_color_btns[i]->format->BytesPerPixel]
	               (img_color_btns[i+NUM_COLORS], x, y,
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
  SDL_Surface * tmp_surf, * surf;
  SDL_Color black = {0, 0, 0, 0};
  TTF_Font * myfont = small_font;

  if (need_own_font && strcmp(gettext(label), label))
    myfont = locale_font;
  char *td_str = textdir(gettext(label));
  char *upstr = uppercase(td_str);
  free(td_str);
  tmp_surf = render_text(myfont, upstr, black);
  free(upstr);
  surf = thumbnail(tmp_surf, min(48, tmp_surf->w), tmp_surf->h, 0);
  SDL_FreeSurface(tmp_surf);

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

  // buttons for the file open dialog
  img_openlabels_open = do_render_button_label(gettext_noop("Open"));
  img_openlabels_erase = do_render_button_label(gettext_noop("Erase"));
  img_openlabels_back = do_render_button_label(gettext_noop("Back"));
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


  draw_image_title(TITLE_TOOLS, r_ttools);

  for (i = 0; i < NUM_TOOLS + TOOLOFFSET; i++)
    {
      dest.x = ((i % 2) * 48);
      dest.y = ((i / 2) * 48) + 40;


      if (i < NUM_TOOLS)
	{
	  SDL_Surface *button_color;
	  SDL_Surface *button_body;
	  if (i == cur_tool)
	    {
	      button_body  = img_btn_down;
	      button_color = img_black;
	    }
	  else if (tool_avail[i])
	    {
	      button_body  = img_btn_up;
	      button_color = img_black;
	    }
	  else
	    {
	      button_body  = img_btn_off;
	      button_color = img_grey;
	    }
          SDL_BlitSurface(button_body,  NULL, screen, &dest);
          SDL_BlitSurface(button_color, NULL, img_tools[i], NULL);
          SDL_BlitSurface(button_color, NULL, img_tool_names[i], NULL);

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

  // restore these to black (stamp and text controls borrow them)
  SDL_BlitSurface(img_black, NULL, img_magics[MAGIC_FLIP], NULL);
  SDL_BlitSurface(img_black, NULL, img_magics[MAGIC_MIRROR], NULL);

  /* FIXME: Should we worry about more than 14 magic effects? :^/ */


  draw_image_title(TITLE_MAGIC, r_ttoolopt);

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

static unsigned colors_state = COLORSEL_ENABLE | COLORSEL_CLOBBER;

static unsigned draw_colors(unsigned action)
{
  unsigned i;
  SDL_Rect dest;
  static unsigned old_color;

  unsigned old_colors_state = colors_state;
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
    return old_colors_state;
  if (cur_color==old_color && colors_state==old_colors_state)
    return old_colors_state;
  old_color = cur_color;

  for (i = 0; i < NUM_COLORS; i++)
    {
      dest.x = r_colors.x + i%gd_colors.cols*color_button_w;
      dest.y = r_colors.y + i/gd_colors.cols*color_button_h;
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
      dest.w = color_button_w;
      dest.h = color_button_h;

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
	  dest.y += 4;
	  SDL_BlitSurface(img_paintcan, NULL, screen, &dest);
	}
#endif

    }
  update_screen_rect(&r_colors);

  // if only the color changed, no need to draw the title
  if (colors_state==old_colors_state)
    return old_colors_state;

  if (colors_state == COLORSEL_ENABLE)
    {
      SDL_BlitSurface(img_title_large_on, NULL, screen, &r_tcolors);
    
      dest.x = r_tcolors.x + (r_tcolors.w - img_title_names[TITLE_COLORS]->w) / 2;
      dest.y = r_tcolors.y + (r_tcolors.h - img_title_names[TITLE_COLORS]->h) / 2;
      SDL_BlitSurface(img_title_names[TITLE_COLORS], NULL, screen, &dest);
    }
  else
    {
      SDL_BlitSurface(img_title_large_off, NULL, screen, &r_tcolors);
    }

  update_screen_rect(&r_tcolors);
  return old_colors_state;
}


/* Draw brushes: */

static void draw_brushes(void)
{
  int i, off_y, max, brush;
  SDL_Rect dest;


  /* Draw the title: */
  draw_image_title(TITLE_BRUSHES, r_ttoolopt);


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
  draw_image_title(TITLE_LETTERS, r_ttoolopt);


  /* How many can we show? */

  int most = 10;
  if (disable_stamp_controls)
    most = 14;

  /* Do we need scrollbars? */

  if (num_font_families > most + TOOLOFFSET)
    {
      off_y = 24;
      max = most-2 + TOOLOFFSET;

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

      if (!disable_stamp_controls)
	dest.y = dest.y - (48 * 2);

      if (font_scroll < num_font_families - (most-2) - TOOLOFFSET)
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
      else if (font < num_font_families)
	{
	  SDL_BlitSurface(img_btn_up, NULL, screen, &dest);
	}
      else
	{
	  SDL_BlitSurface(img_btn_off, NULL, screen, &dest);
	}

    
      if (font < num_font_families)
	{
	  SDL_Surface * tmp_surf_1;
	  
	  tmp_surf_1 = render_text(getfonthandle(font),
                                            gettext("Aa"), black);

	  if (tmp_surf_1->w > 48 || tmp_surf_1->h > 48)
	  {
	    tmp_surf = thumbnail(tmp_surf_1, 48, 48, 1);
	    SDL_FreeSurface(tmp_surf_1);
	  }
	  else
	    tmp_surf = tmp_surf_1;

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


  /* Draw text controls: */

  if (!disable_stamp_controls)
    {
      SDL_Surface *button_color;
      SDL_Surface *button_body;

      /* Show bold button: */

      dest.x = WINDOW_WIDTH - 96;
      dest.y = 40 + ((5 + TOOLOFFSET / 2) * 48);

      if (text_state & TTF_STYLE_BOLD)
        SDL_BlitSurface(img_btn_down, NULL, screen, &dest);
      else
        SDL_BlitSurface(img_btn_up, NULL, screen, &dest);
    
      dest.x = WINDOW_WIDTH - 96 + (48 - img_bold->w) / 2;
      dest.y = (40 + ((5 + TOOLOFFSET / 2) * 48) +
		(48 - img_bold->h) / 2);

      SDL_BlitSurface(img_bold, NULL, screen, &dest);

    
      /* Show italic button: */

      dest.x = WINDOW_WIDTH - 48;
      dest.y = 40 + ((5 + TOOLOFFSET / 2) * 48);

      if (text_state & TTF_STYLE_ITALIC)
        SDL_BlitSurface(img_btn_down, NULL, screen, &dest);
      else
        SDL_BlitSurface(img_btn_up, NULL, screen, &dest);

      dest.x = WINDOW_WIDTH - 48 + (48 - img_italic->w) / 2;
      dest.y = (40 + ((5 + TOOLOFFSET / 2) * 48) +
		(48 - img_italic->h) / 2);
    
      SDL_BlitSurface(img_italic, NULL, screen, &dest);

    
      /* Show shrink button: */

      dest.x = WINDOW_WIDTH - 96;
      dest.y = 40 + ((6 + TOOLOFFSET / 2) * 48);

      if (text_size > MIN_TEXT_SIZE)
        {
          button_color = img_black;
          button_body  = img_btn_up;
	}
      else
        {
          button_color = img_grey;
          button_body  = img_btn_off;
        }
      SDL_BlitSurface(button_body, NULL, screen, &dest);
    
      dest.x = WINDOW_WIDTH - 96 + (48 - img_shrink->w) / 2;
      dest.y = (40 + ((6 + TOOLOFFSET / 2) * 48) +
		(48 - img_shrink->h) / 2);

      SDL_BlitSurface(button_color, NULL, img_shrink, NULL);
      SDL_BlitSurface(img_shrink, NULL, screen, &dest);


      /* Show grow button: */

      dest.x = WINDOW_WIDTH - 48;
      dest.y = 40 + ((6 + TOOLOFFSET / 2) * 48);

      if (text_size < MAX_TEXT_SIZE)
        {
          button_color = img_black;
          button_body  = img_btn_up;
	}
      else
        {
          button_color = img_grey;
          button_body  = img_btn_off;
        }
      SDL_BlitSurface(button_body, NULL, screen, &dest);

      dest.x = WINDOW_WIDTH - 48 + (48 - img_grow->w) / 2;
      dest.y = (40 + ((6 + TOOLOFFSET / 2) * 48) +
		(48 - img_grow->h) / 2);
    
      SDL_BlitSurface(button_color, NULL, img_grow, NULL);
      SDL_BlitSurface(img_grow, NULL, screen, &dest);
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
  draw_image_title(TITLE_STAMPS, r_ttoolopt);


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
      SDL_Surface *button_color;
      SDL_Surface *button_body;

      /* Show mirror button: */

      dest.x = WINDOW_WIDTH - 96;
      dest.y = 40 + ((5 + TOOLOFFSET / 2) * 48);

      if (inf_stamps[cur_stamp]->mirrorable)
	{
	  if (state_stamps[cur_stamp]->mirrored)
	    {
	      button_color = img_black;
	      button_body  = img_btn_down;
	    }
	  else
	    {
	      button_color = img_black;
	      button_body  = img_btn_up;
	    }
	}
      else
	{
	  button_color = img_grey;
	  button_body  = img_btn_off;
	}
      SDL_BlitSurface(button_body, NULL, screen, &dest);
    
      dest.x = WINDOW_WIDTH - 96 + (48 - img_magics[MAGIC_MIRROR]->w) / 2;
      dest.y = (40 + ((5 + TOOLOFFSET / 2) * 48) +
		(48 - img_magics[MAGIC_MIRROR]->h) / 2);

      SDL_BlitSurface(button_color, NULL, img_magics[MAGIC_MIRROR], NULL);
      SDL_BlitSurface(img_magics[MAGIC_MIRROR], NULL, screen, &dest);

    
      /* Show flip button: */

      dest.x = WINDOW_WIDTH - 48;
      dest.y = 40 + ((5 + TOOLOFFSET / 2) * 48);

      if (inf_stamps[cur_stamp]->flipable)
	{
	  if (state_stamps[cur_stamp]->flipped)
	    {
	      button_color = img_black;
	      button_body  = img_btn_down;
	    }
	  else
	    {
	      button_color = img_black;
	      button_body  = img_btn_up;
	    }
	}
      else
	{
	  button_color = img_grey;
	  button_body  = img_btn_off;
	}
      SDL_BlitSurface(button_body, NULL, screen, &dest);

      dest.x = WINDOW_WIDTH - 48 + (48 - img_magics[MAGIC_FLIP]->w) / 2;
      dest.y = (40 + ((5 + TOOLOFFSET / 2) * 48) +
		(48 - img_magics[MAGIC_FLIP]->h) / 2);

      SDL_BlitSurface(button_color, NULL, img_magics[MAGIC_FLIP], NULL);
      SDL_BlitSurface(img_magics[MAGIC_FLIP], NULL, screen, &dest);

    
      /* Show shrink button: */

      dest.x = WINDOW_WIDTH - 96;
      dest.y = 40 + ((6 + TOOLOFFSET / 2) * 48);

      if (state_stamps[cur_stamp]->size > MIN_STAMP_SIZE)
        {
          button_color = img_black;
          button_body  = img_btn_up;
	}
      else
        {
          button_color = img_grey;
          button_body  = img_btn_off;
        }
      SDL_BlitSurface(button_body, NULL, screen, &dest);
    
      dest.x = WINDOW_WIDTH - 96 + (48 - img_shrink->w) / 2;
      dest.y = (40 + ((6 + TOOLOFFSET / 2) * 48) +
		(48 - img_shrink->h) / 2);

      SDL_BlitSurface(button_color, NULL, img_shrink, NULL);
      SDL_BlitSurface(img_shrink, NULL, screen, &dest);

    
      /* Show grow button: */

      dest.x = WINDOW_WIDTH - 48;
      dest.y = 40 + ((6 + TOOLOFFSET / 2) * 48);

      if (state_stamps[cur_stamp]->size < MAX_STAMP_SIZE)
        {
          button_color = img_black;
          button_body  = img_btn_up;
	}
      else
        {
          button_color = img_grey;
          button_body  = img_btn_off;
        }
      SDL_BlitSurface(button_body, NULL, screen, &dest);
    
      dest.x = WINDOW_WIDTH - 48 + (48 - img_grow->w) / 2;
      dest.y = (40 + ((6 + TOOLOFFSET / 2) * 48) +
		(48 - img_grow->h) / 2);

      SDL_BlitSurface(button_color, NULL, img_grow, NULL);
      SDL_BlitSurface(img_grow, NULL, screen, &dest);

    }
}


/* Draw the shape selector: */

static void draw_shapes(void)
{
  int i;
  SDL_Rect dest;


  draw_image_title(TITLE_SHAPES, r_ttoolopt);

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


  draw_image_title(TITLE_ERASERS, r_ttoolopt);

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
  Uint32 (*getpixel)(SDL_Surface *, int, int) = getpixels[src->format->BytesPerPixel];


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

  void (*putpixel)(SDL_Surface *, int, int, Uint32) = putpixels[s->format->BytesPerPixel];

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
static Uint32 getpixel8(SDL_Surface * surface, int x, int y)
{
  Uint8 * p;

  /* get the X/Y values within the bounds of this surface */
  if (unlikely( (unsigned)x > (unsigned)surface->w - 1u ))
    x = (x<0) ? 0 : surface->w - 1;
  if (unlikely( (unsigned)y > (unsigned)surface->h - 1u ))
    y = (y<0) ? 0 : surface->h - 1;

  /* Set a pointer to the exact location in memory of the pixel
     in question: */

  p = (Uint8 *) (((Uint8 *)surface->pixels) +  /* Start at top of RAM */
    (y * surface->pitch) +  /* Go down Y lines */
    x);             /* Go in X pixels */


  /* Return the correctly-sized piece of data containing the
   * pixel's value (an 8-bit palette value, or a 16-, 24- or 32-bit
   * RGB value) */

  return(*p);
}


/* Get a pixel: */
static Uint32 getpixel16(SDL_Surface * surface, int x, int y)
{
  Uint8 * p;

  /* get the X/Y values within the bounds of this surface */
  if (unlikely( (unsigned)x > (unsigned)surface->w - 1u ))
    x = (x<0) ? 0 : surface->w - 1;
  if (unlikely( (unsigned)y > (unsigned)surface->h - 1u ))
    y = (y<0) ? 0 : surface->h - 1;

  /* Set a pointer to the exact location in memory of the pixel
     in question: */

  p = (Uint8 *) (((Uint8 *)surface->pixels) +  /* Start at top of RAM */
    (y * surface->pitch) +  /* Go down Y lines */
    (x * 2));             /* Go in X pixels */


  /* Return the correctly-sized piece of data containing the
   * pixel's value (an 8-bit palette value, or a 16-, 24- or 32-bit
   * RGB value) */

  return(*(Uint16 *)p);
}

/* Get a pixel: */
static Uint32 getpixel24(SDL_Surface * surface, int x, int y)
{
  Uint8 * p;
  Uint32 pixel;

  /* get the X/Y values within the bounds of this surface */
  if (unlikely( (unsigned)x > (unsigned)surface->w - 1u ))
    x = (x<0) ? 0 : surface->w - 1;
  if (unlikely( (unsigned)y > (unsigned)surface->h - 1u ))
    y = (y<0) ? 0 : surface->h - 1;

  /* Set a pointer to the exact location in memory of the pixel
     in question: */

  p = (Uint8 *) (((Uint8 *)surface->pixels) +  /* Start at top of RAM */
    (y * surface->pitch) +  /* Go down Y lines */
    (x * 3));             /* Go in X pixels */


  /* Return the correctly-sized piece of data containing the
   * pixel's value (an 8-bit palette value, or a 16-, 24- or 32-bit
   * RGB value) */

  /* Depending on the byte-order, it could be stored RGB or BGR! */

  if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
    pixel = p[0] << 16 | p[1] << 8 | p[2];
  else
    pixel = p[0] | p[1] << 8 | p[2] << 16;

  return pixel;
}

/* Get a pixel: */
static Uint32 getpixel32(SDL_Surface * surface, int x, int y)
{
  Uint8 * p;

  /* get the X/Y values within the bounds of this surface */
  if (unlikely( (unsigned)x > (unsigned)surface->w - 1u ))
    x = (x<0) ? 0 : surface->w - 1;
  if (unlikely( (unsigned)y > (unsigned)surface->h - 1u ))
    y = (y<0) ? 0 : surface->h - 1;

  /* Set a pointer to the exact location in memory of the pixel
     in question: */

  p = (Uint8 *) (((Uint8 *)surface->pixels) +  /* Start at top of RAM */
    (y * surface->pitch) +  /* Go down Y lines */
    (x * 4));             /* Go in X pixels */


  /* Return the correctly-sized piece of data containing the
   * pixel's value (an 8-bit palette value, or a 16-, 24- or 32-bit
   * RGB value) */

  return *(Uint32 *)p;  // 32-bit display
}


/* Draw a single pixel into the surface: */
static void putpixel8(SDL_Surface * surface, int x, int y, Uint32 pixel)
{
  Uint8 * p;

  /* Assuming the X/Y values are within the bounds of this surface... */
  if (likely( likely((unsigned)x<(unsigned)surface->w) && likely((unsigned)y<(unsigned)surface->h) ))
    {
      // Set a pointer to the exact location in memory of the pixel
      p = (Uint8 *) (((Uint8 *)surface->pixels) + /* Start: beginning of RAM */
		     (y * surface->pitch) +  /* Go down Y lines */
                     x);             /* Go in X pixels */


      /* Set the (correctly-sized) piece of data in the surface's RAM
       *          to the pixel value sent in: */

      *p = pixel;
    }
}

/* Draw a single pixel into the surface: */
static void putpixel16(SDL_Surface * surface, int x, int y, Uint32 pixel)
{
  Uint8 * p;

  /* Assuming the X/Y values are within the bounds of this surface... */
  if (likely( likely((unsigned)x<(unsigned)surface->w) && likely((unsigned)y<(unsigned)surface->h) ))
    {
      // Set a pointer to the exact location in memory of the pixel
      p = (Uint8 *) (((Uint8 *)surface->pixels) + /* Start: beginning of RAM */
		     (y * surface->pitch) +  /* Go down Y lines */
                     (x * 2));             /* Go in X pixels */


      /* Set the (correctly-sized) piece of data in the surface's RAM
       *          to the pixel value sent in: */

      *(Uint16 *)p = pixel;
    }
}

/* Draw a single pixel into the surface: */
static void putpixel24(SDL_Surface * surface, int x, int y, Uint32 pixel)
{
  Uint8 * p;

  /* Assuming the X/Y values are within the bounds of this surface... */
  if (likely( likely((unsigned)x<(unsigned)surface->w) && likely((unsigned)y<(unsigned)surface->h) ))
    {
      // Set a pointer to the exact location in memory of the pixel
      p = (Uint8 *) (((Uint8 *)surface->pixels) + /* Start: beginning of RAM */
		     (y * surface->pitch) +  /* Go down Y lines */
                     (x * 3));             /* Go in X pixels */


      /* Set the (correctly-sized) piece of data in the surface's RAM
       *          to the pixel value sent in: */

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

/* Draw a single pixel into the surface: */
static void putpixel32(SDL_Surface * surface, int x, int y, Uint32 pixel)
{
  Uint8 * p;

  /* Assuming the X/Y values are within the bounds of this surface... */
  if (likely( likely((unsigned)x<(unsigned)surface->w) && likely((unsigned)y<(unsigned)surface->h) ))
    {
      // Set a pointer to the exact location in memory of the pixel
      p = (Uint8 *) (((Uint8 *)surface->pixels) + /* Start: beginning of RAM */
		     (y * surface->pitch) +  /* Go down Y lines */
                     (x * 4));             /* Go in X pixels */


      /* Set the (correctly-sized) piece of data in the surface's RAM
       *          to the pixel value sent in: */

      *(Uint32 *)p = pixel;  // 32-bit display
    }
}


// XOR must show up on black, white, 0x7f grey, and 0x80 grey.
// XOR must be exactly 100% perfectly reversable.
static void xorpixel(int x, int y)
{
  // if outside the canvas, return
  if( (unsigned)x >= (unsigned)canvas->w || (unsigned)y >= (unsigned)canvas->h)
    return;
  // now switch to screen coordinates
  x += r_canvas.x;
  y += r_canvas.y;

  Uint8 * p;
  // Always 4, except 3 when loading a saved image.
  int BytesPerPixel = screen->format->BytesPerPixel;

  // Set a pointer to the exact location in memory of the pixel
  p = (Uint8 *) (((Uint8 *)screen->pixels) + /* Start: beginning of RAM */
                 (y * screen->pitch) +  /* Go down Y lines */
                 (x * BytesPerPixel));             /* Go in X pixels */

  // XOR the (correctly-sized) piece of data in the screen's RAM
  if (likely(BytesPerPixel == 4))
    *(Uint32 *)p ^= 0x80808080u;  // 32-bit display
  else if (BytesPerPixel == 1)
    *p ^= 0x80;
  else if (BytesPerPixel == 2)
    *(Uint16 *)p ^= 0xd6d6;
  else if (BytesPerPixel == 3)
    {
      p[0] ^= 0x80;
      p[1] ^= 0x80;
      p[2] ^= 0x80;
    }
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
	  update_screen_rect(&r_tools);
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
  update_screen_rect(&r_tools);
}


/* Create the current brush in the current color: */

static void render_brush(void)
{
  Uint32 amask;
  int x, y;
  Uint8 r, g, b, a;
  Uint32 (*getpixel_brush)(SDL_Surface *, int, int) = getpixels[img_brushes[cur_brush]->format->BytesPerPixel];
  void (*putpixel_brush)(SDL_Surface *, int, int, Uint32) = putpixels[img_brushes[cur_brush]->format->BytesPerPixel];


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
	  SDL_GetRGBA(getpixel_brush(img_brushes[cur_brush], x, y),
		      img_brushes[cur_brush]->format,
		      &r, &g, &b, &a);
	
	  putpixel_brush(img_cur_brush, x, y,
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
	        xorpixel(x1,y);
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
	        xorpixel(x1,y);
	    }
	}
      else
        {
          for (y = y1; y <= y2; y++)
	    {
	      num_drawn++;
	  
	      if (num_drawn < 10 || dont_do_xor == 0)
	        xorpixel(x1,y);
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
  SDL_FillRect(screen, &r_tuxarea, SDL_MapRGB(screen->format, 255, 255, 255));

  /* Draw tux: */
  dest.x = r_tuxarea.x;
  dest.y = r_tuxarea.y + r_tuxarea.h - img_tux[which_tux]->h;

  // if he's too tall to fit, go off the bottom (not top) edge
  if (dest.y < r_tuxarea.y)
    dest.y = r_tuxarea.y;

  SDL_BlitSurface(img_tux[which_tux], NULL, screen, &dest);

  wordwrap_text(str, black,
	        img_tux[which_tux]->w + 5,
	        r_tuxarea.y,
	        r_tuxarea.w,
		want_right_to_left);

  update_screen_rect(&r_tuxarea);
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
	locale_str = textdir(gettext(str));


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
		  text = render_text(locale_font, utf8_str, color);

		  if (!text) continue;  /* Didn't render anything... */

		  /* ----------- */
		  if (text->w > right - left)
		    {
		      /* Move left and down (if not already at left!) */

		      if (x > left)
			{
			  if (need_right_to_left && want_right_to_left)
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
			      text = render_text(locale_font, utf8_char, color);
			      if (text != NULL)
				{
				  if (x + text->w > right)
				    {
				      if (need_right_to_left && want_right_to_left)
					anti_carriage_return(left, right, top, top + text->h,
							     y + text->h, x - left);
	        
				      x = left;
				      y = y + text->h;
				    }

				  dest.x = x;

				  if (need_right_to_left && want_right_to_left)
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

			  if (need_right_to_left && want_right_to_left)
			    anti_carriage_return(left, right, top, top + text->h, y + text->h,
						 x - left);
          
			  x = left;
			  y = y + text->h;
			}

		      dest.x = x;

		      if (need_right_to_left && want_right_to_left)
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

      {
        char *s1 = gettext(str);
        if (want_right_to_left)
          {
            char *freeme = s1;
            s1 = textdir(s1);
            free(freeme);
          }
	tstr = uppercase(s1);
	free(s1);
      }

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

  
	  text = render_text(locale_font, substr, color);


	  /* If it won't fit on this line, move to the next! */

	  if (x + text->w > right)  /* Correct? */
	    {
	      if (need_right_to_left && want_right_to_left)
		anti_carriage_return(left, right, top, top + text->h, y + text->h,
				     x - left);
        
	      x = left;
	      y = y + text->h;
	    }


	  /* Draw the word: */

	  dest.x = x;

	  if (need_right_to_left && want_right_to_left)
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
  
  if (need_right_to_left && want_right_to_left && last_text_height > 0)
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

  snd_fname = malloc(strlen(fname) + strlen(lang_prefix) + 2);

  strcpy(snd_fname, fname);
  snprintf(tmp_str, sizeof(tmp_str), "_%s.wav", lang_prefix);


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
	
	      if (strstr(buf, lang_prefix) == buf)
		{

		  debug(buf + strlen(lang_prefix));
		  if (strstr(buf + strlen(lang_prefix), ".utf8=") ==
			   buf + strlen(lang_prefix))
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
	  return(strdup(buf + (strlen(lang_prefix)) + 6));
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
  FILE * fi;
  info_type *inf = malloc(sizeof(info_type));

  inf->ratio = 1.0;
  inf->colorable = 0;
  inf->tintable = 0;
  inf->mirrorable = 1;
  inf->flipable = 1;
  inf->tinter = TINTER_NORMAL;

  dat_fname = strdup(fname);
  char *pngptr = strstr(dat_fname, ".png");
  if (!pngptr)  // TODO: see if this can ever happen
    {
      free(dat_fname);
      return inf;
    }

  memcpy(pngptr+1, "dat", 3);

  fi = fopen(dat_fname, "r");
  if (!fi)
    {
      free(dat_fname);
      return inf;
    }

  free(dat_fname);

  do
    {
      fgets(buf, sizeof(buf), fi);

      if (!feof(fi))
        {
          strip_trailing_whitespace(buf);

          if (strcmp(buf, "colorable") == 0)
            inf->colorable = 1;
          else if (strcmp(buf, "tintable") == 0)
            inf->tintable = 1;
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
                    inf->ratio = tmp;
                }
              else if (strchr(cp,'/'))
                {
                  tmp = strtod(cp,&cp);
                  while(*cp && !isdigit(*cp))
                    cp++;
                  tmp2 = strtod(cp,NULL);
                  if (tmp>0.0001 && tmp<10000.0 && tmp2>0.0001 && tmp2<10000.0 && tmp/tmp2>0.0001 && tmp/tmp2<10000.0)
                    inf->ratio = tmp/tmp2;
                }
              else if (strchr(cp,':'))
                {
                  tmp = strtod(cp,&cp);
                  while(*cp && !isdigit(*cp))
                    cp++;
                  tmp2 = strtod(cp,NULL);
                  if (tmp>0.0001 && tmp<10000.0 && tmp2>0.0001 && tmp2<10000.0 && tmp2/tmp>0.0001 && tmp2/tmp<10000.0)
                    inf->ratio = tmp2/tmp;
                }
              else
                {
                  tmp = strtod(cp,NULL);
                  if (tmp > 0.0001 && tmp < 10000.0)
                    inf->ratio = 1.0 / tmp;
                }
            }
          else if (!memcmp(buf, "tinter", 6) && (isspace(buf[6]) || buf[6]=='='))
            {
              char *cp = buf+7;
              while (isspace(*cp) || *cp=='=')
                cp++;
              if (!strcmp(cp,"anyhue"))
                {
                    inf->tinter = TINTER_ANYHUE;
                }
              else if (!strcmp(cp,"narrow"))
                {
                    inf->tinter = TINTER_NARROW;
                }
              else if (!strcmp(cp,"normal"))
                {
                    inf->tinter = TINTER_NORMAL;
                }
              else if (!strcmp(cp,"vector"))
                {
                    inf->tinter = TINTER_VECTOR;
                }
              else
                {
                    debug(cp);
                }
            }
          else if (strcmp(buf, "nomirror") == 0)
            inf->mirrorable = 0;
          else if (strcmp(buf, "noflip") == 0)
            inf->flipable = 0;
        }
    }
  while (!feof(fi));

  fclose(fi);

  return inf;
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


static int SDLCALL NondefectiveBlit(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect)
{
  int dstx = 0;
  int dsty = 0;
  int srcx = 0;
  int srcy = 0;
  int srcw = src->w;
  int srch = src->h;
  Uint32 (*getpixel)(SDL_Surface *, int, int) = getpixels[src->format->BytesPerPixel];
  void (*putpixel)(SDL_Surface *, int, int, Uint32) = putpixels[dst->format->BytesPerPixel];

  
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
  return(do_prompt_image(text, btn_yes, btn_no, NULL, NULL, NULL));
}


static int do_prompt_image(const char * const text, const char * const btn_yes, const char * const btn_no, SDL_Surface * img1, SDL_Surface * img2, SDL_Surface * img3)
{
  return(do_prompt_image_flash(text, btn_yes, btn_no, img1, img2, img3, 0));
}


static int do_prompt_image_flash(const char * const text, const char * const btn_yes, const char * const btn_no, SDL_Surface * img1, SDL_Surface * img2, SDL_Surface * img3, int animate)
{
  SDL_Event event;
  SDL_Rect dest;
  int done, ans, w, counter;
  SDL_Color black = {0, 0, 0, 0};
  SDLKey key;
  SDLKey key_y, key_n;
  char *keystr;
#ifndef NO_PROMPT_SHADOWS
  int i;
  SDL_Surface * alpha_surf;
#endif


  /* FIXME: Move elsewhere! Or not?! */

  keystr = textdir(gettext("Yes"));
  key_y = tolower(keystr[0]);
  free(keystr);

  keystr = textdir(gettext("No"));
  key_n = tolower(keystr[0]);
  free(keystr);


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


  /* If we're showing any images on the right, determine the widest width
     for them: */

  /* FIXME: This needs to be fixed for right-to-left interfaces! */

  int img1_w, img2_w, img3_w, max_img_w, img_x, img_y, offset;

  offset = img1_w = img2_w = img3_w = 0;
  
  if (img1 != NULL)
    img1_w = img1->w;
  if (img2 != NULL)
    img2_w = img2->w;
  if (img3 != NULL)
    img3_w = img3->w;

  max_img_w = max(img1_w, max(img2_w, img3_w));

  if (max_img_w > 0)
    offset = max_img_w + 8;
  
  
  /* Draw the question: */

  wordwrap_text(text, black,
		166 + PROMPTOFFSETX, 100 + PROMPTOFFSETY,
		475 + PROMPTOFFSETX - offset,
		1);


  /* Draw the images (if any, and if not animated): */

  img_x = 457 + PROMPTOFFSETX - offset;
  img_y = 100 + PROMPTOFFSETY + 4;

  if (img1 != NULL)
  {
    dest.x = img_x + (max_img_w - img1->w) / 2;
    dest.y = img_y;

    SDL_BlitSurface(img1, NULL, screen, &dest);

    if (!animate)
      img_y = img_y + img1->h + 4;
  }

  if (!animate)
  {
    if (img2 != NULL)
    {
      dest.x = img_x + (max_img_w - img2->w) / 2;
      dest.y = img_y;

      SDL_BlitSurface(img2, NULL, screen, &dest);

      img_y = img_y + img2->h + 4;
    }

    if (img3 != NULL)
    {
      dest.x = img_x + (max_img_w - img3->w) / 2;
      dest.y = img_y;

      SDL_BlitSurface(img3, NULL, screen, &dest);

      img_y = img_y + img3->h + 4;  // unnecessary
    }
  }


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
  counter = 0;
  ans = 0;

  do
    {
      while (mySDL_PollEvent(&event))
      {
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
	         valid_click(event.button.button))
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

      SDL_Delay(100);

      if (animate)
      {
        counter++;

	if (counter == 5)
	{
          dest.x = img_x + (max_img_w - img2->w) / 2;
          dest.y = img_y;

          SDL_BlitSurface(img2, NULL, screen, &dest);
	  SDL_Flip(screen);
	}
	else if (counter == 10)
	{
	  if (img3 != NULL)
	  {
            dest.x = img_x + (max_img_w - img3->w) / 2;
            dest.y = img_y;

            SDL_BlitSurface(img3, NULL, screen, &dest);
	    SDL_Flip(screen);
	  }
	  else
	    counter = 15;
	}

	if (counter == 15)
	{
          dest.x = img_x + (max_img_w - img1->w) / 2;
          dest.y = img_y;

          SDL_BlitSurface(img1, NULL, screen, &dest);
	  SDL_Flip(screen);

	  counter = 0;
	}
      }
    }
  while (!done);


  /* FIXME: Sound effect! */
  /* ... */


  /* Erase question prompt: */

  update_canvas(0, 0, canvas->w, canvas->h);

  return ans;
}


/* Free memory and prepare to quit: */

static void cleanup(void)
{
  int i;

  for (i = 0; i < num_stamps; i++)
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
  free_surface_array( img_stamps, num_stamps );
  free_surface_array( img_stamps_premirror, num_stamps );

  free_surface_array( img_brushes, num_brushes );
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
  free_surface( &img_trash );

  free_surface( &img_printer );
  free_surface( &img_printer_wait );
  free_surface( &img_save_over );

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

  free_surface( &img_grow );
  free_surface( &img_shrink );

  free_surface( &img_bold );
  free_surface( &img_italic );

  free_surface( &img_sparkles );
  free_surface( &img_grass );

  free_surface_array( undo_bufs, NUM_UNDO_BUFS );

#ifdef LOW_QUALITY_COLOR_SELECTOR
  free_surface( &img_paintcan );
#else
  free_surface_array( img_color_btns, NUM_COLORS*2 );
#endif

  free_surface_array( img_stamp_thumbs, num_stamps );

  free_surface( &screen );
  free_surface( &img_starter );
  free_surface( &img_starter_bkgd );
  free_surface( &canvas );
  free_surface( &img_cur_brush );

  if (medium_font != NULL)
    {
      TTF_CloseFont(medium_font);
      medium_font = NULL;
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

#ifdef FORKED_FONTS
  free(user_font_families); // we'll leak the bodies... oh well
#else
  for (i = 0; i < num_font_families; i++)
    {
      if (user_font_families[i])
	{
	  char ** cpp = user_font_families[i]->filename - 1;
	  if (*++cpp)
	    free(*cpp);
	  if (*++cpp)
	    free(*cpp);
	  if (*++cpp)
	    free(*cpp);
	  if (*++cpp)
	    free(*cpp);
	  if (user_font_families[i]->handle)
	    TTF_CloseFont(user_font_families[i]->handle);
	  free(user_font_families[i]->directory);
	  free(user_font_families[i]->family);
	  free(user_font_families[i]);
	  user_font_families[i] = NULL;
	}
    }
#endif

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
  int xx;


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
    }


  if (use_brush && shape_filled[cur_shape])
    {
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

	  if (do_prompt_image(PROMPT_SAVE_OVER_TXT,
			PROMPT_SAVE_OVER_YES,
			PROMPT_SAVE_OVER_NO,
			img_save_over, NULL, NULL) == 0)
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
  Uint32 (*getpixel)(SDL_Surface *, int, int) = getpixels[surf->format->BytesPerPixel];
 

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
	      info_ptr->valid = 0;  // will be updated by various png_set_FOO() functions

              png_set_sRGB_gAMA_and_cHRM(png_ptr, info_ptr, PNG_sRGB_INTENT_PERCEPTUAL);

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

///////////////////////////////////// PostScript printing ///////////
#ifdef PRINTMETHOD_PS

/* Actually save the PostScript data to the file stream: */
static int do_ps_save(FILE * fi, const char *restrict  const fname, SDL_Surface * surf)
{
  unsigned char *restrict const ps_row = malloc(surf->w * 3);
  int x, y;
  char buf[256];
  Uint32 (*getpixel)(SDL_Surface *, int, int) = getpixels[surf->format->BytesPerPixel];
  
  fprintf(fi, "%%!PS-Adobe-3.0 EPSF-3.0\n");  // probably broken, but close enough maybe
  fprintf(fi, "%%%%Title: (%s)\n", fname);
  time_t t = time(NULL);
  strftime(buf, sizeof buf - 1, "%a %b %e %H:%M:%S %Y", localtime(&t));
  fprintf(fi, "%%%%CreationDate: (%s)\n", buf);
  fprintf(fi, "%%%%Creator: (Tux Paint " VER_VERSION ", " VER_DATE ")\n");
  fprintf(fi, "%%%%LanguageLevel: 2\n");
//  fprintf(fi, "%%%%BoundingBox: 72 214 540 578\n");  // doubt we have the needed info
  fprintf(fi, "%%%%DocumentData: Binary\n");
  fprintf(fi, "%%%%EndComments\n");
  fprintf(fi, "\n");
  fprintf(fi, "gsave\n");
  fprintf(fi, "\n");
//  fprintf(fi, "90 rotate\n");  // landscape mode
  fprintf(fi, "%% First, grab the page size.\n");
  fprintf(fi, "gsave\n");
  fprintf(fi, "  clippath\n");
  fprintf(fi, "  pathbbox\n");
  fprintf(fi, "grestore\n");
  fprintf(fi, "/ury exch def\n");
  fprintf(fi, "/urx exch def\n");
  fprintf(fi, "/lly exch def\n");
  fprintf(fi, "/llx exch def\n");
  fprintf(fi, "\n");
  fprintf(fi, "llx lly translate\n");
  fprintf(fi, "\n");
  fprintf(fi, "/width %u def\n", surf->w);
  fprintf(fi, "/height %u def\n", surf->h);
  fprintf(fi, "width height scale\n");
  fprintf(fi, "\n");
  fprintf(fi, "urx llx sub width div\n");
  fprintf(fi, "ury lly sub height div\n");
  fprintf(fi, "%% now do a 'min' operation\n");
  fprintf(fi, "2 copy gt { exch } if pop\n");
  fprintf(fi, "\n");
  fprintf(fi, "dup scale\n");
  fprintf(fi, "/DeviceRGB setcolorspace\n");
  fprintf(fi, "<<\n");
  fprintf(fi, "  /ImageType 1\n");
  fprintf(fi, "  /Width width /Height height\n");
  fprintf(fi, "  /BitsPerComponent 8\n");
  fprintf(fi, "  /ImageMatrix [width 0 0 height neg 0 height]\n");
  fprintf(fi, "  /Decode [0 1 0 1 0 1]\n");
  fprintf(fi, "  /DataSource currentfile\n");
  fprintf(fi, ">>\n");
  fprintf(fi, "%%%%BeginData: %u Binary Bytes\n", surf->w * surf->h * 3u);
  fprintf(fi, "image\n");

  /* Save the picture: */
  for (y = 0; y < surf->h; y++)
    {
      for (x = 0; x < surf->w; x++)
        {
          Uint8 r, g, b;
          SDL_GetRGB(getpixel(surf, x, y), surf->format, &r, &g, &b);
          ps_row[x * 3 + 0] = r;
          ps_row[x * 3 + 1] = g;
          ps_row[x * 3 + 2] = b;
        }
      fwrite(ps_row,surf->w,3,fi);
    }
  free(ps_row);

  fprintf(fi, "\n");
  fprintf(fi, "%%%%EndData\n");
  fprintf(fi, "grestore\n");
  fprintf(fi, "showpage\n");
  fprintf(fi, "%%%%EOF\n");

  fclose(fi);
  return 1;
}

#endif
/////////////////////////////////////////////////////////////////

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

    char *freeme = 
  		textdir(gettext_noop("Choose the picture you want, "
  				     "then click “Open”."));
    draw_tux_text(TUX_BORED,freeme, 1);
    free(freeme);

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
              event.button.button >= 1)
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
	      
		  if (do_prompt_image(PROMPT_ERASE_TXT,
				      PROMPT_ERASE_YES, PROMPT_ERASE_NO,
				      thumbs[which],
				      img_popup_arrow,
				      img_trash))
		    {
		      snprintf(fname, sizeof(fname), "saved/%s%s",
			       d_names[which], d_exts[which]);
		  
		      rfname = get_fname(fname);
		      debug(rfname);
		  
		      if (unlink(rfname) == 0)
			{
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
			  
			  thumbs[which] = NULL;
		      
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
		  if (do_prompt_image(PROMPT_OPEN_SAVE_TXT,
				PROMPT_OPEN_SAVE_YES,
				PROMPT_OPEN_SAVE_NO,
				img_tools[TOOL_SAVE], NULL, NULL))
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

  Uint32 (*getpixel)(SDL_Surface *, int, int) = getpixels[src->format->BytesPerPixel];
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
          sx = x + xx - stamp_outline_w/2;
          sy = y + yy - stamp_outline_h/2;
          if (stiple[sx%STIPLE_W + sy%STIPLE_H * STIPLE_W] != '8')
            continue;
          xorpixel(sx, sy);
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
#ifdef PRINTMETHOD_PNG_PNM_PS
      if (do_png_save(pi, printcommand, canvas))
	do_prompt(PROMPT_PRINT_TXT, PROMPT_PRINT_YES, "");
#elif defined(PRINTMETHOD_PNM_PS)
      // nothing here
#elif defined(PRINTMETHOD_PS)
      if (do_ps_save(pi, printcommand, canvas))
	do_prompt(PROMPT_PRINT_TXT, PROMPT_PRINT_YES, "");
#else
#error No print method defined!
#endif
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
		  TTF_FontHeight(getfonthandle(cur_font))))
    {
      cursor_y = ((48 * 7 + 40 + HEIGHTOFFSET) -
		  TTF_FontHeight(getfonthandle(cur_font)));
    }
  
  
  /* Render the text: */

  if (texttool_len > 0)
    {
      str = uppercase(texttool_str);
    
      tmp_surf = render_text(getfonthandle(cur_font), str, color);

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
    // FIXME: uppercase chars may need extra bytes
    ustr = malloc(strlen(str) + 1);

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

  dstr = malloc(strlen(str) + 5);

  if (need_right_to_left)
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
      double r, g, b;
      SDL_GetRGB(c1, canvas->format, &r1, &g1, &b1);
      SDL_GetRGB(c2, canvas->format, &r2, &g2, &b2);

      // use distance in linear RGB space
      r = sRGB_to_linear_table[r1] - sRGB_to_linear_table[r2];
      r *= r;
      g = sRGB_to_linear_table[g1] - sRGB_to_linear_table[g2];
      g *= g;
      b = sRGB_to_linear_table[b1] - sRGB_to_linear_table[b2];
      b *= b;

      // easy to confuse:
      //   dark grey, brown, purple
      //   light grey, tan
      //   red, orange
      return r+g+b < 0.04;
    }
#endif
}


/* Flood fill! */

static void do_flood_fill(int x, int y, Uint32 cur_colr, Uint32 old_colr)
{
  int fillL, fillR, i, in_line;
  static unsigned char prog_anim;
  Uint32 (*getpixel)(SDL_Surface *, int, int) = getpixels[canvas->format->BytesPerPixel];
  void (*putpixel)(SDL_Surface *, int, int, Uint32) = putpixels[canvas->format->BytesPerPixel];


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
	  else if (strcmp(str, "nosysfonts=yes") == 0)
	    {
	      no_system_fonts = 1;
	    }
	  else if (strcmp(str, "nosysfonts=no") == 0 ||
		   strcmp(str, "sysfonts=yes") == 0)
	    {
	      no_system_fonts = 0;
	    }
	  else if (strcmp(str, "nobuttondistinction=yes") == 0)
	    {
	      no_button_distinction= 1;
	    }
	  else if (strcmp(str, "nobuttondistinction=no") == 0 ||
		   strcmp(str, "buttondistinction=yes") == 0)
	    {
	      no_button_distinction= 0;
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
	  // Should "locale=" be here as well???
	  // Comments welcome ... bill@newbreedsoftware.com
	  else if (strstr(str, "lang=") == str)
	    {
	      set_langstr(str + 5);
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

static void draw_image_title(int t, SDL_Rect dest)
{
  SDL_BlitSurface(img_title_on, NULL, screen, &dest);
  
  dest.x += (dest.w - img_title_names[t]->w) / 2;
  dest.y += (dest.h - img_title_names[t]->h) / 2;
  SDL_BlitSurface(img_title_names[t], NULL, screen, &dest);
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


int valid_click(Uint8 button)
{
  if (button == 1 ||
      ((button == 2 || button == 3) && no_button_distinction))
    return(1);
  else
    return(0);
}
