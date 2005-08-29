#ifndef LOADING_WIN_H__
#define LOADING_WIN_H__

extern Uint32 loading_win;
extern Uint32 loading_win_progress_bar;
extern float progress;
extern int loading_texture;

int create_loading_win(int width, int height, int snapshot);
void update_loading_win(char *text, float progress_increase);
int destroy_loading_win(void);

#endif //LOADING_WIN_H__
