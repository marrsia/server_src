#ifndef BOMBERMAN_MESSAGES_H
#define BOMBERMAN_MESSAGES_H

#include <exception>
#include <string>
#include <utility>
#include <variant>
#include <vector>
#include <unordered_map>
#include "comm.hpp"

typedef uint32_t BombId;
typedef uint8_t PlayerId;
typedef uint32_t Score;

enum struct Direction : uint8_t {Up = 0, Right = 1, Down = 2, Left = 3};

struct Position {
  uint16_t x;
  uint16_t y;
  void serialize(Buffer &buffer);
  bool operator<(const Position&) const;
};

struct Player {
  std::string name;
  std::string	address;
  void serialize(Buffer &buffer);
};


enum struct EventId : uint8_t {BombPlaced = 0, BombExploded = 1, PlayerMoved = 2, BlockPlaced = 3};

struct Event {
  EventId id;
  BombId bomb_id;
  Position position;
  std::vector<PlayerId> robots_destroyed;
  std::vector<Position> blocks_destroyed;
  PlayerId player_id;
  
  void serialize(Buffer &buffer);
};

struct DeserializationError: public std::exception {
  inline const char * what () const throw () {
    return "Deserialization error";
  }
};


enum struct ClientMessageId : uint8_t {Join = 0, PlaceBomb = 1, PlaceBlock = 2, Move = 3};

struct ClientMessage {
  ClientMessageId id;
  std::string name;
  Direction direction;

  ClientMessage() = default;
  ClientMessage(Session &session);
};


enum struct ServerMessageId: uint8_t {Hello = 0, AcceptedPlayer = 1, GameStarted = 2, Turn = 3, GameEnded = 4};

struct ServerMessage {
  ServerMessageId id;
  std::string server_name;
  uint8_t players_count;
  uint16_t size_x;
  uint16_t size_y;
  uint16_t game_length;
  uint16_t explosion_radius;
  uint16_t bomb_timer;
  PlayerId player_id;
  Player player;
  std::map<PlayerId, Player> players;
  uint16_t turn;
  std::vector<Event> events;
  std::unordered_map<PlayerId, Score> scores;

  void serialize(Buffer &buffer);
};


#endif // BOMBERMAN_MESSAGES_HPP