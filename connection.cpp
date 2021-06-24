#ifdef USE_SSL

#include <cstring>
#include <thread>
#include "actor_scripts.h"
#include "asc.h"
#include "connection.h"
#include "console.h"
#include "buddy.h"
#include "counters.h"
#include "draw_scene.h"
#include "elc_private.h"
#include "elloggingwrapper.h"
#include "engine/logging.hpp"
#include "errors.h"
#include "filter.h"
#include "main.h"
#include "multiplayer.h"
#include "pathfinder.h"
#include "questlog.h"
#include "sound.h"
#include "text.h"
#include "translate.h"

int always_pathfinding = 0;
int encrypt_connection = 1;

namespace eternal_lands
{

void Connection::set_server(const char* name, std::uint16_t port)
{
	_server_name = name;
	_server_port = port;
}

void Connection::connect_to_server()
{
	if (_invalid_version)
		return;

	//clear the buddy list so we don't get multiple entries
	clear_buddy();

	LOG_TO_CONSOLE(c_red1, connect_to_server_str);
	draw_scene();	// update the screen

	_socket.close();
	try
	{
		_socket.connect(_server_name, _server_port);
	}
	catch (const ResolutionFailure&)
	{
		LOG_TO_CONSOLE(c_red2, failed_resolve);
		do_disconnect_sound();
		return;
	}
	catch (const ConnectionFailure&)
	{
		LOG_TO_CONSOLE(c_red1, failed_connect);
		LOG_TO_CONSOLE(c_red1, reconnect_str);
		LOG_TO_CONSOLE(c_red1, alt_x_quit);
		do_disconnect_sound();
		return;
	}
	_socket.set_no_delay();

	have_storage_list = 0; // With a reconnect, our cached copy of what's in storage may no longer be accurate

	send_version();

	//ask for the opening screen
	if (!_previously_logged_in)
	{
		send(SEND_OPENING_SCREEN);
	}
	else
	{
		yourself = -1;
		you_sit = 0;
		destroy_all_actors();
		// FIXME: If encrypting, don't send login info until after encryption is set up
		send_login_info();
	}

	//clear out info
	clear_waiting_for_questlog_entry();
	clear_today_is_special_day();
	clear_now_harvesting();
	send_heart_beat();     // prime the heart beat to prevent some stray issues when there is lots of lag
	hide_window_MW(MW_TRADE);
	do_connect_sound();

	flush();               // make sure tcp output buffer is empty
}

void Connection::disconnect_from_server(const std::string& message)
{
	if (is_disconnected())
		return;

	char str[256];
	short tgm = real_game_minute;
	safe_snprintf(str, sizeof(str), "<%1d:%02d>: %s [%s]", tgm/60, tgm%60,
		disconnected_from_server, message.c_str());
	LOG_TO_CONSOLE(c_red2, str);
	LOG_TO_CONSOLE(c_red2, alt_x_quit);
#ifdef NEW_SOUND
	stop_all_sounds();
	do_disconnect_sound();
#endif // NEW_SOUND
	disconnect_time = SDL_GetTicks();
	clear_now_harvesting();
	if (login_root_win >= 0)
		set_login_error(disconnected_from_server, strlen(disconnected_from_server), 1);

	std::lock_guard<std::mutex> guard(_out_mutex);
	_socket.close();
}

void Connection::start_connection_test()
{
	LOG_TO_CONSOLE(c_green1, test_server_connect_str);
	_connection_test_tick = SDL_GetTicks();
	command_ping(NULL, 0);
}

void Connection::check_connection_test()
{
	if (_connection_test_tick > 0)
	{
		if (SDL_GetTicks() - _connection_test_tick > 10000)
		{
			stop_connection_test();
			disconnect_from_server(server_connect_test_failed_str);
		}
	}
}

std::size_t Connection::send(std::uint8_t cmd, const std::uint8_t *data, std::size_t data_len)
{
	if ((cmd == TURN_LEFT || cmd == TURN_RIGHT) && on_the_move(get_our_actor()))
		// Ignore turn commands while walking
		return 0;

	// LabRat's anti-bagspam code
	// Grum: Adapted. Converting every movement to a path caused too much
	// trouble. Instead we now check the current actor animation for
	// movement.
	if (cmd == DROP_ITEM  && on_the_move(get_our_actor()))
	{
		// The anti bagspam code in all its glory - don't allow us to drop a bag if following
		// a path - I tried coding every DROP_ALL part of the code, but it was longwinded and
		// this way, after a couple of hours break, seemed the more logical and straightforward
		// solution.
		// 1% of the produce from manufacturers may be donated to Labrat for this patch,
		// or for the bagspammers, sell the items you were going to spam and give the proceeds
		// to a noob on IP :)
		return 0;
	}

	std::lock_guard<std::mutex> guard(_out_mutex);
	if (is_disconnected())
		return 0;

	if (!_out_buffer.empty() && _out_buffer.size() + data_len + 3 >= max_out_buffer_size)
		// Wouldn't fit, send what we have
		flush_locked();

	// Check to see if we have too many packets being sent of the same to reduce server flood
	if (data_len + 1 < max_cache_size) // only if it fits
	{
		if (cmd == MOVE_TO || cmd == RUN_TO || cmd == SIT_DOWN || cmd == HARVEST
			|| cmd == MANUFACTURE_THIS || cmd == CAST_SPELL || cmd == RESPOND_TO_NPC
			|| cmd == ATTACK_SOMEONE || cmd == SEND_PM || cmd == RAW_TEXT
			|| cmd == TURN_LEFT || cmd == TURN_RIGHT)
		{
			std::uint32_t time_limit = (cmd == SEND_PM || cmd == RAW_TEXT || cmd == SIT_DOWN) ? 1500 : 600;
			if (cmd == SIT_DOWN)
			{
				if (cur_time - _last_sit_tick < time_limit)
					return 0;
				_last_sit_tick = cur_time;
			}
			if (cmd == TURN_RIGHT || cmd == TURN_LEFT)
			{
				// Turns do not interrupt queued moves
				if (!_cache.empty() && _cache[0] == MOVE_TO)
					return 0;
				if (cur_time - _last_turn_tick < time_limit)
					return 0;
				_last_turn_tick = cur_time;
			}

			// If too close together
			if (data_len + 1 == _cache.size() && cmd == _cache[0] && _cache_tick - cur_time < time_limit)
			{
				// And the same packet
				if (std::equal(data, data + data_len, _cache.begin() + 1))
					// Ignore this packet
					return 0;
			}

			// Memorize the data we are sending for next time
			_cache.clear();
			_cache.push_back(cmd);
			_cache.insert(_cache.end(), data, data + data_len);
			_cache_tick = cur_time;
		}
	}

	// Update the heartbeat timer
	_last_heart_beat = time(nullptr);

	uint16_t tot_len = data_len + 1;
	// Check to see if the data would fit in the buffer
	if (data_len + 3 < max_out_buffer_size)
	{
		// Yes, buffer it for later processing
		_out_buffer.push_back(cmd);
		_out_buffer.push_back(std::uint8_t(tot_len & 0xff));
		_out_buffer.push_back(std::uint8_t(tot_len >> 8));
		_out_buffer.insert(_out_buffer.end(), data, data + data_len);
		return data_len + 3;
	}

	// No, send it as is now
	std::vector<uint8_t> msg;
	msg.push_back(cmd);
	msg.push_back(std::uint8_t(tot_len & 0xff));
	msg.push_back(std::uint8_t(tot_len >> 8));
	msg.insert(msg.end(), data, data + data_len);
	return do_send_data(msg.data(), msg.size());
}

std::size_t Connection::flush_locked()
{
	if (is_disconnected() || _out_buffer.empty())
		return 0;

	// If we are already sending data, lets see about sending a heartbeat a little bit early
	// FIXME: this will deadlock
// 	if (time(nullptr) - _last_heart_beat >= 20)
// 		send_heart_beat();

	// send all the data in the buffer
	size_t nr_bytes_sent = do_send_data(_out_buffer.data(), _out_buffer.size());
	_out_buffer.clear();
	_cache.clear();

	return nr_bytes_sent;
}

void Connection::send_encryption_reply()
{
	std::uint8_t answer = encrypt_connection;
	send(LETS_ENCRYPT, &answer, 1);

	if (answer)
	{
		// Ensure nothing else is touching the connection
		std::lock_guard<std::mutex> out_guard(_out_mutex);
		std::lock_guard<std::mutex> in_guard(_in_mutex);
		// ensure our answer is sent before we start the TLS handshake
		flush_locked();
		// If no error occurred sending the outgoing data, set up TLS
		if (!is_disconnected())
		{
			try
			{
				_socket.encrypt();
			}
			catch (const NotConnected&)
			{
				// shouldn't happen
			}
			catch (const EncryptError& err)
			{
				LOG_ERROR("Failed to set up encryption: %s", err.what());
				LOG_TO_CONSOLE(c_red1, "Failed to encrypt the data connection with the server.");
				disconnect_from_server("Failed to set up encryption");
				// FIXME: show a popup or something with a proper error message that lets the user turn off encryption
			}
		}
	}
}

void Connection::send_heart_beat()
{
	_last_heart_beat = time(nullptr);
#ifdef OLC
	std::uint8_t data[64];
	len = olc_heartbeat(data);
	send(HEART_BEAT, command, len);
#else
	send(HEART_BEAT);
#endif // OLC
}

void Connection::send_login_info()
{
	if (!valid_username_password())
		return;

	if (is_disconnected())
		connect_to_server();

	// join the username and password, and send them to the server
	std::string username = get_username();
	if (caps_filter && my_isupper(username.c_str(), username.size()))
	{
		set_username(get_lowercase_username());
		username = get_username();
	}
	std::string password = get_password();

	std::vector<std::uint8_t> data(username.begin(), username.end());
	data.push_back(' ');
	data.insert(data.end(), password.begin(), password.end());
	data.push_back(0);

	send(LOG_IN, data.data(), data.size());
	flush();
}

void Connection::send_move_to(std::int16_t x, std::int16_t y, bool try_pathfinder)
{
	if (try_pathfinder && always_pathfinding)
	{
		actor *me = get_our_actor();
		// Check distance
		if (me && (abs(me->x_tile_pos - x) + abs(me->y_tile_pos - y)) > 2)
		{
			// If path finder fails, try standard move
			if (pf_find_path(x, y))
				return;
		}
	}

	std::uint16_t ux = x;
	std::uint16_t uy = y;
	std::uint8_t data[] = { std::uint8_t(ux), std::uint8_t(ux >> 8), std::uint8_t(uy), std::uint8_t(uy >> 8) };
	send(MOVE_TO, data, 4);
}

void Connection::send_new_char(const std::string& username, const std::string& password,
	std::uint8_t skin, std::uint8_t hair, std::uint8_t eyes, std::uint8_t shirt, std::uint8_t pants,
	std::uint8_t boots, std::uint8_t head, std::uint8_t type)
{
	std::vector<uint8_t> data(username.begin(), username.end());
	data.push_back(' ');
	data.insert(data.end(), password.begin(), password.end());
	data.push_back(0);
	data.push_back(skin);
	data.push_back(hair);
	data.push_back(shirt);
	data.push_back(pants);
	data.push_back(boots);
	data.push_back(type);
	data.push_back(head);
	data.push_back(eyes);
	send(CREATE_CHAR, data.data(), data.size());
	flush();    // make sure tcp output buffer is empty
}

void Connection::send_version()
{
	const IPAddress& server_address = _socket.peer_address();
	const std::uint8_t* host = server_address.host_bytes();
	std::uint16_t port = server_address.port();
	const uint8_t data[64] = {
		(protocol_version_first_digit & 0xff),
		protocol_version_first_digit >> 8,
		(protocol_version_second_digit & 0xff),
		protocol_version_second_digit >> 8,
		VER_MAJOR,
		VER_MINOR,
		VER_RELEASE,
		VER_BUILD,
		host[0],
		host[1],
		host[2],
		host[3],
		std::uint8_t(port & 0xff),
		std::uint8_t(port >> 8)
	};

	std::size_t len = 14;
#ifdef OLC
	len += olc_version(data + len);
#endif // OLC
	send(SEND_VERSION, data, len);
}

std::size_t Connection::do_send_data(const std::uint8_t* data, size_t data_len)
{
#ifdef	OLC
	// FIXME: olc_tcp_send expects an SDL socket, which we got rid of.
	ssize_t nr_bytes_sent = olc_tcp_send(my_socket, tcp_out_data, tcp_out_loc);
	if (nr_bytes_sent > 0)
	{
		nr_bytes_sent = olc_tcp_flush();
	}
	return nr_bytes_sent;
#else	//OLC
	try
	{
		return _socket.send(data, data_len);
	}
	catch (const NotConnected&)
	{
		// shouldn't happen
		return 0;
	}
	catch (const SendError& err)
	{
		LOG_ERROR("Send failure: %s", err.what());
		disconnect_from_server("Failed to send data to the server");
		return 0;
	}
#endif	//OLC
}

void Connection::process_incoming_data(queue_t *queue)
{
	std::size_t offset = 0;
	while (offset + 3 <= _in_buffer_used)
	{
		std::size_t size = _in_buffer[offset+1] + (_in_buffer[offset+2] << 8) + 2;
		if (size > max_in_buffer_size)
		{
			LOG_ERROR("Packet overrun, protocol = %d, size = %u\n", _in_buffer[offset], size);
			_in_buffer_used = 0;
			disconnect_from_server(packet_overrun);
			break;
		}
		if (offset + size > _in_buffer_used)
			break;

		message_t *message = static_cast<message_t*>(std::malloc(sizeof(message_t)));
		message->data = static_cast<std::uint8_t*>(std::malloc(size));
		message->length = size;
		std::copy(_in_buffer.begin() + offset, _in_buffer.begin() + offset + size, message->data);
		queue_push(queue, message);

		if (log_conn_data)
			log_conn(_in_buffer.data() + offset, size);

		offset += size;
	}

	if (offset > 0 && offset < _in_buffer_used)
		std::copy(_in_buffer.begin() + offset, _in_buffer.begin() + _in_buffer_used, _in_buffer.begin());
	_in_buffer_used -= offset;
}

void Connection::receive(queue_t *queue, int *done)
{
	init_thread_log("server_message");

	int timeout_ms = 100;
	while (!*done)
	{
		// Sleep while disconnected
		if (is_disconnected())
		{
			// 10 times per second should be often enough
			std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms));
			continue;
		}

		try
		{
			if (_socket.wait_incoming(timeout_ms))
			{
				size_t nr_bytes;
				{
					std::lock_guard<std::mutex> guard(_in_mutex);
					nr_bytes = _socket.receive(_in_buffer.data() + _in_buffer_used,
						_in_buffer.size() - _in_buffer_used);
				}
				_in_buffer_used += nr_bytes;
				process_incoming_data(queue);
			}
		}
		catch (const NotConnected&)
		{
			// Shouldn't happen
			continue;
		}
		catch (const PollError& err)
		{
			// An error occurred while checking the socket
			_in_buffer_used = 0;
			disconnect_from_server(strerror(err.error));
		}
		catch (const LostConnection&)
		{
			// Connection was closed by the server
			_in_buffer_used = 0;
			disconnect_from_server();
		}
		catch (const ReceiveError& err)
		{
			// An error occurred while reading data
			_in_buffer_used = 0;
			disconnect_from_server(err.what());
		}
	}
}

} // namespace eternal_lands

using namespace eternal_lands;

extern "C" int is_disconnected()
{
	return Connection::get_instance().is_disconnected();
}

extern "C" void connection_set_server(const char* name, std::uint16_t port)
{
	Connection::get_instance().set_server(name, port);
}

extern "C" void connect_to_server()
{
	Connection::get_instance().connect_to_server();
}

extern "C" void force_server_disconnect(const char* msg)
{
	if (msg)
		Connection::get_instance().disconnect_from_server(msg);
	else
		Connection::get_instance().disconnect_from_server();
}

extern "C" void start_testing_server_connection()
{
	Connection::get_instance().start_connection_test();
}

extern "C" void check_if_testing_server_connection()
{
	Connection::get_instance().check_connection_test();
}

extern "C" void stop_testing_server_connection()
{
	Connection::get_instance().stop_connection_test();
}

extern "C" void check_heart_beat()
{
	Connection::get_instance().check_heart_beat();
}

extern "C" void send_login_info()
{
	Connection::get_instance().send_login_info();
}

extern "C" void send_new_char(const char* user_str, const char* pass_str, char skin, char hair,
	char eyes, char shirt, char pants, char boots, char head, char type)
{
	Connection::get_instance().send_new_char(user_str, pass_str, skin, hair, eyes, shirt, pants,
		boots, head, type);
}

extern "C" void move_to(short int x, short int y, int try_pathfinder)
{
	Connection::get_instance().send_move_to(x, y, try_pathfinder);
}

extern "C" void handle_encryption_invitation()
{
	Connection::get_instance().send_encryption_reply();
}

extern "C" int my_tcp_send(const Uint8* str, int len)
{
	if (len > 0)
		return Connection::get_instance().send(str[0], str+1, len-1);
	else
		return 0;
}

extern "C" int my_tcp_flush()
{
	return Connection::get_instance().flush();
}

extern "C" int get_message_from_server(void *thread_args)
{
	void **ptrs = static_cast<void**>(thread_args);
	queue_t *queue = static_cast<queue_t*>(ptrs[0]);
	int *done = static_cast<int*>(ptrs[1]);
	Connection::get_instance().receive(queue, done);
	return 1;
}

#endif // USE_SSL
