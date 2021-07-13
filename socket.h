#ifdef USE_SSL

#ifndef SOCKET_H
#define SOCKET_H

/*!
 * \file
 * \brief C++ class for a TCP network socket
 */

#include <atomic>
#include <cstdint>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>
#include <openssl/ssl.h>
#include "ipaddress.h"

#ifdef WINDOWS
#include <winsock2.h>
typedef SOCKET SocketDescriptor;
#else // WINDOWS
#include <sys/socket.h>
typedef int SocketDescriptor;
#define  INVALID_SOCKET -1
#endif // WINDOWS


namespace eternal_lands
{

//! Base class for error related to the TCP cllient socket
struct TCPSocketError: public std::runtime_error
{
	TCPSocketError(const std::string& msg): std::runtime_error(msg) {}
};
//! Error thrown when the server host name cannot be resolved to an IP address
struct ResolutionFailure: public TCPSocketError
{
	ResolutionFailure(const std::string& name): TCPSocketError("Failed to resolve host " + name) {}
};
//! Error thrown when setting up a connection to the server fails
struct ConnectionFailure: public TCPSocketError
{
	ConnectionFailure(const std::string& name): TCPSocketError("Failed to connect to " + name) {}
};
//! Error thrown when trying to use a socket which is not connected
struct NotConnected: public TCPSocketError
{
	NotConnected(): TCPSocketError("Not connected") {}
};
//! Error thrown when sending data over the network fails
struct SendError: public TCPSocketError
{
	SendError(const std::string& err_msg): TCPSocketError(err_msg) {}
};
//! Error thrown when there an error occurs while watcjin for incoming network data on a socket
struct PollError: public TCPSocketError
{
	PollError(const std::string& msg): TCPSocketError(msg) {}
};
//! Error thrown when the connection is broken while waiting for incoming traffic
struct LostConnection: public TCPSocketError
{
	LostConnection(): TCPSocketError("Connection lost") {}
};
//! Error thrown when an error occurs while reading incoming network data
struct ReceiveError: public TCPSocketError
{
	ReceiveError(const std::string& err_msg): TCPSocketError(err_msg) {}
};
//! Error thrown when setting up encryption on the conneection with the server fails.
struct EncryptError: public TCPSocketError
{
	EncryptError(const std::string& err_msg): TCPSocketError(err_msg) {}
};
//! Error thrown when the certificate presented by the server cannot be validated.
struct InvalidCertificate: public EncryptError
{
	InvalidCertificate(const std::string& err_msg): EncryptError(err_msg) {}
};
//! Error thrown when the host name of the game server does not match the server certificate
struct HostnameMismatch: public InvalidCertificate
{
	HostnameMismatch(const std::string& serv_name, const std::string& cert_name):
		InvalidCertificate("The host name of the game server does not match the certificate"),
		server_name(serv_name), certificate_name(cert_name) {}

	std::string server_name;
	std::string certificate_name;
};

/*!
 * \brief Class for (client-side) network sockets
 *
 * Class TCPSocket represents a TCP network socket for the client side of the connection.
 * The connection can optionally be encrypted using SSL. Client sockets are by default blocking,
 * though that can be overriden if desired.
 */
class TCPSocket
{
public:
	/*!
	 * \brief Constructor
	 *
	 * Create a new, unconnected, client socket
	 */
	TCPSocket(): _fd(INVALID_SOCKET), _peer(), _ssl_ctx(nullptr), _ssl(nullptr), _ssl_mutex(),
		_state(State::NOT_CONNECTED), _ssl_fatal_error(false)
	{
		initialize();
	}
	//! Destructor
	~TCPSocket()
	{
		close();
		clean_up();
	}

	//! Return whether the client is currently connected to the server
	bool is_connected() const { return _state == State::CONNECTED_UNENCRYPTED || _state == State::CONNECTED_ENCRYPTED; }
	//! Return whether the connection to the server is encrypted
	bool is_encrypted() const { return _state == State::CONNECTED_ENCRYPTED; }
	//! Return the IP address of the server this socket is connected to
	const IPAddress& peer_address() const { return _peer; }

	/*!
	 * \brief Connect to the server
	 *
	 * Connect to the server with host name \a hostname on port \a port. If \a do_encrypt is \c true,
	 * the connection will be encrypted using TLS.
	 * \param hostanem   The host name or IP address of the server
	 * \param port       The port number to connect to
	 * \param do_encrypt Whether to encrypt the connection
	 */
	void connect(const std::string& hostname, std::uint16_t port, bool do_encrypt);
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

	/*!
	 * \brief Send data to the server
	 *
	 * Send \a data_len bytes of data in buffer \a data to the server,
	 * \param data     Pointer to the data to send
	 * \param data_len The number of bytes to send
	 * \return The number of bytes actuallt sent. This can be less than \a data_len only if the
	 * 	socket is non-blocking, which should not happen in regular usage.
	 */
	size_t send(const std::uint8_t* data, size_t data_len);
	/*!
	 * \brief Check for incoming data
	 *
	 * Check the socket for incoming network traffic, waiting at most \a timeout_ms milliseconds.
	 * \param timeout_ms Upper limit on the number of milliseconds to wait for incoming data
	 * \return \c true if there is data to be read, \c false if nothing has arrived within the
	 * 	specified timeout.
	 */
	bool wait_incoming(int timeout_ms);
	/*!
	 * \brief Read incoming network data
	 *
	 * Read data at most \a max_len bytes of data from the server coming in over the network, into
	 * buffer \a buffer.
	 * \param buffer  Pointer to the buffer in which the incoming data should be stored
	 * \param max_len The maximum number of bytes to read.
	 * \return The actual number of bytes read.
	 */
	size_t receive(std::uint8_t* buffer, size_t max_len)
	{
		return receive_or_peek(buffer, max_len, false);
	}
	/*!
	 * \brief Look at incoming network data
	 *
	 * Read data at most \a max_len bytes of data from the server coming in over the network, into
	 * buffer \a buffer, wuthout removing the data from the network buffers. The next call to read()
	 * or peek() will return the same data.
	 * \param buffer  Pointer to the buffer in which the incoming data should be stored
	 * \param max_len The maximum number of bytes to read.
	 * \return The actual number of bytes read.
	 */
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
	/*!
	 * \brief Send data as soon as possible.
	 *
	 * This function turns off Nagle's algorithm in TCP, which buffers data so as to decrease the
	 * overhead associated with sending data over the network. When this function is called, data
	 * is sent as soon as possible, even when there is only a small amount of data is sent. This may
	 * increase responsiveness, at the cost of increased network usage.
	 */
	void set_no_delay();

	/*!
	 * \brief Accept a certificate
	 *
	 * When an SSL connection is successfully set up, but the server certificate cannot be verified,
	 * the user can choose to accept the certificate anyway, and continue to connect. This function
	 * changes the state of the socket to reflect that,
	 */
	void accept_certificate()
	{
		if (_state == State::CONNECTED_CERTIFICATE_FAIL)
			_state = State::CONNECTED_ENCRYPTED;
	}

private:
	//! Enumeration for the possible states of the connection
	enum class State
	{
		//! Not connected to the server
		NOT_CONNECTED,
		//! Connected to the server on an unencrypted connection
		CONNECTED_UNENCRYPTED,
		//! Connected to the server, and encrypted, but the certifcate could not be verified
		CONNECTED_CERTIFICATE_FAIL,
		//! Connected on an ecnrypted connection
		CONNECTED_ENCRYPTED
	};

	//! The directory containing SSL certificates for known servers
	static const std::string certificates_directory_name;

	//! Mutex serializing access to shared network data
	static std::mutex _init_mutex;
	//! The number of socket objects currently existing
	static int _nr_sockets;
#ifdef WINDOWS
	//! Whether the network has been initialized
	static bool _initialized;
#endif

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
	//! The current state of the connection to the server
	State _state;
	//! Whether a fatal error in the TLS protocol occurred
	bool _ssl_fatal_error;

	//! Do any global initialization required for networking
	static void initialize();
	//! Clean up after the last socket has been closed
	static void clean_up();

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
	 * Set up encryption on the connection to the server. The host name \a hostname of the
	 * selected game server must match the common name in the security certificate sent by the
	 * server connected to.
	 *
	 * \param hostname The host name of the selected game server
	 */
	void encrypt(const std::string& hostname);

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

	/*!
	 * \brief Read incoming network data
	 *
	 * Read data at most \a max_len bytes of data from the server coming in over the network, into
	 * buffer \a buffer. If \a peek is \c true, the data is left in the network buffers, and a
	 * succesive call to read() or ]eek() will return the same data. If ]a peek is \c false, the
	 * data is removed from the network.
	 * \param buffer  Pointer to the buffer in which the incoming data should be stored
	 * \param max_len The maximum number of bytes to read.
	 * \param peek    Whether to keep the data in the network buffers
	 * \return The actual number of bytes read.
	 */
	size_t receive_or_peek(std::uint8_t* buffer, size_t max_len, bool peek);
};

} // namespace

#endif // SOCKET_H

#endif // USE_SSL
