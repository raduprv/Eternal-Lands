#ifndef __ERRORS_H__
#define __ERRORS_H__

void clear_error_log();
void log_error(char * message);
void clear_conn_log();
void log_conn(unsigned char *in_data, int data_lenght);
#endif
