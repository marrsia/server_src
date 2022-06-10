#include <exception>
#include <cstdlib>
#include <string>
#include <arpa/inet.h>
#include <cstring>
#include <boost/asio.hpp>

#include "comm.hpp"

//BUFFER
Buffer::Buffer(char* buffer, size_t size) : size(size), buffer(buffer), ptr(buffer) { }

void Buffer::write_8(uint8_t data) {
	checkSize(sizeof(data));
  *(uint8_t *)ptr = data;
  ptr += sizeof(uint8_t);
}

void Buffer::write_16(uint16_t data) {
	checkSize(sizeof(data));
  *(uint16_t *)ptr = htons(data);
  ptr += sizeof(uint16_t);
}

void Buffer::write_32(uint32_t data) {
	checkSize(sizeof(data));
  *(uint32_t *)ptr = htonl(data);
  ptr += sizeof(uint32_t);
}

void Buffer::write_64(uint64_t data) {
	checkSize(sizeof(data));
  *(uint64_t *)ptr = htobe64(data);
  ptr += sizeof(uint64_t);
}

void Buffer::write_string(std::string &data) {
  uint8_t size = data.size();
	checkSize(size + 1);
	write_8(size);
	memcpy(ptr, data.c_str(), size);
  ptr += data.size() * sizeof(char);
}

void Buffer::checkSize(size_t dataSize) {
	if (ptr + dataSize > buffer + size) {
		throw BufferException();
	}
}

void Buffer::reset() {
	ptr = buffer;
}

char* Buffer::get_data() {
	return buffer;
}

size_t Buffer::get_size() {
	return (size_t) (ptr - buffer);
}

//SESSION
Session::Session(boost::asio::ip::tcp::socket &&socket, boost::asio::ip::tcp::endpoint &remote_ep) 
	: socket(std::move(socket)), remote_ep(remote_ep) {}

void Session::read_8(uint8_t &location) {
	boost::asio::read(socket, boost::asio::buffer(&location, sizeof(uint8_t)));
}

void Session::read_16(uint16_t &location) {
	boost::asio::read(socket, boost::asio::buffer(&location, sizeof(uint16_t)));
	location = ntohs(location);
}

void Session::read_32(uint32_t &location) {
	boost::asio::read(socket, boost::asio::buffer(&location, sizeof(uint32_t)));
	location = ntohl(location);
}

void Session::read_string(std::string &location) {
	uint8_t len;
	read_8(len);
	char c_str[len + 1];
	memset(c_str, 0, len + 1);
	boost::asio::read(socket, boost::asio::buffer(c_str, len));
	location = std::string(c_str);
}

void Session::send(Buffer &buffer) {
	boost::asio::write(socket, boost::asio::buffer(buffer.get_data(), buffer.get_size()));
}

void Session::close() {
	socket.close();
}

std::string Session::get_address() {
	std::ostringstream address;
  address << socket.remote_endpoint();
	return address.str();
}

