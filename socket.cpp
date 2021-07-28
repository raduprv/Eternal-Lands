#ifdef USE_SSL

#include <cstring>
#ifdef WINDOWS
#define SHUT_RDWR SD_BOTH
#else // WINDOWS
#include <fcntl.h>
#include <netinet/tcp.h>
#endif // WINDOWS
#include <openssl/err.h>
#include <openssl/x509v3.h>
#include <unistd.h>
#include "socket.h"
#include "elloggingwrapper.h"
#include "init.h"
#include "io/elpathwrapper.h"
#include "ipaddress.h"

namespace eternal_lands
{

const std::string TCPSocket::certificates_directory_name = "certificates";
std::mutex TCPSocket::_init_mutex;
int TCPSocket::_nr_sockets = 0;
#ifdef WINDOWS
bool TCPSocket::_initialized = false;
#endif // WINDOWS

void TCPSocket::initialize()
{
	std::lock_guard<std::mutex> guard(_init_mutex);
	if (_nr_sockets++ == 0)
	{
#ifdef WINDOWS
		WSADATA wsa_data;
		_initialized = WSAStartup(MAKEWORD(1, 1), &wsa_data) == 0;
		if (!_initialized)
		{
			LOG_ERROR("Failed to initialize Windows sockets 1.1");
		}
#endif // WINDOWS
	}
}

void TCPSocket::clean_up()
{
	std::lock_guard<std::mutex> guard(_init_mutex);
	if (--_nr_sockets == 0)
	{
#ifdef WINDOWS
		if (_initialized)
		{
			WSACleanup();
			_initialized = false;
		}
#endif // WINDOWS
	}
}

void TCPSocket::connect(const std::string& hostname, std::uint16_t port, bool do_encrypt)
{
	if (_fd != INVALID_SOCKET)
		close();

	std::string service = std::to_string(port);
	struct addrinfo hints;
	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	struct addrinfo *all_info;
	int res = getaddrinfo(hostname.c_str(), service.c_str(), &hints, &all_info);
	if (res)
		throw ResolutionFailure(hostname);

	for (struct addrinfo *info = all_info; info; info = info->ai_next)
	{
		if (info->ai_family != AF_INET && info->ai_family != AF_INET6)
			// We only do TCP/IP
			continue;

		_fd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
		if (_fd != INVALID_SOCKET)
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
			_fd = INVALID_SOCKET;
		}
	}

	freeaddrinfo(all_info);

	if (_fd == INVALID_SOCKET)
		throw ConnectionFailure(hostname);

	// Disable Nagle's algorithm
	set_no_delay();
	// Make client sockets blocking by default.
	set_blocking(true);

	if (do_encrypt)
		encrypt(hostname);
	else
		_state = State::CONNECTED_UNENCRYPTED;
}

void TCPSocket::close_locked()
{
	if (_fd != INVALID_SOCKET)
	{
		if (_ssl)
		{
			if (!_ssl_fatal_error)
				SSL_shutdown(_ssl);
			SSL_free(_ssl);
			_ssl = nullptr;
		}
		if (_ssl_ctx)
		{
			SSL_CTX_free(_ssl_ctx);
			_ssl_ctx = nullptr;
		}
		_ssl_fatal_error = false;

		shutdown(_fd, SHUT_RDWR);
		::close(_fd);
		_fd = INVALID_SOCKET;
		_state = State::NOT_CONNECTED;
	}
}

const char* TCPSocket::check_ssl_error_locked(int ret, int errno_val)
{
	int ssl_err = SSL_get_error(_ssl, ret);
	switch (ssl_err)
	{
		case SSL_ERROR_NONE:
			return "Success";
		case SSL_ERROR_ZERO_RETURN:
			return "SSL connection closed by peer";
		case SSL_ERROR_SYSCALL:
		case SSL_ERROR_SSL:
			_ssl_fatal_error = true;
		default:
			/* nothing */ ;
	}

	unsigned long err = ERR_get_error();
	if (err)
		return ERR_reason_error_string(err);
	if (ssl_err == SSL_ERROR_SYSCALL && errno_val != 0)
		return strerror(errno);

	return "Unknown error";
}

size_t TCPSocket::send(const std::uint8_t* data, size_t data_len)
{
	if (!is_connected())
		throw NotConnected();

	size_t nr_bytes_sent = 0;
	if (is_encrypted())
	{
		std::lock_guard<std::mutex> guard(_ssl_mutex);
		ERR_clear_error();
		int ret = SSL_write_ex(_ssl, data, data_len, &nr_bytes_sent);
		if (!ret)
		{
			const char* err_msg = check_ssl_error_locked(ret, errno);
			throw SendError(err_msg);
		}
	}
	else
	{
		while (data_len > 0)
		{
			ssize_t sent;
			errno = 0;
#ifdef WINDOWS
			sent = ::send(_fd, reinterpret_cast<const char*>(data), data_len, 0);
#else // WINDOWS
			sent = ::send(_fd, data, data_len, 0);
#endif // WINDOWS
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
	errno = 0;
	do
	{
		ret = ::select(_fd + 1, &set, nullptr, nullptr, &select_timeout);
	} while (ret < 0 && errno == EINTR);

	if (ret < 0)
		throw PollError(strerror(errno));

	return ret > 0;
}

size_t TCPSocket::receive_or_peek(std::uint8_t* buffer, size_t max_len, bool peek)
{
	if (!is_connected())
		throw NotConnected();

	if (is_encrypted())
	{
		std::lock_guard<std::mutex> guard(_ssl_mutex);
		size_t nr_bytes;
		ERR_clear_error();
		int ret = SSL_read_ex(_ssl, buffer, max_len, &nr_bytes);
		if (!ret)
		{
			const char* err_msg = check_ssl_error_locked(ret, errno);
			throw ReceiveError(err_msg);
		}
		return nr_bytes;
	}
	else
	{
		ssize_t nr_bytes;
		int flags = peek ? MSG_PEEK : 0;
		errno = 0;
		do
		{
#ifdef WINDOWS
			nr_bytes = recv(_fd, reinterpret_cast<char*>(buffer), max_len, flags);
#else // WINDOWS
			nr_bytes = recv(_fd, buffer, max_len, flags);
#endif
		}
		while (nr_bytes < 0 && errno == EINTR);

		if (nr_bytes == 0)
			throw LostConnection();
		if (nr_bytes < 0)
			throw ReceiveError(strerror(errno));

		return nr_bytes;
	}
}

void TCPSocket::set_blocking(bool blocking)
{
	// NOTE: this code has been adapted from SDLNet

	if (_fd != INVALID_SOCKET)
	{
#if defined(__BEOS__) && defined(SO_NONBLOCK)
		/* On BeOS r5 there is O_NONBLOCK but it's for files only */
		long mode = !blocking;
		setsockopt(_fd, SOL_SOCKET, SO_NONBLOCK, &mode, sizeof(mode));
#elif defined(O_NONBLOCK)
		int flags = fcntl(_fd, F_GETFL, 0);
		if (blocking)
			flags &= ~O_NONBLOCK;
		else
			flags |= O_NONBLOCK;
		fcntl(_fd, F_SETFL, flags);
#elif defined(WIN32)
		unsigned long mode = !blocking;
		ioctlsocket(_fd, FIONBIO, &mode);
#elif defined(__OS2__)
		int mode = !blocking;
		ioctl(_fd, FIONBIO, &mode);
#else
#error Unknown how to set socket blocking mode for this system.
#endif
	}
}

void TCPSocket::set_no_delay()
{
	if (_fd != INVALID_SOCKET)
	{
		int nodelay = 1;
#ifdef WINDOWS
		setsockopt(_fd, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&nodelay), sizeof(nodelay));
#else // WINDOWS
		setsockopt(_fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));
#endif // WINDOWS
	}
}

void TCPSocket::encrypt(const std::string& hostname)
{
	if (is_encrypted())
		return;

	std::lock_guard<std::mutex> guard(_ssl_mutex);
	if (!_ssl_ctx)
	{
		ERR_clear_error();
		_ssl_ctx = SSL_CTX_new(TLS_client_method());
		if (!_ssl_ctx)
		{
			unsigned long err = ERR_get_error();
			throw EncryptError(ERR_reason_error_string(err));
		}

		// Load certificates from the game data directory, as well as the user's configuration directory
		std::string dir_names[] = {
			datadir + ('/' + certificates_directory_name),
			get_path_config_base() + ('/' + certificates_directory_name)
		};

		X509_STORE *store = SSL_CTX_get_cert_store(_ssl_ctx);
		ERR_clear_error();
		X509_LOOKUP *lookup = X509_STORE_add_lookup(store, X509_LOOKUP_hash_dir());
		if (lookup)
		{
			for (const std::string& dir_name: dir_names)
			{
				ERR_clear_error();
				if (!X509_LOOKUP_add_dir(lookup, dir_name.c_str(), X509_FILETYPE_PEM))
				{
					unsigned long err = ERR_get_error();
					LOG_ERROR("Failed to load certificates from directory \"%s\": %s", dir_name.c_str(),
						ERR_reason_error_string(err));
				}
			}
		}
		else
		{
			unsigned long err = ERR_get_error();
			LOG_ERROR("Failed to add lookup to store: %s", ERR_reason_error_string(err));
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

		SSL_set_hostflags(_ssl, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);
	}

	if (!SSL_set1_host(_ssl, hostname.c_str()))
	{
		unsigned long err = ERR_get_error();
		throw EncryptError(std::string("Failed to set host name for verification: ") + ERR_reason_error_string(err));
	}

	ERR_clear_error();
	if (!SSL_set_fd(_ssl, _fd))
	{
		unsigned long err = ERR_get_error();
		throw EncryptError(ERR_reason_error_string(err));
	}

	ERR_clear_error();
	errno = 0;
	int ret = SSL_connect(_ssl);
	if (ret <= 0)
	{
		const char* msg = check_ssl_error_locked(ret, errno);
		throw EncryptError(msg);
	}

	X509* cert = SSL_get_peer_certificate(_ssl);
	if (!cert)
	{
		LOG_ERROR("The server did not present an encryption certificate");
		_state = State::CONNECTED_CERTIFICATE_FAIL;
		throw InvalidCertificate("No certificate sent by server");
	}

	long res = SSL_get_verify_result(_ssl);
	if (res != X509_V_OK)
	{
		_state = State::CONNECTED_CERTIFICATE_FAIL;
		switch (res)
		{
			case X509_V_ERR_HOSTNAME_MISMATCH:
			{
				// Mingw-w64 manages to confuse itself on the X509_NAME typedef somehow
				//X509_NAME* name = X509_get_subject_name(cert);
				struct X509_name_st* name = X509_get_subject_name(cert);
				int loc = X509_NAME_get_index_by_NID(name, NID_commonName, -1);
				if (loc < 0)
					throw HostnameMismatch(hostname, "<unknown>");
				X509_NAME_ENTRY *entry = X509_NAME_get_entry(name, loc);
				if (!entry)
					throw HostnameMismatch(hostname, "<unknown>");
				ASN1_STRING *cn = X509_NAME_ENTRY_get_data(entry);
				throw HostnameMismatch(hostname, reinterpret_cast<const char*>(ASN1_STRING_get0_data(cn)));
			}
			default:
				throw InvalidCertificate("The server certificate could not be verified");
		}
	}

	const char* version = SSL_get_version(_ssl);
	const char* cipher = SSL_get_cipher(_ssl);
	LOG_INFO("Set up an encrypted connection to server %s using %s with %s cipher\n",
		hostname.c_str(), version, cipher);

	_state = State::CONNECTED_ENCRYPTED;
}

} // namespace eternal_lands

#endif // USE_SSL
