#include "elpawn.h"
#include "../global.h"
#include "amx.h"
#include "amxaux.h"

// includes for our native functions
#include "amxcons.h"

// the machine itself
static AMX el_amx;
static void *el_amx_memory = NULL; 
static int el_amx_initialized = 0;

int initialize_pawn (const char* fname)
{
	int err;
	size_t memsize;

	memsize = aux_ProgramSize (fname);
	if (memsize == 0)
	{
		log_error ("Unable to determine memory size for Pawn file %s", fname);
		return 0;
	}

	el_amx_memory = malloc (memsize);
	if (el_amx_memory == NULL)
	{
		log_error ("unable to allocate memory for Pawn file %s", fname);
		return 0;
	}

	err = aux_LoadProgram (&el_amx, fname, el_amx_memory);
	if (err != AMX_ERR_NONE)
	{
		log_error ("unable to load Pawn file %s", fname);
		return 0;
	}

	err = amx_ConsoleInit (&el_amx);
	if (err != AMX_ERR_NONE)
	{
		log_error ("Unable to initialize all native functions for Pawn file %s", fname);
		return 0;
	}

	el_amx_initialized = 1;
	return 1;
}

void cleanup_pawn ()
{
	el_amx_initialized = 0;
	
	amx_ConsoleCleanup (&el_amx);
	amx_Cleanup (&el_amx);
	if (el_amx_memory)
	{
		free (el_amx_memory);
		el_amx_memory = NULL;
	}
}

int run_pawn_function (const char* fun)
{
	int err = AMX_ERR_NONE, index=-1;
	if (!el_amx_initialized)
	{
		log_error ("Unable to execute Pawn function: machine not initialized");
		return 0;
	}

	err = amx_FindPublic (&el_amx, fun, &index);
	if (err != AMX_ERR_NONE)
	{
		log_error ("Unable to locate Pawn function %s", fun);
		return 0;
	}

	err = amx_Exec (&el_amx, NULL, index);
	if (err != AMX_ERR_NONE)
	{
		log_error ("Error %d executing Pawn function %s", err, fun);
		return 0;
	}

	return 1;
}
