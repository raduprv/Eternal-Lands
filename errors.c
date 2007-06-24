#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#ifdef MAP_EDITOR2
#include "../map_editor2/global.h"
#else
#include "global.h"
#endif

FILE* open_log (const char *fname, const char *mode)
{
#ifndef NEW_FILE_IO
	FILE *file = fopen (fname, mode);
#else /* NEW_FILE_IO */
	FILE *file = open_file_config (fname, mode);
#endif /* NEW_FILE_IO */
	char starttime[200], sttime[200];
	struct tm *l_time; time_t c_time;
	if (file == NULL)
	{
		fprintf (stderr, "Unable to open log file \"%s\"\n", fname);
		exit (1);
	}
	time (&c_time);
	l_time = localtime (&c_time);
	strftime(sttime, sizeof(sttime), "\n\nLog started at %Y-%m-%d %H:%M:%S localtime", l_time);
	safe_snprintf(starttime, sizeof(starttime), "%s (%s)\n\n", sttime, tzname[l_time->tm_isdst>0]);
	fwrite (starttime, strlen(starttime), 1, file);
	return file;
}

FILE *err_file = NULL;
void clear_error_log()
{
#ifndef NEW_FILE_IO
	char error_log[256];

	safe_snprintf(error_log, sizeof(error_log), "%serror_log.txt", configdir);
#endif /* not NEW_FILE_IO */
	if(!err_file) {
#ifndef NEW_FILE_IO
		err_file = open_log (error_log, "w");
#else /* NEW_FILE_IO */
		err_file = open_log ("error_log.txt", "w");
#endif /* NEW_FILE_IO */
	}
	fflush (err_file);
}

char last_error[512];
int repeats =0;

void log_error (const char* message, ...)
{
	va_list ap;
	struct tm *l_time; time_t c_time;
	char logmsg[512];
	char errmsg[512];
	va_start(ap, message);
        vsnprintf(errmsg, sizeof(errmsg), message, ap);
        errmsg[sizeof(errmsg) - 1] = '\0';
	va_end(ap);
	if(!strcmp(errmsg,last_error)){
		++repeats;
		return;
	}
	if(repeats) fprintf(err_file, "Last message repeated %d time%c\n", repeats,(repeats>1?'s':' '));
	repeats=0;
	safe_strncpy(last_error, errmsg, sizeof(last_error));

	if(err_file == NULL)
	{
#ifndef NEW_FILE_IO
		char error_log[256];
		safe_snprintf (error_log, sizeof(error_log), "%serror_log.txt", configdir);
		err_file = open_log (error_log, "a");
#else /* NEW_FILE_IO */
		err_file = open_log ("error_log.txt", "a");
#endif /* NEW_FILE_IO */
	}
	time(&c_time);
	l_time = localtime(&c_time);
	strftime(logmsg, sizeof(logmsg), "[%H:%M:%S] ", l_time);
	strcat(logmsg, errmsg);

	if(message[strlen(message)-1] != '\n') {
		strcat(logmsg, "\n");
	}
	fprintf(err_file, logmsg);
  	fflush (err_file);
}

void log_error_detailed(const char *message, const char *file, const char *func, unsigned line, ...)
{
	va_list ap;
	struct tm *l_time; time_t c_time;
	char logmsg[2048];
	char errmsg[2048];
	va_start(ap, line);
        vsnprintf(logmsg, sizeof(logmsg), message, ap);
        logmsg[sizeof(logmsg) - 1] = '\0';
	va_end(ap);
	safe_snprintf(errmsg, sizeof(errmsg), "%s.%s:%u - %s", file, func, line, logmsg);
	if(!strcmp(errmsg,last_error)){
		++repeats;
		return;
	}
	if(repeats) fprintf(err_file, "Last message repeated %d time%c\n", repeats,(repeats>1?'s':' '));
	repeats=0;
	safe_strncpy(last_error, errmsg, sizeof(last_error));

	if(err_file == NULL)
	{
#ifndef NEW_FILE_IO
		char error_log[256];
		safe_snprintf (error_log, sizeof(error_log), "%serror_log.txt", configdir);
		err_file = open_log (error_log, "a");
#else /* NEW_FILE_IO */
		err_file = open_log ("error_log.txt", "a");
#endif /* NEW_FILE_IO */
	}
	time(&c_time);
	l_time = localtime(&c_time);
	strftime(logmsg, sizeof(logmsg), "[%H:%M:%S] ", l_time);
	strcat(logmsg, errmsg);

	if(message[strlen(message)-1] != '\n') {
		strcat(logmsg, "\n");
	}
	fprintf(err_file, logmsg);
  	fflush (err_file);
}


#ifdef EXTRA_DEBUG
FILE *func_file = NULL;
void clear_func_log()
{
	if(!func_file) {
#ifndef NEW_FILE_IO
		char func_log[256];
		safe_snprintf(func_log, sizeof(func_log), "%sfunction_log.txt", configdir);
		func_file = open_log(func_log, "w");
#else /* NEW_FILE_IO */
		func_file = open_log("function_log.txt", "w");
#endif /* NEW_FILE_IO */
	}
	fflush(func_file);
}

void log_func_err(const char * file, const char * func, unsigned line)
{
	if(!func_file)
		clear_func_log();
	if(func_file)
		{
			fprintf(func_file,"%s.%s:%u\n",file,func,line);
			fflush(func_file);
		}
}
#endif


FILE *conn_file = NULL;
void clear_conn_log()
{
	if(!conn_file) {
#ifndef NEW_FILE_IO
		char connection_log[256];
		safe_snprintf(connection_log, sizeof(connection_log), "%sconnection_log.txt", configdir);
		conn_file = open_log (connection_log, "w");
#else /* NEW_FILE_IO */
		conn_file = open_log ("connection_log.txt", "w");
#endif /* NEW_FILE_IO */
	}
	fflush (conn_file);
}

void log_conn(const Uint8 *in_data, Uint16 data_length)
{
  	if(!conn_file) {
#ifndef NEW_FILE_IO
		char connection_log[256];
		safe_snprintf(connection_log, sizeof(connection_log), "%sconnection_log.txt", configdir);
		conn_file = open_log (connection_log, "a");
#else /* NEW_FILE_IO */
		conn_file = open_log ("connection_log.txt", "a");
#endif /* NEW_FILE_IO */
	}
  	fwrite (in_data, data_length, 1, conn_file);
  	fflush (conn_file);
}
