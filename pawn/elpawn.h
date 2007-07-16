#ifdef PAWN

#ifndef ELPAWNRUN_H
#define ELPAWNRUN_H

#include <SDL_types.h>

/*!
 * \brief Initialize the Pawn Abstract Machines
 *
 * Initialize the two (server and map) Pawn Abstract Machines, registering 
 * all functions in the EL Pawn library and the public functions they're 
 * using.
 * \retval int 1 on succes, 0 on failure
 */
int initialize_pawn ();

/*!
 * \brief Clean up after Pawn
 *
 * Clean up the data used by Pawn and release the memory buffers allocated
 * for it.
 */
void cleanup_pawn ();

/*!
 * \brief Execute a function on the server Pawn machine
 *
 * Try to load and execute the Pawn public function with name \a fun on 
 * the Pawn machine dedicated to handling server messages. The function 
 * must be declared public, and must reside in the Pawn script loaded by 
 * the machine in initialize_pawn().
 *
 * The format string \a fmt is a series of characters describing the types
 * of the parameters that follow. Currently recognized types are:
 * \li \c 'i' for an \c int parameter
 * \li \c 'f' for a floating point parameter (\c float on 32 bit systems, 
 *     \c double on 64 bit systems.
 *
 * \param name The name of the function to run
 * \param fmt  A string describing the types of the parameters that follow
 * \retval int 1 on succes, 0 on failure
 * \sa initialize_pawn(), run_pawn_map_function()
 */
int run_pawn_server_function (const char* fun, const char* fmt, ...);

/*!
 * \brief Execute a function on the map Pawn machine
 *
 * Try to load and execute the Pawn public function with name \a fun on
 * the Pawn machine dedicated to handling map events. The function
 * must be declared public, and must reside in the Pawn script loaded by
 * the machine in initialize_pawn().
 *
 * \param name The name of the function to run
 * \param fmt  A string describing the parameter types that follow, see 
 *             run_pawn_server_function() for a description.
 * \retval int 1 on succes, 0 on failure
 * \sa initialize_pawn(), run_pawn_server_function()
 */
int run_pawn_map_function (const char* fun, const char* fmt, ...);

/*!
 * \brief Checks the Pawn timer queue to see if a callback needs to be executed
 *
 * Check the timer queue to see if the current time is greater than the 
 * scheduled time for the first event. If so, generate an SDL event to
 * execute all callbacks that need to be finished.
 */
void check_pawn_timers ();

/*!
 * \brief Execute all necessary timer callbacks.
 *
 * Execute all timer call backs on the queue for which the scheduled time has
 * passed.
 */
void handle_pawn_timers ();

/*!
 * \brief Schedule a function for execution
 *
 * Schedule a Pawn function to be executed on the Pawn machine for map events  
 * at a certain point in the future. If \a interval is non zero, it will be 
 * executed every \a interval milliseconds after the first execution.
 * \param offset   The number of milliseconds between now and the first call
 * \param name     The name of the function to run
 * \param interval If > 0, the number of milliseconds between succesive calls to
 *                 the function
 */
void add_map_timer (Uint32 offset, const char* name, Uint32 interval);

/*!
 * \brief Clear the timer queue
 *
 * Remove all current callbacks from the Pawn timer queue on the map Pawn 
 * machine
 */
void clear_map_timers ();

#endif

#endif // PAWN
