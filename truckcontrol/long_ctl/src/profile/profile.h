/* FILE
 *   profile.h
 *		For includes and defines common to all profile
 *		routines	
 *
 * Copyright (c) 2002   Regents of the University of California
 *
 */

#ifndef PROFILE_H
#define PROFILE_H

#include <sys_os.h>
#include <sys_list.h>
#include <sys/select.h>
#include <sys_lib.h>
#include <sys_rt.h>
#include "db_comm.h"
#include "db_clt.h"
#include "timestamp.h"
#include "jbus_vars.h"
#include "jbus_extended.h"
#include "j1939.h"
#include "j1939pdu_extended.h"
#include "j1939db.h"
#include <timing.h>
#include "clt_vars.h"
#include "vehicle.h"

#define MAX_PROFILE_LINE_LENGTH	256

typedef struct {
	int dbn;	/* Database variable number */
	int size;	/* Size of database variable type */
} dbv_size_type;

#endif /* PROFILE_H */
