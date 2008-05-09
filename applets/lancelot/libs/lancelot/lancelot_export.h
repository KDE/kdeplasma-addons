
#ifndef LANCELOT_EXPORT_H_
#define LANCELOT_EXPORT_H_

/* needed for KDE_EXPORT and KDE_IMPORT macros */
#include <kdemacros.h>

#ifndef LANCELOT_EXPORT
# if defined(MAKE_LANCELOT_LIB)
   /* We are building this library */
#  define LANCELOT_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define LANCELOT_EXPORT KDE_IMPORT
# endif
#endif

#ifndef LANCELOT_EXPORT_DEPRECATED
# define LANCELOT_EXPORT_DEPRECATED KDE_DEPRECATED LANCELOT_EXPORT
#endif

// Override annotation. Does nothing.
#define L_Override

// Include annotation. Does nothing. Needed by PUCK module generator
#define L_INCLUDE(A)

#endif // LANCELOT_EXPORT_H_

