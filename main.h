#ifndef __MAIN_H__
#define __MAIN_H__

#ifdef __cplusplus
extern "C" {
#endif

void read_command_line(void);

extern int exit_now; /*!< a flag that indicates we should exit the programm immediately */
extern int restart_required; /*!< a flag that the exit should attempt to restart, not just exit */
extern Uint32 cur_time, last_time; /*!< timestamps to check whether we need to resync */

#ifdef __cplusplus
} // extern "C"
#endif

#endif
