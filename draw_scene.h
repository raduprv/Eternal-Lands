#ifndef __DRAW_SCENE_H__
#define __DRAW_SCENE_H__

void draw_scene();
void Move();
void update_camera();
Uint32 my_timer(unsigned int some_int);
int	my_timer_adjust;
int	my_timer_clock;

void CalculateFrustum();
#endif
