#ifndef	_MEMORYBUFFER_HPP_
#define	_MEMORYBUFFER_HPP_

#ifdef NEW_FILE_IO

#include "allio.hpp"

class memory_buffer
{
	private:
		Uint8* memory;
		int* reference_count;
		int* size;

		inline void reduce_count()
		{
			*reference_count -= 1;

			if (*reference_count == 0)
			{
				free(memory);
				delete reference_count;
				delete size;
			}
		}

		inline void copy_data(const memory_buffer& mb)
		{
			memory = mb.memory;
			reference_count = mb.reference_count;
			size = mb.size;
			*reference_count += 1;
		}

	public:

		inline memory_buffer(int _size = 0)
		{
			memory = reinterpret_cast<Uint8*>(malloc(_size));
			reference_count = new int;
			size = new int;
			*reference_count = 1;
			*size = _size;
		}

		/**
		 * @brief Const copy constructor.
		 *
		 * Const copy constructor, calls copy_data.
		 * @param mb The source from where to copy.
		 * @see copy_data
		 */
		inline memory_buffer(const memory_buffer &mb)
		{
			copy_data(mb);
		}

		/**
		 * @brief Copy constructor.
		 *
		 * Copy constructor, calls copy_data.
		 * @param mb The source from where to copy.
		 * @see copy_data
		 */
		inline memory_buffer(memory_buffer &mb)
		{
			copy_data(mb);
		}

		/**
		 * @brief Destructor.
		 *
		 * Destructor, calls reduce_count.
		 * @see reduce_count
		 */
		inline ~memory_buffer()
		{
			reduce_count();
		}

		/**
		 * @brief Copy operator.
		 *
		 * Copy operator, calls copy_data.
		 * @param mb The source from where to copy.
		 * @return Returns this.
		 * @see copy_data
		 */
		memory_buffer& operator= (const memory_buffer& mb)
		{
			reduce_count();
			copy_data(mb);

			return *this;
		}
		/**
		 * @brief Gets the pointer of the memory buffer.
		 *
		 * Gets the pointer of the memory buffer.
		 * @return Returns the pointer of the memory buffer.
		 */
		inline Uint8* get_memory() const
		{
			return memory;
		}

		/**
		 * @brief Gets the pointer of the memory buffer.
		 *
		 * Gets the pointer of the memory buffer at the given index.
		 * @param index The index to use.
		 * @return Returns the pointer of the memory buffer.
		 */
		inline Uint8* get_memory(int index) const
		{
			return &memory[index];
		}

		/**
		 * @brief Gets the size of the memory buffer.
		 *
		 * Gets the size of the memory buffer.
		 * @return Returns the size of the memory buffer.
		 */
		inline int get_size() const
		{
			return *size;
		}

		/**
		 * @brief Gets the reference count of the memory buffer.
		 *
		 * Gets the reference count of the memory buffer.
		 * @return Returns the reference count of the memory buffer.
		 */
		inline int get_reference_count() const
		{
			return *reference_count;
		}

		inline void resize(int new_size)
		{
			if (get_reference_count() > 1)
			{
				EXTENDED_EXCEPTION("Can't resize, because reference count is greater than one!");
			}

			memory = static_cast<Uint8*>(realloc(memory, new_size));

			*size = new_size;
		}
};

#endif //NEW_FILE_IO

#endif	// _MEMORYBUFFER_HPP_
