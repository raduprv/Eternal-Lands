#ifdef USE_SSL

#ifndef SOCKET_H
#define SOCKET_H

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>
#include <openssl/ssl.h>
#ifdef WINDOWS
#include <winsock2.h>
typedef SOCKET SocketDescriptor;
#else // WINDOWS
#include <sys/socket.h>
typedef int SocketDescriptor;
#endif // WINDOWS
#include "ipaddress.h"

namespace eternal_lands
{

struct TCPSocketError: public std::runtime_error
{
	TCPSocketError(const std::string& msg): std::runtime_error(msg) {}
};
struct ResolutionFailure: public TCPSocketError
{
	ResolutionFailure(const std::string& name):
		TCPSocketError("Failed to resolve host " + name) {}
};
struct ConnectionFailure: public TCPSocketError
{
	ConnectionFailure(const std::string& name):
		TCPSocketError("Failed to connect to " + name) {}
};
struct NotConnected: public TCPSocketError
{
	NotConnected(): TCPSocketError("Not connected") {}
};
struct SendError: public TCPSocketError
{
	SendError(const std::string& err_msg): TCPSocketError(err_msg) {}
};
struct PollError: public TCPSocketError
{
	PollError(int error): TCPSocketError("Poll error"), error(error) {}
	int error;
};
struct LostConnection: public TCPSocketError
{
	LostConnection(): TCPSocketError("Connection lost") {}
};
struct ReceiveError: public TCPSocketError
{
	ReceiveError(const std::string& err_msg): TCPSocketError(err_msg) {}
};
struct EncryptError: public TCPSocketError
{
	EncryptError(const std::string& err_msg): TCPSocketError(err_msg) {}
};

class TCPSocket
{
public:
	TCPSocket(): _fd(-1), _peer(), _ssl_ctx(nullptr), _ssl(nullptr), _connected(false), _encrypted(false) {}
	~TCPSocket() { close(); }

	bool is_connected() const { return _connected; }
	bool is_encrypted() const { return _encrypted; }
	const IPAddress& peer_address() const { return _peer; }

	void connect(const std::string& address, std::uint16_t port);
	void close();

	size_t send(const std::uint8_t* data, size_t data_len);
	bool wait_incoming(int timeout_ms);
	size_t receive(std::uint8_t* buffer, size_t max_len)
	{
		return receive_or_peek(buffer, max_len, false);
	}
	size_t peek(std::uint8_t* buffer, size_t max_len)
	{
		return receive_or_peek(buffer, max_len, true);
	}

	void set_no_delay();
	void encrypt();

private:
	SocketDescriptor _fd;
	IPAddress _peer;
	SSL_CTX *_ssl_ctx;
	SSL *_ssl;
	bool _connected;
	bool _encrypted;

	size_t receive_or_peek(std::uint8_t* buffer, size_t max_len, bool peek);
};

} // namespace

#endif // SOCKET_H

#endif // USE_SSL
