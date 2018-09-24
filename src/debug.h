/* #define DEBUG */
/* #define VERBOSE */

/*
* Enable verbose logging if requested on platforms that support it.
*
* Verbose logging adds metadata to printf, including the source file location
* from where printf was called and the time it was called at runtime.
*/
#if defined(DEBUG) && defined(VERBOSE) && defined(__GNUC__)
#include <stdio.h>
#include <time.h>

#define printf(args...) do { \
    time_t now = time(NULL); \
    printf("\n### %s, %d @ %s", __FILE__, __LINE__, ctime(&now)); \
    printf(args); \
} while(0)
#endif
