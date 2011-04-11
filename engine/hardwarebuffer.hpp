/****************************************************************************
 *            hardwarebuffer.hpp
 *
 * Author: 2011  Daniel Jungmann <el.3d.source@googlemail.com>
 * Copyright: See COPYING file that comes with this distribution
 ****************************************************************************/

#ifndef	UUID_ebe8ffbc_ae1a_4db5_bdd0_e78938bd6dfc
#define	UUID_ebe8ffbc_ae1a_4db5_bdd0_e78938bd6dfc

#ifndef	__cplusplus
#error	"Including C++ header in C translation unit!"
#endif	/* __cplusplus */

#include "../platform.h"

/**
 * @file
 * @brief The @c class HardwareBuffer.
 * This file contains the @c class HardwareBuffer.
 */
namespace eternal_lands
{

	/**
	 * Defines how to access the hardware buffer.
	 * @{
	 */
	enum HardwareBufferAccessType
	{
		hbat_read_only = GL_READ_ONLY,
		hbat_read_write = GL_READ_WRITE,
		hbat_write_only = GL_WRITE_ONLY
	};
	/**
	 * @}
	 */

	/**
	 * Defines for what we use the hardware buffer. This is not static,
	 * so you can use render-to-vertex-buffer etc.
	 * @{
	 */
	enum HardwareBufferType
	{
		hbt_vertex = GL_ARRAY_BUFFER,
		hbt_index = GL_ELEMENT_ARRAY_BUFFER,
		hbt_pixel_pack = GL_PIXEL_PACK_BUFFER,
		hbt_pixel_unpack = GL_PIXEL_UNPACK_BUFFER
	};
	/**
	 * @}
	 */

	/**
	 * Defines how to use the hardware buffer.
	 * @{
	 */
	enum HardwareBufferUsageType
	{
		hbut_dynamic_copy = GL_DYNAMIC_COPY,
		hbut_dynamic_draw = GL_DYNAMIC_DRAW,
		hbut_dynamic_read = GL_DYNAMIC_READ,
		hbut_static_copy = GL_STATIC_COPY,
		hbut_static_draw = GL_STATIC_DRAW,
		hbut_static_read = GL_STATIC_READ,
		hbut_stream_copy = GL_STREAM_COPY,
		hbut_stream_draw = GL_STREAM_DRAW,
		hbut_stream_read = GL_STREAM_READ
	};
	/**
	 * @}
	 */

	/**
	 * @brief @c class for opengl hardware buffers.
	 *
	 * @c class for opengl hardware buffers.
	 */
	class HardwareBuffer
	{
		private:
			/**
			 * This is the size of the buffer in bytes.
			 */
			Uint64 m_size;

			/**
			 * This is the OpenGL buffer id.
			 */
			GLuint m_id;

		public:
			/**
			 * Default constructor, creates the OpenGL id.
			 */
			HardwareBuffer();

			/**
			 * Default destructor, frees the memory and the
			 * OpenGL id.
			 */
			~HardwareBuffer() throw();

			/**
			 * Returns the size of the buffer in bytes.
			 */
			inline Uint64 get_size() const
			{
				return m_size;
			}

			/**
			 * Sets the size of the buffer in bytes.
			 * @param type The type of the buffer.
			 * @param size The new size to use.
			 * @param usage The usage of the buffer.
			 */
			void set_size(const HardwareBufferType type,
				const Uint64 size,
				const HardwareBufferUsageType usage);

			/**
			 * Binds the buffer.
			 * @param type The type of the buffer to bind.
			 */
			void bind(const HardwareBufferType type);

			/**
			 * Unbinds the currently bound buffer.
			 * @param type The type of the buffer to unbind.
			 */
			static void unbind(const HardwareBufferType type);

			/**
			 * Map the buffer the currently bound buffer.
			 * @param type The hardware buffer type to unmap.
			 * @param access The access type that is performed.
			 * @return Pointer of the buffer data.
			 */
			static void* map(const HardwareBufferType type,
				const HardwareBufferAccessType access);

			/**
			 * Unmap the buffer the currently bound buffer.
			 * @param type The hardware buffer type to unmap.
			 * @return bool Returns true on success, else false.
			 */
			static bool unmap(const HardwareBufferType type);

	};

}

#endif	/* UUID_ebe8ffbc_ae1a_4db5_bdd0_e78938bd6dfc */

