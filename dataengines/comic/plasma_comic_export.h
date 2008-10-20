#ifndef PLASMA_COMIC_EXPORT_H
#define PLASMA_COMIC_EXPORT_H

/* needed for KDE_EXPORT macros */
#include <kdemacros.h>

#if defined _WIN32 || defined _WIN64
#ifndef PLASMA_COMIC_EXPORT
# ifdef MAKE_PLASMACOMICPROVIDERCORE_LIB
#  define PLASMA_COMIC_EXPORT KDE_EXPORT
# else
#  define PLASMA_COMIC_EXPORT KDE_IMPORT
# endif
#endif

#else /* UNIX*/

/* export statements for unix */
#define PLASMA_COMIC_EXPORT KDE_EXPORT
#endif

#endif
