/* FILE:  pack.h (For the Intersection Decision Support project)
 *
 * Copyright (c) 2004  Regents of the University of California
 *
 * For packing binary data into null-terminated strings compatibly
 * with the implementation in db_clt.
 *
 */

void pack_binary_data(int len, char *data, char *str);
int unpack_binary_data(char *str, int len, char *data);
