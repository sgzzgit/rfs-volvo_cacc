#include <stdio.h>
#include <stddef.h>
#include <libconffile.h>
#include <getopt.h>
#include <signal.h>
#include "libeloop.h"
#include "wmefwd.h"

ConfFileParseData_t config[] = {
/*   Name            , Type  , NULL, NULL, value offset                       , -1, -1, def, min, max, incr, def_string */
	{"Enabled"       , CF_INT, NULL, NULL, offsetof (struct config, enabled  ), -1, -1,   1,   0,   1,    1, NULL},
	{"MsgName"       , CF_STR, NULL, NULL, offsetof (struct config, name     ), -1, -1,   0,   0,   0,    0, "WME_FWD"},
	{"PSC"           , CF_STR, NULL, NULL, offsetof (struct config, psc      ), -1, -1,   0,   0,   0,    0, "PSC"},
	{"ChannelMode"   , CF_INT, NULL, NULL, offsetof (struct config, mode     ), -1, -1,   1,   0,   1,    1, NULL},  
	{"Direction"     , CF_INT, NULL, NULL, offsetof (struct config, direction), -1, -1,   1,   0,   1,    1, NULL},  
	{"ChannelNumber" , CF_INT, NULL, NULL, offsetof (struct config, channel  ), -1, -1, 172, 170, 184,    1, NULL},  
	{"InterfaceName" , CF_STR, NULL, NULL, offsetof (struct config, iface    ), -1, -1,   0,   0,   0,    0, "ath0"},
	{"PSID"          , CF_PSID, NULL, NULL, offsetof (struct config, PSID     ), -1, -1, 0x10,  0, 0xFFFF, 0, NULL},
	{"RemoteIP"      , CF_STR, NULL, NULL, offsetof (struct config, ip       ), -1, -1,   0,   0,   0,    0, "127.0.0.1"},
	{"RemotePort"    , CF_INT, NULL, NULL, offsetof (struct config, port     ), -1, -1, 9999, 0 , 65536,1, NULL},
	{"MsgLength"     , CF_INT, NULL, NULL, offsetof (struct config, length   ), -1, -1, 128,  32, 4096,    1, NULL},
	{"RemoteProtocol", CF_STR, NULL, NULL, offsetof (struct config, protocol ), -1, -1,   0,   0,   0,    0, "UDP"},
	{0},
};

void socket_main ();
int wme_main ();
void end_application(int); // , void *, void*);

void usage(char *name) {
	fprintf(stderr, "usage: ./%s -f </path/to/config/file>\n", name);
	exit(0);
}

struct config config_data;

int main(int argc, char *argv[]) {
	conffile_type type = CONFFILE_PARSE_TYPE_HASH;
	char *file;
	int opt;
	int ret;

	if (argc == 1) usage(argv[0]);

	/* signal (SIGINT, end_application); */

	while ((opt = getopt(argc, argv, "df:h")) != -1) {
		switch (opt) {
			case 'f':
				file = optarg;
				break;
			case 'd':
				debug_flag = 1;
				break;
			case 'h':
			default:
				usage(argv[1]);
		}
	}

	ret = conffile_parse(file, config, &config_data, type);
	if (ret) {
		conffile_free(config, &config_data);
		return -1;
	}

	if (config_data.enabled == 0) {
		fprintf (stderr, "Set Enabled flag to 1 in the config file to use this configuration\n");
		exit (0);
	}

	if (!debug_flag) {
		ret = fork();
		if (ret < 0) {
			perror ("Couldn't send application to background. Exiting...\n");
			exit(-1);
		} else if (ret > 0) {
			exit (0);
		} else {
			fprintf (stderr, "Going to background...\n");
		}
	}

	log ("MsgName   : %s\n", config_data.name);
	log ("PSC       : %s\n", config_data.psc);
	log ("Chan Mode : %d\n", config_data.mode);
	log ("Direction : %d\n", config_data.direction);
	log ("Channel   : %d\n", config_data.channel);
	log ("Interface : %s\n", config_data.iface);
	log ("IP        : %s\n", config_data.ip);
	log ("Port      : %d\n", config_data.port);
	log ("Protocol  : %s\n", config_data.protocol);
	log ("PSID      : 0x%X\n", config_data.PSID);
	log ("MsgLength : %d\n", config_data.length);

	socket_main ();
	wme_main ();
	conffile_free(config, &config_data);

	return 0;
}
