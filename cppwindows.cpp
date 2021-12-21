#include "cppwindows.h"

namespace eternal_lands
{
namespace window_static_handlers
{

int button_click(widget_list *widget, int mx, int my, std::uint32_t flags, void* cb_ptr)
{
	auto callback = reinterpret_cast<const std::function<bool()>*>(cb_ptr);
	return (*callback)();
}

int delete_button_callback(widget_list *widget, void* cb_ptr)
{
	auto callback = reinterpret_cast<const std::function<bool()>*>(cb_ptr);
	delete callback;
	return 1;
}

int scrollbar_click(widget_list *widget, int mx, int my, std::uint32_t flags, void* cb_ptr)
{
	const vscrollbar *scrollbar = reinterpret_cast<const vscrollbar*>(widget->widget_info);
	auto callback = reinterpret_cast<const std::function<bool(int)>*>(cb_ptr);
	return (*callback)(scrollbar->pos);
}

int scrollbar_drag(widget_list *widget, int mx, int my, std::uint32_t flags, int dx, int dy, void* cb_ptr)
{
	const vscrollbar *scrollbar = reinterpret_cast<const vscrollbar*>(widget->widget_info);
	auto callback = reinterpret_cast<const std::function<bool(int)>*>(cb_ptr);
	return (*callback)(scrollbar->pos);
}

int delete_scrollbar_callback(widget_list *widget, void* cb_ptr)
{
	auto callback = reinterpret_cast<const std::function<bool(int)>*>(cb_ptr);
	delete callback;
	return 1;
}

} // namespace window_static_handlers
} // namespace eternal_lands
