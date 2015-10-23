/* FILE:  pack.c (For the Intersection Decision Support project)
 *
 * Copyright (c) 2004  Regents of the University of California
 *
 * For packing binary data into null-terminated strings compatibly
 * with the implementation in db_clt.
 *
 */

#define ESC_CHAR	'\033'	
#define NULL_CHAR	'\0'	
#define EOT_CHAR	'\004'

// Copy binary data (len bytes) into null-terminated str, escaping null/esc
// chars, as in db_clt.c. The length available in str must be at least
// 2*len+1 bytes.
void
pack_binary_data(int len, char *data, char *str)
{
    int i;

    for (i = 0; i < len; i++) {
        if (data[i] == NULL_CHAR){
            *str++ = ESC_CHAR;
            *str++ = EOT_CHAR;
        } else if (data[i] == ESC_CHAR) {
            *str++ = ESC_CHAR;
            *str++ = ESC_CHAR;
        } else
            *str++ = data[i];
    }
    *str = NULL_CHAR;
}

// Copy null-terminated str to data (up to len bytes), unescaping null/esc
// chars, as in db_clt.c. If there are not enough chars in str to fill 
// data, returns a negative number. If there are too many chars in str,
// returns 1. If the str has exactly len bytes of packed data, returns 0.
int
unpack_binary_data(char *str, int len, char *data)
{
    int i;

    for (i = 0; i < len; i++) { 
        if (*str == NULL_CHAR) {
            return i - len;
        } else if (*str == ESC_CHAR) {
            str++;
            if (*str == ESC_CHAR) {    
                data[i] = ESC_CHAR;
            } else {    /* must be EOT_CHAR */
                data[i] = NULL_CHAR;
            }
            str++;
        } else 
            data[i] = *str++;
    }
    
    return *str == NULL_CHAR ? 0 : 1;
}
