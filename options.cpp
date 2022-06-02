#include "options.hpp"
#include <boost/program_options.hpp>
#include <boost/optional.hpp>
#include <string>
#include <exception>

namespace params {

	struct addressException : public std::exception {
    const char * what () const throw () {
      return "Incorrect address.";
    }
	};

	void separateAddress(std::string &addressPort, std::string &address, port_t &port) {
		std::cout << addressPort << "\n";
		size_t found = addressPort.find(':');
		if (found == std::string::npos) {
			throw params::addressException();
		}
		address = addressPort.substr(0, found);
		errno = 0;
		int tmpPort = stoi(addressPort.substr(found + 1, address.size()));
		if (errno ||tmpPort > UINT16_MAX) {
			throw params::addressException();
		}
		port = (port_t) tmpPort;
	}
}; //params


ServerOptions::ServerOptions(int argc, char* argv[]) {

	port_t clientPort;
  std::string gsakjaauiAddress, playerName, serverAddress;
	
  try {
    po::options_description desc("Allowed options");
      desc.add_options()
      ("help,h", "produce help message")
      ("gui_address,d", po::value<std::string>(&guiAddress)->required(), "Address of the GUI server")
	    ("player-name,n", po::value<std::string>(&playerName)->required(), "Player name")
	    ("port,p", po::value<port_t>(&clientPort)->required(),"Port on which the client listens for messages from the GUI server")
	    ("server-address,s", po::value<std::string>(&serverAddress)->required(), "Address of the server")
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
  catch(...) {
    std::cerr << "Exception of unknown type!\n";
  }


}
