#include "elpawn.h"
#include "../global.h"
#include "amx.h"
#include "amxaux.h"

// includes for our native functions
#include "amxcons.h"
#include "amxfloat.h"
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

// the machines themselves
static pawn_machine srv_amx = {0, 0, NULL};
static pawn_machine map_amx = {0, 0, NULL};

int initialize_pawn_machine (pawn_machine *machine, const char* fname)
{
	int err;
	size_t memsize;
	void* buffer;

	machine->initialized = 0;

	memsize = aux_ProgramSize (fname);
	if (memsize == 0)
	{
		log_error ("Unable to determine memory size for Pawn file %s", fname);
		return 0;
	}

	buffer = malloc (memsize);
	if (buffer == NULL)
	{
		log_error ("unable to allocate memory for Pawn file %s", fname);
		return 0;
	}

	err = aux_LoadProgram (&(machine->amx), fname, buffer);
	if (err != AMX_ERR_NONE)
	{
		free (buffer);
		log_error ("unable to load Pawn file %s", fname);
		return 0;
	}

	// The amx_*Init functions return an error when not all natives
	// used in the amx script are defined, so we only need to check
	// the last Init (when we have defined our entire library).
	amx_ConsoleInit (&(machine->amx));
	amx_FloatInit (&(machine->amx));
	err = amx_ElInit (&(machine->amx));
	if (err != AMX_ERR_NONE)
	{
		free (buffer);
		log_error ("Unable to initialize all native functions for Pawn file %s", fname);
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

int run_pawn_function (pawn_machine *machine, const char* fun)
{
	int err = AMX_ERR_NONE, index=-1;
	if (!machine->initialized)
	{
		log_error ("Unable to execute Pawn function: machine not initialized");
		return 0;
	}

	err = amx_FindPublic (&(machine->amx), fun, &index);
	if (err != AMX_ERR_NONE)
	{
		log_error ("Unable to locate Pawn function %s", fun);
		return 0;
	}

	err = amx_Exec (&(machine->amx), NULL, index);
	if (err != AMX_ERR_NONE)
	{
		log_error ("Error %d executing Pawn function %s", err, fun);
		return 0;
	}

	return 1;
}

int run_pawn_server_function (const char* fun)
{
	return run_pawn_function (&srv_amx, fun);
}

int run_pawn_map_function (const char* fun)
{
	return run_pawn_function (&map_amx, fun);
}
