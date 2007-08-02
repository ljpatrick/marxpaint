/*
  get_fname.h
*/

#ifndef GET_FNAME_H
#define GET_FNAME_H

extern char *savedir;
extern char *datadir;

enum {
  DIR_SAVE,
  DIR_DATA
};

char *get_fname(const char *const name, int kind);


#endif
