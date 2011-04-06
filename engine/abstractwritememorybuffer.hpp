/****************************************************************************
 *            abstractwritememorybuffer.hpp
 *
 * Author: 2011  Daniel Jungmann <el.3d.source@googlemail.com>
 * Copyright: See COPYING file that comes with this distribution
 ****************************************************************************/

#ifndef	UUID_24e315c8_034b_49d7_8199_45c39961e71f
#define	UUID_24e315c8_034b_49d7_8199_45c39961e71f

#ifndef	__cplusplus
#error	"Including C++ header in C translation unit!"
#endif	/* __cplusplus */

#include "prerequisites.hpp"

/**
 * @file
 * @brief The @c class AbstractWriteMemoryBuffer.
 * This file contains the @c class AbstractWriteMemoryBuffer.
 */
namespace eternal_lands
{

	class AbstractWriteMemoryBuffer: public boost::noncopyable
	{
		protected:
			/**
			 * Default constructor.
			 */
			AbstractWriteMemoryBuffer();

		public:
			/**
			 * Default destructor.
			 */
			virtual ~AbstractWriteMemoryBuffer() throw();

			/**
			 * @brief Gets the pointer of the memory.
			 *
			 * Gets the pointer of the memory.
			 * @return Returns the pointer of the memory.
			 */
			virtual void* get_ptr() = 0;

			/**
			 * @brief Gets the size of the memory.
			 *
			 * Gets the size of the memory.
			 * @return Returns the size of the memory.
			 */
			virtual Uint64 get_size() const = 0;

	};

}

#endif	/* UUID_24e315c8_034b_49d7_8199_45c39961e71f */
