/*
  i18n.c

  For Tux Paint
  Language-related functions

  Copyright (c) 2002-2008 by Bill Kendrick and others
  bill@newbreedsoftware.com
  http://www.tuxpaint.org/

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

  $Id$

  June 14, 2002 - October 27, 2010
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libintl.h>
#include <locale.h>
#include "i18n.h"
#include "debug.h"

#ifdef WIN32
#include <sys/types.h>
#endif

#ifdef __BEOS__
#include <wchar.h>
#else
#include <wchar.h>
#include <wctype.h>
#endif


/* Globals: */

static int langint = LANG_EN;

const char *lang_prefixes[NUM_LANGS] = {
  "af",
  "ak",
  "ar",
  "ast",
  "az",
  "be",
  "bg",
  "bo",
  "br",
  "ca",
  "cs",
  "cy",
  "da",
  "de",
  "el",
  "en",
  "en_AU",
  "en_CA",
  "en_GB",
  "en_ZA",
  "eo",
  "es",
  "es_MX",
  "et",
  "eu",
  "fa",
  "ff",
  "fi",
  "fo",
  "fr",
  "ga",
  "gd",
  "gl",
  "gos",
  "gu",
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
  "km",
  "ko",
  "ku",
  "lb",
  "lg",
  "lt",
  "lv",
  "ml",
  "mk",
  "ms",
  "nb",
  "nl",
  "nn",
  "nr",
  "nso",
  "oc",
  "oj",
  "pl",
  "pt_BR",
  "pt",
  "ro",
  "ru",
  "rw",
  "shs",
  "sk",
  "sl",
  "son",
  "sq",
  "sr",
  "sv",
  "sw",
  "ta",
  "te",
  "th",
  "tl",
  "tr",
  "twi",
  "uk",
  "ve",
  "vec",
  "vi",
  "wa",
  "wo",
  "xh",
  "zh_CN",
  "zh_TW",
  "zw"
};


// languages which don't use the default font
static int lang_use_own_font[] = {
  LANG_AR,
  LANG_BO,
  LANG_GU,
  LANG_HI,
  LANG_JA,
  LANG_KA,
  LANG_KO,
  LANG_ML,
  LANG_TA,
  LANG_TE,
  LANG_TH,
  LANG_ZH_CN,
  LANG_ZH_TW,
  -1
};

static int lang_use_right_to_left[] = {
  LANG_AR,
  LANG_FA,
  LANG_HE,
  -1
};

static int lang_use_right_to_left_word[] = {
#ifdef NO_SDLPANGO
  LANG_HE,
#endif
  -1
};

static int lang_y_nudge[][2] = {
  {LANG_KM, 4},
  {-1, -1}
};


int need_own_font;
int need_right_to_left;
int need_right_to_left_word;
const char *lang_prefix, *short_lang_prefix;

static const language_to_locale_struct language_to_locale_array[] = {
  {"english", "C"},
  {"american-english", "C"},
  {"akan", "ak_GH.UTF-8"},
  {"twi-fante", "ak_GH.UTF-8"},
  {"arabic", "ar_SA.UTF-8"},
  {"asturian", "ast_ES.UTF-8"},
  {"azerbaijani", "az_AZ.UTF-8"},
  {"croatian", "hr_HR.UTF-8"},
  {"hrvatski", "hr_HR.UTF-8"},
  {"catalan", "ca_ES.UTF-8"},
  {"catala", "ca_ES.UTF-8"},
  {"belarusian", "be_BY.UTF-8"},
  {"bielaruskaja", "be_BY.UTF-8"},
  {"czech", "cs_CZ.UTF-8"},
  {"cesky", "cs_CZ.UTF-8"},
  {"danish", "da_DK.UTF-8"},
  {"dansk", "da_DK.UTF-8"},
  {"german", "de_DE.UTF-8"},
  {"deutsch", "de_DE.UTF-8"},
  {"estonian", "et_EE.UTF-8"},
  {"greek", "el_GR.UTF-8"},
  {"gronings", "gos_NL.UTF-8"},
  {"zudelk-veenkelonioals", "gos_NL.UTF-8"},
  {"gujarati", "gu_IN.UTF-8"},
  {"british-english", "en_GB.UTF-8"},
  {"british", "en_GB.UTF-8"},
  {"australian-english", "en_AU.UTF-8"},
  {"canadian-english", "en_CA.UTF-8"},
  {"southafrican-english", "en_ZA.UTF-8"},
  {"esperanto", "eo.UTF-8"},
  {"spanish", "es_ES.UTF-8"},
  {"mexican", "es_MX.UTF-8"},
  {"mexican-spanish", "es_MX.UTF-8"},
  {"espanol-mejicano", "es_MX.UTF-8"},
  {"espanol", "es_ES.UTF-8"},
  {"persian", "fa_IR.UTF-8"},
  {"fula", "ff_SN.UTF-8"},
  {"fulah", "ff_SN.UTF-8"},
  {"pulaar", "ff_SN.UTF-8"},
  {"finnish", "fi_FI.UTF-8"},
  {"suomi", "fi_FI.UTF-8"},
  {"faroese", "fo_FO.UTF-8"},
  {"french", "fr_FR.UTF-8"},
  {"francais", "fr_FR.UTF-8"},
  {"gaelic", "ga_IE.UTF-8"},
  {"irish-gaelic", "ga_IE.UTF-8"},
  {"gaidhlig", "ga_IE.UTF-8"},
  {"scottish", "gd_GB.UTF-8"},
  {"ghaidhlig", "gd_GB.UTF-8"},
  {"scottish-gaelic", "gd_GB.UTF-8"},
  {"galician", "gl_ES.UTF-8"},
  {"galego", "gl_ES.UTF-8"},
  {"hebrew", "he_IL.UTF-8"},
  {"hindi", "hi_IN.UTF-8"},
  {"hungarian", "hu_HU.UTF-8"},
  {"magyar", "hu_HU.UTF-8"},
  {"indonesian", "id_ID.UTF-8"},
  {"bahasa-indonesia", "id_ID.UTF-8"},
  {"icelandic", "is_IS.UTF-8"},
  {"islenska", "is_IS.UTF-8"},
  {"italian", "it_IT.UTF-8"},
  {"italiano", "it_IT.UTF-8"},
  {"japanese", "ja_JP.UTF-8"},
  {"venda", "ve_ZA.UTF-8"},
  {"venetian", "vec.UTF-8"},
  {"veneto", "vec.UTF-8"},
  {"vietnamese", "vi_VN.UTF-8"},
  {"afrikaans", "af_ZA.UTF-8"},
  {"albanian", "sq_AL.UTF-8"},
  {"breton", "br_FR.UTF-8"},
  {"brezhoneg", "br_FR.UTF-8"},
  {"bulgarian", "bg_BG.UTF-8"},
  {"welsh", "cy_GB.UTF-8"},
  {"cymraeg", "cy_GB.UTF-8"},
  {"bokmal", "nb_NO.UTF-8"},
  {"basque", "eu_ES.UTF-8"},
  {"euskara", "eu_ES.UTF-8"},
  {"georgian", "ka_GE"},
  {"kinyarwanda", "rw_RW.UTF-8"},
  {"klingon", "tlh.UTF-8"},
  {"tlhIngan", "tlh.UTF-8"},
  {"tlhingan", "tlh.UTF-8"},
  {"korean", "ko_KR.UTF-8"},
  {"kurdish", "ku_TR.UTF-8"},
  {"tamil", "ta_IN.UTF-8"},
  {"telugu", "te_IN.UTF-8"},
  {"lithuanian", "lt_LT.UTF-8"},
  {"lietuviu", "lt_LT.UTF-8"},
  {"latvian", "lv_LV.UTF-8"},
  {"luganda", "lg_UG.UTF-8"},
  {"luxembourgish", "lb_LU.UTF-8"},
  {"letzebuergesch", "lb_LU.UTF-8"},
  {"macedonian", "mk_MK.UTF-8"},
  {"malay", "ms_MY.UTF-8"},
  {"dutch", "nl_NL.UTF-8"},
  {"nederlands", "nl_NL.UTF-8"},
  {"norwegian", "nn_NO.UTF-8"},
  {"nynorsk", "nn_NO.UTF-8"},
  {"norsk", "nn_NO.UTF-8"},
  {"ndebele", "nr_ZA.UTF-8"},
  {"northern-sotho", "nso_ZA.UTF-8"},
  {"sesotho-sa-leboa", "nso_ZA.UTF-8"},
  {"occitan", "oc_FR.UTF-8"},
  {"ojibwe", "oj_CA.UTF-8"}, // Proper spelling
  {"ojibway", "oj_CA.UTF-8"}, // For compatibility
  {"polish", "pl_PL.UTF-8"},
  {"polski", "pl_PL.UTF-8"},
  {"brazilian-portuguese", "pt_BR.UTF-8"},
  {"portugues-brazilian", "pt_BR.UTF-8"},
  {"brazilian", "pt_BR.UTF-8"},
  {"portuguese", "pt_PT.UTF-8"},
  {"portugues", "pt_PT.UTF-8"},
  {"romanian", "ro_RO.UTF-8"},
  {"russian", "ru_RU.UTF-8"},
  {"russkiy", "ru_RU.UTF-8"},
  {"serbian", "sr_YU.UTF-8"},
  {"shuswap", "shs_CA.UTF-8"},
  {"secwepemctin", "shs_CA.UTF-8"},
  {"slovak", "sk_SK.UTF-8"},
  {"slovenian", "sl_SI.UTF-8"},
  {"slovensko", "sl_SI.UTF-8"},
  {"songhay", "son.UTF-8"},
  {"swedish", "sv_SE.UTF-8"},
  {"svenska", "sv_SE.UTF-8"},
  {"swahili", "sw_TZ.UTF-8"},
  {"tagalog", "tl_PH.UTF-8"},
  {"thai", "th_TH.UTF-8"},
  {"tibetan", "bo_CN.UTF-8"},	/* Based on: http://texinfo.org/pipermail/texinfo-pretest/2005-April/000334.html */
  {"turkish", "tr_TR.UTF-8"},
  {"twi", "twi_GH.UTF-8"},
  {"ukrainian", "uk_UA.UTF-8"},
  {"walloon", "wa_BE.UTF-8"},
  {"walon", "wa_BE.UTF-8"},
  {"wolof", "wo_SN.UTF-8"},
  {"xhosa", "xh_ZA.UTF-8"},
  {"chinese", "zh_CN.UTF-8"},
  {"simplified-chinese", "zh_CN.UTF-8"},
  {"traditional-chinese", "zh_TW.UTF-8"},
  {"zapotec", "zam.UTF-8"},
  {"miahuatlan-zapotec", "zam.UTF-8"},
  {"khmer", "km_KH.UTF-8"},
  {"malayalam", "ml_IN.UTF-8"}
};

/* FIXME: All this should REALLY be array-based!!! */
/* Show available languages: */
static void show_lang_usage(int exitcode)
{
  FILE * f = exitcode ? stderr : stdout;
  const char *const prg = "tuxpaint";
  fprintf(f,
	  "\n"
	  "Usage: %s [--lang LANGUAGE]\n" "\n" "LANGUAGE may be one of:\n"
/* C */ "  english      american-english\n"
/* af */ "  afrikaans\n"
/* ak */ "  akan        twi-fante\n"
/* sq */ "  albanian\n"
/* ar */ "  arabic\n"
/* ast */ "  asturian\n"
/* en_AU */ "  australian-english\n"
/* az */ "  azerbaijani\n"
/* eu */ "  basque       euskara\n"
/* be */ "  belarusian   bielaruskaja\n"
/* nb */ "  bokmal\n"
/* pt_BR */
	  "  brazilian    brazilian-portuguese   portugues-brazilian\n"
/* br */ "  breton       brezhoneg\n"
/* en_GB */ "  british      british-english\n"
/* bg_BG */ "  bulgarian\n"
/* en_CA */ "  canadian-english\n"
/* ca */ "  catalan      catala\n"
/* zh_CN */ "  chinese      simplified-chinese\n"
/* zh_TW */ "               traditional-chinese\n"
/* hr */ "  croatian     hrvatski\n"
/* cs */ "  czech        cesky\n"
/* da */ "  danish       dansk\n"
/* nl */ "  dutch        nederlands\n"
/* eo */ "  esperanto\n"
/* et */ "  estonian\n"
/* fo */ "  faroese\n"
/* fi */ "  finnish      suomi\n"
/* fr */ "  french       francais\n"
/* ff */ "  fula         fulah                  pulaar\n"
/* ga */ "  gaelic       irish-gaelic           gaidhlig\n"
/* gl */ "  galician     galego\n"
/* ka */ "  georgian\n"
/* de */ "  german       deutsch\n"
/* el */ "  greek\n"
/* gos */ "  gronings     zudelk-veenkelonioals\n"
/* gu */ "  gujarati\n"
/* he */ "  hebrew\n"
/* hi */ "  hindi\n"
/* hu */ "  hungarian    magyar\n"
/* is */ "  icelandic    islenska\n"
/* id */ "  indonesian   bahasa-indonesia\n"
/* it */ "  italian      italiano\n"
/* ja */ "  japanese\n"
/* rw */ "  kinyarwanda\n"
/* tlh */ "  klingon      tlhIngan\n"
/* ko */ "  korean\n"
/* ku */ "  kurdish\n"
/* lv */ "  latvian\n"
/* lt */ "  lithuanian   lietuviu\n"
/* lg */ "  luganda\n"
/* lb */ "  luxembourgish letzebuergesch\n"
/* mk */ "  macedonian\n"
/* ms */ "  malay\n"
/* ml */ "  malayalam\n"
/* es_MX */ "  mexican      mexican-spanish        espanol-mejicano\n"
/* nr */ "  ndebele\n"
/* nso */ "  northern-sotho                      sesotho-sa-leboa\n"
/* nn */ "  norwegian    nynorsk                norsk\n"
/* oc */ "  occitan\n"
/* oj */ "  ojibwe       ojibway\n"
/* fa */ "  persian\n"
/* pl */ "  polish       polski\n"
/* pt */ "  portuguese   portugues\n"
/* ro */ "  romanian\n"
/* ru */ "  russian      russkiy\n"
/* gd */ "  scottish     scottish-gaelic        ghaidhlig\n"
/* sr */ "  serbian\n"
/* shs */ "  shuswap      secwepemctin\n"
/* sk */ "  slovak\n"
/* sl */ "  slovenian    slovensko\n"
/* en_ZA */ "  southafrican-english\n"
/* son */ "  songhay\n"
/* es */ "  spanish      espanol\n"
/* sw */ "  swahili\n"
/* sv */ "  swedish      svenska\n"
/* tl */ "  tagalog\n"
/* ta */ "  tamil\n"
/* te */ "  telugu\n"
/* th */ "  thai\n"
/* twi */ "  twi\n"
/* bo */ "  tibetan\n"
/* tr */ "  turkish\n"
/* uk */ "  ukrainian\n"
/* ve */ "  venda\n"
/* vec */"   venetian     veneto\n"
/* vi */ "  vietnamese\n"
/* wa */ "  walloon      walon\n"
/* wo */ "  wolof\n"
/* cy */ "  welsh        cymraeg\n"
/* xh */ "  xhosa\n"
/* zam */"  zapotec      miahuatlan-zapotec\n"
	  "\n", prg);
  exit(exitcode);
}


/* FIXME: Add accented characters to the descriptions */
/* Show available locales: */
static void show_locale_usage(FILE * f, const char *const prg)
{
  fprintf(f,
	  "\n"
	  "Usage: %s [--locale LOCALE]\n"
	  "\n"
	  "LOCALE may be one of:\n"
	  "  C       (English      American English)\n"
	  "  af_ZA   (Afrikaans)\n"
          "  ak_GH   (Akan         Twi-Fante)\n"
	  "  ar_SA   (Arabic)\n"
	  "  ast_ES  (Asturian)\n"
	  "  az_AZ   (Azerbaijani)\n"
	  "  eu_ES   (Basque       Euskara)\n"
	  "  be_BY   (Belarusian   Bielaruskaja)\n"
	  "  nb_NO   (Bokmal)\n"
	  "  pt_BR   (Brazilian    Brazilian Portuguese   Portugues Brazilian)\n"
	  "  br_FR   (Breton       Brezhoneg)\n"
	  "  en_AU   (Australian English)\n"
	  "  en_CA   (Canadian English)\n"
	  "  en_GB   (British      British English)\n"
	  "  en_ZA   (South African English)\n"
	  "  bg_BG   (Bulgarian)\n"
	  "  ca_ES   (Catalan      Catala)\n"
	  "  zh_CN   (Chinese-Simplified)\n"
	  "  zh_TW   (Chinese-Traditional)\n"
	  "  cs_CZ   (Czech        Cesky)\n"
	  "  da_DK   (Danish       Dansk)\n"
	  "  nl_NL   (Dutch)\n"
          "  fa_IR   (Persian)\n"
          "  ff_SN   (Fula)\n"
	  "  fi_FI   (Finnish      Suomi)\n"
	  "  fo_FO   (Faroese)\n"
	  "  fr_FR   (French       Francais)\n"
	  "  ga_IE   (Irish Gaelic Gaidhlig)\n"
	  "  gd_GB   (Scottish Gaelic  Ghaidhlig)\n"
	  "  gl_ES   (Galician     Galego)\n"
	  "  gos_NL  (Gronings     Zudelk Veenkelonioals)\n"
	  "  gu_IN   (Gujarati)\n"
	  "  de_DE   (German       Deutsch)\n"
          "  eo      (Esperanto)\n"
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
	  "  ku_TR   (Kurdish)\n"
	  "  ms_MY   (Malay)\n"
          "  ml_IN   (Malayalam)\n"
          "  lg_UG   (Luganda)\n"
          "  lb_LU   (Luxembourgish Letzebuergesch)\n"
	  "  lv_LV   (Latvian)\n"
	  "  lt_LT   (Lithuanian   Lietuviu)\n"
	  "  mk_MK   (Macedonian)\n"
          "  nr_ZA   (Ndebele)\n"
          "  nso_ZA  (Northern Sotho                      Sotho sa Leboa)\n"
	  "  nn_NO   (Norwegian    Nynorsk                Norsk)\n"
	  "  oc_FR   (Occitan)\n"
	  "  oj_CA   (Ojibway)\n"
	  "  pl_PL   (Polish       Polski)\n"
	  "  pt_PT   (Portuguese   Portugues)\n"
	  "  ro_RO   (Romanian)\n"
	  "  ru_RU   (Russian      Russkiy)\n"
	  "  rw_RW   (Kinyarwanda)\n"
          "  shs_CA  (Shuswap      Secwepemctin)\n"
	  "  sk_SK   (Slovak)\n"
	  "  sl_SI   (Slovenian)\n"
          "  son     (Songhay)\n"
	  "  sq_AL   (Albanian)\n"
	  "  sr_YU   (Serbian)\n"
	  "  es_ES   (Spanish      Espanol)\n"
	  "  es_MX   (Mexican      Mexican Spanish       Espanol Mejicano)\n"
	  "  sw_TZ   (Swahili)\n"
	  "  sv_SE   (Swedish      Svenska)\n"
	  "  ta_IN   (Tamil)\n"
          "  te_IN   (Telugu)\n"
	  "  tl_PH   (Tagalog)\n"
	  "  bo_CN   (Tibetan)\n"
	  "  th_TH   (Thai)\n"
	  "  tr_TR   (Turkish)\n"
	  "  twi_GH  (Twi)\n"
	  "  uk_UA   (Ukrainian)\n"
	  "  ve_ZA   (Venda)\n"
	  "  vi_VN   (Vietnamese)\n"
	  "  wa_BE   (Walloon)\n"
	  "  wo_SN   (Wolof)\n"
	  "  cy_GB   (Welsh        Cymraeg)\n"
	  "  xh_ZA   (Xhosa)\n"
	  "  zam     (Zapoteco-Miahuatlan)\n"
	  "\n", prg);
}


int get_current_language(void)
{
  return langint;
}


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

// This is to ensure that iswprint() works beyond ASCII,
// even if the locale wouldn't normally support that.
static void ctype_utf8(void)
{
#ifndef _WIN32
  const char *names[] = {"en_US.UTF8","en_US.UTF-8","UTF8","UTF-8","C.UTF-8"};
  int i = sizeof(names)/sizeof(names[0]);
  for(;;){
    if(iswprint((wchar_t)0xf7)) // division symbol -- which is in Latin-1 :-/
      return;
    if(--i < 0)
      break;
    setlocale(LC_CTYPE,names[i]);
  }
  fprintf(stderr, "Failed to find a locale with iswprint() working!\n");
#endif
}


static const char *language_to_locale(const char *langstr)
{
  int i = sizeof language_to_locale_array / sizeof language_to_locale_array[0];
  while (i--)
  {
    if (!strcmp(langstr, language_to_locale_array[i].language))
      return language_to_locale_array[i].locale;
  }
  if (strcmp(langstr, "help") == 0 || strcmp(langstr, "list") == 0)
    show_lang_usage(0);
  fprintf(stderr, "%s is an invalid language\n", langstr);
  show_lang_usage(59);
  return NULL;
}

static void set_langint_from_locale_string(const char *restrict loc)
{
  char *baseloc = strdup(loc);
  char *dot = strchr(baseloc, '.');
  size_t len_baseloc = strlen(baseloc);
  int found = 0;
  int i;

  if (!loc)
    return;

  if(dot)
    *dot = '\0';

  /* Which, if any, of the locales is it? */

  for (i = 0; i < NUM_LANGS && found == 0; i++)
  {
    // Case-insensitive (both "pt_BR" and "pt_br" work, etc.)
    if (len_baseloc == strlen(lang_prefixes[i]) &&
        !strncasecmp(baseloc, lang_prefixes[i], strlen(lang_prefixes[i])))
    {
      langint = i;
      found = 1;
    }
  }

  for (i = 0; i < NUM_LANGS && found == 0; i++)
  {
    // Case-insensitive (both "pt_BR" and "pt_br" work, etc.)
    if (!strncasecmp(loc, lang_prefixes[i], strlen(lang_prefixes[i])))
    {
      langint = i;
      found = 1;
    }
  }
}

#define DEBUG

static int set_current_language(const char *restrict locale_choice) MUST_CHECK;
static int set_current_language(const char *restrict loc)
{
  int i;
  int y_nudge = 0;
  char * oldloc;

  oldloc = strdup(loc);

  setlocale(LC_ALL, loc);
  ctype_utf8();

  bindtextdomain("tuxpaint", LOCALEDIR);
  /* Old version of glibc does not have bind_textdomain_codeset() */
#if defined(_WIN32) || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 2) || __GLIBC__ > 2 || defined(__NetBSD__) || __APPLE__
  bind_textdomain_codeset("tuxpaint", "UTF-8");
#endif
  textdomain("tuxpaint");

#ifdef _WIN32
  if (!*loc)
    loc = _nl_locale_name(LC_MESSAGES, "");
#else
  // NULL: Used to direct setlocale() to query the current
  // internationalised environment and return the name of the locale().
  loc = setlocale(LC_MESSAGES, NULL);
#endif

  if (strcmp(loc, oldloc) != 0) {
    /* System doesn't recognize that locale!  Hack, per Albert C., is to set LC_ALL to a valid UTF-8 locale, then set LANGUAGE to the locale we want to force -bjk 2010.10.05 */

    /* Alberts comments from December 2009:
       FIXME: gettext() won't even bother to look up messages unless it
       is totally satisfied that you are using one of the locales that
       it ships with! Make gettext() unhappy, and it'll switch to the
       lobotomized 7-bit Linux "C" locale just to spite you.
       
       http://sources.redhat.com/cgi-bin/cvsweb.cgi/libc/localedata/SUPPORTED?content-type=text/x-cvsweb-markup&cvsroot=glibc
       
       You can confuse gettext() into mostly behaving. For example, a
       user could pick a random UTF-8 locale and change the messages.
       In that case, Tux Paint thinks it's in the other locale but the
       messages come out right. Like so: LANGUAGE=zam LC_ALL=fr_FR.UTF-8
       It doesn't work to leave LC_ALL unset, set it to "zam", set it to "C",
       or set it to random nonsense. Yeah, Tux Paint will think it's in
       a French locale, but the messages will be Zapotec nonetheless.
       
       Maybe it's time to give up on gettext().

       [see also: https://sourceforge.net/mailarchive/message.php?msg_name=787b0d920912222352i5ab22834x92686283b565016b%40mail.gmail.com ]
    */

    setlocale(LC_ALL, "en_US.UTF-8"); /* Is it dumb to assume "en_US" is pretty close to "C" locale? */
    setenv("LANGUAGE", oldloc, 1);
    set_langint_from_locale_string(oldloc);
  } else {
    set_langint_from_locale_string(loc);
  }

  lang_prefix = lang_prefixes[langint];

  short_lang_prefix = strdup(lang_prefix);
  /* When in doubt, cut off country code */
  if (strchr(short_lang_prefix, '_'))
    *strchr(short_lang_prefix, '_') = '\0';

  need_own_font = search_int_array(langint, lang_use_own_font);
  need_right_to_left = search_int_array(langint, lang_use_right_to_left);
  need_right_to_left_word = search_int_array(langint, lang_use_right_to_left_word);

  for (i = 0; lang_y_nudge[i][0] != -1; i++)
  {
    // printf("lang_y_nudge[%d][0] = %d\n", i, lang_y_nudge[i][0]);
    if (lang_y_nudge[i][0] == langint)
    {
      y_nudge = lang_y_nudge[i][1];
      //printf("y_nudge = %d\n", y_nudge);
      break;
    }
  }

#ifdef DEBUG
  fprintf(stderr, "DEBUG: Language is %s (%d) %s/%s\n",
	  lang_prefix, langint,
	  need_right_to_left ? "(RTL)" : "",
	  need_right_to_left_word ? "(RTL words)" : "");
  fflush(stderr);
#endif

  free(oldloc);

  printf("lang_prefixes[%d] is \"%s\"\n", get_current_language(), lang_prefixes[get_current_language()]);
  return y_nudge;
}

int setup_i18n(const char *restrict lang, const char *restrict locale)
{
  printf("lang %p, locale %p\n", lang, locale);
  printf("lang \"%s\", locale \"%s\"\n", lang, locale);

  if(locale)
  {
    if(!strcmp(locale,"help"))
    {
      show_locale_usage(stdout,"tuxpaint");
      exit(0);
    }
  }
  else
    locale = "";

  if(lang)
    locale = language_to_locale(lang);

  return set_current_language(locale);
}

#ifdef NO_SDLPANGO
int smash_i18n(void)
{
  return set_current_language("C");
}
#endif
