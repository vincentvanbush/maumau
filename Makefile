all:
	g++ -pthread -std=c++11 server.cpp utils.cpp games.cpp -ljsoncpp -o server -Wall

clean:
	rm server