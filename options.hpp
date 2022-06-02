#ifndef BOMBERMAN_SERVER_OPTIONS_HPP
#define BOMBERMAN_SERVER_OPTIONS_HPP

#include <string>
#include <exception>



struct ServerOptions {
	uint16_t bomb_timer;
	uint8_t player_count;
	uint64_t turn_duration;
	uint16_t explosion_radius;
	uint16_t initial_blocks;
	uint16_t game_length;
	std::string server_name;
	uint16_t port;
	uint32_t seed;
	uint16_t size_x;
	uint16_t size_y;

	ServerOptions(int, char*[]);
};


#endif // BOMBERMAN_SERVER_OPTIONS_HPP