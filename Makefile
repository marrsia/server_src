CC = g++
CFLAGS = -std=gnu++20 -Wall -Wextra -Wconversion -Werror -O2
LIBS = -L/usr/local/lib -lboost_program_options -lboost_system -pthread

robots-server: main.o server.o options.o messages.o comm.o 
							$(CC) $(CFLAGS) -o $@ main.o server.o messages.o options.o comm.o $(LIBS)

.cpp.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf robots-server *.o