/*!
 * \file
 * \brief Handle extended protocol commands
 *
 * Handles messages in the extended protocol, driven by Other Life development. The messages
 * handled by handle_extended_command() are shared between Eternal Lands and Other Life.
 */
#ifndef PROXY_EXT_H
#define PROXY_EXT_H

#include <stdlib.h>
#include <stdint.h>

/*!
 * \brief Handle an extended protocol command
 *
 * Handle an extended protocol command. If the command is sent encapsulated in a PROXY command,
 * the data should start with the internal proxy command, i.e. the encapsulating PROXY opcode
 * and data length should be skipped.
 * \param data     Data for the proxy command
 * \param data_len The number of bytes in \a data
 */
void handle_extended_command(const uint8_t* data, size_t data_len);

#endif // PROXY_PROTO_H
