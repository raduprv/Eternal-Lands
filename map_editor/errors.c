#include "global.h"

void clear_error_log()
{
	FILE *f = NULL;
	char *file_name="error_log.txt";
	f = fopen (file_name, "wb");
	if(!f) return;
	fclose (f);
}

void log_error(const char * message, ...)
{
	va_list ap;
	FILE *f = NULL;
	char *file_name="error_log.txt";

	f = fopen (file_name, "a");
	if(!f) return;
	va_start(ap, message);
		vfprintf (f, message, ap);
	va_end(ap);
  	fclose (f);
}
