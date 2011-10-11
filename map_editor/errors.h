#ifndef	__ERRORS_H
#define	__ERRORS_H

void clear_error_log();
void log_error(const char * message, ...);

#ifdef DEBUG
 #ifdef _MSC_VER
  #define LOG_ERROR log_error //MSVC doesn't support variadic macros.
 #else
  #define LOG_ERROR(msg, args ...) log_error_detailed(msg, __FILE__, __FUNCTION__, __LINE__, ## args) /*!< detailed log of error */
 #endif //_MSC_VER
#else
 #define LOG_ERROR log_error
#endif	//DEBUG

#endif	//__ERRORS_H
