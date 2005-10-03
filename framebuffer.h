#ifdef	USE_FRAMEBUFFER
/*!
 * \file
 * \ingroup 	display_utils
 * \brief 	Framebuffer utils.
 */
#ifndef	FRAMEBUFFER_H
#define	FRAMEBUFFER_H

/*!
 * \ingroup 	display_utils
 * \brief 	Free framebuffer.
 * 
 * Frees the given frame buffer, render buffer and texture.
 * \param 	FBO The frame buffer.
 * \param 	FBORenderBuffer The render buffer.
 * \param 	FBOTexture The destination texture.
 *  
 * \callgraph
 */
void free_color_framebuffer(int *FBO, int *FBORenderBuffer, int *FBOTexture);

/*!
 * \ingroup 	display_utils
 * \brief 	Creates a new frame buffer, attachs a new render buffer and a destination texture.
 * 
 * Creates a new frame buffer, attachs a new render buffer and a destination texture.
 * \param	width The width of the texture.
 * \param	height The height of the texture.
 * \param 	FBO The frame buffer.
 * \param 	FBORenderBuffer The render buffer.
 * \param 	FBOTexture The destination texture.
 *  
 * \callgraph
 */
void make_color_framebuffer(int width, int height, int *FBO, int *FBORenderBuffer, int *FBOTexture);

/*!
 * \ingroup 	display_utils
 * \brief 	Changes the size of the destinatoin texture of a given framebuffer.
 * 
 * Changes the size of the destinatoin texture of a given framebuffer
 * \param	width The width of the texture.
 * \param	height The height of the texture.
 * \param 	FBO The frame buffer.
 * \param 	FBORenderBuffer The render buffer.
 * \param 	FBOTexture The destination texture.
 *  
 * \callgraph
 */
void change_color_framebuffer_size(int width, int height, int *FBO, int *FBORenderBuffer, int *FBOTexture);

#endif
#endif
