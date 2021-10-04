#ifndef SERVERS_H
#define SERVERS_H

/**
 * @brief Set the server info vars
 *
 * Find the server we are using, set up the address and port variables and
 * check the config dir exists.
 */
void set_server_details(void);

/**
 * @brief Return the server subdirectory
 *
 * Return the config dir sub directory for the current server
 */
const char * get_server_dir(void);

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

/**
 * \brief Set the default server ID
 *
 * The specified server ID is saved to file in the config base directory and used at client start.
 */
void set_def_server_id(const char *server_id);

/**
 * \brief Display server list information to console
 */
void show_servers(void);

#endif
