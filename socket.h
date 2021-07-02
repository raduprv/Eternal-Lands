#ifdef USE_SSL

#ifndef SOCKET_H
#define SOCKET_H

#include <cstdint>
#include <mutex>
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
	ResolutionFailure(const std::string& name): TCPSocketError("Failed to resolve host " + name) {}
};
struct ConnectionFailure: public TCPSocketError
{
	ConnectionFailure(const std::string& name): TCPSocketError("Failed to connect to " + name) {}
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
	PollError(const std::string& msg): TCPSocketError(msg) {}
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
struct InvalidCertificate: public EncryptError
{
	InvalidCertificate(): EncryptError("Invalid server certificate") {}
};

class TCPSocket
{
public:
	/*!
	 * \brief Constructor
	 *
	 * Create a new, unconnected, client socket
	 */
	TCPSocket(): _fd(-1), _peer(), _ssl_ctx(nullptr), _ssl(nullptr), _ssl_mutex(),
		_connected(false), _encrypted(false), _ssl_fatal_error(false) {}
	//! Destructor
	~TCPSocket() { close(); }

	bool is_connected() const { return _connected; }
	bool is_encrypted() const { return _encrypted; }
	const IPAddress& peer_address() const { return _peer; }

	/*!
	 * \brief Connect to the server
	 *
	 * Connect to the server with host name \a address on port \a port. If \a do_encrypt is \c true,
	 * the connection will be encrypted using TLS.
	 * \param address    The host name or IP address of the server
	 * \param port       The port number to connect to
	 * \param do_encrypt Whether to encrypt the connection
	 */
	void connect(const std::string& address, std::uint16_t port, bool do_encrypt);
	/*!
	 * \brief Close the connection
	 *
	 * Close the connection to the server.
	 * \sa close_locked
	 */
	void close()
	{
		std::lock_guard<std::mutex> guard(_ssl_mutex);
		close_locked();
	}

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

	/*!
	 * \brief Set the blocking mode.
	 *
	 * Set the blocking mode for this socket. If \a blocking is \c true, the socket will block
	 * until read or write operations can be performed. If blocking id \c false, the socket will
	 * return immediately when no data has arrived to be received, or return a short byte count
	 * (which can be 0) when not all data can be sent.
	 *
	 * \param blocking Whether to block when I/O cannot be performed immediately.
	 */
	void set_blocking(bool blocking);
	void set_no_delay();

private:
	static constexpr const char* certificates_directory = "certificates";

	//! The file descriptor for this socket
	SocketDescriptor _fd;
	//! The address of the server this socket is connected to
	IPAddress _peer;
	//! The SSL context for encrypting the connection
	SSL_CTX *_ssl_ctx;
	//! The SSL object encrypting the connection
	SSL *_ssl;
	//! Mutex serializing access to the SSL object
	std::mutex _ssl_mutex;
	//! Whether this socket is currenty connected to the server
	bool _connected;
	//! Whether the connection to the server is encrypted
	bool _encrypted;
	//! Whether a fatal error in the TLS protocol occurred
	bool _ssl_fatal_error;

	/*!
	 * \brief Close the socket
	 *
	 * Close the socket. If an encrypted connection was set up, close that as well and free the
	 * resources associated with it.
	 *
	 * \note
	 * The SSL mutex must be locked when calling this function.
	 */
	void close_locked();
	/*!
	 * \brief Encrypt the connection
	 *
	 * Set up encryption on the connection to the server.
	 */
	void encrypt();

	/*!
	 * \brief Check an SSL error
	 *
	 * Determine the SSL error code for the last call to an SSL read or write operation, and check
	 * if it is potentially fatal to the SSL connection. If so, not attempt should be made to shut
	 * down the connection cleanly.
	 *
	 * \note
	 * The SSL mutex must be locked when calling this function.
	 *
	 * \param ret       The return value of the last SSL function call
	 * \param errno_val The value of errno immediately after the call
	 * \return The last error message on the error stack on error, or \c "Success" when no error
	 * 	occurred.
	 */
	const char* check_ssl_error_locked(int ret, int errno_val);

	size_t receive_or_peek(std::uint8_t* buffer, size_t max_len, bool peek);
};

} // namespace

#endif // SOCKET_H

#endif // USE_SSL
