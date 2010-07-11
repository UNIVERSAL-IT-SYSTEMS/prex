
#if defined(__arm__)
#include "arm/types.h"
#elif defined (__x86__)
#include "x86/types.h"
#elif defined (__ppc__)
#include "ppc/types.h"
#elif defined (__mips__)
#include "mips/types.h"
#elif defined (__sh__)
#include "sh/types.h"
#elif defined (__h8300s__)
#include "h8300s/types.h"
#else
#error architecture not supported
#endif
