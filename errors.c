#include "global.h"

void clear_error_log()
{
	FILE *f = NULL;
	char *file_name="error_log.txt";
	f = fopen (file_name, "wb");
	if(!f) return;
	fclose (f);
}

void log_error(char * message)
{
	FILE *f = NULL;
	char *file_name="error_log.txt";

  	f = fopen (file_name, "a");
	if(!f) return;
  	fwrite (message, strlen(message), 1, f);
  	fclose (f);
}
