#ifndef	__TEXTURES_h
#define	__TEXTURES_H

GLuint load_bmp8_color_key(char * FileName);
GLuint load_bmp8_fixed_alpha(char * FileName, Uint8 a);
char * load_bmp8_color_key_no_texture(char * FileName);
char * load_bmp8_alpha_map(char * FileName);
int load_texture_cache(char * file_name,unsigned char alpha);
int get_texture_id(int i);
GLuint load_bmp8_remapped_skin(char * FileName, Uint8 a,char skin, char hair, char shirt, char pants, char boots);

#endif	//__TEXTURES_H
