#ifndef __ERRORS_H__
#define __ERRORS_H__

void clear_error_log();
void log_error(const Uint8 *message);
void log_error_detailed(const Uint8 *message, const Uint8 *file, const Uint8 *function, Uint32 line);
void clear_conn_log();
void log_conn(const Uint8 *in_data, Uint32 data_lenght);

#ifdef	DEBUG
#define	LogError(msg)	log_error_detailed(msg, __FILE__, __FUNCTION__, __LINE__)
#else
#define	LogError(msg)	log_error(msg)
#endif	//DEBUG

#endif
