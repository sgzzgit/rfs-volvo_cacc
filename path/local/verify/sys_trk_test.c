/**\file
 *
 *      Verification program for routines in sys_trk.c
 */

#include <sys_os.h>

#include "local.h"
#include "sys_list.h"
#include "sys_trk.h"

int main( int argc, char *argv[] )
{
	bool_typ status;
	float value, deriv;
	trk_profile_typ *ptrack;

	if( (ptrack = trk_init( DEFAULT_TRACK_FILE, 0.0 )) == NULL )
	{
		fprintf( stderr, "trk_init()\n" );
		return( EXIT_FAILURE );
	}

	do
	{
		status = trk_run( ptrack, &value, &deriv );
	}
	while( status == TRUE );
		
	trk_done( ptrack );
	return( EXIT_SUCCESS );
}
