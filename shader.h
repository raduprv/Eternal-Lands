#ifdef	TERRAIN
#ifdef	USE_SHADER
#ifndef SHADER_H
#define SHADER_H

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief	Frees the given shader.
 *
 * Frees the given shader.
 * \param	ProgramObject The shader object to free.
 *
 * \callgraph
 */
extern void free_shader(GLhandleARB ProgramObject);

/*!
 * \brief	Inits the shader for normal mapping.
 *
 * Inits the shader for normal mapping.
 * \retval	GLhandleARB The shader object for normal mapping.
 *
 * \callgraph
 */
extern GLhandleARB init_normal_mapping_shader();

#ifdef __cplusplus
} // extern "C"
#endif

#endif
#endif
#endif
