#ifdef USE_SSL

#ifndef IPADDRESS_H
#define IPADDRESS_H

#include <cstdint>
#include <string>
#include "netdb.h"

namespace eternal_lands
{

class IPAddress
{
public:
	/*!
	 * \brief Constructor
	 *
	 * Create a new empty IPAddress, with all contents initialised to 0. This may be used when
	 * the address is passed to an external function to be filled, e.g. when calling accept().
	 */
	IPAddress();
	/*!
	 * \brief Constructor
	 *
	 * Create a new IP address from an IPv4 \c sockaddr_in structure, as returned by e.g.
	 * accept() or getaddrinfo().
	 * \param address Pointer to the "raw" underlying socket address
	 */
	IPAddress(const struct sockaddr_in* address) { _address.ipv4 = *address; }
	/*!
	 * \brief Constructor
	 *
	 * Create a new IP address from an IPv6 \c sockaddr_in structure, as returned by e.g.
	 * accept() or getaddrinfo().
	 * \param address Pointer to the "raw" underlying socket address
	 */
	IPAddress(const struct sockaddr_in6* address) { _address.ipv6 = *address; }

	//! Return the maximum size of the address in bytes
	std::size_t size() const { return sizeof(_address); }
	//! Check whether this IP address is an IPv4 address
	bool is_ipv4() const { return family() == AF_INET; }
	//! Check whether this IP address is an IPv6 address
	bool is_ipv6() const { return family() == AF_INET6; }

	/*!
	 * \brief Return the bytes of the host address
	 *
	 * Return a pointer to the bytes describing the host address. IPv4 addresses consist of
	 * 4 bytes, IPv6 adresses are 16 bytes.
	 */
	const std::uint8_t* host_bytes() const;
	//! Return the port number of this IP address (in host native byte order).
	in_port_t port() const;

	std::string to_string() const;

private:
	//! The address data itself
	union
	{
		//! Generic socket address, only used when uninitialized
		struct sockaddr generic;
		//! IPv4 address
		struct sockaddr_in ipv4;
		//! IPv6 address
		struct sockaddr_in6 ipv6;
	} _address;

	//! Return the address family of this IP address
	sa_family_t family() const { return _address.generic.sa_family; }
};

} // namespace eternal_lands

#endif // IPADDRESS_H

#endif // USE_SSL
