/*!
 * \file
 * \ingroup misc
 * \brief miscellaneous functions
 */
#ifndef __MISC_H__
#define __MISC_H__

/*!
 * \ingroup misc
 * \brief
 *
 *      Detail
 *
 * \param wx
 * \param wy
 * \param wz
 * \param ox
 * \param oy
 * \param oz
 * \return None
 */
void unproject_ortho(GLfloat wx,GLfloat wy,GLfloat wz,GLfloat *ox,GLfloat *oy,GLfloat *oz);

/*!
 * \ingroup misc
 * \brief
 *
 *      Detail
 *
 * \param ox
 * \param oy
 * \param oz
 * \param wx
 * \param wy
 * \return None
 */
void project_ortho(GLfloat ox, GLfloat oy, GLfloat oz, GLfloat * wx, GLfloat * wy);

/*!
 * \ingroup misc
 * \brief
 *
 *      Detail
 *
 * \return None
 */
void reset_under_the_mouse();

/*!
 * \ingroup misc
 * \brief
 *
 *      Detail
 *
 * \param object_id
 * \param object_type
 * \return int
 */
int anything_under_the_mouse(int object_id, int object_type);

/*!
 * \ingroup misc
 * \brief
 *
 *      Detail
 *
 * \return None
 */
void save_scene_matrix();

/*!
 * \ingroup misc
 * \brief
 *
 *      Detail
 *
 * \param x
 * \param y
 * \param z
 * \param radius
 * \return int
 */
int mouse_in_sphere(float x, float y, float z, float radius);

/*!
 * \ingroup misc
 * \brief
 *
 *      Detail
 *
 * \param source_string
 * \param len
 * \return None
 */
void find_last_url(char * source_string, int len);

/*!
 * \ingroup misc
 * \brief
 *
 *      Detail
 *
 * \param dummy
 * \return int
 */
int go_to_url(void *dummy);

//some prototypes, that won't fit somewhere else

/*!
 * \ingroup misc
 * \brief
 *
 *      Detail
 *
 * \param x
 * \param y
 * \param z
 * \param radius
 * \return int
 */
int SphereInFrustum(float x, float y, float z, float radius);

/*!
 * \ingroup misc
 * \brief
 *
 *      Detail
 *
 * \param x
 * \param y
 * \return int
 */
int check_tile_in_frustrum(float x,float y);

/*!
 * \ingroup misc
 * \brief Opens a file and check the result
 *
 *      Tries to open a file, and logs an error message if it fails
 *
 * \param fname The file name
 * \param mode The mode in which the file is to be opened
 * \return Pointer to the file on success, NULL otherwise
 */
FILE *my_fopen (const char *fname, const char *mode);

#endif
