#ifndef __PARTICLES_WINDOW_H__
#define __PARTICLES_WINDOW_H__

extern int view_particles_window;
extern int particles_window;
void init_particles_window();
void display_particles_window();
void toggle_particles_window();
int display_particles_window_handler();
int check_particles_window_interface();
void particles_win_move_preview(float zmove);

#endif
