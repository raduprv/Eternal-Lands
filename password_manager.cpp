/*
 * Save username and passwords details at login and allow selection from the login screen.
 * Also store new character and successful password change details.
 * http://www.eternal-lands.com/forum/index.php?/topic/61189-new-login-screen-for-multiple-characters/
 *
 * Author bluap/pjbroad March 2019.
 *
*/

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

#include "elloggingwrapper.h"
#include "elwindows.h"
#include "font.h"
#include "gl_init.h"
#include "loginwin.h"
#include "multiplayer.h"
#include "named_colours.h"
#include "io/elpathwrapper.h"
#include "password_manager.h"
#include "translate.h"
#include "sound.h"
#include "xor_cipher.hpp"

extern "C"
{
	static int display_pm_handler(window_info *win);
	static int ui_scale_pm_handler(window_info *win);
	static int change_pm_font_handler(window_info *win, font_cat cat);
	static int click_pm_handler(window_info *win, int mx, int my, Uint32 flags);
	static int mouseover_pm_handler(window_info *win, int mx, int my);
	static int click_show_password(widget_list *w, int mx, int my, Uint32 flags);
}

namespace Password_Manaager
{
	//	A simple class to hold a username and password pair
	//
	class Login
	{
		public:
			Login(const std::string& the_name, const std::string& the_password):
				user_name(the_name), clear_password(the_password) {}
			const std::string & get_name(void) const { return user_name; }
			const std::string & get_password(void) const { return clear_password; }
			static bool sort_compare(const Login &a, const Login &b);
		private:
			std::string user_name, clear_password;
	};

	// Used by the sort algorithm to alphabetically compare two Login names, case insensitive
	//
	bool Login::sort_compare(const Login &a, const Login &b)
	{
		std::string alower(a.get_name());
		std::transform(alower.begin(), alower.end(), alower.begin(), tolower);
		std::string blower(b.get_name());
		std::transform(blower.begin(), blower.end(), blower.begin(), tolower);
		return alower < blower;
	}

	//	A simple class to storge and manage a collection of username/password pairs.
	//
	class Logins
	{
		public:
			Logins(void) :
				file_name(std::string(get_path_config()) + "passmngr_logins"),
				cipher(std::string(get_path_config()) + "passmngr_key", MAX_USERNAME_LENGTH)
				{ load(); }
			void load(void);
			void set_details(void) const;
			void add(const std::string& user_name, const std::string& password);
			void pending_change(const char * old_and_new_password);
			void confirm_change(void);
			size_t size(void) const { return logins.size(); }
			std::vector<Login>::const_iterator begin() const { return logins.begin(); }
			std::vector<Login>::const_iterator end() const { return logins.end(); }
		private:
			bool common_add(const std::string& user_name, const std::string& password);
			void save(void);
			std::string pending_old_and_new_password;
			std::vector<Login> logins;
			std::string file_name;
			XOR_Cipher::Cipher cipher;
	};

	//	Add if the specified username/password are valid and new or modified.
	//	Return true if added.
	//
	bool Logins::common_add(const std::string& user_name, const std::string& password)
	{
		if ((user_name.size() >= MAX_USERNAME_LENGTH) || (password.size()  >= MAX_USERNAME_LENGTH))
		{
			LOG_ERROR("%s: Error adding username [%s]\n", __PRETTY_FUNCTION__, user_name.c_str());
			return false;
		}
		std::string lc_username = user_name;
		std::transform(lc_username.begin(), lc_username.end(), lc_username.begin(), tolower);
		for(std::vector<Login>::iterator i=logins.begin(); i<end(); ++i)
		{
			std::string temp_lower = i->get_name();
			std::transform(temp_lower.begin(), temp_lower.end(), temp_lower.begin(), ::tolower);
			if (temp_lower == lc_username)
			{
				if ((i->get_name() == user_name) && (i->get_password() == password))
					return false;
				logins.erase(i);
				break;
			}
		}
		logins.push_back(Login(user_name, password));
		return true;
	}

	//	Add a single new username/password pair if valid and new, then save.  Not used when loading from file.
	//
	void Logins::add(const std::string& user_name, const std::string& password)
	{
		if (common_add(user_name, password))
		{
			std::sort(logins.begin(), logins.end(), Login::sort_compare);
			save();
		}
	}

	//	Save details when a password change is requested with a #change_pass command.  It will not be saved until confirmed.
	//
	void Logins::pending_change(const char * old_and_new_password)
	{
		pending_old_and_new_password = std::string(old_and_new_password);
	}

	//	Called when the server sends the change password confirmation.  Save the new password.
	void Logins::confirm_change(void)
	{
		std::istringstream iss(pending_old_and_new_password);
		std::vector<std::string> words((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());
		if ((words.size() == 2) && (words[1].size() < MAX_USERNAME_LENGTH))
		{
			add(get_username(), words[1]);
			set_password(words[1].c_str());
		}
		pending_old_and_new_password = std::string();
	}

	//	Set the current password assiociated with the current username.
	//
	void Logins::set_details(void) const
	{
		std::string user_name = std::string(get_username());
		set_username(get_username());  // set all versions of the username
		for(std::vector<Login>::const_iterator i=begin(); i<end(); ++i)
		{
			if (i->get_name() == user_name)
			{
				set_password(i->get_password().c_str());
				return;
			}
		}
	}

	// Load username/password pairs from the file in the current config directory.
	//
	void Logins::load(void)
	{
		logins.clear();
		std::ifstream in(file_name.c_str());
		if (!in)
		{
			LOG_ERROR("%s: Failed to open [%s]\n", __PRETTY_FUNCTION__, file_name.c_str());
			return;
		}
		std::string line;
		while (getline(in, line))
		{
			std::istringstream iss(line);
			std::vector<std::string> words((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());
			if ((words.size() == 2))
			{
				if (!common_add(words[0], cipher.decrypt(cipher.hex_to_cipher(words[1]))))
					LOG_ERROR("%s: Not loading username [%s]\n", __PRETTY_FUNCTION__, words[0].c_str());
			}
			else
				LOG_ERROR("%s: Invalid line [%s]\n", __PRETTY_FUNCTION__, line.c_str());
		}
		std::sort(logins.begin(), logins.end(), Login::sort_compare);
		in.close();
	}

	// Save username/password pairs to the file in the current config directory.
	//
	void Logins::save(void)
	{
		std::ofstream out(file_name.c_str(), std::ios_base::out | std::ios_base::trunc);
		if (out)
		{
			for(std::vector<Login>::const_iterator i=begin(); i<end(); ++i)
				out << i->get_name() << " " << cipher.cipher_to_hex(cipher.encrypt(i->get_password())) << std::endl;
			out.close();
		}
		else
			LOG_ERROR("%s: Failed to write [%s]\n", __PRETTY_FUNCTION__, file_name.c_str());
	}

	//	A simple class to manage the username list section window.
	//
	class Window
	{
		public:
			Window() : window_id(-1), checkbox_id(0), checkbox_label_id(0), show_passwords(0), mouse_over_line(-1) {}
			~Window() { destroy(); }
			void open(void);
			int display(Logins *logins, window_info *win);
			int click(Logins *logins, window_info *win, int mx, int my, Uint32 flags);
			int mouseover(Logins *logins, window_info *win, int mx, int my);
			int ui_scale(Logins *logins, window_info *win);
			int change_font(Logins *logins, window_info *win, eternal_lands::FontManager::Category cat);
			void toggle_show_password(Logins *logins, widget_list *w);
			void resize(Logins *logins) { ui_scale(logins, &windows_list.window[window_id]); }
		private:
			void destroy(void);
			int border_x, border_y, username_sep_y;
			int window_id;
			Uint32 scroll_id, checkbox_id, checkbox_label_id;
			int show_passwords;
			int mouse_over_line;
			size_t max_displayed;
	};

	//  Set and update the list window scale.
	//
	int Window::ui_scale(Logins *logins, window_info *win)
	{
		border_x = border_y = username_sep_y = (int)(0.5 + win->current_scale * 10);
		size_t max_available = static_cast<size_t>(0.8 * window_height - 2 * border_y + username_sep_y) / (username_sep_y + win->default_font_len_y);
		max_displayed = std::min(max_available, logins->size());
		int height = 2 * border_y + max_displayed * win->default_font_len_y + (max_displayed - 1) * username_sep_y;
		int width = 2 * border_x + win->default_font_max_len_x * (MAX_USERNAME_LENGTH - 1) + win->box_size;
		if (show_passwords)
			width += border_x + win->default_font_max_len_x * (MAX_USERNAME_LENGTH - 1);
		height = std::max(height, 4 * win->box_size);

		int y_box, y_label;
		if (checkbox_id > 0)
			widget_destroy(win->window_id, checkbox_id);
		if (checkbox_label_id > 0)
			widget_destroy(win->window_id, checkbox_label_id);
		if (win->box_size >= win->default_font_len_y)
		{
			y_box = height;
			y_label = height + (win->box_size - win->default_font_len_y) / 2;
		}
		else
		{
			y_box = height + (win->default_font_len_y - win->box_size) / 2;
			y_label = height;
		}
		checkbox_id = checkbox_add_extended(win->window_id, 2, NULL, border_x, y_box,
			win->box_size, win->box_size, 0, win->current_scale, &show_passwords);
		checkbox_label_id = label_add_extended(window_id, 3, NULL, 2 * border_x + win->box_size, y_label, 0, win->current_scale, show_passwords_str);
		widget_set_OnClick(window_id, checkbox_id, (int (*)())&click_show_password);
		widget_set_OnClick(window_id, checkbox_label_id, (int (*)())&click_show_password);
		height += std::max(win->box_size, win->default_font_len_y) + border_y;
		width = std::max(width, 4 * border_x + 2 * win->box_size + widget_get_width(win->window_id, checkbox_label_id));

		widget_resize(win->window_id, scroll_id, win->box_size, height - win->box_size);
		widget_move(win->window_id, scroll_id, width - win->box_size, win->box_size);
		vscrollbar_set_bar_len(win->window_id, scroll_id, ((logins->size() < max_displayed) ?0: logins->size() - max_displayed));

		resize_window(window_id, width, height);
		move_window(window_id, win->pos_id, win->pos_loc, window_width / 2 + ((window_width / 2 - width) / 2), (window_height - height) / 2);
		return 1;
	}

	int Window::change_font(Logins *logins, window_info *win, eternal_lands::FontManager::Category cat)
	{
		if (cat != win->font_category)
			return 0;
		ui_scale(logins, win);
		return 1;
	}

	//	Handle if the show password checkbox or label are clicked
	//
	void Window::toggle_show_password(Logins *logins, widget_list *w)
	{
		if (w->id == checkbox_label_id)
		{
			do_click_sound();
			show_passwords ^= 1;
		}
		ui_scale(logins, &windows_list.window[window_id]);
	}

	//	Handle mouse over events, highlighting the username under the mouse.
	//
	int Window::mouseover(Logins *logins, window_info *win, int mx, int my)
	{
		if ((my > border_y) && (my < win->len_y - border_y))
		{
			if ((mx > border_x) && (mx < (win->len_x - win->box_size - border_x)))
			{
				mouse_over_line = (my - border_y) / (username_sep_y + win->default_font_len_y);
				return 1;
			}
		}
		return 0;
	}

	//  Response to clicking on a username or scrolling the list window.
	//
	int Window::click(Logins *logins, window_info *win, int mx, int my, Uint32 flags)
	{
		if (flags & ELW_WHEEL_UP)
		{
			vscrollbar_scroll_up(win->window_id, scroll_id);
			return 1;
		}
		else if (flags & ELW_WHEEL_DOWN)
		{
			vscrollbar_scroll_down(win->window_id, scroll_id);
			return 1;
		}
		else if (((flags & ELW_LEFT_MOUSE) || (flags & ELW_RIGHT_MOUSE)) && (my > border_y) && (my < win->len_y - border_y))
		{
			if ((mx > border_x) && (mx < (win->len_x - win->box_size - border_x)))
			{
				size_t over_item = vscrollbar_get_pos(win->window_id, scroll_id) + (my - border_y) / (username_sep_y + win->default_font_len_y);
				if (over_item < logins->size())
				{
					set_username((logins->begin() + over_item)->get_name().c_str());
					set_password((logins->begin() + over_item)->get_password().c_str());
					do_click_sound();
					hide_window(window_id);
					if (flags & ELW_LEFT_MOUSE)
						send_login_info();
					return 1;
				}
			}
		}
		return 0;
	}

	//	Dispalay the username list window.
	//
	int Window::display(Logins *logins, window_info *win)
	{
		if (!passmngr_enabled)
		{
			hide_window(window_id);
			return 1;
		}
		int y = 0;
		size_t scroll_off = vscrollbar_get_pos(win->window_id, scroll_id);
		for (size_t i = scroll_off; i < (scroll_off + max_displayed); ++i)
		{
			if (i >= logins->size())
				break;
			std::vector<Login>::const_iterator curr = logins->begin() + i;
			if (i == static_cast<size_t>(scroll_off + mouse_over_line))
				elglColourN("global.mousehighlight");
			else
				glColor3f(1.0f, 1.0f, 1.0f);
			draw_string_zoomed (border_x, border_y + y, (const unsigned char*)curr->get_name().c_str(), 1, win->current_scale);
			if (show_passwords)
				draw_string_zoomed (2 * border_x + win->default_font_max_len_x * (MAX_USERNAME_LENGTH - 1), border_y + y, (const unsigned char*)curr->get_password().c_str(), 1, win->current_scale);
			y += win->default_font_len_y + username_sep_y;
		}
		mouse_over_line = -1;
		return 1;
	}

	//	Create or toggle open/close the username list window.
	//
	void Window::open(void)
	{
		if (window_id < 0)
		{
			window_id = create_window (login_select_window_str, -1, 0, 0, 0, 0, 0, ELW_USE_UISCALE|ELW_WIN_DEFAULT);
			set_window_handler (window_id, ELW_HANDLER_DISPLAY, (int (*)())&display_pm_handler);
			set_window_handler (window_id, ELW_HANDLER_MOUSEOVER, (int (*)())&mouseover_pm_handler);
			set_window_handler (window_id, ELW_HANDLER_CLICK, (int (*)())&click_pm_handler);
			set_window_handler (window_id, ELW_HANDLER_SHOW, (int (*)())&ui_scale_pm_handler);
			set_window_handler (window_id, ELW_HANDLER_UI_SCALE, (int (*)())&ui_scale_pm_handler);
			set_window_handler (window_id, ELW_HANDLER_FONT_CHANGE, (int (*)())&change_pm_font_handler);
			scroll_id = vscrollbar_add_extended (window_id, 1, NULL, 0, 0, 0, 0, 0, 1.0, 0, 1, 0);
			if (window_id >=0 && window_id < windows_list.num_windows)
				ui_scale_pm_handler(&windows_list.window[window_id]);
		}
		else
			toggle_window(window_id);
	}

	//	Destroy the username list window.
	//
	void Window::destroy(void)
	{
		destroy_window(window_id);
		window_id = -1;
	}
}

static Password_Manaager::Logins * logins = 0;
static Password_Manaager::Window * pm_window = 0;

static int display_pm_handler(window_info *win) { return pm_window->display(logins, win); }
static int ui_scale_pm_handler(window_info *win) { return pm_window->ui_scale(logins, win); }
static int change_pm_font_handler(window_info *win, font_cat cat) { return pm_window->change_font(logins, win, cat); }
static int click_pm_handler(window_info *win, int mx, int my, Uint32 flags) { return pm_window->click(logins, win, mx, my, flags); }
static int mouseover_pm_handler(window_info *win, int mx, int my) { return pm_window->mouseover(logins, win, mx, my); }
static int click_show_password(widget_list *w, int mx, int my, Uint32 flags) { pm_window->toggle_show_password(logins, w); return 1; }

//	The externam interface.
//
extern "C"
{
	int passmngr_enabled = 0;

	void passmngr_open_window(void)
	{
		if (!passmngr_enabled)
			return;
		if (!pm_window)
			pm_window = new Password_Manaager::Window();
		passmngr_init();
		logins->load();
		pm_window->open();
	}

	void passmngr_destroy_window(void)
	{
		if (pm_window)
		{
			delete pm_window;
			pm_window = 0;
		}
	}

	void passmngr_save_login(void)
	{
		if (!passmngr_enabled)
			return;
		passmngr_init();
		logins->add(get_username(), get_password());
	}

	void passmngr_set_login(void)
	{
		if (!passmngr_enabled)
			return;
		passmngr_init();
		logins->set_details();
	}

	void passmngr_resize(void)
	{
		if (!passmngr_enabled)
			return;
		if (pm_window)
		{
			passmngr_init();
			pm_window->resize(logins);
		}
	}

	void passmngr_pending_pw_change(const char * old_and_new_password)
	{
		if (!passmngr_enabled)
			return;
		passmngr_init();
		logins->pending_change(old_and_new_password);
	}

	void passmngr_confirm_pw_change(void)
	{
		if (!passmngr_enabled)
			return;
		passmngr_init();
		logins->confirm_change();
	}

	void passmngr_init(void)
	{
		if (passmngr_enabled && !logins)
			logins = new Password_Manaager::Logins();
	}

	void passmngr_destroy(void)
	{
		passmngr_destroy_window();
		if (logins)
		{
			delete logins;
			logins = 0;
		}
	}
}
