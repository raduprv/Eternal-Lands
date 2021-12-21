#ifdef USE_SSL

#ifndef CONNECTION_H
#define CONNECTION_H

/*!
 * \file
 * \brief Handling the connection with the server
 */

#ifdef __cplusplus

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>
#include "queue.h"
#include "socket.h"
#include "textpopup.h"

namespace eternal_lands
{

/*!
 * \brief Class for the connection with the server
 *
 * Class Connection handles the network connection with the game server. It has functions for
 * sending message to, and receiving messages from, the game server.
 */
class Connection
{
public:
	//! Return the singleton Connection object
	static Connection& get_instance()
	{
		static Connection connection;
		return connection;
	}

	//! Return whether the connection to the game server is currently broken
	bool is_disconnected() const { return !_socket.is_connected() && !_socket.in_tls_handshake(); }

	/*!
	 * \brief Set server to connect to
	 *
	 * Set the parameters of the server to connect to. This information is cached here so that we
	 * can easily reconnect after a network failure.
	 *
	 * \param name      The host name of the server to connect to
	 * \param port      The port number on the host to connect to
	 * \param encrypted If \c true, encrypt the connection.
	 */
	void set_server(const char* name, std::uint16_t port, bool encrypted);
	/*!
	 * \brief Connect to the game server
	 *
	 * Set up a connection to the game server specified by a previous call to set_server(). If the
	 * user was previously logged in, this also resends the login information so the user can
	 * immediately continue playing.
	 */
	void connect_to_server();
	//! Break the connection with the server with a default message
	void disconnect_from_server() { disconnect_from_server("Grue?"); }
	/*!
	 * \brief Disconnect from the server
	 *
	 * Break the connection with the game server. and write a message to the text console.
	 *
	 * \param message The reason for the disconnect
	 * \sa disconnect_from_server_locked
	 */
	void disconnect_from_server(const std::string& message)
	{
		std::lock_guard<std::mutex> guard(_out_mutex);
		disconnect_from_server_locked(message);
	}

	/*!
	 * \brief Start encrypting the connection
	 *
	 * If \a encrypt is \c true, Set up an encrypted connection with the server. This is done
	 * after a LETS_ENCRYPT message has been sent to the server, and it responds with an affirmative
	 * LETS_ENCRYPT reply. If the server does not support encryption, \a encrypt is \c false, and
	 * the connection will be broken.
	 * \param encrypt Whether the server supports encryption.
	 */
	void start_tls_handshake(bool encrypt);

	void start_connection_test();
	void check_connection_test();
	void stop_connection_test() { _connection_test_tick = 0; }

	//! Send a single byte command \a cmd without further data to the server
	std::size_t send(std::uint8_t cmd) { return send(cmd, nullptr, 0); }
	/*!
	 * \brief Send a message to the server
	 *
	 * Send a message with command code \a cmd and \a len bytes of additional data in \a data
	 * to the server. It is possible that the message is buffered first, to be sent later.
	 *
	 * \param cmd  The command code for the message
	 * \param data Additional data for the command
	 * \param len  The number of bytes in \a data
	 * \return The number of bytes actually sent or buffered.
	 */
	std::size_t send(std::uint8_t cmd, const std::uint8_t* data, std::size_t len);
	/*!
	 * \brief Flush the output buffer
	 *
	 * Send all data in the output buffer to the server.
	 * \sa flush_locked
	 */
	std::size_t flush()
	{
		std::lock_guard<std::mutex> guard(_out_mutex);
		return flush_locked();
	}
	void receive(queue_t* queue, int *done);

	void check_heart_beat()
	{
		if (!is_disconnected() && time(nullptr) - _last_heart_beat >= 25)
			send_heart_beat();
	}

	void send_login_info();
	/*!
	 * \brief Set whether a login attempt succeeded
	 *
	 * Set whether the last attemp to log in on the server succeeeded. This information is used
	 * to determine whether to automaticcaly send the login information again after a reconnect,
	 * \param success \c true if the login attempt was successful, \c false otherwise.
	 */
	void set_logged_in(bool success) { _previously_logged_in = success; }
	void send_move_to(std::int16_t x, std::int16_t y, bool try_pathfinder);
	void send_new_char(const std::string& username, const std::string& password,
		std::uint8_t skin, std::uint8_t hair, std::uint8_t eyes, std::uint8_t shirt,
		std::uint8_t pants, std::uint8_t boots, std::uint8_t head, std::uint8_t type);
	//! Send a ping request to the server
	void send_ping_request();

	void clean_up()
	{
		_socket.close();
		_error_popup.reset();
	}

private:
	static const std::uint16_t protocol_version_first_digit = 10; // protocol/game version sent to server
	static const std::uint16_t protocol_version_second_digit = 29;
	static const size_t max_out_buffer_size = 8192;
	static const size_t max_in_buffer_size = 8192;
	static const size_t max_cache_size = 256;
	//! How long too wait for a response to an invitation to encrypt, in milliseconds
	std::uint32_t lets_encrypt_timeout = 5000;

	std::string _server_name;
	unsigned short _server_port;
	bool _encrypted;

	TCPSocket _socket;

	std::unique_ptr<TextPopup> _error_popup;

	std::mutex _out_mutex;
	std::vector<std::uint8_t> _out_buffer;
	std::vector<std::uint8_t> _cache;
	std::array<std::uint8_t, max_in_buffer_size> _in_buffer;
	size_t _in_buffer_used;

	//! \c true when we are currently waiting for a response from the server to an encryption request
	bool _awaiting_encrypt_response;
	//! Time tick of when invitation to encrypt was sent
	std::uint32_t _lets_encrypt_tick;
	time_t _last_heart_beat;
	std::uint32_t _last_sit_tick;
	std::uint32_t _last_turn_tick;
	std::uint32_t _cache_tick;
	std::uint32_t _connection_test_tick;
	bool _invalid_version;
	bool _previously_logged_in;

	/*!
	 * \brief Constructor
	 *
	 * Set up a new Connection object, that is not yet connected to a game server,
	 * \sa set_server, connect_to_server
	 */
	Connection(): _server_name(), _server_port(2000), _encrypted(false), _socket(), _error_popup(),
		_out_mutex(), _out_buffer(), _cache(), _in_buffer(), _in_buffer_used(0),
		_awaiting_encrypt_response(false),  _lets_encrypt_tick(0), _last_heart_beat(0),
		_last_sit_tick(0), _last_turn_tick(0), _connection_test_tick(0),
		_invalid_version(false), _previously_logged_in(false) {}
	//! Destructor
	~Connection() { clean_up(); }

	/*!
	 * \brief Disconnect from the server
	 *
	 * Break the connection with the game server. and write a message to the text console. The
	 * output mutex must be locked by the caller.
	 *
	 * \param message The reason for the disconnect
	 * \sa disconnect_from_server
	 */
	void disconnect_from_server_locked(const std::string& message);

	/*!
	 * \brief Actually send the data
	 *
	 * Send \a data_len bytes of data in \a data to the server. The output mutex must be locked
	 * by the caller.
	 * \param data     The data bytes to send
	 * \param data_len The number of bytes in \a data to send
	 * \return The number of bytes actuallt sent
	 */
	std::size_t send_data_locked(const std::uint8_t* data, size_t data_len);
	/*!
	 * \brief Flush the output buffer
	 *
	 * Send all data in the output buffer to the server. The output mutex must be locked by the
	 * caller.
	 * \sa flush
	 */
	std::size_t flush_locked();
	void process_incoming_data(queue_t *queue);

	void send_heart_beat();
	void send_version();

	void close_after_invalid_certificate();
	void finish_connect_to_server();
	/*!
	 * \brief Finish connecting to the server
	 *
	 * Finish connecting to the server after encryption was succesfully set up. Logs a message to
	 * the console, then calls finish_connect_to_server().
	 */
	void finish_connect_to_server_encrypted();

};

} // namespace eternal_lands

#endif // __cplusplus

#ifdef __cplusplus
extern "C"
{
#endif

int is_disconnected(void);
void start_tls_handshake(int encrypt);
void connection_set_server(const char* name, uint16_t port, int encrypted);
void connect_to_server(void);
void force_server_disconnect(const char *message);
void start_testing_server_connection(void);
void check_if_testing_server_connection(void);
void stop_testing_server_connection(void);
void check_heart_beat(void);
void send_login_info(void);
void set_logged_in(int success);
void send_new_char(const char* user_str, const char* pass_str, char skin, char hair, char eyes,
	char shirt, char pants, char boots,char head, char type);
void move_to (short int x, short int y, int try_pathfinder);
void send_ping_request(void);
void handle_encryption_invitation(void);
int my_tcp_send(const Uint8* str, int len);
int my_tcp_flush(void);
void cleanup_tcp(void);
int get_message_from_server(void *thread_args);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CONNECTION_H

#endif // USE_SSL
