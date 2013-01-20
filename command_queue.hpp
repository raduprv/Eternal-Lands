#if !defined(COMMAND_QUEUE_HPP)
#define COMMAND_QUEUE_HPP

#include <string>
#include <vector>
#include <queue>

#include "notepad.h"


namespace CommandQueue
{
	//
	//	A single command created from one of the command fields in
	//	a command line.
	//
	class Command
	{
		public:
			Command(const std::string &command_text);
			void action(const std::vector<std::string> &params) const;
			void echo(void) const;
			const std::vector<std::string>  & get_prompts(void) const { return param_prompts; }
		private:
			bool invalid_command;
			std::vector<std::string> text_segments;
			std::vector<std::string> param_prompts;
	};

	
	//
	//	Manages a queue of commands to be executed, provides input when
	//	required and introduces a delay between commands to avoid spamming.
	//
	class Queue
	{
		public:
			Queue(void);
			~Queue(void) { close_ipu(&ipu); }
			void process(bool just_echo = false);
			void add(const Command &new_command) { commands.push(new_command); }
			static void set_wait_time_ms(Uint32 wait_time);
			void clear(void);
		private:
			static void input_handler(const char *input_text, void *data) { Queue *q = static_cast<Queue *>(data); assert(q!=NULL); q->input(input_text); };
			static void cancel_handler(void *data) { Queue *q = static_cast<Queue *>(data); assert(q!=NULL); q->cancel(); };
			void input(const char* input_text) { params.push_back(input_text); }
			void cancel(void);
			std::queue<Command> commands;
			std::vector<std::string> params;
			Uint32 last_time;
			INPUT_POPUP ipu;
			static Uint32 wait_time_ms;
			static const Uint32 min_wait_time_ms;
	};


	//
	//	A single command line, command name and assiociated commands extracted 
	//	from the string passed the to the constructor.
	//
	class Line
	{
		public:
			Line(const std::string &line_text);
			const std::string & get_text(void) const { return text; }
			void action(Queue &cq) const;
		private:
			std::string text;
			std::vector<Command> command_list;
	};
}

#endif
