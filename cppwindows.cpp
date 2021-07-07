#include "cppwindows.h"

namespace eternal_lands
{
namespace window_static_handlers
{

int button_click(widget_list *widget, int mx, int my, std::uint32_t flags, void* cb_ptr)
{
	const std::function<int()>* callback = reinterpret_cast<const std::function<int()>*>(cb_ptr);
	return (*callback)();
}

int delete_button_click(widget_list *widget, void* cb_ptr)
{
	const std::function<int()>* callback = reinterpret_cast<const std::function<int()>*>(cb_ptr);
	delete callback;
	return 1;
}

} // namespace window_static_handlers
} // namespace eternal_lands
