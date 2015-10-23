#ifndef __UTIL_H__
#define __UTIL_H__

/** \file util.h
 *
 */

#include <unistd.h>

#ifndef MIN
#define MIN(x,y) (x>y?y:x)
#endif

#ifndef MAX
#define MAX(x,y) (x<y?y:x)
#endif

void hexdump(void *ptr, size_t len);

#endif // __UTIL_H__
