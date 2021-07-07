#ifndef CPPWINDOWS_H
#define CPPWINDOWS_H

/*!
 * \file
 * \ingroup elwindows
 * \brief C++ abstraction for client windows
 */

#include <atomic>
#include <cstdint>
#include <functional>
#include "elwindows.h"
#include "exceptions/extendedexception.hpp"
#include "font.h"

//! Concatenate tokens \a a and \a b
#define CONCAT_IMPL(a, b) a ## b
//! Concatenate tokens \a a and \a b
#define CONCAT(a, b) CONCAT_IMPL(a, b)
//! Return the function name for a handler for \a action events
#define HANDLER_NAME(action) CONCAT(action, _handler)
//! Return the function name for a static handler for \a action events
#define STATIC_HANDLER_NAME(action) CONCAT(static_, HANDLER_NAME(action))
//! Return the name of a function that sets the callback handler for events of type \a action
#define SET_HANDLER_NAME(action) CONCAT(set_, HANDLER_NAME(action))
//! Macro for the declaration of a static handler for \a action events
#define DECLARE_STATIC_HANDLER(action, ...) \
	static int STATIC_HANDLER_NAME(action)(window_info *win, ##__VA_ARGS__)
//! Extrcat the member function handler for events of type \a action from a \window_info structure
#define MEMBER_HANDLER(action) get_cpp_window(win).HANDLER_NAME(action)
//! Define a static callback handler for \a action events that takes no arguments
#define DEFINE_STATIC_HANDLER0(action) \
	DECLARE_STATIC_HANDLER(action) { return MEMBER_HANDLER(action)(); }
//! Define a static callback handler for \a action events that takes a single argument of type T0
#define DEFINE_STATIC_HANDLER1(action, T0) \
	DECLARE_STATIC_HANDLER(action, T0 a0) { return MEMBER_HANDLER(action)(a0); }
//! Define a static callback handler for \a action events that takes two arguments of type T0 and T1
#define DEFINE_STATIC_HANDLER2(action, T0, T1) \
	DECLARE_STATIC_HANDLER(action, T0 a0, T1 a1) { return MEMBER_HANDLER(action)(a0, a1); }
//! Define a static callback handler for \a action events that takes three argument of type T0, T1, and T2
#define DEFINE_STATIC_HANDLER3(action, T0, T1, T2) \
	DECLARE_STATIC_HANDLER(action, T0 a0, T1 a1, T2 a2) { return MEMBER_HANDLER(action)(a0, a1, a2); }
//! Check if class \c Derived ahs a member function named <em>action<tt>_handler</tt>,
#define HAS_HANDLER(action) \
	std::is_member_function_pointer<decltype(&Derived::HANDLER_NAME(action))>::value
/*!
 * \brief Macro to declare a handler setter
 *
 * Macro DECLARE_SET_HANDLER declares functions to set the callback function for an event of type
 * \a action (identified by \a constant) in a Window instance. The Window<Derived> class uses SFINAE
 * to determine which function is used: if an <em>action</em><tt>_handler</tt> member function
 * is defined in \c Derived, the function that sets the handler is used, otherwise the variant
 * that does nothing is used.
 * \param action The event for which a callback setter is defined
 * \param constant The associated constant that defines the type of event.
 */
#define DECLARE_SET_HANDLER(action, constant) \
	template <typename Derived> \
	typename std::enable_if<HAS_HANDLER(action), void>::type \
	SET_HANDLER_NAME(action)(Window<Derived> *window) \
	{ \
		set_window_handler(window->id(), constant, \
			(int(*)())&Window<Derived>::STATIC_HANDLER_NAME(action)); \
	} \
	inline void SET_HANDLER_NAME(action)(...) {}

namespace eternal_lands
{

namespace window_static_handlers
{
int button_click(widget_list *widget, int mx, int my, std::uint32_t flags, void* cb_ptr);
int delete_button_click(widget_list *widget, void* cb_ptr);
} // namespace window_static_handlers

/*!
 * \brief Base class for client windows
 *
 * Class Window is a template base class that represents a client window. The template argument
 * \a Derived is the type name of the class deriving from Window. It provides
 * access to the underlying data through getter and setter functions, and will automatically
 * register event handlers if they are implementedin \a Derived (and accessible to Window).
 */
template <typename Derived>
class Window
{
public:
	/*!
	 * \brief Constructor
	 *
	 * Create a new window with title \a title and size \a width × \a height at position
	 * (\a x, \a y) on the screen. The window has no parent window. Optional window flags
	 * can be set through \a flags; these then <em>replace</em> the default window flags entirely,
	 * so if any default window flags should be retained they should be included in \a flags as
	 * well.
	 * \param title  The window title
	 * \param x      The x coordinate of the upper left corner of the window
	 * \param y      The y coordinate of the upper left corner of the window
	 * \param width  The width of the window in pixels
	 * \param height The height of the window in pixels
	 * \param flags  If set, the set of flags controlling th e windows appearance and behaviour
	 */
	Window(const std::string& title, int x, int y, int width, int height, std::uint32_t flags=ELW_WIN_DEFAULT):
		_id(create_window(title.c_str(), -1, 0, x, y, width, height, flags)),
		_next_widget_id(0)
	{
		check_and_set_handlers();
	}
	/*!
	 * \brief Constructor
	 *
	 * Create a new Window using the ID of a window previously created using create_window.
	 * \param id The unique identifier of the existing window
	 */
	Window(int id): _id(id), _next_widget_id(0)
	{
		check_and_set_handlers();
	}
	//! Destructor
	~Window()
	{
		destroy_window(id());
	}

	//! Return a constant reference to the underlying window_info structure
	const window_info& window() const { return windows_list.window[_id]; }
	//! Return a variable reference to the underlying window_info structure
	window_info& window() { return windows_list.window[_id]; }
	//! Return a pointer to the underlying window_info structure
	window_info* window_ptr() { return &window(); }

	//! Return the identifier for this window
	int id() const { return _id; }
	//! Return the ID of the parent window of this window. Will be < 0 for root windows
	int parent_id() const { return window().pos_id; }
	//! Return the width in pixels of this window
	int width() const { return window().len_x; }
	//! Return the height in pixels of this window
	int height() const { return window().len_y; }
	//! Return the current UI scale factor for this window
	float current_scale() const { return window().current_scale; }
	//! Return the size (in pixels) of a close box in this window
	int box_size() const { return window().box_size; }
	//! Return the height of the title bar
	int title_height() const { return window().title_height; }
	//! Return the default text font category for this window
	FontManager::Category font_category() const { return window().font_category; }
	//! Return the maximum width of a character in the default sized font for this window
	int default_font_max_char_width() const { return window().default_font_max_len_x; }
	//! Return the height of a text line in the default sized font for this window
	int default_font_height() const { return window().default_font_len_y; }
	//! Return the maximum width of a character in the small sized font for this window
	int small_font_max_char_width() const { return window().small_font_max_len_x; }
	//! Return the height of a text line in the small sized font for this window
	int small_font_height() const { return window().small_font_len_y; }

	//! Return the width of the widget with ID \a id in this window
	int widget_width(int widget_id) const { return widget_get_width(id(), widget_id); }
	//! Return the height of the widget with ID \a id in this window
	int widget_height(int widget_id) const { return widget_get_height(id(), widget_id); }
	//! Return the flags controlling the appearance of the widget with ID \a id in this window
	int widget_flags(int widget_id) const
	{
		widget_list *widget = widget_find(id(), widget_id);
		return widget ? widget->Flags : 0;
	}
	//! Get the content scale factor for widget \a widget_id
	float widget_size(int widget_id) const
	{
		widget_list *widget = widget_find(id(), widget_id);
		return widget ? widget->size : 0.0;
	}
	//! Set the window flags in \a flags in addition to the current flags
	void add_flags(std::uint32_t flags) { window().flags |= flags; }
	//! Unset the window flags in \a flags, leaving the other flags as they are
	void remove_flags(std::uint32_t flags) { window().flags &= ~flags; }
	//! Move this window to position (\a x, \a y);
	void move(int x, int y)
	{
		move_window(id(), parent_id(), 0, x, y);
	}
	//! Resize this window to \a width × \a height pixels
	void resize(int width, int height)
	{
		resize_window(id(), width, height);
	}
	//! Set the minimum size of this window to \a min_width × \a min_height pixels.
	void set_minimum_size(int min_width, int min_height)
	{
		if (min_width >= 0 && min_height >= 0)
		{
			window().min_len_x = min_width;
			window().min_len_y = min_height;
		}
	}
	//! Set the custom UI scale factor for this window to that defined for window type \a managed_win.
	void set_custom_scale(managed_window_enum managed_win)
	{
		set_window_custom_scale(id(), managed_win);
	}
	//! Set the text font category for this window to \a font_cat.
	void set_font_category(FontManager::Category font_cat)
	{
		set_window_font_category(id(), font_cat);
	}

	/*!
	 * \brief Add a text button
	 *
	 * Add a text button with label \a label to this window.
	 * \param label    The text string to draw on the button
	 * \param x        The x coordinate of the top left corner of the widget
	 * \param x        The y coordinate of the top left corner of the widget
	 * \param width    The width of the widget, in pixels
	 * \param height   The height of the widget, in pixels
	 * \param flags    Flags controlling the widget's appearance
	 * \param on_init  Function to call on initialization of the widget
	 * \param size     Scale factor for the text within the widget
	 * \return The ID of the new button
	 */
	template <typename Callback>
	int add_button(const std::string& label, Callback&& on_click,
		std::uint16_t x, std::uint16_t y, std::uint32_t flags,
		std::uint16_t width=0, std::uint16_t height=0, int (*on_init)()=nullptr, float size=-1.0)
	{
		float act_size = size >= 0.0 ? size : current_scale();
		int button_id = button_add_extended(id(), _next_widget_id++, on_init, x, y, width, height,
			flags, act_size, label.c_str());
		widget_set_args(id(), button_id, new std::function<int()>(on_click));
		widget_set_OnClick(id(), button_id, reinterpret_cast<int (*)()>(window_static_handlers::button_click));
		widget_set_OnDestroy(id(), button_id, reinterpret_cast<int (*)()>(window_static_handlers::delete_button_click));
		return button_id;
	}
	/*!
	 * \brief Add a scrollbar
	 *
	 * Add a scrollbar to this window.
	 * \param bar_length    The range of the scrollbar values
	 * \param increment     The change in value between consecutive scrollbar positions
	 * \param x             The x coordinate of the top left corner of the widget
	 * \param x             The y coordinate of the top left corner of the widget
	 * \param width         The width of the widget, in pixels
	 * \param height        The height of the widget, in pixels
	 * \param flags         Flags controlling the widget's appearance
	 * \param on_init       Function to call on initialization of the widget
	 * \param size          Scale factor for the text within the widget
	 * \param init_position Initial value of the slider
	 * \return The ID of the new scrollbar
	 */
	int add_scrollbar(int bar_length, int increment, std::uint16_t x, std::uint16_t y,
		std::uint16_t width, std::uint16_t height, std::uint32_t flags,
		int (*on_init)()=nullptr, float size=-1.0, int init_position=0)
	{
		float act_size = size >= 0.0 ? size : current_scale();
		return vscrollbar_add_extended(id(), _next_widget_id++, on_init, x, y, width, height, flags,
			act_size, init_position, increment, bar_length);
	}
	/*!
	 * \brief Add a text field
	 *
	 * Add a new text field widget to this window.
	 * \param buf      The text messages to draw
	 * \param buf_len  The number of messages in \a buf
	 * \param x        The x coordinate of the top left corner of the widget
	 * \param x        The y coordinate of the top left corner of the widget
	 * \param width    The width of the widget, in pixels
	 * \param height   The height of the widget, in pixels
	 * \param x_margin Horizontal margin between border and text within the widget
	 * \param y_margin Vertical margin between border and text within the widget
	 * \param flags    Flags controlling the widget's appearance
	 * \param font_cat The catgeory of the font in which the widget text is drawn
	 * \param on_init  Function to call on initialization of the widget
	 * \param size     Scale factor for the text within the widget
	 * \param filter   Message channel filter to show only certain types of messages
	 * \return The ID of the new text widget
	 */
	int add_text_field(text_message *buf, size_t buf_len, std::uint16_t x, std::uint16_t y,
		std::uint16_t width, std::uint16_t height, std::uint16_t x_margin, std::uint16_t y_margin,
		std::uint32_t flags, FontManager::Category font_cat,
		int (*on_init)()=nullptr, float size=-1.0, std::uint8_t filter=FILTER_ALL)
	{
		float act_size = size >= 0.0 ? size : current_scale();
		return text_field_add_extended(id(), _next_widget_id++, on_init, x, y, width, height, flags,
			font_cat, act_size, buf, buf_len, filter, x_margin, y_margin);
	}

	//! Move the widget with ID \a widget_id to position (\a x, \a y)
	void move_widget(int widget_id, std::uint16_t x, std::uint16_t y)
	{
		widget_move(id(), widget_id, x, y);
	}
	//! Change the size of the widget with ID \a widget_id in this window to \a width × \a height.
	void resize_widget(int widget_id, std::uint16_t width, std::uint16_t height)
	{
		widget_resize(id(), widget_id, width, height);
	}
	/*!
	 * \brief Change the button size
	 *
	 * Change the size of button \a button_id to \a width × \a height, and set the font size to
	 * \a size. Both \a width and \a height can be zero, in which case the corresponding values
	 * for the button remain unchanged.
	 * \param button_id The ID of the button widget in this window
	 * \param width     The new width of the button, or 0
	 * \param height    The new width of the button, or 0
	 * \param size      The new text size of the button label
	 */
	void resize_button(int button_id, std::uint16_t width, std::uint16_t height, float size)
	{
		button_resize(id(), button_id, width, height, size);
	}
	//! Remove the widget with ID \a widget_id from this window
	void destroy_widget(int widget_id) { widget_destroy(id(), widget_id); }
	//! Enable the widget with ID \a widget_id in this window
	void enable_widget(int widget_id) { remove_widget_flags(widget_id, WIDGET_DISABLED); }
	//! Disable the widget with ID \a widget_id in this window
	void disable_widget(int widget_id) { add_widget_flags(widget_id, WIDGET_DISABLED); }
	//! Add the flags in \a flags to the widget flags of the widget with ID \a id in this window.
	void add_widget_flags(int widget_id, std::uint32_t flags)
	{
		std::uint32_t cur_flags = widget_flags(widget_id);
		widget_set_flags(id(), widget_id, cur_flags | flags);
	}
	//! Remove the flags in \a flags from the widget flags of the widget with ID \a id in this window.
	void remove_widget_flags(int widget_id, std::uint32_t flags)
	{
		widget_unset_flags(id(), widget_id, flags);
	}
	//! Set the widget specific arguments for widget \a widget_id to \a args.
	void set_widget_arguments(int widget_id, void* args) { widget_set_args(id(), widget_id, args); }

	/*!
	 * \brief Static display handler
	 *
	 * Static display handler that calls \c Derived::display_handler(). This function is installed
	 * as the display handler in the underlying window_info structure if a
	 * \c Derived::display_handler() exists.
	 */
	DEFINE_STATIC_HANDLER0(display)
	/*!
	 * \brief Static click handler
	 *
	 * Static mouse click handler that calls \c Derived::click_handler(). This function is installed
	 * as the click handler in the underlying window_info structure if a
	 * \c Derived::click_handler() exists.
	 */
	DEFINE_STATIC_HANDLER3(click, int, int, std::uint32_t);
	/*!
	 * \brief Static mouseover handler
	 *
	 * Static mouseover handler that calls \c Derived::mouseover_handler(). This function is
	 * installed as the mouseover handler in the underlying window_info structure if a
	 * \c Derived::mouseover_handler() exists.
	 */
	DEFINE_STATIC_HANDLER2(mouseover, int, int)
	/*!
	 * \brief Static resize handler
	 *
	 * Static resize handler that calls \c Derived::resize_handler(). This function is installed
	 * as the resize handler in the underlying window_info structure if a
	 * \c Derived::resize_handler() exists.
	 */
	DEFINE_STATIC_HANDLER2(resize, int, int)
	/*!
	 * \brief Static UI scale handler
	 *
	 * Static user interface scale handler that calls \c Derived::ui_scale_handler(). This function
	 * is installed as the UI scale handler in the underlying window_info structure if a
	 * \c Derived::ui_scale_handler() exists.
	 */
	DEFINE_STATIC_HANDLER0(ui_scale)
	/*!
	 * \brief Static font change handler
	 *
	 * Static font change handler that calls \c Derived::font_change_handler(). This function is
	 * installed as the font change handler in the underlying window_info structure if a
	 * \c Derived::font_change_handler() exists.
	 */
	DEFINE_STATIC_HANDLER1(font_change, FontManager::Category)

protected:
	/*!
	 * \brief Extract a Window from the underlying window_info structure
	 *
	 * Return the pointer to itself that a Window stores in the \c data member of its underlying
	 * window_info structure.
	 * \param win Pointer to a window_info structure represented by a Window.
	 * \return Pointer to the child class deriving from Window that was stored in \a win.
	 */
	static Derived& get_cpp_window(window_info *win)
	{
		return *reinterpret_cast<Derived*>(win->data);
	}

private:
	//! The unique identifier for this window
	int _id;
	//! ID counter for widgets added to this window
	std::atomic_int _next_widget_id;

	/*!
	 * \brief Check the window ID, and set event handlers
	 *
	 * This function is called by the constructors. It checks if the stored window ID is valid,
	 * stores a pointer to this window in the underlying window struct so this object can be
	 * retrieved from underlying data, and sets any window handlers defined by the \a Derived class.
	 */
	void check_and_set_handlers()
	{
		if (_id < 0 || _id >= windows_list.num_windows)
			EXTENDED_EXCEPTION(ExtendedException::ec_invalid_parameter, "Invalid window id " << _id);
		window().data = this;

		set_display_handler(this);
		set_mouseover_handler(this);
		set_click_handler(this);
		set_resize_handler(this);
		set_ui_scale_handler(this);
		set_font_change_handler(this);
	}
};


DECLARE_SET_HANDLER(display, ELW_HANDLER_DISPLAY)
DECLARE_SET_HANDLER(click, ELW_HANDLER_CLICK)
DECLARE_SET_HANDLER(mouseover, ELW_HANDLER_MOUSEOVER)
DECLARE_SET_HANDLER(resize, ELW_HANDLER_RESIZE)
DECLARE_SET_HANDLER(ui_scale, ELW_HANDLER_UI_SCALE)
DECLARE_SET_HANDLER(font_change, ELW_HANDLER_FONT_CHANGE)

} // namespace eternal_lands

#endif // CPPWINDOWS_H
