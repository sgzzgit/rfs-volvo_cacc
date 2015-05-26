/* ser_dev.h - list of port assignments for OBMS serial devices
** Multiply all of the values listed below by 4 to get the actual
** baud rate: the Xtreme card requires this.
*/

#define J1939STB_SERIAL_DEVICE_NAME	"/dev/ttyS1"  /* 2400 -> 9600 at boot, then reset to 57600 */
#define GYRO_SER_PORT			"/dev/ttyS4"  /* 9600 -> 38400 */
#define LIDAR1_SER_PORT			"/dev/ttyS5"  /* 4800 -> 19200 */
#define SAFETRAC_SER_PORT		"/dev/ttyS0"  /* 4800 -> 19200 */
#define ROADSURFACE_SER_PORT	"/dev/ttyS7"  /* 2400 -> 9600 */
#define GPS_SER_PORT          "/dev/ttyS8"  /* 4800 -> 19200 */
#define J1587_SER_PORT			"/dev/ttyS10" /* 2400 -> 9600 */
#define EVT300A_SER_PORT		"/dev/ttyS11" /* 4800 -> 19200 */
