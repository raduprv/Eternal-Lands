#ifdef USE_SSL

#include <cstring>
#include <netdb.h>
#include <netinet/tcp.h>
#include <openssl/err.h>
#include <unistd.h>
#include "socket.h"
#include "ipaddress.h"

namespace eternal_lands
{

void TCPSocket::connect(const std::string& address, std::uint16_t port)
{
	if (_fd >= 0)
		close();

	std::string service = std::to_string(port);
	struct addrinfo hints;
	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	struct addrinfo *all_info;
	int res = getaddrinfo(address.c_str(), service.c_str(), &hints, &all_info);
	if (res)
		throw ResolutionFailure(address);

	for (struct addrinfo *info = all_info; info; info = info->ai_next)
	{
		if (info->ai_family != AF_INET && info->ai_family != AF_INET6)
			// We only do TCP/IP
			continue;

		_fd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
		if (_fd >= 0)
		{
			int res = ::connect(_fd, info->ai_addr, info->ai_addrlen);
			if (res == 0)
			{
				if (info->ai_family == AF_INET)
					_peer = IPAddress(reinterpret_cast<const struct sockaddr_in*>(info->ai_addr));
				else
					_peer = IPAddress(reinterpret_cast<const struct sockaddr_in6*>(info->ai_addr));
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
		if (_ssl)
		{
			SSL_free(_ssl);
			_ssl = nullptr;
		}
		if (_ssl_ctx)
		{
			SSL_CTX_free(_ssl_ctx);
			_ssl_ctx = nullptr;
		}
		_encrypted = false;

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

	size_t nr_bytes_sent = 0;
	if (is_encrypted())
	{
		int ret = SSL_write_ex(_ssl, data, data_len, &nr_bytes_sent);
		if (!ret)
		{
			int err = SSL_get_error(_ssl, ret);
			throw SendError(ERR_reason_error_string(err));
		}
	}
	else
	{
		while (data_len > 0)
		{
			ssize_t sent;
			sent = ::send(_fd, data, data_len, 0);
			if (sent < 0 && errno != EINTR)
				// something went wrong
				throw SendError(strerror(errno));

			if (sent > 0)
			{
				data += sent;
				data_len -= sent;
				nr_bytes_sent += sent;
			}
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

size_t TCPSocket::receive_or_peek(std::uint8_t* buffer, size_t max_len, bool peek)
{
	if (!is_connected())
		throw NotConnected();

	if (is_encrypted())
	{
		size_t nr_bytes;
		int ret = SSL_read_ex(_ssl, buffer, max_len, &nr_bytes);
		if (!ret)
		{
			int err = SSL_get_error(_ssl, ret);
			throw ReceiveError(ERR_reason_error_string(err));
		}
		return nr_bytes;
	}
	else
	{
		ssize_t nr_bytes;
		int flags = peek ? MSG_PEEK : 0;
		do
		{
			nr_bytes = recv(_fd, buffer, max_len, flags);
		}
		while (nr_bytes < 0 && errno == EINTR);

		if (nr_bytes == 0)
			throw LostConnection();
		if (nr_bytes < 0)
			throw ReceiveError(strerror(errno));

		return nr_bytes;
	}
}

void TCPSocket::set_no_delay()
{
	if (_fd >= 0)
	{
		int nodelay = 1;
		setsockopt(_fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));
	}
}

void TCPSocket::encrypt()
{
	if (is_encrypted())
		return;
	if (!is_connected())
		throw NotConnected();

	if (!_ssl_ctx)
	{
		ERR_clear_error();
		_ssl_ctx = SSL_CTX_new(TLS_client_method());
		if (!_ssl_ctx)
		{
			unsigned long err = ERR_get_error();
			throw EncryptError(ERR_reason_error_string(err));
		}
	}
	if (!_ssl)
	{
		ERR_clear_error();
		_ssl = SSL_new(_ssl_ctx);
		if (!_ssl)
		{
			unsigned long err = ERR_get_error();
			throw EncryptError(ERR_reason_error_string(err));
		}
	}

	ERR_clear_error();
	if (!SSL_set_fd(_ssl, _fd))
	{
		unsigned long err = ERR_get_error();
		throw EncryptError(ERR_reason_error_string(err));
	}

	ERR_clear_error();
	int ret = SSL_connect(_ssl);
	if (ret <= 0)
	{
		int errno_save = errno;
		int ssl_err = SSL_get_error(_ssl, ret);
		unsigned long err = ERR_get_error();
		const char* msg;

		if (err)
		{
			msg = ERR_reason_error_string(err);
		}
		else if (ssl_err == SSL_ERROR_SYSCALL && errno_save != 0)
		{
			msg = strerror(errno);
		}
		else
		{
			msg = "Unknown error";
		}
		throw EncryptError(msg);
	}

	_encrypted = true;
}

} // namespace eternal_lands

#endif // USE_SSL
