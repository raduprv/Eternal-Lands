#include "../platform.h"
#include "../errors.h"

unsigned int get_fsaa_modes()
{
	LOG_DEBUG("Using dummy to get fsaa modes");

	return 0x0115;
}
