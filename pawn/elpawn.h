#ifndef ELPAWNRUN_H
#define ELPAWNRUN_H

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

#endif
