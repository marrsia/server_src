#ifndef BOMBERMAN_SERVER_HPP
#define BOMBERMAN_SERVER_HPP

#include <cstdlib>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>
#include <queue>
#include <boost/smart_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <random>

#include "options.hpp"
#include "messages.hpp"

struct Bomb {
	uint16_t timer;
	Position position;
};


enum struct GameStateId : uint8_t {Lobby = 0, Game = 1};

struct GameData {
	uint32_t turn;
	std::unordered_map<PlayerId, Position> player_positions;
	std::unordered_map<PlayerId, Score> deaths;
	std::map<BombId, Bomb> bombs; 
	std::set<Position> blocks;
	std::set<PlayerId> current_deaths;
	std::vector<Event> events;
	BombId next_bomb_id;

	GameData(std::minstd_rand &random, 
					std::map<PlayerId, Player> &players, 
					uint16_t initial_blocks, uint16_t size_x, uint16_t size_y);

	GameData();
	
};


class Server {
public:
	Server(ServerOptions &options);
	
	void reset();

	void initGame();

	PlayerId add_player(Player &player);

	void add_player_action(PlayerId player_id, ClientMessage &message); 

	GameStateId game_state;

	std::vector<ServerMessage>& get_turns();
	
	void process_turn();

	void end_game();

	ServerMessage& get_hello();
	
	std::condition_variable player_joined;
	PlayerId newest_player = 0;

	ServerMessage get_new_player();
	std::mutex players_mutex;
	std::map<PlayerId, Player> players;

private:
	std::minstd_rand random;	

	ServerOptions options;

	PlayerId next_player_id;

	std::mutex actions_mutex;
	std::map<PlayerId, ClientMessage> actions;
	
	GameData game_data; // information about the game
	
	std::vector<ServerMessage> turns;
	ServerMessage game_ended;
	ServerMessage hello;
	ServerMessage game_started;

	bool check_position(int x, int y);
	void process_bombs();
	void process_deaths();
	void process_actions();
};

#endif // BOMBERMAN_SERVER_HPP