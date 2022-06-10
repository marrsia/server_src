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
#include "server.hpp"

//GAMEDATA
GameData::GameData(std::minstd_rand &random, 
									std::map<PlayerId, Player> &players, 
									uint16_t initial_blocks, uint16_t size_x, uint16_t size_y) {
	next_bomb_id = 1;
	turn = 0;
	for (auto player: players) {
		uint16_t x = random() % size_x;
		uint16_t y = random() % size_y;
		player_positions[player.first] = {x, y};
		deaths[player.first] = 0;
	}
	
	for (auto player: player_positions) {
		Event event;
		event.id = EventId::PlayerMoved;
		event.player_id = player.first;
		event.position = player.second;
		events.push_back(event);
	}
	
	auto blocks_left = initial_blocks;
	while (blocks_left > 0) {
		uint16_t x = random() & size_x;
		uint16_t y = random() % size_y;
		if (! blocks.contains({x, y})) {
			blocks.insert({x, y});
			blocks_left--;
		} 
	}
	
	for (auto block: blocks) {
		Event event;
		event.id = EventId::BlockPlaced;
		event.position = block;
		events.push_back(event);
	}

}

GameData::GameData() = default;

//SERVER
Server::Server(ServerOptions &options) :
							 options(options),
						 	 random(options.seed), game_state(GameStateId::Lobby),
							 next_player_id(1) 
	{
		hello.id = ServerMessageId::Hello;
		hello.server_name = options.server_name;
		hello.players_count = options.player_count;
		hello.size_x = options.size_x;
		hello.size_y = options.size_y;
		hello.game_length = options.game_length;
		hello.explosion_radius = options.explosion_radius;
		hello.bomb_timer = options.bomb_timer; 					 
	};

void Server::reset() {
	next_player_id = 1;
	actions.clear();
	players.clear();
}

void Server::initGame() {
	game_data = GameData(random, players, options.initial_blocks, options.size_x, options.size_y);
}

PlayerId Server::add_player(Player &player) {
	players_mutex.lock();
	PlayerId ret = next_player_id;
	players.insert({next_player_id, player});
	next_player_id++;
	players_mutex.unlock();
	return ret;
}

void Server::add_player_action(PlayerId player_id, ClientMessage &message) {
	actions_mutex.lock();
	actions[player_id] = message;
	actions_mutex.unlock();
}

bool Server::check_position(int x, int y) {
	if (x < 0 || x >= options.size_x || y < 0 || y >= options.size_y) {
		return false;
	}
	if (game_data.blocks.contains({(uint16_t) x, (uint16_t) y})) {
		return false;
	}
	return true;
}

void Server::process_bombs() {
	static const std::pair<int, int> changes[] = {{0, 1}, {1, 0}, {-1, 0}, {0, -1}};
	for (auto bomb_pair: game_data.bombs) {
		BombId bomb_id = bomb_pair.first;
		Bomb bomb = bomb_pair.second;
		bomb.timer--;
		if (!bomb.timer) {
			game_data.bombs.erase(bomb_id);
			Event event;
			event.id = EventId::BombExploded;
			event.bomb_id = bomb_id;
			for (int i = 0; i < 4; i++) {
				for (int dist = 0; dist <= options.explosion_radius; dist++) {
					int new_x, new_y;
					new_x = bomb.position.x + changes[i].first * dist;
					new_y = bomb.position.y + changes[i].second * dist;
					if (!check_position(new_x, new_y)) {
						break;
					}
					//check im not outside of the map!!
					Position pos = {(uint16_t) new_x, (uint16_t) new_y};
					if (game_data.blocks.contains(pos)) {
						game_data.blocks.erase(pos);
						event.blocks_destroyed.push_back(pos);
						break;
					}
				}
			}
			
			for (auto player: game_data.player_positions) {
				if (game_data.current_deaths.contains(player.first)) {
					continue;
				}
				uint16_t dist_x, dist_y;
				dist_x = abs((int) bomb.position.x - (int) player.second.x);
				dist_y = abs((int) bomb.position.y - (int) player.second.y);
				if ((dist_x == 0 && dist_y <= options.explosion_radius) ||
						(dist_x <= options.explosion_radius && dist_y == 0)) {
					game_data.current_deaths.insert(player.first);
					event.robots_destroyed.push_back(player.first);
				}	
			}
			game_data.events.push_back(event);
		}
	}
};

void Server::process_deaths() {
	for (auto player: game_data.current_deaths) {
		Event event;
		event.id = EventId::PlayerMoved;
		event.player_id = player;
		uint16_t x = random() & options.size_x;
		uint16_t y = random() % options.size_y;
		event.position = {x, y};
		game_data.events.push_back(event);
		game_data.deaths[player]++;
	}
}


void Server::process_actions() {
	static const std::pair<int, int> changes[] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
	for (auto action: actions) {
		PlayerId player_id = action.first;
		ClientMessage message = action.second;
		if (game_data.current_deaths.contains(player_id)) {
			continue;
		}

		switch (message.id) {
			case ClientMessageId::PlaceBomb: {
				Event event;
				event.id = EventId::BombPlaced;
				event.bomb_id = game_data.next_bomb_id++;
				event.position = game_data.player_positions[player_id];
				Bomb bomb = {options.bomb_timer, event.position};
				game_data.events.push_back(event);
				game_data.bombs[event.bomb_id] = bomb;
				break;
			}
			case ClientMessageId::PlaceBlock: {
				if (game_data.blocks.contains(game_data.player_positions[player_id])) {
					break;
				}
				Event event;
				event.id = EventId::BlockPlaced;
				event.position = game_data.player_positions[player_id];
				game_data.events.push_back(event);
				game_data.blocks.insert(event.position);
				break;
			}
			case ClientMessageId::Move: {
				int new_x, new_y;
				new_x = (int) game_data.player_positions[player_id].x + changes[(int) message.direction].first;
				new_y = (int) game_data.player_positions[player_id].y + changes[(int) message.direction].second;
				if (check_position(new_x, new_y)) {
					Event event;
					event.id = EventId::PlayerMoved;
					event.player_id = player_id;
					event.position = {(uint16_t) new_x,(uint16_t) new_y};
					game_data.events.push_back(event);
				}
				break;
			}
		}
	}
}

void Server::process_turn() {
	process_bombs();
	process_deaths();
	process_actions();
	game_data.current_deaths.clear();
	actions.clear();
	ServerMessage turn;
	turns.push_back(turn);
	turns[turns.size() - 1].id = ServerMessageId::Turn;
	turns[turns.size() - 1].turn = game_data.turn++;
	for (auto event: game_data.events) {
		turns[turns.size() - 1].events.push_back(event);
	}
	game_data.events.clear();
}

std::vector<ServerMessage>& Server::get_turns() {
	return turns;
}

ServerMessage& Server::get_hello() {
	return hello;
}