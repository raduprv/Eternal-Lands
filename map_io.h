#ifndef __MAP_IO_H__
#define __MAP_IO_H__

extern char map_file_name[60];

typedef struct
{
	char file_name[80];
	float x_pos;
	float y_pos;
	float z_pos;

	float x_rot;
	float y_rot;
	float z_rot;

	char self_lit;
	char blended;
	float r,g,b;
	char reserved[24];

}object3d_io;

typedef struct
{
	char file_name[80];
	float x_pos;
	float y_pos;
	float z_pos;
	float x_rot;
	float y_rot;
	float z_rot;
	char reserved[24];
}obj_2d_io;

typedef struct
{
	float pos_x;
	float pos_y;
	float pos_z;
	float r;
	float g;
	float b;
	char reserved[15];
}light_io;

typedef struct
{
	char file_name[80];
	float x_pos;
	float y_pos;
	float z_pos;
	char reserved[10];
}particles_io;

typedef struct
{
	char file_sig[4];//should be "elmf", or else the map is invalid
	int tile_map_x_len;
	int tile_map_y_len;
	int tile_map_offset;
	int height_map_offset;
	int obj_3d_struct_len;
	int obj_3d_no;
	int obj_3d_offset;
	int obj_2d_struct_len;
	int obj_2d_no;
	int obj_2d_offset;
	int lights_struct_len;
	int lights_no;
	int lights_offset;
	char dungeon;//no sun
	char res_2;
	char res_3;
	char res_4;
	float ambient_r;
	float ambient_g;
	float ambient_b;
	int particles_struct_len;
	int particles_no;
	int particles_offset;
	int reserved_8;
	int reserved_9;
	int reserved_10;
	int reserved_11;
	int reserved_12;
	int reserved_13;
	int reserved_14;
	int reserved_15;
	int reserved_16;
	int reserved_17;


}map_header;

extern char dungeon;//no sun
extern float ambient_r;
extern float ambient_g;
extern float ambient_b;

void destroy_map();
int save_map(char * file_name);
int load_map(char * file_name);
void new_map(int m_x_size,int m_y_size);
#endif
