#include "global.h"

int root_win = -1;

void display_root ()
{
	if (root_win < 0)
	{
		root_win = create_window ("root", -1, -1, 0, 0, window_width, window_height, ELW_WIN_INVISIBLE|ELW_SHOW_LAST);
		resize_root_window();
	}
}
