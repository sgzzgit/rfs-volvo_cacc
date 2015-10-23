#include "util.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

void hexdump(void *ptr, size_t len)
{
    int i,j,min;
    char *buf, *cur;
    unsigned char *s = ptr;
    
    cur = buf = malloc((len/16 + 1)*80);
    
    for (i = 0; i < len; i += 16) {
        cur += sprintf(cur, "%08x ", i);
        for (j = 0; j < 8; j++) {
            if (i+j < len)
                cur += sprintf(cur, " %02x", s[i+j]);
            else
                cur += sprintf(cur, "   ");
        }
        cur += sprintf(cur, " ");
        for (j = 8; j < 16; j++) {
            if (i+j < len)
                cur += sprintf(cur, " %02x", s[i+j]);
            else
                cur += sprintf(cur, "   ");
        }
        cur += sprintf(cur, "  |");
        min = MIN(len-i, 16);
        for (j = 0; j < min; j++) {
            cur += sprintf(cur, "%c", isprint(s[i+j]) ? s[i+j] : '.');
        }
        cur += sprintf(cur, "|\n");
    }
    
    fprintf(stderr, buf);
    free(buf);
}
