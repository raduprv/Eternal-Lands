#ifndef LOADING_WIN_H__
#define LOADING_WIN_H__

#ifdef __cplusplus
extern "C" {
#endif

extern Uint32 loading_win;

int create_loading_win(int width, int height, int snapshot);
void update_loading_win(char *text, float progress_increase);
int destroy_loading_win(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif //LOADING_WIN_H__
