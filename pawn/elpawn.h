#ifndef ELPAWNRUN_H
#define ELPAWNRUN_H

/*!
 * \brief Initialize the Pawn Abstract Machine
 *
 * Initialize the Pawn Abstract Machine, registering all functions in the 
 * EL Pawn library and the public functions declared in the script \a 
 * fname.
 * \param fname Name of the amx file to load
 * \retval int 1 on succes, 0 on failure
 */
int initialize_pawn (const char *fname);

/*!
 * \brief Clean up after Pawn
 *
 * Clean up the data used by Pawn and release the memory buffers allocated
 * for it.
 */
void cleanup_pawn ();

/*!
 * \brief Execute a Pawn function
 *
 * Try to load and execute the Pawn public function with name \a fun. 
 * The function must be declared public, and must reside in the Pawn
 * script loaded by initialize_pawn().
 *
 * \param name The name of the function to run
 * \retval int 1 on succes, 0 on failure
 * \sa initialize_pawn()
 */
int run_pawn_function (const char* fun);

#endif
