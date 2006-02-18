/*
  i18n.c

  For Tux Paint
  Language-related functions

  Copyright (c) 2002-2006 by Bill Kendrick and others
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/tuxpaint/

  June 14, 2002 - February 17, 2006
  $Id$
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libintl.h>
#include "i18n.h"


/* Globals: */

const char * lang_prefixes[NUM_LANGS] = {
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
  "es_mx",
  "et",
  "eu",
  "fi",
  "fr",
  "ga",
  "gl",
  "gos",
  "he",
  "hi",
  "hr",
  "hu",
  "tlh",
  "id",
  "is",
  "it",
  "ja",
  "ka",
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
  "rw",
  "sk",
  "sl",
  "sq",
  "sr",
  "sv",
  "sw",
  "ta",
  "th",
  "tr",
  "uk",
  "vi",
  "wa",
  "zh_cn",
  "zh_tw"
};


// languages which don't use the default font
int lang_use_own_font[] = {
  LANG_HI,
  LANG_JA,
  LANG_KA,
  LANG_KO,
  LANG_TA,
  LANG_TH,
  LANG_ZH_CN,
  LANG_ZH_TW,
  -1
};

int lang_use_right_to_left[] = {
  LANG_HE,
  -1
};


char * langstr;
int need_own_font;
int need_right_to_left;
const char * lang_prefix;

const language_to_locale_struct language_to_locale_array[] = {
{"english",              "C"},
{"american-english",     "C"},
{"croatian",             "hr_HR.UTF-8"},
{"hrvatski",             "hr_HR.UTF-8"},
{"catalan",              "ca_ES.UTF-8"},
{"catala",               "ca_ES.UTF-8"},
{"belarusian",           "be_BY.UTF-8"},
{"bielaruskaja",         "be_BY.UTF-8"},
{"czech",                "cs_CZ.UTF-8"},
{"cesky",                "cs_CZ.UTF-8"},
{"danish",               "da_DK.UTF-8"},
{"dansk",                "da_DK.UTF-8"},
{"german",               "de_DE.UTF-8"},
{"deutsch",              "de_DE.UTF-8"},
{"estonian",             "et_EE.UTF-8"},
{"greek",                "el_GR.UTF-8"},
{"gronings",             "gos_NL.UTF-8"},
{"zudelk-veenkelonioals","gos_NL.UTF-8"},
{"british-english",      "en_GB.UTF-8"},
{"british",              "en_GB.UTF-8"},
{"spanish",              "es_ES.UTF-8"},
{"mexican",              "es_MX.UTF-8"},
{"mexican-spanish",      "es_MX.UTF-8"},
{"espanol-mejicano",     "es_MX.UTF-8"},
{"espanol",              "es_ES.UTF-8"},
{"finnish",              "fi_FI.UTF-8"},
{"suomi",                "fi_FI.UTF-8"},
{"french",               "fr_FR.UTF-8"},
{"francais",             "fr_FR.UTF-8"},
{"gaelic",               "ga_IE.UTF-8"},
{"gaidhlig",             "ga_IE.UTF-8"},
{"galician",             "gl_ES.UTF-8"},
{"galego",               "gl_ES.UTF-8"},
{"hebrew",               "he_IL.UTF-8"},
{"hindi",                "hi_IN.UTF-8"},
{"hungarian",            "hu_HU.UTF-8"},
{"magyar",               "hu_HU.UTF-8"},
{"indonesian",           "id_ID.UTF-8"},
{"bahasa-indonesia",     "id_ID.UTF-8"},
{"icelandic",            "is_IS.UTF-8"},
{"islenska",             "is_IS.UTF-8"},
{"italian",              "it_IT.UTF-8"},
{"italiano",             "it_IT.UTF-8"},
{"japanese",             "ja_JP.UTF-8"},
{"vietnamese",           "vi_VN.UTF-8"},
{"afrikaans",            "af_ZA.UTF-8"},
{"albanian",             "sq_AL.UTF-8"},
{"breton",               "br_FR.UTF-8"},
{"brezhoneg",            "br_FR.UTF-8"},
{"bulgarian",            "bg_BG.UTF-8"},
{"welsh",                "cy_GB.UTF-8"},
{"cymraeg",              "cy_GB.UTF-8"},
{"bokmal",               "nb_NO.UTF-8"},
{"basque",               "eu_ES.UTF-8"},
{"euskara",              "eu_ES.UTF-8"},
{"georgian",             "ka_GE"},
{"korean",               "ko_KR.UTF-8"},
{"kinyarwanda",          "rw_RW.UTF-8"},
{"klingon",              "tlh.UTF-8"},
{"tlhIngan",             "tlh.UTF-8"},
{"tlhingan",             "tlh.UTF-8"},
{"tamil",                "ta_IN.UTF-8"},
{"lithuanian",           "lt_LT.UTF-8"},
{"lietuviu",             "lt_LT.UTF-8"},
{"malay",                "ms_MY.UTF-8"},
{"dutch",                "nl_NL.UTF-8"},
{"nederlands",           "nl_NL.UTF-8"},
{"norwegian",            "nn_NO.UTF-8"},
{"nynorsk",              "nn_NO.UTF-8"},
{"norsk",                "nn_NO.UTF-8"},
{"polish",               "pl_PL.UTF-8"},
{"polski",               "pl_PL.UTF-8"},
{"brazilian-portuguese", "pt_BR.UTF-8"},
{"portugues-brazilian",  "pt_BR.UTF-8"},
{"brazilian",            "pt_BR.UTF-8"},
{"portuguese",           "pt_PT.UTF-8"},
{"portugues",            "pt_PT.UTF-8"},
{"romanian",             "ro_RO.UTF-8"},
{"russian",              "ru_RU.UTF-8"},
{"russkiy",              "ru_RU.UTF-8"},
{"slovak",               "sk_SK.UTF-8"},
{"slovenian",            "sl_SI.UTF-8"},
{"slovensko",            "sl_SI.UTF-8"},
{"serbian",              "sr_YU.UTF-8"},
{"swedish",              "sv_SE.UTF-8"},
{"svenska",              "sv_SE.UTF-8"},
{"swahili",              "sw_TZ.UTF-8"},
{"thai",                 "th_TH.UTF-8"},
{"turkish",              "tr_TR.UTF-8"},
{"ukrainian",            "uk_UA.UTF-8"},
{"walloon",              "wa_BE.UTF-8"},
{"walon",                "wa_BE.UTF-8"},
{"chinese",              "zh_CN.UTF-8"},
{"simplified-chinese",   "zh_CN.UTF-8"},
{"traditional-chinese",  "zh_TW.UTF-8"},
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

void set_langstr(const char *s)
{
  if (langstr)
    free(langstr);
  langstr = strdup(s);
}

/* Determine the current language/locale, and set the language string: */

void set_current_language(void)
{
  char * loc;
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
  bind_textdomain_codeset("tuxpaint", "UTF-8");
  loc = getenv("LANGUAGE");
  if (!loc)
    {
      loc = _nl_locale_name(LC_MESSAGES, "");
      if (loc)
        {
          char *s;
          int  len;
          len = strlen("LANGUAGE=")+strlen(loc)+1;
          s = malloc(len);
          snprintf(s, len, "LANGUAGE=%s", loc);
          putenv(s);
        }
    }
#endif

  //debug(loc);

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
  fprintf(stderr,"DEBUG: Language is %s (%d)\n", lang_prefix, lang);
  fflush(stderr);
#endif

}


/* FIXME: All this should REALLY be array-based!!! */
/* Show available languages: */
void show_lang_usage(FILE * f, const char * const prg)
{
  fprintf(f,
          "\n"
          "Usage: %s [--lang LANGUAGE]\n"
          "\n"
          "LANGUAGE may be one of:\n"
/* C */      "  english      american-english\n"
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
/* et */     "  estonian\n"
/* fi */     "  finnish      suomi\n"
/* fr */     "  french       francais\n"
/* ga */     "  gaelic       gaidhlig\n"
/* gl */     "  galician     galego\n"
/* ka */     "  georgian\n"
/* de */     "  german       deutsch\n"
/* el */     "  greek\n"
/* gos */    "  gronings     zudelk-veenkelonioals\n"
/* he */     "  hebrew\n"
/* hi */     "  hindi\n"
/* hu */     "  hungarian    magyar\n"
/* is */     "  icelandic    islenska\n"
/* id */     "  indonesian   bahasa-indonesia\n"
/* it */     "  italian      italiano\n"
/* ja */     "  japanese\n"
/* rw */     "  kinyarwanda\n"
/* tlh */    "  klingon      tlhIngan\n"
/* ko */     "  korean\n"
/* lt */     "  lithuanian   lietuviu\n"
/* ms */     "  malay\n"
/* es_MX */  "  mexican      mexican-spanish        espanol-mejicano\n"
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
/* th */     "  thai\n"
/* tr */     "  turkish\n"
/* uk */     "  ukrainian\n"
/* vi */     "  vietnamese\n"
/* wa */     "  walloon      walon\n"
/* cy */     "  welsh        cymraeg\n"
          "\n",
          prg);
}


/* FIXME: Add accented characters to the descriptions */
/* Show available locales: */
void show_locale_usage(FILE * f, const char * const prg)
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
          "  gos_NL  (Gronings     Zudelk Veenkelonioals)\n"
          "  de_DE   (German       Deutsch)\n"
          "  et_EE   (Estonian)\n"
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
          "  ka_GE   (Georgian)\n"
          "  ko_KR   (Korean)\n"
          "  ms_MY   (Malay)\n"
          "  lt_LT   (Lithuanian   Lietuviu)\n"
          "  nn_NO   (Norwegian    Nynorsk                Norsk)\n"
          "  pl_PL   (Polish       Polski)\n"
          "  pt_PT   (Portuguese   Portugues)\n"
          "  ro_RO   (Romanian)\n"
          "  ru_RU   (Russian      Russkiy)\n"
          "  rw_RW   (Kinyarwanda)\n"
          "  sk_SK   (Slovak)\n"
          "  sl_SI   (Slovenian)\n"
          "  sq_AL   (Albanian)\n"
          "  sr_YU   (Serbian)\n"
          "  es_ES   (Spanish      Espanol)\n"
          "  es_MX   (Mexican      Mexican Spanish       Espanol Mejicano)\n"
          "  sw_TZ   (Swahili)\n"
          "  sv_SE   (Swedish      Svenska)\n"
          "  th_TH   (Thai)\n"
          "  tr_TR   (Turkish)\n"
          "  uk_UA   (Ukrainian)\n"
          "  vi_VN   (Vietnamese)\n"
          "  wa_BE   (Walloon)\n"
          "  cy_GB   (Welsh        Cymraeg)\n"
          "\n",
          prg);
}

void setup_language(const char * const prg)
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

        {
          char *s;
          ssize_t len;

          len = strlen("LANGUAGE=")+strlen(locale)+1;
          s = malloc(len);
          snprintf(s, len, "LANGUAGE=%s", locale);
          putenv(s);

          len = strlen("LC_ALL=")+strlen(locale)+1;
          s = malloc(len);
          snprintf(s, len, "LC_ALL=%s", locale);
          putenv(s);
        }

      setlocale(LC_ALL, "");
      free(langstr);
    }

  set_current_language();
}


// handle --locale arg
void do_locale_option(const char * const arg)
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
