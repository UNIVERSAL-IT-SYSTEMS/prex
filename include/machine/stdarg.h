
#if defined(__arm__)
#include "arm/stdarg.h"
#elif defined (__x86__)
#include "x86/stdarg.h"
#elif defined (__ppc__)
#include "ppc/stdarg.h"
#elif defined (__mips__)
#include "mips/stdarg.h"
#elif defined (__sh__)
#include "sh/stdarg.h"
#elif defined (__h8300s__)
#include "h8300s/stdarg.h"
#else
#error architecture not supported
#endif
