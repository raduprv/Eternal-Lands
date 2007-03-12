#ifndef __EXT_SESSION_H__
#define __EXT_SESSION_H__

#ifdef __cplusplus
extern "C" {
#endif

extern int session_win;

void fill_session_win(void);
void init_session(void);
int session_reset_handler(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
