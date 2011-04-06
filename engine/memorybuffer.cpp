/****************************************************************************
 *            memorybuffer.cpp
 *
 * Author: 2011  Daniel Jungmann <el.3d.source@googlemail.com>
 * Copyright: See COPYING file that comes with this distribution
 ****************************************************************************/

#include "memorybuffer.hpp"

namespace eternal_lands
{

	MemoryBuffer::MemoryBuffer(): m_size(0)
	{
	}

	MemoryBuffer::~MemoryBuffer() throw()
	{
	}

	void MemoryBuffer::resize(const Uint64 size)
	{
		Uint8ScopedArray ptr;

		if (size != get_size())
		{
			if ((size > 0) && (get_size() > 0))
			{
				ptr.reset(new Uint8[size]);

				memcpy(ptr.get(), m_ptr.get(),
					std::min(size, get_size()));

				ptr.swap(m_ptr);
			}
			else
			{
				if (size > 0)
				{
					m_ptr.reset(new Uint8[size]);
				}
				else
				{
					m_ptr.reset();
				}
			}
		}

		m_size = size;
	}

}
