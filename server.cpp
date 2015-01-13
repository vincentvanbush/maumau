#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <map>
#include <string>

#include "utils.h"
#include "messages.h"
#include "games.h"

ushort service_port = 1234;
pthread_t main_thread;

void *main_loop(void *arg) {
	std::map <int, struct game_info*> games;
	std::map <std::string, struct player_info*> players;



	struct game_msg msg_buffer;

	struct sockaddr_in srv_addr, cl_addr;

	srv_addr = local_address(service_port);
	int srv_socket = create_socket();
	bind_socket(srv_socket, srv_addr);

	// Message loop
	while (true) {
		socklen_t fromlen = sizeof cl_addr;
		if (recvfrom (srv_socket, &msg_buffer, sizeof msg_buffer, 0, (struct sockaddr*) &cl_addr, &fromlen) < 0) {
			perror ("Error receiving data on socket");
			exit (EXIT_FAILURE);
		}
		int msg_type = msg_buffer.msg_type;
		printf("(%d) ", msg_type);
		switch (msg_type) {
		case JOIN_GAME: {
			printf("Received join game message from %s\n", msg_buffer.message.join_game.player_name);
			struct join_game_msg msg = msg_buffer.message.join_game;
			
			if (games[msg.game_id] == nullptr) {
				// Game does not exist yet, we have to create it
				printf("--- Game %d does not exist yet\n", msg.game_id);
				games[msg.game_id] = new_game(msg.game_id);
			}
			else {
				// Game exists. Let the player join or send him an error.
				printf("--- %s wants to join game %d\n", msg.player_name, msg.game_id);
			}

			new_game (1);
			}
			break;
		
		case MOVE: {
			printf("Received move message\n");
			}
			break;
		case LEAVE_GAME: {
			printf("Received leave game message\n");
			}
			break;
		case READY: {
			printf("Received ready message\n");
			}
			break;
		default: {
			printf("Unrecognized message type %d\n", msg_type);
			}
		}
	}

	close_socket(srv_socket);
} 	

int main(int argc, char* argv[]) {

	srand (time(NULL));

	if (pthread_create (&main_thread, NULL, main_loop, NULL) != 0) {
		printf ("Thread create error");
    	exit (EXIT_FAILURE);
	}

	printf ("Press ENTER to shutdown server \n");
	getc (stdin);
  
	exit (EXIT_SUCCESS);
}