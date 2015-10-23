#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wmefwd.h>
#include <stdlib.h>

int debug_flag = 0;

void log (char *fmt, ...) {
	static FILE *logfp = NULL;
	static char filename[100];
	static va_list myargs;
	va_start (myargs, fmt);

	if (NULL == logfp) {
		time_t curtime;
		struct tm *tm;
		sprintf (filename, "/nojournal/bin/log/wmefwd_%s.log", config_data.name);
		fprintf (stderr, "writing log to file %s\n", filename);
		if ((logfp = fopen (filename, "a")) < 0) {
			fprintf (stderr, "Couldn't open log file\n");
			exit (-1);
		}
		curtime = time (NULL);
		tm = localtime (&curtime);
		fprintf (logfp, "Logging start time : [%02d-%02d][%02d:%02d:%02d]\n", 
				tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
	}

	if (ftell (logfp) > LOG_FILE_SIZE) freopen (filename, "w", logfp);

	vfprintf (logfp, fmt, myargs);
	if (debug_flag)
		vfprintf (stderr,fmt, myargs);
	va_end (myargs);
	fflush (logfp);
}

void logt (char *fmt, ...) {
	static va_list myargs;
	va_start (myargs, fmt);
}
