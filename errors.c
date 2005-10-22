#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#ifdef MAP_EDITOR2
#include "../map_editor2/global.h"
#else
#include "global.h"
#endif

FILE* open_log (const char *fname, const char *mode)
{
	FILE *file = fopen (fname, mode);
	Uint8 starttime[200];
	struct tm *l_time; time_t c_time;
	if (file == NULL)
	{
		fprintf (stderr, "Unable to open log file \"%s\"\n", fname);
		exit (1);
	}
	time (&c_time);
	l_time = localtime (&c_time);
	strftime (starttime, sizeof(starttime), "\n\nLog started at %Y-%m-%d %H:%M:%S\n\n", l_time);
	fwrite (starttime, strlen(starttime), 1, file);
	return file;
}

FILE *err_file = NULL;
void clear_error_log()
{
	char error_log[256];

	snprintf(error_log, sizeof(error_log), "%serror_log.txt", configdir);
	if(!err_file) {
		err_file = open_log (error_log, "w");
	}
	fflush (err_file);
}

void log_error (const char* message, ...)
{
	va_list ap;

	if(err_file == NULL)
	{
		char error_log[256];
		snprintf (error_log, sizeof(error_log), "%serror_log.txt", configdir);
		err_file = open_log (error_log, "a");
	}
	va_start(ap, message);
		vfprintf (err_file, message, ap);
	va_end(ap);
	if(message[strlen(message)-1] != '\n') {
		fprintf(err_file, "\n");
	}
  	fflush (err_file);
}

void log_error_detailed(const Uint8 *message, const Uint8 *file, const Uint8 *func, Uint32 line, ...)
{
	Uint8	str[2048];
	va_list ap;

	if(err_file == NULL) {
		char error_log[256];
		snprintf(error_log, sizeof(error_log), "%serror_log.txt", configdir);
		err_file = open_log (error_log, "a");
	}
	snprintf(str, 2048, "%s.%s:%d - %s", file, func, line, message);

	va_start(ap, line);
		vfprintf(err_file, str, ap);
	va_end(ap);
	fprintf (err_file, "\n");
  	fflush (err_file);
}

#ifdef EXTRA_DEBUG
FILE *func_file = NULL;
void clear_func_log()
{
	if(!func_file) {
		char func_log[256];
		snprintf(func_log, sizeof(func_log), "%sfunction_log.txt", configdir);
		func_file = open_log(func_log, "w");
	}
	fflush(func_file);
}

void log_func_err(const Uint8 * file, const Uint8 * func, Uint32 line)
{
	if(!func_file)
		clear_func_log();
	if(func_file)
		{
			fprintf(func_file,"%s.%s:%d\n",file,func,line);
			fflush(func_file);
		}
}
#endif


FILE *conn_file = NULL;
void clear_conn_log()
{
	if(!conn_file) {
		char connection_log[256];
		snprintf(connection_log, sizeof(connection_log), "%sconnection_log.txt", configdir);
		conn_file = open_log (connection_log, "w");
	}
	fflush (conn_file);
}

void log_conn(const Uint8 *in_data, Uint32 data_length)
{
  	if(!conn_file) {
		char connection_log[256];
		snprintf(connection_log, sizeof(connection_log), "%sconnection_log.txt", configdir);
		conn_file = open_log (connection_log, "a");
	}
  	fwrite (in_data, data_length, 1, conn_file);
  	fflush (conn_file);
}
