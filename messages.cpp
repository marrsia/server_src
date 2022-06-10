#include <exception>
#include <string>
#include <utility>
#include <variant>
#include <vector>
#include <unordered_map>
#include "comm.hpp"
#include "messages.hpp"

bool Position::operator<(const Position &pos) const {
	if (x == pos.x) {
		return y < pos.y;
	}
	return x < pos.x;
}

void Position::serialize(Buffer &buffer) {
	buffer.write_16(x);
	buffer.write_16(y);
}

void Player::serialize(Buffer &buffer) {
	buffer.write_string(name);
	buffer.write_string(address);
}

void Event::serialize(Buffer &buffer) {
	buffer.write_8((uint8_t)id);
	switch (id) {
		case EventId::BombPlaced:
			buffer.write_32(bomb_id);
			position.serialize(buffer);
			break;
		case EventId::BombExploded:
			buffer.write_32(bomb_id);
			buffer.write_32((uint32_t) robots_destroyed.size());
			for (auto player_id : robots_destroyed) {
				buffer.write_8(player_id);
			}
			buffer.write_32((uint32_t) blocks_destroyed.size());
			for (auto block_position : blocks_destroyed) {
				block_position.serialize(buffer);
			}
			break;
		case EventId::PlayerMoved:
			buffer.write_8(player_id);
			position.serialize(buffer);
			break;
		case EventId::BlockPlaced:
			position.serialize(buffer);
	}
}


ClientMessage::ClientMessage(Session &session) {
	uint8_t message_id;
	session.read_8(message_id);
	id = ClientMessageId(message_id);
	switch (id) {
	case ClientMessageId::Join:
		session.read_string(name);
		break;
	case ClientMessageId::Move:
		uint8_t dir;
		session.read_8(dir);
		direction = Direction(dir);
		break;
	}
}

void ServerMessage::serialize(Buffer &buffer) {
	buffer.write_8((uint8_t) id);
	switch (id) {
		case ServerMessageId::Hello:
			buffer.write_string(server_name);
			buffer.write_8(players_count);
			buffer.write_16(size_x);
			buffer.write_16(size_y);
			buffer.write_16(game_length);
			buffer.write_16(explosion_radius);
			buffer.write_16(bomb_timer);
			break;
		
		case ServerMessageId::AcceptedPlayer:
			buffer.write_8(player_id);
			player.serialize(buffer);
			break;
		
		case ServerMessageId::GameStarted:
			buffer.write_32(players.size());
			for (auto mapped_player: players) {
				buffer.write_8(mapped_player.first);
				mapped_player.second.serialize(buffer);
			}
			break;

		case ServerMessageId::Turn:
			buffer.write_16(turn);
			buffer.write_32(events.size());
			for (auto event: events) {
				event.serialize(buffer);
			}
			break;
		
		case ServerMessageId::GameEnded:
			buffer.write_32(scores.size());
			for (auto score: scores) {
				buffer.write_8(score.first);
				buffer.write_32(score.second);
			}
	}
}