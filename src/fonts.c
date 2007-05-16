/*
  fonts.c
*/

#include <stdio.h>
#ifndef __USE_GNU
#define __USE_GNU		/* for strcasestr() */
#endif
#include <string.h>
#include <stdlib.h>
#include <locale.h>

#include <errno.h>

#include <libintl.h>
#ifndef gettext_noop
#define gettext_noop(String) String
#endif

#include "fonts.h"
#include "i18n.h"
#include "progressbar.h"
#include "dirwalk.h"
#include "get_fname.h"
#include "debug.h"

#ifdef WIN32
#include "win32_print.h"
#endif

#ifdef __APPLE__
#include "wrapperdata.h"
extern WrapperData macosx;
#endif

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

#endif

#ifndef FORKED_FONTS
SDL_Thread *font_thread;
#endif
int no_system_fonts;
volatile long font_thread_done = 0, font_thread_aborted = 0;
volatile long waiting_for_fonts = 0;
int font_scanner_pid;
int font_socket_fd;

TTF_Font *medium_font, *small_font, *large_font, *locale_font;

family_info **user_font_families;
int num_font_families = 0;
int num_font_families_max = 0;

style_info **user_font_styles;
int num_font_styles = 0;
int num_font_styles_max = 0;

int text_state;
unsigned text_size = 4;		// initial text size

/* Unfortunately, there is a bug in SDL_ttf-2.0.6, the current version
   that causes a segmentation fault if an attempt is made to call
   TTF_OpenFont() with the filename of a font that doesn't exist. This
   is an old and well documented bug that is fixed in CVS.
*/
TTF_Font *BUGFIX_TTF_OpenFont206(const char *const file, int ptsize)
{
  FILE *fp;

#ifdef DEBUG
  printf("Loading font: %s\n", file);
#endif

  if ((fp = fopen(file, "rb")) == NULL)
    return NULL;
  fclose(fp);

#undef TTF_OpenFont
  return TTF_OpenFont(file, ptsize);
}


#define TTF_OpenFont    BUGFIX_TTF_OpenFont206


TTF_Font *try_alternate_font(int size)
{
  char str[128];
  char prefix[64];
  char *p;

  strcpy(prefix, lang_prefix);
  if ((p = strrchr(prefix, '_')) != NULL)
  {
    *p = 0;
    snprintf(str, sizeof(str), "%sfonts/locale/%s.ttf", DATA_PREFIX, prefix);

    return TTF_OpenFont(str, size);
  }
  return NULL;
}

TTF_Font *load_locale_font(TTF_Font * fallback, int size)
{
  TTF_Font *ret = NULL;
  if (need_own_font)
  {
    char str[128];
    snprintf(str, sizeof(str), "%sfonts/locale/%s.ttf",
	     DATA_PREFIX, lang_prefix);

    ret = TTF_OpenFont(str, size);
	 
#ifdef __APPLE__
    if (ret == NULL)
    {
      snprintf(str, sizeof(str), "%sfonts/%s.ttf", DATA_PREFIX, lang_prefix);
      ret = TTF_OpenFont(str, size);
    }	 
	 
    if (ret == NULL)
    {
      snprintf(str, sizeof(str), "/Library/Fonts/%s.ttf", lang_prefix);
      ret = TTF_OpenFont(str, size);
    }
	 
    if (ret == NULL)
    {
      snprintf(str, sizeof(str), "%s/%s.ttf", macosx.fontsPath, lang_prefix);
      ret = TTF_OpenFont(str, size);
    }	 
#endif

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
	ctype_utf8();

	set_current_language();
      }
    }
  }
  return ret ? ret : fallback;
}


#ifdef FORKED_FONTS

void reliable_write(int fd, const void *buf, size_t count)
{
  struct pollfd p;
  do
  {
    ssize_t rc = write(fd, buf, count);
    if (rc == -1)
    {
      switch (errno)
      {
      default:
	return;
      case EAGAIN:
      case ENOSPC:
	;			// satisfy a C syntax abomination
	p = (struct pollfd)
	{
	fd, POLLOUT, 0};
	poll(&p, 1, -1);	// try not to burn CPU time
	// FALL THROUGH
      case EINTR:
	continue;
      }
    }
    buf += rc;
    count -= rc;
  }
  while (count);
}


void reliable_read(int fd, void *buf, size_t count)
{
  struct pollfd p;
  do
  {
    ssize_t rc = read(fd, buf, count);
    if (rc == -1)
    {
      switch (errno)
      {
      default:
	return;
      case EAGAIN:
	;			// satisfy a C syntax abomination
	p = (struct pollfd)
	{
	fd, POLLIN, 0};
	poll(&p, 1, -1);	// try not to burn CPU time
	// FALL THROUGH
      case EINTR:
	continue;
      }
    }
    if (rc == 0)
      break;			// EOF. Better not happen before the end!
    buf += rc;
    count -= rc;
  }
  while (count);
}


void run_font_scanner(SDL_Surface * screen)
{
  int sv[2];
  int size, i;
  char *buf, *walk;

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv))
    exit(42);
  font_scanner_pid = fork();
  if (font_scanner_pid)
  {
    // parent (or error -- but we're screwed in that case)
    font_socket_fd = sv[0];
    close(sv[1]);
    return;
  }
  nice(42);			// be nice, letting the main thread get the CPU
  sched_yield();		// try to let the parent run right now
  prctl(PR_SET_PDEATHSIG, 9);	// get killed if parent exits
  font_socket_fd = sv[1];
  close(sv[0]);
  progress_bar_disabled = 1;
  reliable_read(font_socket_fd, &no_system_fonts, sizeof no_system_fonts);
  sched_yield();		// try to let the parent run right now
  SDL_Init(SDL_INIT_NOPARACHUTE);
  TTF_Init();
  load_user_fonts(screen, NULL);

  size = 0;
  i = num_font_families;
  while (i--)
  {
    char *s;
    s = user_font_families[i]->directory;
    if (s)
      size += strlen(s);
    s = user_font_families[i]->family;
    if (s)
      size += strlen(s);
    s = user_font_families[i]->filename[0];
    if (s)
      size += strlen(s);
    s = user_font_families[i]->filename[1];
    if (s)
      size += strlen(s);
    s = user_font_families[i]->filename[2];
    if (s)
      size += strlen(s);
    s = user_font_families[i]->filename[3];
    if (s)
      size += strlen(s);
    size += 6;			// for '\0' on each of the above
  }
  size += 2;			// for 2-byte font count
  buf = malloc(size);
  walk = buf;
#ifdef DEBUG
  printf("Sending %u bytes with %u families.\n", size, num_font_families);
#endif
  *walk++ = num_font_families & 0xffu;
  *walk++ = num_font_families >> 8u;
  i = num_font_families;
  while (i--)
  {
    int len;
    char *s;

    s = user_font_families[i]->directory;
    if (s)
    {
      len = strlen(s);
      memcpy(walk, s, len);
      walk += len;
    }
    *walk++ = '\0';

    s = user_font_families[i]->family;
    if (s)
    {
      len = strlen(s);
      memcpy(walk, s, len);
      walk += len;
    }
    *walk++ = '\0';

    s = user_font_families[i]->filename[0];
    if (s)
    {
      len = strlen(s);
      memcpy(walk, s, len);
      walk += len;
    }
    *walk++ = '\0';

    s = user_font_families[i]->filename[1];
    if (s)
    {
      len = strlen(s);
      memcpy(walk, s, len);
      walk += len;
    }
    *walk++ = '\0';

    s = user_font_families[i]->filename[2];
    if (s)
    {
      len = strlen(s);
      memcpy(walk, s, len);
      walk += len;
    }
    *walk++ = '\0';

    s = user_font_families[i]->filename[3];
    if (s)
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


void receive_some_font_info(SDL_Surface * screen)
{
  char *buf = NULL;
  unsigned buf_size = 0;
  unsigned buf_fill = 0;
  ssize_t rc;
  struct pollfd p;
  int status;
  /* unsigned */ char *walk;
  unsigned i;
  family_info *fip;

  fcntl(font_socket_fd, F_SETFL, O_NONBLOCK);
  for (;;)
  {
    if (buf_size <= buf_fill * 9 / 8 + 128)
    {
      buf_size = buf_size * 5 / 4 + 256;
      buf = realloc(buf, buf_size);
    }
    rc = read(font_socket_fd, buf + buf_fill, buf_size - buf_fill);
#ifdef DEBUG
    printf("read: fd=%d buf_fill=%u buf_size=%u rc=%ld\n", font_socket_fd,
	   buf_fill, buf_size, rc);
#endif

    if (rc == -1)
    {
      switch (errno)
      {
      default:
	return;
      case EAGAIN:
	;			// satisfy a C syntax abomination
	p = (struct pollfd)
	{
	font_socket_fd, POLLIN, 0};
	show_progress_bar(screen);
	poll(&p, 1, 29);	// try not to burn CPU time
	continue;
      case EINTR:
	continue;
      }
    }
    buf_fill += rc;
    if (!rc || font_thread_aborted)
      break;
  }
  close(font_socket_fd);

  waitpid(font_scanner_pid, &status, 0);
  if (WIFSIGNALED(status) || font_thread_aborted)
  {
    printf("child killed by signal %u\n", WTERMSIG(status));
    user_font_families = NULL;
    num_font_families = 0;
    font_thread_done = 1;

    return;
  }

  show_progress_bar(screen);
  walk = buf;
  num_font_families = *walk++;
  num_font_families += *walk++ << 8u;
#ifdef DEBUG
  printf("Got %u bytes with %u families.\n", buf_fill, num_font_families);
#endif
  user_font_families = malloc(num_font_families * sizeof *user_font_families);
  fip = malloc(num_font_families * sizeof **user_font_families);
  i = num_font_families;
  while (i--)
  {
    unsigned len;
    user_font_families[i] = fip + i;

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
  font_thread_done = 1;
}

#endif


int load_user_fonts(SDL_Surface * screen, void *vp)
{
  char *homedirdir;

  (void) vp;			// junk passed by threading library

  loadfonts(screen, DATA_PREFIX "fonts");

  if (!no_system_fonts)
  {
#ifdef WIN32
    homedirdir = GetSystemFontDir();
    loadfonts(screen, homedirdir);
    free(homedirdir);
#elif defined(__BEOS__)
    loadfonts(screen, "/boot/home/config/font/ttffonts");
    loadfonts(screen, "/usr/share/fonts");
    loadfonts(screen, "/usr/X11R6/lib/X11/fonts");
#elif defined(__APPLE__)
    loadfonts(screen, "/System/Library/Fonts");
    loadfonts(screen, "/Library/Fonts");
    loadfonts(screen, macosx.fontsPath);
    loadfonts(screen, "/usr/share/fonts");
    loadfonts(screen, "/usr/X11R6/lib/X11/fonts");
#elif defined(__sun__)
    loadfonts(screen, "/usr/openwin/lib/X11/fonts");
    loadfonts(screen, "/usr/share/fonts");
    loadfonts(screen, "/usr/X11R6/lib/X11/fonts");
#else
    loadfonts(screen, "/usr/share/feh/fonts");
    loadfonts(screen, "/usr/share/fonts");
    loadfonts(screen, "/usr/X11R6/lib/X11/fonts");
    loadfonts(screen, "/usr/share/texmf/fonts");
    loadfonts(screen, "/usr/share/grace/fonts/type1");
    loadfonts(screen, "/usr/share/hatman/fonts");
    loadfonts(screen, "/usr/share/icewm/themes/jim-mac");
    loadfonts(screen, "/usr/share/vlc/skins2/fonts");
    loadfonts(screen, "/usr/share/xplanet/fonts");
#endif
  }

  homedirdir = get_fname("fonts");
  loadfonts(screen, homedirdir);
  free(homedirdir);
#ifdef WIN32
  homedirdir = get_fname("data/fonts");
  loadfonts(screen, homedirdir);
  free(homedirdir);
#endif

  groupfonts();

  font_thread_done = 1;
  waiting_for_fonts = 0;
  // FIXME: need a memory barrier here
  return 0;			// useless, wanted by threading library
}


// void qsort(void *base, size_t nmemb, size_t size,
// int(*compar)(const void *, const void *));


// For qsort() and other use, to see if font files are groupable
int compar_fontgroup(const void *v1, const void *v2)
{
  const style_info *s1 = *(style_info **) v1;
  const style_info *s2 = *(style_info **) v2;
  int rc;

  rc = strcmp(s1->directory, s2->directory);
  if (rc)
    return rc;

  rc = s1->truetype - s2->truetype;
  if (rc)
    return rc;

  return strcmp(s1->family, s2->family);
}


// For qsort() and other use, to see if font files are duplicates
int compar_fontkiller(const void *v1, const void *v2)
{
  const family_info *f1 = *(family_info **) v1;
  const family_info *f2 = *(family_info **) v2;
  int rc;

  rc = strcmp(f1->family, f2->family);
  if (rc)
    return rc;

  return f1->score - f2->score;
}


// For qsort() and other use, to order the worst ones last
int compar_fontscore(const void *v1, const void *v2)
{
  const family_info *f1 = *(family_info **) v1;
  const family_info *f2 = *(family_info **) v2;

  return f2->score - f1->score;
}


// Font style names are a mess that we must try to make
// sense of. For example...
//
// Cooper: Light, Medium, Light Bold, Black
// HoeflerText: (nil), Black
void parse_font_style(style_info * si)
{
  int have_light = 0;
  int have_demi = 0;
  int have_bold = 0;
  int have_medium = 0;
  int have_black = 0;
  int have_heavy = 0;

  int stumped = 0;
  char *sp = si->style;

  si->italic = 0;


  while (*sp)
  {
    if (*sp == ' ')
    {
      sp++;
      continue;
    }
    if (!strncasecmp(sp, "Bold", strlen("Bold")))
    {
      sp += strlen("Bold");
      have_bold = 1;
      continue;
    }
    if (!strncasecmp(sp, "Regular", strlen("Regular")))
    {
      sp += strlen("Regular");
      continue;
    }
    if (!strncasecmp(sp, "Italic", strlen("Italic")))
    {
      sp += strlen("Italic");
      si->italic = 1;
      continue;
    }
    if (!strncasecmp(sp, "Oblique", strlen("Oblique")))
    {
      sp += strlen("Oblique");
      si->italic = 1;
      continue;
    }
    // move " Condensed" from style to family
    if (!strncasecmp(sp, "Condensed", strlen("Condensed")))
    {
      size_t len = strlen(si->family);
      char *name = malloc(len + strlen(" Condensed") + 1);
      sp += strlen("Condensed");
      memcpy(name, si->family, len);
      strcpy(name + len, " Condensed");
      free(si->family);
      si->family = name;
      continue;
    }
    if (!strncasecmp(sp, "Light", strlen("Light")))
    {
      sp += strlen("Light");
      have_light = 1;
      continue;
    }
    if (!strncasecmp(sp, "Medium", strlen("Medium")))
    {
      sp += strlen("Medium");
      have_medium = 1;
      continue;
    }
    if (!strncasecmp(sp, "Demi", strlen("Demi")))
    {
      sp += strlen("Demi");
      have_demi = 1;
      continue;
    }
    if (!strncasecmp(sp, "Heavy", strlen("Heavy")))
    {
      sp += strlen("Heavy");
      have_heavy = 1;
      continue;
    }
    if (!strncasecmp(sp, "Normal", strlen("Normal")))
    {
      sp += strlen("Normal");
      continue;
    }
    if (!strncasecmp(sp, "Black", strlen("Black")))
    {
      sp += strlen("Black");
      have_black = 1;
      continue;
    }
    if (!strncasecmp(sp, "Roman", strlen("Roman")))
    {
      sp += strlen("Roman");
      continue;
    }
    if (!strncasecmp(sp, "Book", strlen("Book")))
    {
      sp += strlen("Book");
      continue;
    }
    if (!strncasecmp(sp, "Chancery", strlen("Chancery")))
    {
      sp += strlen("Chancery");
      si->italic = 1;
      continue;
    }
    if (!strncasecmp(sp, "Thin", strlen("Thin")))
    {
      sp += strlen("Thin");
      have_light = 1;
      continue;
    }
    if (!strncmp(sp, "LR", strlen("LR")))
    {
      sp += strlen("LR");
      continue;
    }

    if (!stumped)
    {
      stumped = 1;
#if 0
// THREADED_FONTS
      printf("Font style parser stumped by \"%s\".\n", si->style);
#endif
    }
    sp++;			// bad: an unknown character
  }


  if (have_demi || have_medium)
    si->boldness = 2;
  else if (have_bold || have_black || have_heavy)	// TODO: split these
    si->boldness = 3;
  else if (have_light)
    si->boldness = 0;
  else
    si->boldness = 1;

  // we'll count both TrueType and OpenType
  si->truetype = !!strcasestr(si->filename, ".ttf")
    || !!strcasestr(si->filename, ".otf");
}




void groupfonts_range(style_info ** base, int count)
{
  int boldcounts[4] = { 0, 0, 0, 0 };
  int boldmap[4] = { -1, -1, -1, -1 };
  int i;
  int boldmax;
  int boldmin;
  int bolduse;
  int spot;
  family_info *fi;

#if 0
// THREADED_FONTS
  if (count < 1 || count > 4)
  {
    printf("\n::::::: %d styles in %s:\n", count, base[0]->family);
    i = count;
    while (i--)
    {
      printf("               %s\n", base[i]->style);
    }
  }
#endif

  i = count;
  while (i--)
    boldcounts[base[i]->boldness]++;

  boldmax = base[0]->boldness;
  boldmin = base[0]->boldness;
  bolduse = 0;

  i = 4;
  while (i--)
  {
    if (!boldcounts[i])
      continue;
    if (i > boldmax)
      boldmax = i;
    if (i < boldmin)
      boldmin = i;
    bolduse++;
  }
  if (likely(bolduse <= 2))
  {
    // in case they are same, we want non-bold,
    // so that setting goes second
    boldmap[boldmax] = 1;
    boldmap[boldmin] = 0;
  }
  else if (count == 3)
  {
    int boldmid;
    int zmin = 0, zmid = 0, zmax = 0;

    boldmap[boldmax] = 1;
    boldmap[boldmin] = 0;
    boldmid = boldcounts[boldmin + 1] ? boldmin + 1 : boldmin + 2;

    i = 3;
    while (i--)
    {
      if (base[i]->boldness == boldmin)
	zmin = base[i]->italic;
      if (base[i]->boldness == boldmid)
	zmid = base[i]->italic;
      if (base[i]->boldness == boldmax)
	zmax = base[i]->italic;
    }
    if (zmin != zmid)
      boldmap[boldmid] = 0;
    else if (zmid != zmax)
      boldmap[boldmid] = 1;
    else if (boldmin == 0 && boldmid == 1)
    {
      boldmap[0] = -1;
      boldmap[1] = 0;
    }
  }
  else
  {
    int claimed_bold = boldcounts[3];
    int claimed_norm = boldcounts[1];

    // 3 or 4 boldness levels, 4 or more styles!
    // This is going to be random hacks and hopes.

    // bold is bold
    boldmap[3] = 1;

    // norm is norm
    boldmap[1] = 0;

    // classify demi-bold or medium
    if (claimed_bold < 2)
    {
      boldmap[2] = 1;
      claimed_bold += boldcounts[2];
    }
    else if (claimed_norm < 2)
    {
      boldmap[2] = 0;
      claimed_norm += boldcounts[2];
    }

    // classify lightface
    if (claimed_norm < 2)
    {
      boldmap[0] = 0;
      //claimed_norm += boldcounts[0];
    }
  }

  if (num_font_families == num_font_families_max)
  {
    num_font_families_max = num_font_families_max * 5 / 4 + 30;
    user_font_families =
      realloc(user_font_families,
	      num_font_families_max * sizeof *user_font_families);
  }

  fi = calloc(1, sizeof *fi);
  user_font_families[num_font_families++] = fi;
  fi->directory = strdup(base[0]->directory);
  fi->family = strdup(base[0]->family);
  fi->score = base[0]->truetype + base[0]->score;
  i = count;
  while (i--)
  {
    int b = boldmap[base[i]->boldness];
    if (b == -1)
    {
#if 0
// THREADED_FONTS
      printf("too many boldness levels, discarding: %s, %s\n",
	     base[i]->family, base[i]->style);
#endif
      continue;
    }
    spot = b ? TTF_STYLE_BOLD : 0;
    spot += base[i]->italic ? TTF_STYLE_ITALIC : 0;
    if (fi->filename[spot])
    {
#if 0
// THREADED_FONTS
      printf("duplicates, discarding: %s, %s\n", base[i]->family,
	     base[i]->style);
      printf("b %d, spot %d\n", b, spot);
      printf("occupancy %p %p %p %p\n", fi->filename[0], fi->filename[1],
	     fi->filename[2], fi->filename[3]);
#endif
      continue;
    }
    fi->filename[spot] = strdup(base[i]->filename);
    fi->score += 2;
  }
  if (!fi->filename[0] && !fi->filename[1])
  {
    fi->filename[0] = fi->filename[2];
    fi->filename[2] = NULL;
    fi->filename[1] = fi->filename[3];
    fi->filename[3] = NULL;
  }
  if (!fi->filename[0] && !fi->filename[2])
  {
    fi->filename[0] = fi->filename[1];
    fi->filename[1] = NULL;
    fi->filename[2] = fi->filename[3];
    fi->filename[3] = NULL;
  }
  if (!fi->filename[0])
  {
    fi->filename[0] = strdup(fi->filename[TTF_STYLE_BOLD]);
  }
}


void dupe_markdown_range(family_info ** base, int count)
{
  int bestscore = -999;
  int bestslot = 0;
  int i = count;
  while (i--)
  {
    int score = base[i]->score;
    if (score <= bestscore)
      continue;
    bestscore = score;
    bestslot = i;
  }
  i = count;
  while (i--)
  {
    if (i == bestslot)
      continue;
    base[i]->score = -999;
  }
}


void groupfonts(void)
{
  char **cpp;
  int i = num_font_styles;
  int low = 0;

  while (i--)
    parse_font_style(user_font_styles[i]);

  qsort(user_font_styles, num_font_styles, sizeof user_font_styles[0],
	compar_fontgroup);
  //printf("groupfonts() qsort(user_font_styles...)\n");
  //fflush(stdout);

  for (;;)
  {
    int high = low;
    if (low >= num_font_styles)
      break;
    for (;;)
    {
      if (++high >= num_font_styles)
	break;
      if (compar_fontgroup(user_font_styles + low, user_font_styles + high))
	break;
    }
    groupfonts_range(user_font_styles + low, high - low);
    low = high;
  }

  i = num_font_styles;
  while (i--)
  {
    free(user_font_styles[i]->filename);
    free(user_font_styles[i]->directory);
    free(user_font_styles[i]->family);
    free(user_font_styles[i]->style);
    free(user_font_styles[i]);
  }
  free(user_font_styles);
  user_font_styles = NULL;	// just to catch bugs

  qsort(user_font_families, num_font_families, sizeof user_font_families[0],
	compar_fontkiller);
  //printf(stderr, "groupfonts() qsort(user_font_families 1...)\n");
  //fflush(stdout);
  low = 0;
  for (;;)
  {
    int high = low;
    if (low >= num_font_families)
      break;
    for (;;)
    {
      if (++high >= num_font_families)
	break;
      if (strcmp
	  (user_font_families[low]->family, user_font_families[high]->family))
	break;
    }
    dupe_markdown_range(user_font_families + low, high - low);
    low = high;
  }
  qsort(user_font_families, num_font_families, sizeof user_font_families[0],
	compar_fontscore);
  //printf("groupfonts() qsort(user_font_families 2...)\n");
  //fflush(stdout);
  if (user_font_families[0]->score < 0)
    printf("sorted the wrong way, or all fonts were unusable\n");
#if 0
// THREADED_FONTS
  printf("Trim starting with %d families\n", num_font_families);
#endif
  while (num_font_families > 1
	 && user_font_families[num_font_families - 1]->score < 0)
  {
    i = --num_font_families;
    free(user_font_families[i]->directory);
    free(user_font_families[i]->family);
    cpp = user_font_families[i]->filename;
    if (cpp[0])
      free(cpp[0]);
    if (cpp[1])
      free(cpp[1]);
    if (cpp[2])
      free(cpp[2]);
    if (cpp[3])
      free(cpp[3]);
    free(user_font_families[i]);
    user_font_families[i] = NULL;
  }
#if 0
// THREADED_FONTS
  printf("Trim ending with %d families\n", num_font_families);
#endif
}


TTF_Font *getfonthandle(int desire)
{
  int missing = 0;
  family_info *fi = user_font_families[desire];
  char *name = fi->filename[text_state];
  char *pathname;

  if (fi->handle)
    return fi->handle;

  if (!name)
  {
    name = fi->filename[text_state ^ TTF_STYLE_ITALIC];
    missing = text_state & TTF_STYLE_ITALIC;
  }
  if (!name)
  {
    name = fi->filename[text_state ^ TTF_STYLE_BOLD];
    missing = text_state & TTF_STYLE_BOLD;
  }
  if (!name)
  {
    name = fi->filename[text_state ^ (TTF_STYLE_ITALIC | TTF_STYLE_BOLD)];
    missing = text_state & (TTF_STYLE_ITALIC | TTF_STYLE_BOLD);
  }
  pathname = alloca(strlen(fi->directory) + 1 + strlen(name) + 1);
  sprintf(pathname, "%s/%s", fi->directory, name);

  fi->handle = TTF_OpenFont(pathname, text_sizes[text_size]);
  // if the font doesn't load, we die -- it did load OK before though
  TTF_SetFontStyle(fi->handle, missing);
  return fi->handle;
}


void loadfonts(SDL_Surface * screen, const char *const dir)
{
  char buf[TP_FTW_PATHSIZE];
  unsigned dirlen = strlen(dir);

  memcpy(buf, dir, dirlen);
  tp_ftw(screen, buf, dirlen, 1, loadfont_callback);
}


// backdoor into qsort operations, so we don't have to do work again
int was_bad_font;

// see if two font surfaces are the same
int do_surfcmp(const SDL_Surface * const *const v1,
	       const SDL_Surface * const *const v2)
{
  const SDL_Surface *const s1 = *v1;
  const SDL_Surface *const s2 = *v2;
  int width;
  int cmp;
  int i;

  if (s1 == s2)
  {
    printf("s1==s2?\n");
    return 0;
  }
  if (!s1 || !s2 || !s1->w || !s2->w || !s1->h || !s2->h || !s1->format
      || !s2->format)
  {
    was_bad_font = 1;
    return 0;
  }
  if (s1->format->BytesPerPixel != s2->format->BytesPerPixel)
  {
    // something really strange and bad happened
    was_bad_font = 1;
    return s1->format->BytesPerPixel - s2->format->BytesPerPixel;
  }


  if (s1->w != s2->w)
    return s1->w - s2->w;
  if (s1->h != s2->h)
    return s1->h - s2->h;

  {
    const char *const c1 = (char *const) s1->pixels;
    const char *const c2 = (char *const) s2->pixels;
    width = s1->format->BytesPerPixel * s1->w;
    if (width == s1->pitch)
      return memcmp(c1, c2, width * s1->h);
    cmp = 0;
    i = s1->h;
    while (i--)
    {
      cmp = memcmp(c1 + i * s1->pitch, c2 + i * s2->pitch, width);
      if (cmp)
	break;
    }
  }

  return cmp;
}

// see if two font surfaces are the same
int surfcmp(const void *s1, const void *s2)
{
  int diff = do_surfcmp(s1, s2);
  if (!diff)
    was_bad_font = 1;
  return diff;
}

// check if the characters will render distinctly
int charset_works(TTF_Font * font, const char *s)
{
  SDL_Color black = { 0, 0, 0, 0 };
  SDL_Surface **surfs = malloc(strlen(s) * sizeof surfs[0]);
  unsigned count = 0;
  int ret = 0;
  while (*s)
  {
    char c[8];
    unsigned offset = 0;
    SDL_Surface *tmp_surf;
    do
      c[offset++] = *s++;
    while ((*s & 0xc0u) == 0x80u);	// assume safe input
    c[offset++] = '\0';
    tmp_surf = TTF_RenderUTF8_Blended(font, c, black);
    if (!tmp_surf)
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
  while (count--)
  {
    if (surfs[count] == NULL)
      printf("TRYING TO RE-FREE!");
    else
    {
      SDL_FreeSurface(surfs[count]);
      surfs[count] = NULL;
    }
  }
  free(surfs);
  return ret;
}
