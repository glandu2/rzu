#include "LibGlobal.h"

/* Windows - set up dll import/export decorators. */
#if defined(BUILDING_LIBRZTEST)
/* Building shared library. */
#define RZTEST_EXTERN SYMBOL_EXPORT
#elif defined(USING_RZTEST_SHARED)
/* Using shared library. */
#define RZTEST_EXTERN SYMBOL_IMPORT
#else
/* Building static library. */
#define RZTEST_EXTERN /* nothing */
#endif
