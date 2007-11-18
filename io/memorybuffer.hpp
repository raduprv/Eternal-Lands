#ifndef	_MEMORYBUFFER_HPP_
#define	_MEMORYBUFFER_HPP_

#ifdef NEW_FILE_IO

#include "allio.hpp"
#ifndef	USE_TR1
#include "../templates/el_shared_ptr.hpp"
#else
#include <tr1/memory>
#endif

namespace eternal_lands
{

	class memory_buffer
	{
		private:
			Uint8* memory;
			Uint32 size;

		public:

			inline memory_buffer(Uint32 new_size = 0)
			{
				if (new_size > 0)
				{
					memory = reinterpret_cast<Uint8*>(malloc(new_size));
				}
				else
				{
					memory = 0;
				}
				size = new_size;
			}

			/**
			 * @brief Destructor.
			 *
			 * Destructor, calls reduce_count.
			 * @see reduce_count
			 */
			inline ~memory_buffer()
			{
				if (memory != 0)
				{
					free(memory);
				}
			}

			/**
			 * @brief Gets the pointer of the memory buffer.
			 *
			 * Gets the pointer of the memory buffer.
			 * @param index The index to use.
			 * @return Returns the pointer of the memory buffer.
			 */
			template <typename T>
			inline T get_memory(Uint32 index = 0) const
			{
				assert(index < size);
				return reinterpret_cast<T>(&memory[index]);
			}

			/**
			 * @brief Gets the size of the memory buffer.
			 *
			 * Gets the size of the memory buffer.
			 * @return Returns the size of the memory buffer.
			 */
			inline Uint32 get_size() const
			{
				return size;
			}

			inline void resize(Uint32 new_size)
			{
				memory = static_cast<Uint8*>(realloc(memory, new_size));

				size = new_size;
			}
	};

#ifdef	USE_TR1
	typedef std::tr1::shared_ptr<memory_buffer> memory_ptr;
#else
	typedef shared_ptr<memory_buffer> memory_ptr;
#endif

}

#endif //NEW_FILE_IO

#endif	// _MEMORYBUFFER_HPP_
