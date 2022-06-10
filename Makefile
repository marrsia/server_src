CC = g++
CFLAGS = -Wall -Wextra -std=c++20 -I ../boost_1_79_0
LIBS = -L/usr/local/lib -lboost_program_options -lboost_system -pthread

robots-server: main.o server.o options.o comm.o 
							$(CC) $(CFLAGS) -o $@ main.o server.o options.o comm.o $(LIBS)

server: server.o 
				$(CC) $(CFLAGS) -o $@ chat_server.o $(LIBS)

client: client.o 
				$(CC) $(CFLAGS) -o $@ client.o $(LIBS)

.cpp.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf robots-server *.o
