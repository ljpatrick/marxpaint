/*
  get_fname.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "get_fname.h"
#include "debug.h"

char * savedir;


/* The filename for the current image: */

char * get_fname(const char * const name)
{
  char f[512];
  const char * tux_settings_dir;


  /* Where is the user's data directory?
     This is where their saved files are stored,
     local fonts, brushes and stamps can be found,
     and where the "current_id.txt" file is saved */

#ifdef WIN32
  /* Windows predefines "savedir" as:
     "C:\Documents and Settings\%USERNAME%\Application Data\TuxPaint"
     though it may get overridden with "--savedir" option */

  snprintf(f, sizeof(f), "%s/%s", savedir, name);

#elif __BEOS__
  /* BeOS similarly predefines "savedir" as "./userdata"... */

  if (*name == '\0')
    strcpy(f, savedir);
  else
    snprintf(f, sizeof(f), "%s/%s", savedir, name);

#elif __APPLE__
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

  return strdup(f);
}

