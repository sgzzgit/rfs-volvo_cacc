/* gyro.h - variable definitions for KVH E-Core 2000 Fiber Optic Gyro
*/

#define DB_GYRO_TYPE             319  /* gyro_typ */
#define DB_GYRO_VAR              DB_GYRO_TYPE

/* Following is the definition for DB_GYRO_TYPE, DB_GYRO_VAR in the
 * database. */
typedef struct
{
	float gyro_rate;            /* Gyro rate in degrees/sec for KVH E-Core
	                             * 2000 Gyro */
} gyro_typ;

