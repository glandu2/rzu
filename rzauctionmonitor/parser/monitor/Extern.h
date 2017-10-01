#include "LibGlobal.h"

/* Windows - set up dll import/export decorators. */
#if defined(BUILDING_LIBRZAUCTIONWATCHER)
/* Building shared library. */
#define RZAUCTIONWATCHER_EXTERN SYMBOL_EXPORT
#elif defined(USING_RZAUCTIONWATCHER_SHARED)
/* Using shared library. */
#define RZAUCTIONWATCHER_EXTERN SYMBOL_IMPORT
#else
/* Building static library. */
#define RZAUCTIONWATCHER_EXTERN /* nothing */
#endif
