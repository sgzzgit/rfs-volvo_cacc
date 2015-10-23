#include "libgps.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libeloop.h>

#define MAX_LENGTH 256

struct gps_data_t *gps_handler;
int signalled;

void
close_gps(int signum)
{
    signalled = 1;
    if (savari_gps_close(gps_handler) < 0) {
        printf("failed to close handler\n");
    }
}

static void
gps_recv_callback(int udp_socket, void *eloop_context, void *socket_context)
{
    savari_gps_data_t *sgd = (savari_gps_data_t *)eloop_context;
    struct gps_data_t *gd = (struct gps_data_t *)socket_context;

    if (savari_gps_recv_async(sgd, gd)) { // if changed
        printf("POS %f %f %f %f %f %s\n", sgd->time, sgd->longitude,
                               sgd->latitude, sgd->speed, sgd->heading,
                               sgd->utc_time);
        printf("DOP %f %f %f %f %f\n", sgd->pdop, sgd->hdop, sgd->vdop, sgd->tdop, sgd->gdop);
    }
}

int
main (int argc, char **argv)
{
    int get_baud = 0;
    int set_baud = 0;
    int get_name = 0;
    int is_async = 0;
    int fd;
    int opt;
    int ret;

    while ((opt = getopt(argc, argv, "a:bB:g")) != -1) {
        switch(opt) {
        case 'a':
            is_async = atoi(optarg);
            break;
        case 'b':
            get_baud = 1;
            break;
        case 'B':
            set_baud = atoi(optarg);
            break;
        case 'g':
            get_name = 1;
            break;
        default:
            return 0;
        }
    }

    signal(SIGINT, close_gps);

    savari_gps_data_t gpsdata;

    gps_handler = savari_gps_open(&fd, is_async);
    if (gps_handler == 0) {
        printf("sorry no gpsd running\n");
        return 0;
    }
    
    memset(&gpsdata, 0, sizeof(savari_gps_data_t));

    if (get_baud) {
        int baudrate;
        baudrate = savari_get_gpsdev_baud(gps_handler);
        if (baudrate == FAIL) {
            printf("can't get the baudrate\n");
        } else {
            printf("%d\n", baudrate);
        }
    }

    if (set_baud) {
        int ret = 0;
        ret = savari_set_gpsdev_baud(gps_handler, set_baud);
        if (ret == FAIL) {
            printf("can't set the baudrate\n");
        } else {
            printf("baudrate set\n");
        }
    }

    if (get_name) {
        char name[MAX_LENGTH];
        savari_get_gpsdev_name(gps_handler, name);
    }

    if (is_async) {
        //initialise eloop
        ret = eloop_init(NULL);
        if (ret) {
            return ret;
        }

        eloop_register_read_sock(fd, gps_recv_callback, &gpsdata, gps_handler);
        eloop_run();
    } else {
        while (signalled == 0) {
            savari_gps_read(&gpsdata, gps_handler);
            sleep(1);
            printf("POS %f %f %f %f %f %s\n", gpsdata.time, gpsdata.longitude,
                                   gpsdata.latitude, gpsdata.speed, gpsdata.heading,
                                   gpsdata.utc_time);
            printf("DOP %f %f %f %f %f\n", gpsdata.pdop, gpsdata.hdop, gpsdata.vdop, gpsdata.tdop, gpsdata.gdop);
        }
    }
    return 0;
}
