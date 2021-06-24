#ifdef USE_SSL

#include <cstdlib>
#include <cstring>
#include <arpa/inet.h>
#include "ipaddress.h"

namespace eternal_lands
{

IPAddress::IPAddress()
{
	void* ptr = static_cast<void*>(&_address);
	std::memset(ptr, 0, size());
}

const std::uint8_t* IPAddress::host_bytes() const
{
	if (is_ipv4())
		return reinterpret_cast<const std::uint8_t*>(&_address.ipv4.sin_addr.s_addr);
	else
		return _address.ipv6.sin6_addr.s6_addr;
}

in_port_t IPAddress::port() const
{
	if (is_ipv4())
		return ntohs(_address.ipv4.sin_port);
	else
		return ntohs(_address.ipv6.sin6_port);
}

std::string IPAddress::to_string() const
{
	char buffer[INET6_ADDRSTRLEN];
	const void *src = is_ipv4()
		? static_cast<const void*>(&_address.ipv4.sin_addr)
		: static_cast<const void*>(&_address.ipv6.sin6_addr);
	std::string host = inet_ntop(family(), src, buffer, sizeof(buffer)) ? buffer : "<unknown>";

	if (is_ipv4())
		return host + ':' + std::to_string(port());
	else
		return '[' + host + + "]:" + std::to_string(port());
}

} // namespace eternal_lands

#endif // USE_SSL
