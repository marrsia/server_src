#ifndef BOMBERMAN_COMM_HPP
#define BOMBERMAN_COMM_HPP

#include <exception>
#include <cstdlib>
#include <string>
#include <arpa/inet.h>
#include <cstring>
#include <memory>
#include <boost/asio.hpp>


struct BufferException: public std::exception {
  inline const char * what () const throw () {
    return "Buffer writing error";
  }
};

class Buffer {
public:

	Buffer(char* buffer, size_t size);

	void write_8(uint8_t data);

  void write_16(uint16_t data);

  void write_32(uint32_t data);

  void write_64(uint64_t data);

	void write_string(std::string &data);

	void reset();

	char* get_data();

	size_t get_size();

private:
	char *buffer;
	size_t size;
	char* ptr; //first not-written spot

	void checkSize(size_t dataSize);
};

class Session {
public:
	Session(boost::asio::ip::tcp::socket &&socket, boost::asio::ip::tcp::endpoint &remote_ep);

	void read_8(uint8_t &location);
	void read_16(uint16_t &location);
	void read_32(uint32_t &location);
	void read_string(std::string &location);

	void send(Buffer &buffer);

	void close();

	std::string get_address();

private:
  boost::asio::ip::tcp::socket socket;
	boost::asio::ip::tcp::endpoint remote_ep;
	
};

#endif // BOMBERMAN_COMM_HPP