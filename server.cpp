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

const int MAX_GAME_NUM = 50;

ushort service_port = 1234;
pthread_t main_thread;

void *main_loop(void *arg) {
	std::map <int, struct game_info*> games;

	int games_num = 0;

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
			struct game_info* game;
			if (games[msg.game_id] == nullptr) {
				// Game does not exist yet, we have to create it
				printf("--- Game %d does not exist yet\n", msg.game_id);
				if (games_num > MAX_GAME_NUM - 1 || msg.game_id > 49) {
					printf("--- Server is full\n");
					struct game_msg error_msg;
					error_msg.msg_type = CANNOT_JOIN;
					socklen_t tolen = sizeof cl_addr;
					if (sendto (srv_socket, &error_msg, sizeof error_msg, 0, (struct sockaddr*) &cl_addr, tolen) < 0) {
						perror ("Error sending CANNOT_JOIN to client socket");
						exit (EXIT_FAILURE);
					}
					else {
						printf("Sent CANNOT_JOIN to client\n");
					}
					break;
				}
				game = new_game(msg.game_id);
				games[msg.game_id] = game;
				games_num++;
			}
			else {
				// Game exists. Let the player join or send him an error.
				printf("--- %s wants to join game %d\n", msg.player_name, msg.game_id);
				if (game -> started || game -> players.size() >= 4) {
					puts("--- Cannot join this game");
					struct game_msg error_msg;
					error_msg.msg_type = CANNOT_JOIN;
					socklen_t tolen = sizeof cl_addr;
					if (sendto (srv_socket, &error_msg, sizeof error_msg, 0, (struct sockaddr*) &cl_addr, tolen) < 0) {
						perror ("Error sending CANNOT_JOIN to client socket");
						exit (EXIT_FAILURE);
					}
					else {
						printf("Sent CANNOT_JOIN to client\n");
					}
					break;
				}
			}
			game = games[msg.game_id];
			struct player_info* player = new_player (msg.player_name);
			player_join_game (player, game);
			player -> net_addr = cl_addr;

			struct game_msg join_ok_msg;
			join_ok_msg.msg_type = JOIN_OK;
			join_ok_msg.message.join_ok.player_token = player -> token;
			join_ok_msg.message.join_ok.game_token = game -> game_token;
			join_ok_msg.message.join_ok.slot_number = game -> players.size() - 1;
			puts("--- Join OK");
			socklen_t tolen = sizeof cl_addr;
			if (sendto (srv_socket, &join_ok_msg, sizeof join_ok_msg, 0, (struct sockaddr*) &cl_addr, tolen) < 0) {
				perror ("Error sending JOIN_OK to client socket");
				exit (EXIT_FAILURE);
			}
			else printf("Sent JOIN_OK to client\n");
			
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
			int player_token = msg_buffer.token;
			int game_token = msg_buffer.game_token;
			struct ready_msg ready_msg = msg_buffer.message.ready;

			struct game_info* game = games[msg_buffer.game_id];

			// if received game token is different to the actual one, send error
			if (game -> game_token != game_token) {
				// TODO: send error message to client
				break;	
			}

			// check if received player token is valid
			bool invalid_player_token = true;
			short player_index = -1;
			for (int i = 0; i < game -> players.size(); i++) {
				if (game -> players[i] -> token == player_token) {
					invalid_player_token = false;
					player_index = i;
					break;
				}
			}
			if (invalid_player_token) {
				// TODO: send error message to client
				break;
			}

			game -> players[player_index] -> ready = true;

			if (game -> players.size() >= 2) {
				short ready_players = 0;
				for (int i = 0; i < game -> players.size(); i++)
					if (game -> players[i] -> ready)
						++ready_players;
				if (ready_players == game -> players.size()) {
					// all players are ready, so deal the cards and start the game
					deal_cards (game);

					// send 'Start game' to all players in game
					for (int i = 0; i < game -> players.size(); i++) {
						struct sockaddr_in player_addr = game -> players[i] -> net_addr;
						printf("Player addr: %s:%d\n", inet_ntoa(player_addr.sin_addr), ntohs(player_addr.sin_port));
						struct player_info* player = game -> players[i];

						struct game_msg start_game_msg;
						start_game_msg.msg_type = START_GAME;

						struct start_game_msg* start_game = &start_game_msg.message.start_game;
						for (int j = 0; j < player -> cards.size(); j++)
							start_game -> player_cards[j] = player -> cards[j];
						start_game -> first_card_in_stack = game -> deck.front();
						
						start_game -> turn = 0;

						socklen_t tolen = sizeof player_addr;
						if (sendto (srv_socket, &start_game_msg, sizeof start_game_msg, 0, (struct sockaddr*) &player_addr, tolen) < 0) {
							perror ("Error sending START_GAME to client socket");
							exit (EXIT_FAILURE);
						}
						else printf ("Start game message sent to %s\n", game -> players[i] -> player_name);
					}
					
				}
			}
			

		}
		break;

		case REQUEST_GAME_LIST: {
			printf("Received request game list message\n");

			struct game_list_msg game_list;
			for (int i = 0; i < MAX_GAME_NUM; i++) { 
				if (games[i] == nullptr) {
					game_list.game_exists[i] = false;
				}
				else {
					game_list.game_exists[i] = true;
					game_list.game_id[i] = games[i] -> game_id;
					game_list.players_count[i] = games[i] -> players.size();
					for (int j = 0; j < games[i] -> players.size(); j++) {
						strcpy (game_list.player_nick[i][j], games[i] -> players[j] -> player_name);
					}
					game_list.started[i] = games[i] -> started;
				}
			}

			struct game_msg game_list_msg;
			game_list_msg.msg_type = GAME_LIST;
			game_list_msg.message.game_list = game_list;
			socklen_t tolen = sizeof cl_addr;
			if (sendto (srv_socket, &game_list_msg, sizeof game_list_msg, 0, (struct sockaddr*) &cl_addr, tolen) < 0) {
				perror ("Error sending GAME_LIST to client socket");
				exit (EXIT_FAILURE);
			}
			else printf("Sent GAME_LIST to client\n");
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