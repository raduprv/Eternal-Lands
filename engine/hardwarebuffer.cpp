/****************************************************************************
 *            hardwarebuffer.cpp
 *
 * Author: 2011  Daniel Jungmann <el.3d.source@googlemail.com>
 * Copyright: See COPYING file that comes with this distribution
 ****************************************************************************/

#include "hardwarebuffer.hpp"
#include "../load_gl_extensions.h"

namespace eternal_lands
{

	HardwareBuffer::HardwareBuffer(): m_size(0), m_id(0)
	{
		ELglGenBuffersARB(1, &m_id);
	}

	HardwareBuffer::~HardwareBuffer() throw()
	{
		ELglDeleteBuffersARB(1, &m_id);
	}

	void HardwareBuffer::set_size(const HardwareBufferType type,
		const Uint64 size, const HardwareBufferUsageType usage)
	{
		m_size = size;
		ELglBufferDataARB(type, size, 0, usage);
	}

	void HardwareBuffer::bind(const HardwareBufferType type)
	{
		ELglBindBufferARB(type, m_id);
	}

	void HardwareBuffer::unbind(const HardwareBufferType type)
	{
		ELglBindBufferARB(type, 0);
	}

	void* HardwareBuffer::map(const HardwareBufferType type,
		const HardwareBufferAccessType access)
	{
		return ELglMapBufferARB(type, access);
	}

	bool HardwareBuffer::unmap(const HardwareBufferType type)
	{
		return ELglUnmapBufferARB(type) == GL_TRUE;
	}

}

