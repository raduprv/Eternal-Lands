#ifndef __GL_INIT_H__
#define __GL_INIT_H__

void setup_video_mode();
void check_gl_mode();
void init_video();
void resize_window();
void set_new_video_mode(int fs,int mode);
void toggle_full_screen();
int print_gl_errors(char *file, char *func, int line);
#ifdef	DEBUG
#define check_gl_errors()	print_gl_errors(__FILE__,  __FUNCTION__, __LINE__)
#else	//DEBUG
#define check_gl_errors()	//NOP
#endif	//DEBUG

#ifndef POINT_SIZE_MIN_ARB
#define POINT_SIZE_MIN_ARB 0x8126
#endif

#endif
