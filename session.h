#ifndef __EXT_SESSION_H__
#define __EXT_SESSION_H__

#ifdef __cplusplus
extern "C" {
#endif

extern int session_win;
extern int exp_log_threshold;

void fill_session_win(void);
void init_session(void);
int session_reset_handler(void);
int get_session_exp_ranging(void);
void set_last_skill_exp(size_t skill, int exp);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
