#include <iostream>
#include "options.hpp"

using namespace std;

int main(int argc, char* argv[]) {
		ServerOptions server_options(argc, argv);
		std::cout << server_options.server_name << " running...\n";

}