#include "textpopup.h"
#include "asc.h"
#include "gamewin.h"

namespace eternal_lands
{

const ustring TextPopup::default_message_separator = reinterpret_cast<const std::uint8_t*>("\n\n");

TextPopup::TextPopup(const std::string& title, const ustring& text):
	Window(title, 0, 0, 0, 0, default_window_flags), _max_width(window_width),
	_text_widget_id(-1), _scrollbar_id(-1), _button_ids(),
	_x_margin(0), _y_margin(0), _separator(default_message_separator)
{
	_x_margin = std::round(default_margin * current_scale());
	_y_margin = std::round(default_margin * current_scale());

	init_text_message(&_message, 3 * text.size());
	set_text_message_data(&_message, reinterpret_cast<const char*>(text.c_str()));
	_text_widget_id = add_text_field(&_message, 1, _x_margin, _y_margin, window_width, window_height,
		_x_margin, _y_margin, TEXT_FIELD_NO_KEYPRESS, CHAT_FONT, nullptr, 1.0);
	_scrollbar_id = add_scrollbar(0, 1, [this](int line) {
			scroll_textfield_to_line(_text_widget_id, line);
			return 1;
		}, 0, 0, box_size(), 0, 0);
	add_widget_flags(_scrollbar_id, WIDGET_DISABLED);

	set_size();
}

TextPopup::~TextPopup()
{
	free_text_message_data(&_message);
}

TextPopup& TextPopup::set_max_width(int width)
{
	_max_width = width;
	return *this;
}

TextPopup& TextPopup::add_message(const ustring& text)
{
	// FIXME? Shouldn't there be a function on text_message for this?
	resize_text_message_data(&_message, _message.len + 3*text.size() + _separator.size());
	if (!text_message_is_empty(&_message))
		safe_strcat(_message.data, reinterpret_cast<const char*>(_separator.c_str()), _message.size);
	safe_strcat(_message.data, reinterpret_cast<const char*>(text.c_str()), _message.size);
	_message.len = strlen(_message.data);

	// This will re-wrap the text and add a scrollbar, title and resize widget as required
	set_size();

	return *this;
}

void TextPopup::set_size()
{
	bool have_text = !text_message_is_empty(&_message);
	int min_height = height(have_text ? 0 : 1);
	int min_width = box_size() + 2 * _x_margin
		+ std::max(buttons_width(), (int)(5 * DEFAULT_FIXED_FONT_WIDTH));
	Window::set_minimum_size(min_width, min_height);

	int screen_width = window_width - HUD_MARGIN_X;
	int max_width = std::min(screen_width - 2*_x_margin, std::max(min_width, _max_width));
	int screen_height = window_height - HUD_MARGIN_Y;
	int max_height = screen_height - 2 * _y_margin;

	// Do a pre-wrap of the text to the maximum screen width we can use.
	// This will avoid the later wrap (after the resize) changing the number of lines
	int num_text_lines = 0;
	if (have_text)
		num_text_lines = rewrap_message(&_message, CHAT_FONT, 1.0, max_width - 4*_x_margin, nullptr);

	// Calculate the text widget height from the number of lines...
	int win_height = non_text_height();
	if (have_text)
		win_height += text_height(num_text_lines);

	// ...but limit to a maximum
	bool have_scrollbar = has_scrollbar();
	if (win_height > max_height - (have_scrollbar ? title_height() : 0))
	{
		win_height = max_height - (have_scrollbar ? title_height() : 0);
		int text_widget_height = win_height - non_text_height();

		// If we'll need a scroll bar allow for it in the width calulation
		if (have_text && text_widget_height < text_height(num_text_lines))
			have_scrollbar = true;
	}

	// Calculate the required window width for the text size...
	int text_widget_width = 0;
	if (have_text)
	{
		// The fudge is because the line wrapping code allows for a cursor to fit on the last
		// position in the line, but this is not reflected in the actual width of the line. Hence,
		// if the last character in the line is less wide than the cursor, the calculated width will
		// be too small according to rewrap_message(). Add the width of a cursor to the required
		// width to be certain it is large enough.
		int fudge = get_char_width_zoom('_', font_category(), current_scale());
		text_widget_width = fudge + 2*_x_margin + _message.max_line_width;
	}
	int win_width = text_widget_width + 2*_x_margin;
	if (have_scrollbar)
		win_width += box_size();

	// ...but limit to a maximum
	win_width = std::min(win_width, max_width);

	// Resize the window now we have the required size
	// New sizes and positions for the widgets will be calculated by the callback
	resize(win_width, win_height);

	// Calculate the best position then move the window */
	move((screen_width - width())/2, (screen_height - Window::height())/2);
}

int TextPopup::buttons_width() const
{
	if (_button_ids.empty())
		return 0;
	int width = (_button_ids.size() - 1) * _x_margin;
	for (int id: _button_ids)
		width += widget_width(id);
	return width;
}

int TextPopup::text_height(int nr_lines) const
{
	if (nr_lines > 0)
		return 1 + 2 * _y_margin + get_text_height(nr_lines, CHAT_FONT, 1.0);
	else
		return 0;
}

int TextPopup::non_text_height() const
{
	if (_button_ids.empty())
		return 0;
	else
		return 3 * _y_margin + widget_height(_button_ids[0]);
}

int TextPopup::click_handler(int x, int y, std::uint32_t flags)
{
	if (!has_scrollbar() || (flags & ELW_WHEEL) == 0)
		// We're done
		return 1;

	if (flags & ELW_WHEEL_UP)
		scroll_up(_scrollbar_id);
	else if (flags & ELW_WHEEL_DOWN)
		scroll_down(_scrollbar_id);

	scroll_textfield_to_line(_text_widget_id, scrollbar_position(_scrollbar_id));
	return 1;
}

int TextPopup::resize_handler(int new_width, int new_height)
{
	// If there is no text, we're done
	if (text_message_is_empty(&_message))
		return 1;

	// set the text widget height
	int text_widget_height = new_height - non_text_height();

	// First try if the text fits without a scrollbar
	int text_widget_width = new_width - 2*_x_margin;
	int num_text_lines = rewrap_message(&_message, CHAT_FONT, 1.0, text_widget_width - 2*_x_margin, nullptr);

	// If we need a scroll bar, adjust the width and wrap again
	bool need_scrollbar = text_widget_height < text_height(num_text_lines);
	if (need_scrollbar)
	{
		text_widget_width -= box_size();
		// Rewrap the text again as the available width is now less
		num_text_lines = rewrap_message(&_message, CHAT_FONT, 1.0, text_widget_width - 2*_x_margin, nullptr);
	}
	resize_widget(_text_widget_id, text_widget_width, text_widget_height);

	// If the text widget is really short, the scroll bar can extend beyond the height
	// This could be considered a bug in the scroll widget - try to avoid anyway
	if (need_scrollbar)
	{
		int local_min = non_text_height() + std::max(text_height(1), 3 * box_size());
		if (new_height < local_min)
		{
			resize(new_width, local_min);
			return 1;
		}
	}

	// Create the scroll bar reusing any existing scroll position
	if (need_scrollbar)
	{
		int nr_visible = textfield_visible_lines(_text_widget_id);
		set_scrollbar_length(_scrollbar_id, std::max(0, num_text_lines - nr_visible));
		enable_widget(_scrollbar_id);
		move_widget(_scrollbar_id, width() - box_size(), _y_margin);
		resize_widget(_scrollbar_id, box_size(), text_widget_height);

		// If we have a scroll bar, enable the title and resize properties
		add_flags(ELW_RESIZEABLE | ELW_TITLE_BAR);
	}
	// If no scroll bar, make sure the text is at the start of the buffer
	else
	{
		disable_widget(_scrollbar_id);
		scroll_textfield_to_line(_text_widget_id, 0);
		remove_flags(ELW_RESIZEABLE | ELW_TITLE_BAR);
	}

	// Center the buttons on the bottom of the window
	if (!_button_ids.empty())
	{
		std::uint16_t x = (width() - buttons_width()) / 2;
		std::uint16_t y = Window::height() - _y_margin - widget_height(_button_ids[0]);
		for (int id: _button_ids)
		{
			move_widget(id, x, y);
			x += widget_width(id) + _x_margin;
		}
	}

	return 1;
}

int TextPopup::ui_scale_handler()
{
	for (int id: _button_ids)
		resize_button(id, 0, 0, current_scale());
	set_size();
	return 1;
}

int TextPopup::font_change_handler(FontManager::Category cat)
{
	if (cat != font_category() && cat != CHAT_FONT)
		return 0;
	set_size();
	return 1;
}

} // namespace eternal_lands

