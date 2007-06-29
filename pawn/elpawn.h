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
 * \param name The name of the function to run
 * \retval int 1 on succes, 0 on failure
 * \sa initialize_pawn(), run_pawn_map_function()
 */
int run_pawn_server_function (const char* fun);

/*!
 * \brief Execute a function on the map Pawn machine
 *
 * Try to load and execute the Pawn public function with name \a fun on
 * the Pawn machine dedicated to handling map events. The function
 * must be declared public, and must reside in the Pawn script loaded by
 * the machine in initialize_pawn().
 *
 * \param name The name of the function to run
 * \retval int 1 on succes, 0 on failure
 * \sa initialize_pawn(), run_pawn_server_function()
 */
int run_pawn_map_function (const char* fun);

#endif
