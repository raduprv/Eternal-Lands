#ifndef __TEXTURES_H__
#define __TEXTURES_H__

GLuint load_bmp8_color_key(char * FileName);
GLuint load_bmp8_fixed_alpha(char * FileName, Uint8 a);
char * load_bmp8_color_key_no_texture(char * FileName);
char * load_bmp8_alpha_map(char * FileName);
int load_texture_cache(char * file_name,unsigned char alpha);
int		get_texture_id(int i);
void	bind_texture_id(int texture_id);
int		get_and_set_texture_id(int i);
GLuint	load_bmp8_remapped_skin(char * FileName, Uint8 a, short skin, short hair, short shirt,
							   short pants, short boots);
void	load_bmp8_to_coordinates(char * FileName, Uint8 *texture_space,int x_pos,int y_pos,
							  Uint8 alpha);
#ifdef	ELC
int		load_bmp8_enhanced_actor(enhanced_actor *this_actor, Uint8 a);
#endif	//ELC

#endif
