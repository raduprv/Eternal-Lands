#ifndef __MISC_H__
#define __MISC_H__

void reset_under_the_mouse();
int anything_under_the_mouse(int object_id, int object_type);
void save_scene_matrix();
int mouse_in_sphere(float x, float y, float z, float radius);
void find_last_url(char * source_string, int len);
int go_to_url(void *dummy);

#endif
