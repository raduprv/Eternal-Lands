/*!
 * \file
 * \brief Texture loading and handling
 * \ingroup     render
 */
/*!\defgroup    load_bmp    Load texture bitmaps
 *  \ingroup    load
 */
#ifndef __TEXTURES_H__
#define __TEXTURES_H__

/*!
 * \ingroup load_bmp
 * \brief load_bmp8_color_key
 *
 *      TODO: load_bmp8_color_key
 *
 * \param   FileName
 * \return  GLuint
 */
GLuint load_bmp8_color_key(char * FileName);

/*!
 * \ingroup load_bmp
 * \brief load_bmp8_fixed_alpha
 *
 *      TODO: load_bmp8_fixed_alpha
 *
 * \param   FileName
 * \param   a
 * \return  GLuint
 */
GLuint load_bmp8_fixed_alpha(char * FileName, Uint8 a);

/*!
 * \ingroup load_bmp
 * \brief load_bmp8_color_key_no_texture
 *
 *      TODO: load_bmp8_color_key_no_texture
 *
 * \param   FileName
 * \return  char*
 */
char * load_bmp8_color_key_no_texture(char * FileName);

/*!
 * \ingroup load_bmp
 * \brief load_bmp8_alpha_map
 *
 *      TODO: load_bmp8_alpha_map
 *
 * \param   FileName
 * \return  char*
 */
char * load_bmp8_alpha_map(char * FileName);

/*!
 * \internal check group!
 * \ingroup load
 * \brief load_texture_cache
 *
 *      TODO: load_texture_cache
 *
 * \param   file_name
 * \param   alpha
 * \return  int
 */
int load_texture_cache(char * file_name,unsigned char alpha);

/*!
 * \internal check group!
 * \ingroup render
 * \brief get_texture_id
 *
 *      TODO: get_texture_id
 *
 * \param   i
 * \return  int
 */
int		get_texture_id(int i);

/*!
 * \internal check group!
 * \ingroup render
 * \brief bind_texture_id
 *
 *      TODO: bind_texture_id
 *
 * \param   texture_id
 * \return  None
 */
void	bind_texture_id(int texture_id);

/*!
 * \internal check group!
 * \ingroup render
 * \brief get_and_set_texture_id
 *
 *      TODO: get_and_set_texture_id
 *
 * \param   i
 * \return  int
 */
int		get_and_set_texture_id(int i);

/*!
 * \ingroup load_bmp
 * \brief load_bmp8_remapped_skin
 *
 *      TODO: load_bmp8_remapped_skin
 *
 * \param   FileName
 * \param   a
 * \param   skin
 * \param   hair
 * \param   shirt
 * \param   pants
 * \param   boots
 * \return  GLuint
 */
GLuint	load_bmp8_remapped_skin(char * FileName, Uint8 a, short skin, short hair, short shirt,
							   short pants, short boots);
                               
/*!
 * \ingroup load_bmp
 * \brief load_bmp8_to_coordinates
 *
 *      TODO: load_bmp8_to_coordinates
 *
 * \param   FileName
 * \param   texture_space
 * \param   x_pos
 * \param   y_pos
 * \param   alpha
 * \return  None
 */
void	load_bmp8_to_coordinates(char * FileName, Uint8 *texture_space,int x_pos,int y_pos,
							  Uint8 alpha);
#ifdef	ELC
/*!
 * \ingroup load_bmp
 * \brief load_bmp8_enhanced_actor
 *
 *      TODO: load_bmp8_enhanced_actor
 *
 * \param   this_actor
 * \param   a
 * \return  int
 */
int		load_bmp8_enhanced_actor(enhanced_actor *this_actor, Uint8 a);
#endif	//ELC

#endif
