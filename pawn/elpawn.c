#ifdef PAWN

#include <SDL_timer.h>

#include "elpawn.h"
#include "amx.h"
#include "amxaux.h"

#include "../errors.h"
#include "../events.h"

// includes for our native functions
#include "amxcons.h"
#include "amxfloat.h"
#include "amxstring.h"
#include "amxel.h"

// Set up our own struct with the information we need. We might be able 
// to divine this from the AMX structure itself, but this is easier.
typedef struct 
{
	int initialized; // 1 if the machine was succesfully initialized
	size_t buf_size; // size of the memory buffer
	void *buffer;    // memory buffer for the machine
	AMX amx;         // representation of the actual machine
} pawn_machine;

typedef struct _pawn_timer_queue
{
	Uint32 ticks;
	char* function;
	Uint32 interval;
	struct _pawn_timer_queue *next;
} pawn_timer_queue;

// the machines themselves
static pawn_machine srv_amx = {0};
static pawn_machine map_amx = {0};

static pawn_timer_queue *map_timer_queue = NULL;

static __inline__ int ticks_less (Uint32 ticks, Uint32 limit)
{
	// try to adjust for timer wrap around. We'll assume that no events are
	// planned more than 2*31 ms (a bit more than 24 days) in advance.
	return ticks < limit || (((Sint32) limit) >= 0 && ((Sint32) ticks) < 0);
}

static __inline__ int ticks_less_equal (Uint32 ticks, Uint32 limit)
{
	return !ticks_less (limit, ticks);
}

// static __inline__ int ticks_greater_equal(Uint32 ticks, Uint32 limit)
// {
// 	return !ticks_less (ticks, limit);
// }

void pop_timer_queue (pawn_timer_queue **queue)
{
	if (*queue)
	{
		pawn_timer_queue *head = *queue;
		*queue = head->next;
		free (head->function);
		free (head);
	}
}

void insert_timer_queue_node (pawn_timer_queue **queue, pawn_timer_queue *new_node)
{
	
	if (*queue == NULL || ticks_less_equal (new_node->ticks, (*queue)->ticks))
	{
		new_node->next = *queue;
		*queue = new_node;
	}
	else
	{
		pawn_timer_queue* node = *queue;
		while (node->next && ticks_less (node->next->ticks, new_node->ticks))
			node = node->next;
		new_node->next = node->next;
		node->next = new_node;
	}
}

void reschedule_timer_queue_head (pawn_timer_queue **queue, Uint32 diff)
{
	pawn_timer_queue *head = *queue;
	
	if (head)
	{
		head->ticks += diff;
		if (head->next && ticks_less (head->next->ticks, head->ticks))
		{
			*queue = head->next;
			insert_timer_queue_node (queue, head);
		}
	}
}

void push_timer_queue (pawn_timer_queue **queue, Uint32 ticks, const char* function, Uint32 interval)
{
	pawn_timer_queue* new_node = calloc (1, sizeof (pawn_timer_queue));
	new_node->ticks = ticks;
	new_node->function = strdup (function);
	new_node->interval = interval;
	insert_timer_queue_node (queue, new_node);
}

int initialize_pawn_machine (pawn_machine *machine, const char* fname)
{
	int err;
	size_t memsize;
	void* buffer;

	machine->initialized = 0;

	memsize = aux_ProgramSize (fname);
	if (memsize == 0)
	{
		LOG_ERROR ("Unable to determine memory size for Pawn file %s", fname);
		return 0;
	}

	buffer = malloc (memsize);
	if (buffer == NULL)
	{
		LOG_ERROR ("unable to allocate memory for Pawn file %s", fname);
		return 0;
	}

	err = aux_LoadProgram (&(machine->amx), fname, buffer);
	if (err != AMX_ERR_NONE)
	{
		free (buffer);
		LOG_ERROR ("unable to load Pawn file %s", fname);
		return 0;
	}

	// The amx_*Init functions return an error when not all natives
	// used in the amx script are defined, so we only need to check
	// the last Init (when we have defined our entire library).
	amx_ConsoleInit (&(machine->amx));
	amx_FloatInit (&(machine->amx));
	amx_StringInit (&(machine->amx));
	err = amx_ElInit (&(machine->amx));
	if (err != AMX_ERR_NONE)
	{
		free (buffer);
		LOG_ERROR ("Unable to initialize all native functions for Pawn file %s", fname);
		return 0;
	}

	machine->buf_size = memsize;
	machine->buffer = buffer;
	machine->initialized = 1;

	return 1;
}

int initialize_pawn ()
{
	int srv_ok = initialize_pawn_machine (&srv_amx, "pawn_scripts/pawn_test.amx");
	int map_ok = initialize_pawn_machine (&map_amx, "pawn_scripts/pawn_test.amx");
	return srv_ok && map_ok;
} 

void cleanup_pawn_machine (pawn_machine *machine)
{
	if (machine->initialized)
	{
		amx_ElCleanup (&(machine->amx));
		amx_StringCleanup (&(machine->amx));
		amx_FloatCleanup (&(machine->amx));
		amx_ConsoleCleanup (&(machine->amx));
		amx_Cleanup (&(machine->amx));
		machine->initialized = 0;
	}

	if (machine->buffer)
	{
		free (machine->buffer);
		machine->buffer = NULL;
		machine->buf_size = 0;
	}
}

void cleanup_pawn ()
{
	cleanup_pawn_machine (&srv_amx);
	cleanup_pawn_machine (&map_amx);
}

int run_pawn_function (pawn_machine *machine, const char* fun, const char* fmt, va_list ap)
{
	int err = AMX_ERR_NONE, index=-1;
	int nr_ref_args = 0, ref_args_size = 0;
	cell *ref_args = NULL;
	int nr_args;

	if (!machine->initialized)
	{
		LOG_ERROR ("Unable to execute Pawn function: machine not initialized");
		return 0;
	}

	err = amx_FindPublic (&(machine->amx), fun, &index);
	if (err != AMX_ERR_NONE)
	{
		LOG_ERROR ("Unable to locate Pawn function %s", fun);
		return 0;
	}

	if (fmt != NULL && (nr_args = strlen (fmt)) > 0)
	{
		const char *s;
		int i;
		REAL f;
		cell *phys;
		int iarg;
		cell *args = calloc (nr_args, sizeof (cell));
		
		// first store the arguments in the array
		for (iarg = 0; iarg < nr_args; iarg++)
		{
			switch (fmt[iarg])
			{
				case 'i':
					i = va_arg (ap, int);
					args[iarg] = (cell) i;
					break;
				case 'f':
					f = va_arg (ap, REAL);
					args[iarg] = *((cell*) &f);
					break;
				case 's':
					s = va_arg (ap, const char*);
					args[iarg] = (cell) s;
					break;
				default:
					LOG_ERROR ("unknown format specifier '%c' in Pawn call", fmt[iarg]);
					free (args);
					return 1;
			}
		}
		
		// now push the arguments to Pawn, in reverse order
		for (iarg = nr_args-1; iarg >= 0; iarg--)
		{
			if (fmt[iarg] == 'i' || fmt[iarg] == 'f')
			{			
				amx_Push (&(machine->amx), args[iarg]);
			}
			else if (fmt[iarg] == 's')
			{
				if (nr_ref_args >= ref_args_size)
				{
					ref_args_size += 8;
					ref_args = realloc (ref_args, ref_args_size * sizeof (cell));
				}
				amx_PushString (&(machine->amx), ref_args+nr_ref_args, &phys, (const char *)args[iarg], 0, 0);
				nr_ref_args++;
			}
		}
		
		// we're done with these
		free (args);
	}

	err = amx_Exec (&(machine->amx), NULL, index);
	if (err != AMX_ERR_NONE)
	{
		LOG_ERROR ("Error %d executing Pawn function %s", err, fun);
		return 0;
	}

	if (ref_args)
	{
		int i;
		for (i = 0; i < nr_ref_args; i++)
			amx_Release (&(machine->amx), ref_args[i]);
		free (ref_args);
	}

	return 1;
}

int run_pawn_server_function (const char *fun, const char* fmt, ...)
{
	int res;
	va_list ap;
	
	va_start (ap, fmt);
	res = run_pawn_function (&srv_amx, fun, fmt, ap);
	va_end (ap);
	
	return res;
}

int run_pawn_map_function (const char* fun, const char* fmt, ...)
{
	int res;
	va_list ap;
	
	va_start (ap, fmt);
	res = run_pawn_function (&map_amx, fun, fmt, ap);
	va_end (ap);
	
	return res;
}

void check_pawn_timers ()
{
	if (map_timer_queue && ticks_less_equal (map_timer_queue->ticks, SDL_GetTicks ()))
	{
		SDL_Event event;
		event.type = SDL_USEREVENT;
		event.user.code = EVENT_PAWN_TIMER;
		SDL_PushEvent (&event);
	}
}

void handle_pawn_timers ()
{
	Uint32 now = SDL_GetTicks ();
	while (map_timer_queue && ticks_less_equal (map_timer_queue->ticks, now))
	{
		int ok = run_pawn_map_function (map_timer_queue->function, NULL);
		if (ok && map_timer_queue->interval)
			reschedule_timer_queue_head (&map_timer_queue, map_timer_queue->interval);
		else
			pop_timer_queue (&map_timer_queue);
	}
}

void add_map_timer (Uint32 offset, const char* name, Uint32 interval)
{
	push_timer_queue (&map_timer_queue, SDL_GetTicks () + offset, name, interval);
}

void clear_map_timers ()
{
	while (map_timer_queue)
		pop_timer_queue (&map_timer_queue);
}

#endif // PAWN
