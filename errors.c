#include <string.h>
#include "global.h"

void clear_error_log()
{
	FILE *f = NULL;
	f = fopen ("error_log.txt", "wb");
	fclose (f);
}

void log_error(char * message)
{
	FILE *f = NULL;

  	f = fopen ("error_log.txt", "ab");
  	fwrite (message, strlen(message), 1, f);
  	fclose (f);
}

void clear_conn_log()
{
#ifdef	DEBUG
	FILE *f = NULL;
	f = fopen ("connection_log.txt", "wb");
	fclose (f);
#endif
}

void log_conn(unsigned char *in_data, int data_lenght)
{
#ifdef	DEBUG
	FILE *f = NULL;

  	f = fopen ("connection_log.txt", "ab");
  	fwrite (in_data, data_lenght, 1, f);
  	fclose (f);
#endif
}
