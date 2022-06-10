#include <iostream>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>
#include <queue>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>

#include "options.hpp"
#include "server.hpp"
#include "messages.hpp"


namespace {
using tcp = boost::asio::ip::tcp;

void accepting(Server &server, boost::asio::io_context &io_context, ServerOptions &options) {
	tcp::acceptor acceptor (io_context, tcp::endpoint(tcp::v6(), options.port));

	while (true) {
		tcp::socket socket(io_context);
		acceptor.accept(socket);
		socket.set_option(tcp::no_delay(true));
		tcp::endpoint remote_ep = socket.remote_endpoint();
//		std::make_shared<Session>(socket, remote_ep);
	}

	//start listener thread

	//start sender thread
}

void listening(Server &server, boost::asio::io_context &io_context, std::shared_ptr<Session> session) {
	// get join message from client
	// if game started -> send gameStarted (how do we know to do that?)
	// if lobby - add to server & wait for other players (on a conditional variable from server)
	// after waking up listen for turns and use servers add event for as long as the game is active
	// after/before each listen check that the game has not ended?
	// if ended, go back to the beginning
} 

void sending (Server &server, boost::asio::io_context &io_context, std::shared_ptr<Session> session) {
	// send Hello
	//
	//
	//
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
	Server server(io_context, server_options);
	
}