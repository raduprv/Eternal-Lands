#ifdef USE_SSL

#ifndef SOCKET_H
#define SOCKET_H

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>
#ifdef WINDOWS
#include <winsock2.h>
typedef SOCKET SocketDescriptor;
#else // WINDOWS
#include <sys/socket.h>
typedef int SocketDescriptor;
#endif // WINDOWS

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
	SendError(int error): TCPSocketError("Send error"), error(error) {}
	int error;
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
	ReceiveError(int error): TCPSocketError("Receive error"), error(error) {}
	int error;
};

class IPAddress
{
public:
	IPAddress(): _host(), _port(0) {}
	explicit IPAddress(const struct sockaddr_in *addr);
	explicit IPAddress(const struct sockaddr_in6 *addr);

	const std::vector<std::uint8_t>& host() const { return _host; }
	uint16_t port() const { return _port; }

private:
	std::vector<std::uint8_t> _host;
	std::uint16_t _port;
};

class TCPSocket
{
public:
	TCPSocket(): _fd(-1), _peer(), _connected(false) {}
	~TCPSocket() { close(); }

	bool is_connected() const { return _connected; }
	const IPAddress& peer_address() const { return _peer; }

	void connect(const std::string& address, std::uint16_t port);
	void close();

	size_t send(const std::uint8_t* data, size_t data_len);
	bool wait_incoming(int timeout_ms);
	size_t receive(std::uint8_t* buffer, size_t max_len);

	void set_no_delay();

private:
	SocketDescriptor _fd;
	IPAddress _peer;
	bool _connected;

};

} // namespace

#endif // SOCKET_H

#endif // USE_SSL
