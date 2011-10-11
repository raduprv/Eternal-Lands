#ifndef __PARTICLES_WINDOW_H__
#define __PARTICLES_WINDOW_H__

extern int view_particles_window;
extern int particles_window;
extern particle_sys_def def;
void init_particles_window();
void create_particles_window();
void toggle_particles_window();
int display_particles_window_handler();
int check_particles_window_interface();
void particles_win_move_preview(float zmove);
void particles_win_zoomin ();
void particles_win_zoomout ();

#endif
