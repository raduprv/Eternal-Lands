#include <string.h>
#include "global.h"

FILE *err_file = NULL;
void clear_error_log()
{
	if(!err_file) err_file = fopen ("error_log.txt", "wb");
	fflush (err_file);
}

void log_error(const Uint8 * message)
{
	Uint8	str[2048];
	Uint32	len;

  	if(!err_file) err_file = fopen ("error_log.txt", "ab");
	if(strncmp(message, "Error", 5))	// do we need to add Error:?
		{
			snprintf(str, 2048, "Error: %s\n", message);
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

  	if(!err_file) err_file = fopen ("error_log.txt", "ab");
	snprintf(str, 2048, "Error: %s.%s:%d - %s\n", file, func, line, message);
	len=strlen(str);
	if(str[len-2] == '\n') len--;	// remove excess newline
  	fwrite (str, len, 1, err_file);
  	fflush (err_file);
}


FILE *conn_file = NULL;
void clear_conn_log()
{
	if(!conn_file) conn_file = fopen ("connection_log.txt", "wb");
	fflush (conn_file);
}

void log_conn(const Uint8 *in_data, Uint32 data_lenght)
{
  	if(!conn_file) conn_file = fopen ("connection_log.txt", "ab");
  	fwrite (in_data, data_lenght, 1, conn_file);
  	fflush (conn_file);
}
