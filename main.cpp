#include <iostream>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>
#include <queue>
#include <memory>
#include <boost/asio.hpp>
#include <thread>

#include "options.hpp"
#include "server.hpp"
#include "messages.hpp"


namespace {
using tcp = boost::asio::ip::tcp;
const int BUF_SIZE = 65535;


void listening(Server &server, std::shared_ptr<Session> session) {
	try {
		while(true) {
			ClientMessage message(*session);
			if (message.id == ClientMessageId::Join) {
				Player player {message.name, (*session).get_address()};
				PlayerId player_id = server.add_player(player);
				break;
			}
		}
	}
	catch (std::exception &e) {
		std::cout << "Ending client session\n";
		(*session).close();
	}
} 

void sending (Server &server, std::shared_ptr<Session> session) {
	while(true) {
		try {
		ServerMessage hello = server.get_hello();
		char buf[BUF_SIZE];
		Buffer buffer(buf, BUF_SIZE);
		hello.serialize(buffer);
		(*session).send(buffer);

		while (server.check_lobby()) {
			PlayerId last_player = server.newest_player;
			std::unique_lock lock(server.players_mutex);
			server.player_joined.wait(
				lock,
				[&] {return server.newest_player != last_player;}
			);
			for (auto player: server.players) {
				if (player.first <= last_player) {
					continue;
				}
				ServerMessage message;
				message.id = ServerMessageId::AcceptedPlayer;
				message.player_id = player.first;
				message.player = player.second;
				buffer.reset();
				message.serialize(buffer);
				(*session).send(buffer);
				last_player = player.first;
			}
		}
		ServerMessage game_started;
		game_started.id = ServerMessageId::GameStarted;
		game_started.players = server.players;
		buffer.reset();
		game_started.serialize(buffer);
		(*session).send(buffer);
		}
		catch (std::exception &e) {
			(*session).close();
		}
	}
}


void accepting(Server &server, boost::asio::io_context &io_context, ServerOptions &options) {
	tcp::acceptor acceptor (io_context, tcp::endpoint(tcp::v6(), options.port));

	while (true) {
		tcp::socket socket(io_context);
		acceptor.accept(socket);
		socket.set_option(tcp::no_delay(true));
		tcp::endpoint remote_ep = socket.remote_endpoint();
		std::shared_ptr<Session> session_ptr = std::make_shared<Session>(std::move(socket), remote_ep);

		std::thread listener(listening, std::ref(server),  session_ptr);
		std::thread sender(sending, std::ref(server), session_ptr);
		listener.detach();
		sender.detach();
	}
	
}
}


int main(int argc, char* argv[]) {
	ServerOptions server_options(argc, argv);
	std::cout << server_options.server_name << " running...\n";
	boost::asio::io_context io_context;
	Server server(server_options);
	std::thread acceptor(accepting, std::ref(server), std::ref(io_context), std::ref(server_options));
	acceptor.join();
	
}