#ifndef	__ERRORS_H
#define	__ERRORS_H

void clear_error_log();
void log_error(char * message);

#ifdef	DEBUG
#define	LOG_ERROR(msg)	log_error_detailed(msg, __FILE__, __FUNCTION__, __LINE__)
#else
#define	LOG_ERROR(msg)	log_error(msg)
#endif	//DEBUG

#endif	//__ERRORS_H
