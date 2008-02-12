#ifndef SERVERS_H
#define SERVERS_H

/**
 * @brief Set the server info vars
 *
 * Find the server we are using, set up the address and port variables and
 * check the config dir exists.
 */
void set_server_details();

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

#endif
