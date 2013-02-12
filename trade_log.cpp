/*
	Log trades.

	If enabled, logs all successful trades.  Any time you fully "Accept", the
	items are recorded.  Only after GET_TRADE_EXIT, HERE_YOUT_INVENTORY and
	STORAGE_ITEMS is the trade confirmed and the log entry written.

	Author bluap/pjbroad February 2013
*/

#include <iostream>
#include <sstream>
#include <string>
#include <time.h>
#include <algorithm>

#include "client_serv.h"
#include "interface.h"
#include "item_info.h"
#include "text.h"
#include "trade_log.h"

/*
 * TODO		Write to Trade Log tab in one of the existing windows.
 * TODO		Option to write to per player trade log file.
 * TODO		Finalise the log format.
*/

namespace Trade_Log
{
	//	Class to hold trade details for one party
	//
	class List
	{
		public:
			List(const char *name, const trade_item *stuff, size_t max_size);
			~List(void) { delete [] the_stuff; }
		 	void get_details(std::ostream & out) const;
		private:
			trade_item *the_stuff;
			std::string trade_name;
			size_t size;
	};


	//	Copy the trade information to create the object
	//
	List::List(const char *name, const trade_item *stuff, size_t max_size)
		: the_stuff(0), size(0)
	{
		size = max_size;
		the_stuff = new trade_item[size];
		memcpy(the_stuff, stuff, size*sizeof(trade_item));
		trade_name = name;
	}


	//	Write the trade information to the specified stream
	//
 	void List::get_details(std::ostream & out) const
 	{
		bool no_items = true;
		out << "items from " << trade_name << ":-" << std::endl;
		for(size_t i=0;i<size;i++)
			if (the_stuff[i].quantity > 0)
			{
				// if we have a description, and it is unique use it
				if (get_item_count(the_stuff[i].id, the_stuff[i].image_id)==1)
					out <<
						" " << the_stuff[i].quantity <<
						" " << get_item_description(the_stuff[i].id, the_stuff[i].image_id) <<
						((the_stuff[i].type != 1) ?" (s)" :"") << std::endl;
				// otherwise use the raw ids
				else
					out <<
						" " << the_stuff[i].quantity <<
						" image_id=" << the_stuff[i].image_id <<
						" id=" << the_stuff[i].id <<
						((the_stuff[i].type != 1) ?" (s)" :"") << std::endl;
				no_items = false;
			}
		if (no_items)
			out << " <nothing traded>" << std::endl;
	}


	//	Class to hold the state of current in-progress trade
	//
	class State
	{
		public:
			State(void) : your_stuff(0), their_stuff(0), the_state(TLS_INIT) {}
			~State(void);
			void accepted(const char *name, const trade_item *yours, const trade_item *others, int max_items);
			void completed(void);
			void exit(void) { if (the_state == TLS_ACCEPT) the_state = TLS_EXIT; else init(); }
			void aborted(void) { init(); }
		private:
			void init(void) { the_state = TLS_INIT; }
			List *your_stuff;
			List *their_stuff;
			enum TRADE_LOG_STATE { TLS_INIT, TLS_ACCEPT, TLS_EXIT } the_state;
	};


	//	Make sure we free the memory.
	//
	State::~State(void)
	{
		if (your_stuff)
			delete your_stuff;
		if (their_stuff)
			delete their_stuff;
	}


	//	Each time we accept for the second time, store the current items in case we complete.
	//
	void State::accepted(const char *name, const trade_item *yours, const trade_item *others, int max_items)
	{
		if (your_stuff)
			delete your_stuff;
		if (their_stuff)
			delete their_stuff;

		std::string you = std::string(username_str) + std::string(" (you)");
		your_stuff = new List(you.c_str(), yours, max_items);
		their_stuff = new List(name, others, max_items);

		the_state = TLS_ACCEPT;
	}


	//	The trade completed so write the log text
	//
	void State::completed(void)
	{
		if (!enable_trade_log || (the_state != TLS_EXIT))
		{
			init();
			return;
		}
		init();

		char buf[80];
		time_t now = time(0);
		strftime(buf, sizeof(buf), "Trade log %Y-%m-%d %X", localtime(&now));
		std::ostringstream message;
		message << buf << std::endl;

		if (your_stuff)
			your_stuff->get_details(message);
		if (their_stuff)
			their_stuff->get_details(message);
		std::string message_str =  message.str();
		message_str.erase(std::find_if(message_str.rbegin(), message_str.rend(), std::not1(std::ptr_fun<int, int>(std::iscntrl))).base(), message_str.end());

		LOG_TO_CONSOLE(c_green2, message_str.c_str());
	}

} // end of Trade_Log namespace

// Hold all the current trade log state
static Trade_Log::State the_log;

// The external interface
extern "C"
{
	int enable_trade_log = 0; // The config window option to enable the trade log
	void trade_accepted(const char *name, const trade_item *yours, const trade_item *others, int max_items)
		{ the_log.accepted(name, yours, others, max_items); }
	void trade_exit(void) { the_log.exit(); }
	void trade_aborted(const char *message) { the_log.aborted(); }
	void trade_post_storage(void) { the_log.completed(); }
}
