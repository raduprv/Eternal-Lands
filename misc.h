#ifndef __MISC_H__
#define __MISC_H__

void unproject_ortho(GLfloat wx,GLfloat wy,GLfloat wz,GLfloat *ox,GLfloat *oy,GLfloat *oz);
void project_ortho(GLfloat ox, GLfloat oy, GLfloat oz, GLfloat * wx, GLfloat * wy);
void reset_under_the_mouse();
int anything_under_the_mouse(int object_id, int object_type);
void save_scene_matrix();
int mouse_in_sphere(float x, float y, float z, float radius);
void find_last_url(char * source_string, int len);
int go_to_url(void *dummy);

#endif
