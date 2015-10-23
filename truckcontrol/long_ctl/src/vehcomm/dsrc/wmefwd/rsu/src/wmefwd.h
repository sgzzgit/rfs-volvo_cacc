#ifndef __WMEAPP_H__
#define __WMEAPP_H__

#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>

#define LOG_FILE_SIZE (5 * 1024 * 1024)

struct config {
	char *name;
	int  mode;
	int  direction;
	int  channel;
	char *iface;
	char *ip;
	int  port;
	char *protocol;
	int  PSID;
	char *psc;
	int length;
	int enabled;
};

typedef enum {
	OUTGOING,
	INCOMING,
} direction;

typedef enum {
	ALTERNATING,
	CONTINUOUS,
} channelMode;

void socket_main ();
void setSocketData (uint8_t *);
void getSocketData (uint8_t *);

int wme_main ();
void log (char *fmt, ...);
void end_application(int , void *, void *);
uint8_t *get_mac (char *);

extern struct config config_data;
extern int debug_flag;
extern int sockfd;
#endif
