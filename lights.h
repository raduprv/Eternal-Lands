#ifndef __LIGHTS_H__
#define __LIGHTS_H__

typedef struct
{
  float pos_x;
  float pos_y;
  float pos_z;
  float r;
  float g;
  float b;

}light;

typedef struct
{
  float x;
  float y;
  float z;
  float w;
}sun;


extern GLfloat light_0_position[4];
extern GLfloat light_0_diffuse[4];
extern GLfloat light_0_dist;

extern GLfloat light_1_position[4];
extern GLfloat light_1_diffuse[4];
extern GLfloat light_1_dist;

extern GLfloat light_2_position[4];
extern GLfloat light_2_diffuse[4];
extern GLfloat light_2_dist;

extern GLfloat light_3_position[4];
extern GLfloat light_3_diffuse[4];
extern GLfloat light_3_dist;

extern GLfloat light_4_position[4];
extern GLfloat light_4_diffuse[4];
extern GLfloat light_4_dist;

extern GLfloat light_5_position[4];
extern GLfloat light_5_diffuse[4];
extern GLfloat light_5_dist;

extern GLfloat light_6_position[4];
extern GLfloat light_6_diffuse[4];
extern GLfloat light_6_dist;

//for the lights
#define global_lights_no 60
extern GLfloat global_lights[global_lights_no][4];
extern GLfloat sky_lights_c1[global_lights_no*2][4];
extern GLfloat sky_lights_c2[global_lights_no*2][4];
extern GLfloat sky_lights_c3[global_lights_no*2][4];
extern GLfloat sky_lights_c4[global_lights_no*2][4];

extern sun sun_pos[60*3];


#define max_lights 1000
extern light *lights_list[max_lights];

extern char lights_on;
extern unsigned char light_level;
extern int game_minute;

int add_light(GLfloat x, GLfloat y, GLfloat z, GLfloat r, GLfloat g, GLfloat b, GLfloat intensity);
void set_material(float r, float g, float b);

#endif
