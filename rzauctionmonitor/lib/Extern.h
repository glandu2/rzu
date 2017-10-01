#include "LibGlobal.h"

/* Windows - set up dll import/export decorators. */
#if defined(BUILDING_LIBRZAUCTION)
/* Building shared library. */
#define RZAUCTION_EXTERN SYMBOL_EXPORT
#elif defined(USING_RZAUCTION_SHARED)
/* Using shared library. */
#define RZAUCTION_EXTERN SYMBOL_IMPORT
#else
/* Building static library. */
#define RZAUCTION_EXTERN /* nothing */
#endif
