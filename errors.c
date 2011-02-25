#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include "errors.h"
#include "asc.h"
#include "init.h"
#include "io/elpathwrapper.h"
#include "threads.h"

SDL_mutex* error_mutex = 0;

void create_error_mutex()
{
	error_mutex = SDL_CreateMutex();
}

void destroy_error_mutex()
{
	SDL_DestroyMutex(error_mutex);

	error_mutex = 0;
}

FILE* open_log (const char *fname, const char *mode)
{
	FILE *file = open_file_config (fname, mode);
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
	CHECK_AND_LOCK_MUTEX(error_mutex);

	if (!err_file)
	{
		err_file = open_log ("error_log.txt", "w");
	}

	fflush (err_file);

	CHECK_AND_UNLOCK_MUTEX(error_mutex);
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

	CHECK_AND_LOCK_MUTEX(error_mutex);

	if (!strcmp(errmsg, last_error))
	{
		++repeats;

		CHECK_AND_UNLOCK_MUTEX(error_mutex);

		return;
	}

	if(repeats) fprintf(err_file, "Last message repeated %d time%c\n", repeats,(repeats>1?'s':' '));
	repeats=0;
	safe_strncpy(last_error, errmsg, sizeof(last_error));

	if(err_file == NULL)
	{
		if ( file_exists_config("error_log.txt") == 1 ) {
			/* Move it */
			file_remove_config("error_log.old");
			file_rename_config("error_log.txt","error_log.old");
		}
		err_file = open_log ("error_log.txt", "a");
	}
	time(&c_time);
	l_time = localtime(&c_time);
	strftime(logmsg, sizeof(logmsg), "[%H:%M:%S] ", l_time);
	safe_strcat(logmsg, errmsg, 512);

	if(message[strlen(message)-1] != '\n') {
		safe_strcat(logmsg, "\n", 512);
	}
	fputs(logmsg, err_file);
  	fflush (err_file);

	CHECK_AND_UNLOCK_MUTEX(error_mutex);
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

	CHECK_AND_LOCK_MUTEX(error_mutex);

	if (!strcmp(errmsg,last_error))
	{
		++repeats;

		CHECK_AND_UNLOCK_MUTEX(error_mutex);

		return;
	}

	if(repeats) fprintf(err_file, "Last message repeated %d time%c\n", repeats,(repeats>1?'s':' '));
	repeats=0;
	safe_strncpy(last_error, errmsg, sizeof(last_error));

	if(err_file == NULL)
	{
		err_file = open_log ("error_log.txt", "a");
	}
	time(&c_time);
	l_time = localtime(&c_time);
	strftime(logmsg, sizeof(logmsg), "[%H:%M:%S] ", l_time);
	strcat(logmsg, errmsg);

	if(message[strlen(message)-1] != '\n') {
		strcat(logmsg, "\n");
	}
	fputs(logmsg, err_file);
  	fflush (err_file);

	CHECK_AND_UNLOCK_MUTEX(error_mutex);
}


#ifdef EXTRA_DEBUG
FILE *func_file = NULL;
void clear_func_log()
{
	if(!func_file) {
		func_file = open_log("function_log.txt", "w");
	}
	fflush(func_file);
}

void log_func_err(const char * file, const char * func, unsigned line)
{
	CHECK_AND_LOCK_MUTEX(error_mutex);

	if(!func_file)
		clear_func_log();
	if(func_file)
		{
			fprintf(func_file,"%s.%s:%u\n",file,func,line);
			fflush(func_file);
		}

	CHECK_AND_UNLOCK_MUTEX(error_mutex);
}
#endif


FILE *conn_file = NULL;
void clear_conn_log()
{
	if(!conn_file) {
		conn_file = open_log ("connection_log.txt", "w");
	}
	fflush (conn_file);
}

void log_conn(const Uint8 *in_data, Uint16 data_length)
{
	CHECK_AND_LOCK_MUTEX(error_mutex);

  	if(!conn_file) {
		conn_file = open_log ("connection_log.txt", "a");
	}
  	fwrite (in_data, data_length, 1, conn_file);
  	fflush (conn_file);

	CHECK_AND_UNLOCK_MUTEX(error_mutex);
}

FILE *infos_file = NULL;
void log_info(const char* message, ...)
{
	va_list ap;
	char logmsg[1024];

	va_start(ap, message);
        vsnprintf(logmsg, sizeof(logmsg), message, ap);
        logmsg[sizeof(logmsg) - 1] = '\0';
	va_end(ap);

	CHECK_AND_LOCK_MUTEX(error_mutex);

	if (infos_file == NULL)
	{
		if (file_exists_config("infos.log") == 1)
		{
			/* Move it */
			file_remove_config("infos.old");
			file_rename_config("infos.log", "infos.old");
		}
		infos_file = open_log("infos.log", "a");
	}
	if (message[strlen(message)-1] != '\n')
	{
		strcat(logmsg, "\n");
	}
	fputs(logmsg, infos_file);
  	fflush(infos_file);

	CHECK_AND_UNLOCK_MUTEX(error_mutex);
}

