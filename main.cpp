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
				std::cout << "joining " << message.name << "\n";
				Player player {message.name, (*session).get_address()};
				PlayerId player_id = server.add_player(player);
				break;
			}
		}
	}
	catch (std::exception &e) {
		(*session).close();
	}
	// get join message from client
	// if game started -> send gameStarted (how do we know to do that?)
	// if lobby - add to server & wait for other players (on a conditional variable from server)
	// after waking up listen for turns and use servers add event for as long as the game is active
	// after/before each listen check that the game has not ended?
	// if ended, go back to the beginning
} 

void sending (Server &server, std::shared_ptr<Session> session) {
	try {
		ServerMessage hello = server.get_hello();
		char buf[BUF_SIZE];
		Buffer buffer(buf, BUF_SIZE);
		hello.serialize(buffer);
		(*session).send(buffer);
	}
	catch (std::exception &e) {
		(*session).close();
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


void handle_game(Server &server, ServerOptions &options) {
	while (true) {
		server.reset();

		// wait for game to start in some way?

		//prep first turn message
		server.initGame();
		server.process_turn();
		// it should be sent now
		
		for (int i = 1; i <= options.game_length; i++) {
			std::this_thread::sleep_for(std::chrono::milliseconds(options.turn_duration));
			server.process_turn();
			// should be sent to players now
		}
	//	server.end_game();

	}
}
}


int main(int argc, char* argv[]) {
	ServerOptions server_options(argc, argv);
	std::cout << server_options.server_name << " running...\n";
	std::cout <<
							server_options.bomb_timer << " - bomb timer\n" <<
							server_options.player_count << " - player_count\n" <<
							server_options.turn_duration << " - turn duration\n" <<
							server_options.explosion_radius << "- explosion radius\n" <<
							server_options.initial_blocks << "- initial blocks\n" <<
							server_options.game_length << " - game lenght\n" <<
							server_options.port << " - port \n" <<
							server_options.seed << "- seed\n" <<
							server_options.size_x << " " << server_options.size_y << " - size x, y\n";
	
	boost::asio::io_context io_context;
	Server server(server_options);
	std::thread acceptor(accepting, std::ref(server), std::ref(io_context), std::ref(server_options));
	acceptor.join();
	
}