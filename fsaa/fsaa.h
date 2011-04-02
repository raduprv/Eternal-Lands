/*!
 * \file
 * \ingroup video
 * \brief OpenGL fsaa value functions
 */
#ifndef __FSAA_H__
#define __FSAA_H__

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned int fsaa; /*!< flag that inidicates what level of fsaa to use */

void init_fsaa_modes();
unsigned int get_fsaa_mode_count();
unsigned int get_fsaa_mode(const unsigned int index);
char* get_fsaa_mode_str(const unsigned int index);

#ifdef __cplusplus
} // extern "C"
#endif

#endif	/* __FSAA_H__ */

