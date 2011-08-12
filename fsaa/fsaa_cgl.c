#include "../platform.h"
#include "../errors.h"
#include <OpenGL/OpenGL.h>

unsigned int get_fsaa_modes()
{
	GLint value, renders;
	int i, j, index, result;
	CGLRendererInfoObj render_info;

	LOG_DEBUG("Using cgl to get fsaa modes");

	CGLQueryRendererInfo(1, &render_info, &renders);

	if (renders == 0)
	{
		LOG_ERROR("Can't get render infos");

		return 0;
	}

	result = 1;

	for (i = 0; i < renders; i++)
	{
		CGLDescribeRenderer(render_info, i, kCGLRPAccelerated, &value);

		if (value != 1)
		{
			continue;
		}

		CGLDescribeRenderer(render_info, i, kCGLRPMaxSamples, &value);

		result |= 1 << value;
		index = 2;

		while (index < value)
		{
			result |= 1 << index;
			index += index;
		}
	}

	CGLDestroyRendererInfo(render_info);

	return result;
}

