#cmakedefine01 HAVE_NetworkManagerQt

#include <QtGlobal>

#if HAVE_NetworkManagerQt
#define SUPPORT_METERED_DETECTION 1
#elif QT_VERSION >= QT_VERSION_CHECK(6, 3, 0)
#define SUPPORT_METERED_DETECTION 1
#else
#define SUPPORT_METERED_DETECTION 0
#endif
