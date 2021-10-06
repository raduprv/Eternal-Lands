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
 * \brief Write the default server ID
 *
 * The specified server ID is saved to file in the config base directory and when starting client start.
 */
void write_def_server_ID(size_t server_id_index);

/**
 * \brief Get the index of the default server.
 *
 * Refurns the default index, the index of ID "main" if not set, or 0.
 */
size_t get_def_server_index(void);

/**
 * \brief Populate the specified option window var with the list of server IDs.
 *
 */
void populate_def_server_options(const char *multi_name);

#endif
