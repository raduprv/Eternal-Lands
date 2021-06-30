#ifdef USE_SSL

#ifndef CONNECTION_H
#define CONNECTION_H

#ifdef __cplusplus

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>
#include "queue.h"
#include "socket.h"

namespace eternal_lands
{

class Connection
{
public:
	static Connection& get_instance()
	{
		static Connection connection;
		return connection;
	}

	bool is_disconnected() const { return !_socket.is_connected(); }

	void set_server(const char* name, std::uint16_t port);
	void connect_to_server();
	void disconnect_from_server() { disconnect_from_server("Grue?"); }
	void disconnect_from_server(const std::string& message);

	void start_connection_test();
	void check_connection_test();
	void stop_connection_test() { _connection_test_tick = 0; }

	std::size_t send(std::uint8_t cmd) { return send(cmd, nullptr, 0); }
	std::size_t send(std::uint8_t cmd, const std::uint8_t* data, std::size_t len);
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
	void send_move_to(std::int16_t x, std::int16_t y, bool try_pathfinder);
	void send_new_char(const std::string& username, const std::string& password,
		std::uint8_t skin, std::uint8_t hair, std::uint8_t eyes, std::uint8_t shirt,
		std::uint8_t pants, std::uint8_t boots, std::uint8_t head, std::uint8_t type);

private:
	static const std::uint16_t protocol_version_first_digit = 10; // protocol/game version sent to server
	static const std::uint16_t protocol_version_second_digit = 29;
	static const size_t max_out_buffer_size = 8192;
	static const size_t max_in_buffer_size = 8192;
	static const size_t max_cache_size = 256;

	std::string _server_name;
	unsigned short _server_port;

	TCPSocket _socket;

	std::mutex _out_mutex;
	std::vector<std::uint8_t> _out_buffer;
	std::vector<std::uint8_t> _cache;
	std::mutex _in_mutex;
	std::array<std::uint8_t, max_in_buffer_size> _in_buffer;
	size_t _in_buffer_used;
	time_t _last_heart_beat;
	std::uint32_t _last_sit_tick;
	std::uint32_t _last_turn_tick;
	std::uint32_t _cache_tick;
	std::uint32_t _connection_test_tick;
	bool _invalid_version;
	bool _previously_logged_in;
	bool _in_tls_handshake;

	Connection(): _server_name(), _server_port(2000), _socket(),
		_out_mutex(), _out_buffer(), _cache(), _in_mutex(), _in_buffer(), _in_buffer_used(0),
		_last_heart_beat(0), _last_sit_tick(0), _last_turn_tick(0), _connection_test_tick(0),
		_invalid_version(false), _previously_logged_in(false) {}
	~Connection() { _socket.close(); }

	std::size_t do_send_data(const std::uint8_t* data, size_t data_len);
	std::size_t flush_locked();
	void process_incoming_data(queue_t *queue);

	void send_heart_beat();
	void send_version();
};

} // namespace eternal_lands

#endif // __cplusplus

#ifdef __cplusplus
extern "C"
{
#endif

int is_disconnected(void);
void connection_set_server(const char* name, uint16_t port);
void connect_to_server(void);
void force_server_disconnect(const char *message);
void start_testing_server_connection(void);
void check_if_testing_server_connection(void);
void stop_testing_server_connection(void);
void check_heart_beat(void);
void send_login_info(void);
void send_new_char(const char* user_str, const char* pass_str, char skin, char hair, char eyes,
	char shirt, char pants, char boots,char head, char type);
void move_to (short int x, short int y, int try_pathfinder);
void handle_encryption_invitation(void);
int my_tcp_send(const Uint8* str, int len);
int my_tcp_flush(void);
int get_message_from_server(void *thread_args);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CONNECTION_H

#endif // USE_SSL
