/**
 * @file libgps.h
 * @brief This file contains the APIs and the structures used to
          to communicate with the GPS daemon.
          Link with -lgpsapi, -lgps and -luClibc++.
          See sample program for details.
 *
 */
#ifndef __LIBGPS_H__
#define __LIBGPS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <gps.h>

#define MAX_TIME_STR 256
#define MAX_LINE 256
#define MPS_TO_KPH 3.6
#define MAX_QUERY_STR    1024
/**
 * status returned if an API in this library succeeds
 */
#define SUCCESS            0
/**
 * status returned if an API in this library failes
 */
#define FAIL            -1

/**
 * savari_gps_data_t - contains gps data
 */
typedef struct gps_data {
    /**
     * UTC time in double format
     */
    double time;
    /**
     * mode of fix 
     * - 1 -> no fix
     * - 2 -> 2D fix
     * - 3 -> 3d fix 
     */
    int mode;
    /**
     * UTC time in string format
     */
    char utc_time[MAX_TIME_STR];
    /**
     * Latitude in degrees
     */
    double latitude;
    /**
     * Longitude in degrees
     */
    double longitude;
    /**
     * Altitude in meters
     */
    double altitude;
    /**
     * Milliseconds in the current minute when the last fix was obtained
     */
    int dsecond;
    /**
     * KPH speed
     */
    double speed;
    /**
     * Heading in degrees
     */
    double heading;
    
     /* TBD: need to comment below params */
     /* Not requried for basic gps data, ex. mode, lat, long */
    double semi_major_deviation;
    double semi_minor_deviation;
    double semi_major_orientation;
    double lonaccel;
    double lataccel;
    double vertaccel;
    double yawrate;
    int    gps_update_hz;
    double linear_accel_filter_cutoff_hz;
    double linear_accel_filter_damp_factor;
    double angular_accel_filter_cutoff_hz;
    double angular_accel_filter_damp_factor;

    // internal - should not be used by apps
    void                *yawrate_filter;
    void                *lacceleration_filter;
    void                *vspeed_filter;
    void                *vacceleration_filter;
    double pdop;
    double hdop;
    double vdop;
    double tdop;
    double gdop;
} savari_gps_data_t;

//#define savari_gps_fd(gpshandle) gpshandle->gps_fd
/**
 * @brief opens a connection to GPS daemon
 *
 * This function creates a connection to the gps data and
 * return a handle that is used to communicate with the gps daemon
 *
 * \param gps_fd - if not NULL, returns file descriptor, that can be
 * used to wait on select, to get the gps data.
 * \param is_async - specifies whether application uses async method.
 * Async method is the one where the app gets the gps notification
 * instead of requesting the gps for the gps information.
 * \return returns a handler to the GPS daemon if connection
 * succeeds, or NULL if connection fails.
 */
struct gps_data_t* savari_gps_open(int *gps_fd, int is_async);
/**
 * @brief recevies a message from GPS daemon
 *
 * This function recieves the gps data and fills into the gps_data
 * structure.
 *
 * Called only when there is write on the gps_fd, means that gpsd has
 * responded back with the gps data.
 *
 * caller should use select (2) mechanism to wait on the gps_fd for
 * gps data
 *
 * \param gps_data - library fills up this structure, application
 *                       should pass a valid pointer. the GPS info is
 *                       filled in to this structure.
 * \param gpshandler - the pointer returned by savari_gps_open
 * \return  returns 1 if GPS fix changed, 0 if fix didn't change
 */
int savari_gps_recv_async ( savari_gps_data_t *gps_data,
                                struct gps_data_t *gpshandler);
/**
 * @brief gets the GPS data from the GPS daemon
 *
 * This function reads data from the gps daemon. This involves
 * writing a query to the gpsd socket, and waiting on the socket
 * for the response and processing the response back.
 *
 * \param gps_data - library fills up this structure, application
 *                       should pass a valid pointer. the GPS info is
 *                       filled in to this structure.
 * \param gpshandler - the pointer returned by savari_gps_open
 * \return   returns SUCCESS when GPS data has been read, else
 *           FAIL when GPS data not available.
 */
int savari_gps_read(savari_gps_data_t *gps_data,
                struct gps_data_t *gpshandler);
/**
 * @brief closes the GPS daemon connection
 *
 * \param gpshandler - the pointer returned by savari_gps_open
 * \return  returns SUCCESS on success, FAIL on failure.
 */
int savari_gps_close(struct gps_data_t *gpshandler);
/**
 * @brief gets the GPS device name
 *
 * This function returns the GPS device name into the name field.
 * In case if the name couldn't be found, then the name field will
 * be an empty string (first byte being '\0').
 *
 * \param gpshandler - the pointer returned by savari_gps_open
 * \param name - a valid pointer from application,
                 returns a name of GPS device
 * \return  none.
 */
void savari_get_gpsdev_name(struct gps_data_t *gpshandler, char *name);
/**
 * @brief sets the GPS device baud rate
 *
 * \param gpshandler - the pointer returned from savari_gps_open
 * \param baud - baud to be set
 * \return   returns SUCCESS if the baud is set,
 *           and FAIL if the baud can't be set
 */
int savari_set_gpsdev_baud(struct gps_data_t *gpshandler, int baud);
/**
 * @brief gets the GPS device baud rate
 * \param gpshandler - the pointer returned from savari_gps_open
 * \return - returns baud on success, FAIL on failure.
 */
int savari_get_gpsdev_baud(struct gps_data_t *gpshandler);

#ifdef __cplusplus
}
#endif

#endif
