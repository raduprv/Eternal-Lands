#include <string.h>
#include "global.h"

FILE *err_file = NULL;
void clear_error_log()
{
	char error_log[256];

	strcpy(error_log, configdir);
	strcat(error_log, "error_log.txt");
	if(!err_file) err_file = fopen (error_log, "wb");
	fflush (err_file);
}

void log_error(const Uint8 * message)
{
	Uint8	str[2048];
	Uint32	len;
	char error_log[256];

	strcpy(error_log, configdir);
	strcat(error_log, "error_log.txt");
  	if(!err_file) err_file = fopen (error_log, "ab");
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

	strcpy(error_log, configdir);
	strcat(error_log, "error_log.txt");
  	if(!err_file) err_file = fopen (error_log, "ab");
	snprintf(str, 2048, "Error: %s.%s:%d - %s\n", file, func, line, message);
	len=strlen(str);
	if(str[len-2] == '\n') len--;	// remove excess newline
  	fwrite (str, len, 1, err_file);
  	fflush (err_file);
}


FILE *conn_file = NULL;
void clear_conn_log()
{
	char connection_log[256];

	strcpy(connection_log, configdir);
	strcat(connection_log, "connection_log.txt");
	if(!conn_file) conn_file = fopen (connection_log, "wb");
	fflush (conn_file);
}

void log_conn(const Uint8 *in_data, Uint32 data_lenght)
{
	char connection_log[256];

	strcpy(connection_log, configdir);
	strcat(connection_log, "connection_log.txt");
  	if(!conn_file) conn_file = fopen (connection_log, "ab");
  	fwrite (in_data, data_lenght, 1, conn_file);
  	fflush (conn_file);
}
