#ifdef USE_SSL

#include <cstring>
#include <netdb.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include "socket.h"

namespace eternal_lands
{

IPAddress::IPAddress(const struct sockaddr_in *addr): _host(), _port(addr->sin_port)
{
	std::uint32_t host = addr->sin_addr.s_addr;
#ifdef EL_BIG_ENDIAN
	_host.push_back(std::uint8_t(host >> 24));
	_host.push_back(std::uint8_t(host >> 16));
	_host.push_back(std::uint8_t(host >> 8));
	_host.push_back(std::uint8_t(host));
#else // EL_BIG_ENDIAN
	_host.push_back(std::uint8_t(host));
	_host.push_back(std::uint8_t(host >> 8));
	_host.push_back(std::uint8_t(host >> 16));
	_host.push_back(std::uint8_t(host >> 24));
	_port = (_port << 8) | (_port >> 8);
#endif // EL_BIG_ENDIAN
}

IPAddress::IPAddress(const struct sockaddr_in6 *addr):
	_host(addr->sin6_addr.s6_addr, addr->sin6_addr.s6_addr+16), _port(addr->sin6_port)
{
#ifndef EL_BIG_ENDIAN
	_port = (_port << 8) | (_port >> 8);
#endif // !EL_BIG_ENDIAN
}

void TCPSocket::connect(const std::string& address, std::uint16_t port)
{
	if (_fd >= 0)
		close();

	std::string service = std::to_string(port);
	struct addrinfo hints;
	std::memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	struct addrinfo *all_info;
	int res = getaddrinfo(address.c_str(), service.c_str(), &hints, &all_info);
	if (res)
		throw ResolutionFailure(address);

	for (struct addrinfo *info = all_info; info; info = info->ai_next)
	{
		_fd = socket(info->ai_family, SOCK_STREAM, info->ai_protocol);
		if (_fd >= 0)
		{
			int res = ::connect(_fd, info->ai_addr, info->ai_addrlen);
			if (res == 0)
			{
				if (info->ai_family == AF_INET)
					_peer = IPAddress(reinterpret_cast<const struct sockaddr_in*>(info->ai_addr));
				else if (info->ai_family == AF_INET6)
					_peer = IPAddress(reinterpret_cast<const struct sockaddr_in6*>(info->ai_addr));
				else
					// Ehmm... dunno?
					_peer = IPAddress();
				break;
			}

			// Connection failed
			::close(_fd);
			_fd = -1;
		}
	}

	freeaddrinfo(all_info);

	if (_fd == -1)
		throw ConnectionFailure(address);
	_connected = true;
}

void TCPSocket::close()
{
	if (_fd >= 0)
	{
		shutdown(_fd, SHUT_RDWR);
		::close(_fd);
		_fd = -1;
		_connected = false;
	}
}

size_t TCPSocket::send(const std::uint8_t* data, size_t data_len)
{
	if (!is_connected())
		throw NotConnected();

	ssize_t nr_bytes_sent = 0;
	while (data_len > 0)
	{
		ssize_t sent = ::send(_fd, data, data_len, 0);
		if (sent < 0 && errno != EINTR)
			// something went wrong
			break;

		if (sent > 0)
		{
			data += sent;
			data_len -= sent;
			nr_bytes_sent += sent;
		}
	}

	return nr_bytes_sent;
}

bool TCPSocket::wait_incoming(int timeout_ms)
{
	if (!is_connected())
		throw NotConnected();

	struct timeval select_timeout;
	select_timeout.tv_sec = 0;
	select_timeout.tv_usec = timeout_ms * 1000;

	fd_set set;
	FD_ZERO(&set);
	FD_SET(_fd, &set);

	int ret;
	do
	{
		ret = ::select(_fd + 1, &set, nullptr, nullptr, &select_timeout);
	} while (ret < 0 && errno == EINTR);

	if (ret < 0)
		throw PollError(errno);

	return ret > 0;
}

size_t TCPSocket::receive(std::uint8_t* buffer, size_t max_len)
{
	if (!is_connected())
		throw NotConnected();

	ssize_t nr_bytes;
	do
	{
		nr_bytes = recv(_fd, buffer, max_len, 0);
	}
	while (nr_bytes < 0 && errno == EINTR);

	if (nr_bytes == 0)
		throw LostConnection();
	if (nr_bytes < 0)
		throw ReceiveError(errno);

	return nr_bytes;
}

void TCPSocket::set_no_delay()
{
	if (_fd >= 0)
	{
		int nodelay = 1;
		setsockopt(_fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));
	}
}

} // namespace eternal_lands

#endif // USE_SSL
