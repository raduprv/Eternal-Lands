#include "global.h"

int root_win = -1;

int mouseover_root_handler (window_info *win, int mx, int my)
{
	return 0;
}

int click_root_handler (window_info *win, int mx, int my, Uint32 flags)
{
	return 0;
}

void display_root ()
{
#if 0
	if (root_win < 0)
	{
		root_win = create_window ("root", -1, -1, 0, 0, window_width, window_height, ELW_WIN_INVISIBLE|ELW_SHOW_LAST);
		
        	set_window_handler(root_win, ELW_HANDLER_CLICK, &click_root_handler);
        	set_window_handler(root_win, ELW_HANDLER_MOUSEOVER, &mouseover_root_handler);
		
		resize_root_window();
	}
#endif
}
