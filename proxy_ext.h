#ifndef PROXY_EXT_H
#define PROXY_EXT_H

#include <stdint.h>

/*!
 * \brief Handle a proxy command
 *
 * Handle an extended proxy command. If the command is sent encapsulated in a PROXY command,
 * the data should start with the internal proxy command, i.e. the encapsulating PROXY opcode
 * and data length should be skipped.
 * \param data     Data for the proxy command
 * \param data_len The number of bytes in \a data
 */
void handle_proxy_command(const uint8_t* data, size_t data_len);

#endif // PROXY_PROTO_H
