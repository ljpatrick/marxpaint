#pragma once
#include "compiler.h"

extern const char PARSE_YES[];
extern const char PARSE_NO[];
extern const char PARSE_CLOBBER[];

struct cfginfo
{
  const char *all_locale_fonts;
  const char *alt_print_command_default;
  const char *altprintcommand;
  const char *autosave_on_quit;
  const char *colorfile;
  const char *datadir;
  const char *disable_label;
  const char *disable_magic_controls;
  const char *disable_print;
  const char *disable_quit;
  const char *disable_save;
  const char *disable_screensaver;
  const char *disable_stamp_controls;
  const char *dont_do_xor;
  const char *dont_load_stamps;
  const char *fullscreen;
  const char *grab_input;
  const char *hide_cursor;
  const char *keymouse;
  const char *mirrorstamps;
  const char *native_screensize;
  const char *no_button_distinction;
  const char *no_fancy_cursors;
  const char *no_system_fonts;
  const char *noshortcuts;
  const char *ok_to_use_lockfile;
  const char *only_uppercase;
  const char *papersize;
  const char *parsertmp_fullscreen_native;
  const char *parsertmp_lang;
  const char *parsertmp_locale;
  const char *parsertmp_sysconfig;
  const char *parsertmp_windowsize;
  const char *print_delay;
  const char *printcommand;
  const char *promptless_save;
  const char *rotate_orientation;
  const char *savedir;
  const char *simple_shapes;
  const char *stamp_size_override;
  const char *start_blank;
  const char *use_print_config;
  const char *use_sound;
  const char *wheely;
};

#define CFGINFO_MAXOFFSET  (sizeof(struct cfginfo))

extern void parse_one_option(struct cfginfo *restrict tmpcfg, const char *str, const char *opt, const char *restrict src);
