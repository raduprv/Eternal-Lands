#ifndef SERVERS_H
#define SERVERS_H

/**
 * @brief Set the server info vars
 *
 * Find the server we are using, set up the address and port variables and
 * check the config dir exists.
 */
void set_default_config();


/**
 * @brief Set the server info vars
 *
 * Find the server we are using, set up the address and port variables and
 * check the config dir exists.
 *
 * Return 0 if it fails to set the server details, 1 if success
 *
 */
int set_server_details_from_id(char *id);

/**
 * @brief Return the server subdirectory
 *
 * Return the config dir sub directory for the current server
 */
const char * get_server_dir();

/**
 * @brief Load the list of server profiles
 *
 * Load, parse and validate the list of server profiles from servers.lst
 */
void load_server_list(const char *filename);

/**
 * @brief Return server profile name.
 *
 * Return the the name of the active server profile from servers.lst.
 */
const char * get_server_name(void);

/**
 * \brief Free the servers list
 *
 * Free the memory allocated for the list of known servers.
 */
void free_servers(void);

#endif
