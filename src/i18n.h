/*
  i18n.h

  For Tux Paint
  Language-related functions

  Copyright (c) 2002-2006 by Bill Kendrick and others
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/tuxpaint/

  June 14, 2002 - March 4, 2006
  $Id$
*/


#ifndef I18N_H

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
  LANG_ES_MX,  /* Spanish (Mexican) */
  LANG_ET,     /* Estonian */
  LANG_EU,     /* Basque */
  LANG_FI,     /* Finnish */
  LANG_FR,     /* French */
  LANG_GA,     /* Irish Gaelic */
  LANG_GD,     /* Scottish Gaelic */
  LANG_GL,     /* Galician */
  LANG_GR,     /* Gronings */
  LANG_HE,     /* Hebrew */
  LANG_HI,     /* Hindi */
  LANG_HR,     /* Croatian */
  LANG_HU,     /* Hungarian */
  LANG_I_KLINGON_ROMANIZED,     /* Klingon (Romanized) */
  LANG_ID,     /* Indonesian */
  LANG_IS,     /* Icelandic */
  LANG_IT,     /* Italian */
  LANG_JA,     /* Japanese */
  LANG_KA,     /* Georgian */
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
  LANG_RW,     /* Kinyarwanda */
  LANG_SK,     /* Slovak */
  LANG_SL,     /* Slovenian */
  LANG_SQ,     /* Albanian */
  LANG_SR,     /* Serbian */
  LANG_SV,     /* Swedish */
  LANG_SW,     /* Swahili */
  LANG_TA,     /* Tamil */
  LANG_TH,     /* Thai */
  LANG_TR,     /* Turkish */
  LANG_UK,     /* Ukrainian */
  LANG_VI,     /* Vietnamese */
  LANG_WA,     /* Walloon */
  LANG_ZH_CN,  /* Chinese (Simplified) */
  LANG_ZH_TW,  /* Chinese (Traditional) */
  NUM_LANGS
};


/* Types: */

typedef struct language_to_locale_struct {
  const char *language;
  const char *locale;
} language_to_locale_struct;


/* Globals: */

extern const char * lang_prefixes[NUM_LANGS];
extern int lang_use_own_font[];
extern int lang_use_right_to_left[];
extern char * langstr;
extern int need_own_font;
extern int need_right_to_left;
extern const char * lang_prefix;
extern const language_to_locale_struct language_to_locale_array[];



/* Function prototypes: */

void set_langstr(const char *s);
void set_current_language(void);
void show_lang_usage(FILE * f, const char * const prg);
void show_locale_usage(FILE * f, const char * const prg);
void setup_language(const char * const prg);
void do_locale_option(const char * const arg);


#endif

