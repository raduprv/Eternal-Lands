#ifndef TEXTPOPUP_H
#define TEXTPOPUP_H

/*!
 * \file
 * \ingroup elwindows
 * \brief Simple popup window for displaying text messages
 */

#include <memory>
#include "cppwindows.h"
#include "text.h"

namespace eternal_lands
{

/*!
 * \brief Class for text message popups
 *
 * Class TextPopup can be used to display a simple text message to the user, with one or more
 * buttons underneath for various actions to undertake.
 */
class TextPopup: public Window<TextPopup>
{
public:
	/*!
	 * \brief Constructor
	 *
	 * Create a new text popup window with title \a title, and text contents \a text. No buttons are
	 * added yet.
	 *
	 * \param title The title of the new window
	 * \param text  The initial text of the popup
	 */
	TextPopup(const std::string& title, const ustring& text);
	//! Destructor
	~TextPopup();

	/*!
	 * \brief Append text
	 *
	 * Append \a text to the text currently shown on the window. The new text is separated from
	 * the existing text by an empty line.
	 *
	 * \param text The new text to add
	 */
	void add_message(const ustring& text);
	/*!
	 * \brief Add a button
	 *
	 * Add a button with text \a label to the popup. When the button is clicked, \a on_click
	 * is called without arguments. It should return a \c bool, specifying whether the click
	 * event has been handled.
	 *
	 * \param label    The test displayed on the button
	 * \param on_click The function to call when the button is clicked
	 */
	template <typename Callback>
	void add_button(const std::string& label, Callback&& on_click)
	{
		int id = Window::add_button(label, on_click, 0, 0, 0);
		_button_ids.push_back(id);
		set_size();
	}

	//! Mouse click handler that forwards scroll events to the scrollbar
	int click_handler(int x, int y, std::uint32_t flags);
	//! Handler called when the window is resized
	int resize_handler(int new_width, int new_height);
	//! Handler called when the window's scale factor is changed
	int ui_scale_handler();
	//! Handler called when a font or font size is changed
	int font_change_handler(FontManager::Category cat);

private:
	//! The default window flags to use for the popup
	static const std::uint32_t default_window_flags = ELW_USE_UISCALE | ELW_DRAGGABLE
		| ELW_USE_BACKGROUND | ELW_USE_BORDER | ELW_SHOW | ELW_ALPHA_BORDER | ELW_SWITCHABLE_OPAQUE;
	//! The (unscaled) margin between elements in the popup
	static const std::uint16_t default_margin = 5;
	//! The default separator between messages in the popup
	static const ustring default_message_separator;

	//! The content text of the popup
	text_message _message;
	//! ID of the text widget displaying the text
	int _text_widget_id;
	//! ID of the scrollbar widget for scrolling through longer text messages
	int _scrollbar_id;
	//! IDs of the buttons
	std::vector<int> _button_ids;
	//! The actual scaled horizontal and vertical margins between elements in the popup
	std::uint16_t _x_margin, _y_margin;
	//! Separator between different messages in the popup
	ustring _separator;

	//! Return whether the scrollbar is currently in use
	bool has_scrollbar() const { return (widget_flags(_scrollbar_id) & WIDGET_DISABLED) == 0; }

	//! Calculate and set the minimum and actual size of the popup window, based on the contents
	void set_size();

	//! Return the width of the button row
	int buttons_width() const;
	//! Return the total height of the contents, inclusing text and buttons
	int height(int nr_lines) const { return text_height(nr_lines) + non_text_height(); }
	//! Return the height needed to display the text only
	int text_height(int nr_lines) const;
	//! Return the height of all but the text
	int non_text_height() const;
};

} // namespace eternal_lands

#endif // TEXTPOPUP_H
