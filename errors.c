#include <stdio.h>
#include <string.h>
#include <time.h>
#include "global.h"

FILE* open_log (const char *fname, const char *mode)
{
    FILE *file = fopen (fname, mode);
	Uint8 starttime[200];
	time_t *l_time;
    if (!file)
    {
        fprintf (stderr, "Unable to open log file \"%s\"\n", fname);
        exit (1);
    }
	time(&l_time);
	l_time = localtime(&l_time);
	strftime(starttime, sizeof(starttime), "\n\nLog started at %Y-%m-%d %H:%M:%S\n\n", l_time);
	fwrite (starttime, strlen(starttime), 1, file);
	return file;
}

FILE *err_file = NULL;
void clear_error_log()
{
	char error_log[256];

	strncpy(error_log, configdir,sizeof(error_log));
	strncat(error_log, "error_log.txt",sizeof(error_log));
	if(!err_file) err_file = open_log (error_log, "w");
	fflush (err_file);
}

void log_error(const Uint8 * message)
{
	Uint8	str[2048];
	Uint32	len;
	char error_log[256];

	strncpy(error_log, configdir,sizeof(error_log));
	strncat(error_log, "error_log.txt",sizeof(error_log));
  	if(!err_file) err_file = open_log (error_log, "a");
	if(strncmp(message, "Error", 5))	// do we need to add Error:?
		{
			snprintf(str, 2048, "%s: %s\n", reg_error_str, message);
		}
	else
		{
			snprintf(str, 2048, "%s\n", message);
		}
	len=strlen(str);
	if(str[len-2] == '\n') len--;	// remove excess newline
  	fwrite (str, len, 1, err_file);
  	fflush (err_file);
}

void log_error_detailed(const Uint8 *message, const Uint8 *file, const Uint8 *func, Uint32 line)
{
	Uint8	str[2048];
	Uint32	len;
	char error_log[256];

	strncpy(error_log, configdir,sizeof(error_log));
	strncat(error_log, "error_log.txt",sizeof(error_log));
  	if(!err_file) err_file = open_log (error_log, "a");
	snprintf(str, 2048, "Error: %s.%s:%d - %s\n", file, func, line, message);
	len=strlen(str);
	if(str[len-2] == '\n') len--;	// remove excess newline
  	fwrite (str, len, 1, err_file);
  	fflush (err_file);
}

#ifdef EXTRA_DEBUG
FILE *func_file = NULL;
void clear_func_log()
{
        char func_log[256];

	strncpy(func_log, configdir,sizeof(func_log));
	strncat(func_log, "function_log.txt",sizeof(func_log));
	if(!func_file) func_file = open_log(func_log, "w");
	fflush(func_file);
}

void log_func_err(const Uint8 * file, const Uint8 * func, Uint32 line)
{
	if(!func_file) clear_func_log();
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
	char connection_log[256];

	strncpy(connection_log, configdir,sizeof(connection_log));
	strncat(connection_log, "connection_log.txt",sizeof(connection_log));
	if(!conn_file) conn_file = open_log (connection_log, "w");
	fflush (conn_file);
}

void log_conn(const Uint8 *in_data, Uint32 data_lenght)
{
	char connection_log[256];

	strncpy(connection_log, configdir,sizeof(connection_log));
	strncat(connection_log, "connection_log.txt",sizeof(connection_log));
  	if(!conn_file) conn_file = open_log (connection_log, "a");
  	fwrite (in_data, data_lenght, 1, conn_file);
  	fflush (conn_file);
}
