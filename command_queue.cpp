/*
Queues #commands: handles the muti-command line format and input of parameters.

Extracted from user_menu.cpp so that others can create queues.

Each Line object has two or more fields, the field separator is "||". The
first field is the text tag, the remaining field or fields are the
associated commands.

Commands can prompt for user input. If the command contains text inside "<>",
e.g. "some text <more text> some other text" then the text inside the "<>" will
be used as a prompt and the "<...>" replaced by the text entered by the user.
You can include as many input prompts as you wish.

pjbroad/bluap January 2013

*/


/* To Do:
	- block use of #suicide #reset #killme #change_pass - may be store in file
	- optionally put commands into previous command buffer
	- reuse entered text for a line if prompt repeated - vinoveritas suggestion
	- Need line <rule> <...>
		repeats 0-n times until "." entered.
		Can also use for [<number>]
*/

#include <sstream> 

#include "asc.h"
#include "chat.h"
#include "gl_init.h"
#include "text.h"
#include "translate.h"

#include "command_queue.hpp"

namespace CommandQueue
{
	//
	//	Construct a command object from a command string.  Parsing for
	//	input fields and splitting into text/input sections.
	//
	Command::Command(const std::string &command_text)
	{
		std::string::size_type from_index = 0;
		std::string::size_type to_index = 0;
		std::string::size_type len = 0;
		std::string start_str = "<";
		std::string end_str = ">";
		invalid_command = false;

		// loop extracting command and parameter sections
		// format is "text <parameter name> text <parameter name> text" e.t.c
		while ((to_index = command_text.find(start_str, from_index)) != std::string::npos)
		{
			if ((len = to_index-from_index) > 0)
				text_segments.push_back(command_text.substr(from_index, len));
			from_index = to_index + start_str.size();

			if ((to_index = command_text.find(end_str, from_index)) != std::string::npos)
			{
				if ((len = to_index-from_index) > 0)
					param_prompts.push_back(command_text.substr(from_index, len));
				from_index = to_index + end_str.size();
			}
			else
			{
				text_segments.clear();
				param_prompts.clear();
				text_segments.push_back(command_text);
				invalid_command = true;
				return;
			}
		}
		if ((len = command_text.size()-from_index) > 0)
			text_segments.push_back(command_text.substr(from_index, len));

		if (text_segments.empty() && param_prompts.empty())
			invalid_command = true;
			
	} // end Command::Command()


	//
	//	Given the paramters, contruct the command to issue from
	//	the text sections and parameter values.
	//
	void Command::action(const std::vector<std::string> &params) const
	{
		// log to the user an invalid command, formatting error
		if (invalid_command)
		{
			LOG_TO_CONSOLE(c_red1, um_invalid_command_str);
			return;
		}

		// append command text + parameter + text + paramter e.t.c.
		std::ostringstream command_text;
		for (size_t i=0; i<text_segments.size(); i++)
		{
			command_text << text_segments[i];
			if (params.size() > i)
				command_text << params[i];
		}
		for (size_t i=text_segments.size(); i<params.size(); i++)
			command_text << params[i];

		// issue the command
		size_t command_len = command_text.str().size() + 1;
		char temp[command_len];
		safe_strncpy(temp, command_text.str().c_str(), command_len);
		parse_input(temp, strlen(temp));
	}


	//
	//	Echo the command to the console, a menu window option
	//
	void Command::echo(void) const
	{
		// append command text + parameter + text + paramter e.t.c.
		std::ostringstream command_text;
		for (size_t i=0; i<text_segments.size(); i++)
		{
			command_text << text_segments[i];
			if (param_prompts.size() > i)
				command_text << "<" << param_prompts[i] << ">";
		}
		for (size_t i=text_segments.size(); i<param_prompts.size(); i++)
			command_text << "<" << param_prompts[i] << ">";
		LOG_TO_CONSOLE(c_grey1, command_text.str().c_str());
		
		// log to the user an invalid command, formatting error
		if (invalid_command)
			LOG_TO_CONSOLE(c_red1, um_invalid_command_str);
	}


	//
	//	Initialise the command queue
	//
	Queue::Queue(void) : last_time(0)
	{
		init_ipu(&ipu, -1, 300, 100, MAX_TEXT_MESSAGE_LENGTH, 3, cancel_handler, input_handler);
		ipu.x = (window_width - ipu.popup_x_len) / 2;
		ipu.y = (window_height - ipu.popup_y_len) / 2;
		ipu.data = static_cast<void *>(this);
	}


	//
	//	If the command queue is not empty, process the next command.
	//
	void Queue::process(bool just_echo)
	{
		// if required, print all the commands to the console emptying the queue 
		while (just_echo && !commands.empty())
		{
			commands.front().echo();
			commands.pop();
		}

		if (commands.empty())
			return;

		// delay consecutive commands by a small amount to avoid spamming
		Uint32 curr_time = SDL_GetTicks();
		if ((curr_time >= last_time) && ((curr_time - wait_time_ms) < last_time))
			return;

		// if the command needs parameter(s) prompt and wait for input
		if (params.size() < commands.front().get_prompts().size())
		{
			// if the input window is already open, continue waiting for input
			if (get_show_window(ipu.popup_win))
				return;
			// open the input window and continue waiting for input
			display_popup_win(&ipu, commands.front().get_prompts()[params.size()].c_str());
			return;
		}

		// we have any needed parameters so action the command and remove it form the queue
		commands.front().action(params);
		commands.pop();
		params.clear();
		last_time = curr_time;
	}

	//
	//	The input popup window cancel callback
	//
	void Queue::cancel(void)
	{
		if (commands.empty())
			return;
		while (!commands.empty())
			commands.pop();
		params.clear();
	}


	//
	//	If the user menu window is closed, clear the queue
	//
	void Queue::clear(void)
	{
		cancel();
		hide_window(ipu.popup_win);
	}


	//
	//	Set the delay between executing commands on a single user menu line
	//
	void Queue::set_wait_time_ms(Uint32 time_ms)
	{
		if (time_ms > min_wait_time_ms)
			wait_time_ms = time_ms;
		else
			wait_time_ms = min_wait_time_ms;
	}

	//	protect the server - the minimum wait time, in milli-seconds, between executing commands
	const Uint32 Queue::min_wait_time_ms = 500;
	Uint32 Queue::wait_time_ms = Queue::min_wait_time_ms;


	//
	// construct a menu line from a text string
	//
	Line::Line(const std::string &line_text)
	{
		std::string::size_type from_index = 0;
		std::string::size_type to_index = 0;
		std::string delim = "||";
		std::string::size_type len = 0;
		std::vector<std::string> fields;

		// parse the line extracting the fields separated by the delimitor
		while ((to_index = line_text.find(delim, from_index)) != std::string::npos)
		{
			if ((len = to_index-from_index) > 0)
				fields.push_back(line_text.substr(from_index, len));
			from_index = to_index + delim.size();
		}
		if ((len = line_text.size()-from_index) > 0)
			fields.push_back(line_text.substr(from_index, len));

		// a line with no fields is treated as context menu separator
		if (fields.empty())
		{
			text = "--";
			return;
		}

		// a line must always have at least two fields, the text and a command
		if (fields.size() == 1)
		{
			text = um_invalid_line_str;
			fields.clear();
			return;
		}

		// the first field is the menu text, remaining fields are the assiociated commands
		text = fields[0];
		for (size_t i=1; i<fields.size(); i++)
			command_list.push_back(Command(fields[i]));

	} // end Line::Line()


	//
	//	action the selected menu options 
	//
	void Line::action(Queue &cq) const
	{
		for (size_t i=0; i<command_list.size(); i++)
			cq.add(command_list[i]);
	}

} // end CommandQueue namespace


//
//	External Interface functions
//
extern "C"
{
	void set_command_queue_wait_time_ms(Uint32 wait_time_ms)
	{
		CommandQueue::Queue::set_wait_time_ms(wait_time_ms);
	}
}
