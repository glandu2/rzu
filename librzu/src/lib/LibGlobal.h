#ifndef LIBGLOBAL_H
#define LIBGLOBAL_H

#ifdef _WIN32
#  define SYMBOL_IMPORT __declspec(dllimport)
#  define SYMBOL_EXPORT __declspec(dllexport)
#elif __GNUC__ >= 4
#  define SYMBOL_IMPORT __attribute__((visibility("default")))
#  define SYMBOL_EXPORT __attribute__((visibility("default")))
#else
#  define SYMBOL_IMPORT
#  define SYMBOL_EXPORT
#endif

#ifdef _MSC_VER
#    pragma warning(disable: 4251) /* class 'A' needs to have dll interface for to be used by clients of class 'B'. dll and users must use same crt dll and compiler flags. */
#    pragma warning(disable: 4200) /* nonstandard extension used : zero-sized array in struct/union */
#endif

#ifdef __GNUC__
#define PRINTFCHECK(formatidx, argsidx) __attribute__((format (printf, formatidx, argsidx)))
#else
#define PRINTFCHECK(formatidx, argsidx)
#endif

#if !defined(_MSC_VER) && defined(__GNUC__) && (__GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 7))
#define override
#endif

#endif // LIBGLOBAL_H
