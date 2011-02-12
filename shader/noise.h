#ifndef	_NOISE_H_
#define	_NOISE_H_


#include "../platform.h"

/**
 * @brief Builds a 3d noise textures.
 *
 * Builds a 3d noise texture with the given size, frequency and dimensions of the data.
 *
 * @param size The size of the texture in all tree dimensions.
 * @param frequency The starting frequency of the noise.
 * @param dimensions The number of dimensions from the noise function. One means normal noise, 2 is
 * a 2d dnoise and 3 is a 3d dnoise.
 */
GLuint build_3d_noise_texture(int size, int frequency, int dimensions);


#endif	// _NOISE_H_
