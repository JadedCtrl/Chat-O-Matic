#ifndef _LIBMSN_EXPORT_H_
#define _LIBMSN_EXPORT_H_

/* export classes under windows
 * visibility can be added later under all other systems
*/
#ifdef _MSC_VER
// we need to include windows.h here to not get compiler errors inside excpt.h (system header)
#include <windows.h>
#pragma warning( disable : 4251 )
#pragma warning( disable : 4996 )
#endif

#ifdef _WIN32
# ifdef msn_EXPORTS
#  define LIBMSN_EXPORT __declspec(dllexport)
# else
#  define LIBMSN_EXPORT __declspec(dllimport)
#endif
#else
# define LIBMSN_EXPORT
#endif

#endif // _LIBMSN_EXPORT_H_
