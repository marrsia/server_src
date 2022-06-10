#include "options.hpp"
#include <boost/program_options.hpp>
#include <boost/optional.hpp>
#include <string>
#include <exception>
#include <iostream>
#include <chrono>


ServerOptions::ServerOptions(int argc, char* argv[]) {

  namespace po = boost::program_options;
  seed = (uint32_t) std::chrono::system_clock::now().time_since_epoch().count();
	
  try {
    po::options_description desc("Allowed options");
      desc.add_options()
      ("help,h", "produce help message")
      ("bomb-timer,b", po::value<uint16_t>(&bomb_timer)->required(), "Bomb timer")
      ("players-count,c", po::value<uint8_t>(&player_count)->required(), "Number of players")
      ("turn-duration,d", po::value<uint64_t>(&turn_duration)->required(), "Duration of turn in milliseconds")
      ("explosion-radius,e", po::value<uint16_t>(&explosion_radius)->required(), "Radius of explosion")
      ("initial-blocks,k", po::value<uint16_t>(&initial_blocks)->required(), "Number of blocked squares at the start of the game")
      ("game-length,l", po::value<uint16_t>(&game_length)->required(), "Length of the game")
      ("server-name,n", po::value<std::string>(&server_name)->required(), "Server name")
      ("port,p", po::value<uint16_t>(&port)->required(), "Server port")
      ("seed,s", po::value<uint32_t>(&seed), "Seed for pseudorandom number generation")
      ("size-x,x", po::value<uint16_t>(&size_x)->required(), "Width of the game map")
      ("size-y,y", po::value<uint16_t>(&size_y)->required(), "Height of the game map")
	  ;

    po::variables_map vm;        
    po::store(po::parse_command_line(argc, argv, desc), vm);

    if (vm.count("help")) {
      std::cout << desc << "\n";
      exit(EXIT_SUCCESS);
    }

    po::notify(vm);  
  }
  catch(std::exception& e) {
    std::cerr << "error: " << e.what() << "\n";
    exit(EXIT_FAILURE);
  }

}
