unsigned int fsaa = 0;
unsigned int fsaa_modes = 0;

char* fsaa_modes_strings[32] =
{
	"x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
	"x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
	"x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23",
	"x24", "x25", "x26", "x27", "x28", "x29", "x30", "x31",
};

unsigned int get_fsaa_modes();

void init_fsaa_modes()
{
	fsaa_modes = get_fsaa_modes();
}

unsigned int get_fsaa_mode_count()
{
	return 32;
}

unsigned int get_fsaa_mode(const unsigned int index)
{
	unsigned int mask;

	mask = 1 << index;

	if ((fsaa_modes & mask) == mask)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

const char* get_fsaa_mode_str(const unsigned int index)
{
	if (index < get_fsaa_mode_count())
	{
		return fsaa_modes_strings[index];
	}
	else
	{
		return 0;
	}
}

