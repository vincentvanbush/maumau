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

struct game_info** games;
pthread_mutex_t games_lock = PTHREAD_MUTEX_INITIALIZER;

const int MAX_GAME_NUM = 50;

ushort service_port = 1234;
pthread_t main_thread;

int games_num = 0;

void *client_loop(void *arg) {
	int rcv_sck = *(int*)arg;

	printf ("Hello, this is the client thread (%d)\n", rcv_sck);
	struct game_msg msg_buffer;
	struct sockaddr_in cl_addr;


	while (recv (rcv_sck, &msg_buffer, sizeof msg_buffer, 0) > 0) {
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
					struct game_msg error_msg;
					error_msg.msg_type = CANNOT_JOIN;
					if (send (rcv_sck, &error_msg, sizeof error_msg, 0) < 0) {
						perror ("Error sending CANNOT_JOIN to client socket");
						exit (EXIT_FAILURE);
					}
					else {
						printf("Sent CANNOT_JOIN to client\n");
					}
					break;
				}
				game = new_game(msg.game_id);
				pthread_mutex_lock(&games_lock);
				games[msg.game_id] = game;
				games_num++;
				pthread_mutex_unlock(&games_lock);
			}
			else {
				// Game exists. Let the player join or send him an error.
				game = games[msg.game_id];
				printf("--- %s wants to join game %d\n", msg.player_name, msg.game_id);
				printf("Game id %d has token %d\n", game -> game_id, game -> game_token);
				if (game -> started || game -> players.size() >= 4) {
					puts("--- Cannot join this game");
					struct game_msg error_msg;
					error_msg.msg_type = CANNOT_JOIN;
					if (send (rcv_sck, &error_msg, sizeof error_msg, 0) < 0) {
						perror ("Error sending CANNOT_JOIN to client socket");
						exit (EXIT_FAILURE);
					}
					else {
						printf("Sent CANNOT_JOIN to client\n");
					}
					break;
				}
			}			
			
			struct player_info* player = new_player (msg.player_name);
			pthread_mutex_lock(&games_lock);
			player_join_game (player, game);
			pthread_mutex_unlock(&games_lock);
			player -> socket = rcv_sck;

			struct game_msg player_joined_msg;
			player_joined_msg.msg_type = PLAYER_JOINED;
			strcpy(player_joined_msg.message.player_joined.player_name, msg.player_name);
			player_joined_msg.message.player_joined.slot_number = game -> players.size() - 1;
			for (int i = 0; i < game -> players.size() && game -> players[i] -> socket != rcv_sck; i++) {
				if (send (game -> players[i] -> socket, &player_joined_msg, sizeof player_joined_msg, 0) < 0) {
					perror ("Error sending PLAYER_JOINED to client socket");
					exit (EXIT_FAILURE);
				}
				else printf("Sent PLAYER_JOINED to client %s\n", game -> players[i] -> player_name);
			}


			struct game_msg join_ok_msg;
			join_ok_msg.msg_type = JOIN_OK;
			join_ok_msg.message.join_ok.player_token = player -> token;
			join_ok_msg.message.join_ok.game_token = game -> game_token;
			join_ok_msg.message.join_ok.slot_number = game -> players.size() - 1;
			puts("--- Join OK");
			if (send (rcv_sck, &join_ok_msg, sizeof join_ok_msg, 0) < 0) {
				perror ("Error sending JOIN_OK to client socket");
				exit (EXIT_FAILURE);
			}
			else printf("Sent JOIN_OK to client\n");
			
		}
		break;
		
		case MOVE: {
			printf("Received move message\n");

			int player_token = msg_buffer.token;
			int game_token = msg_buffer.game_token;
			int game_id = msg_buffer.game_id;
			struct move_msg move = msg_buffer.message.move;

			struct game_info* game = games[game_id];

			bool valid_move = validate_move (&move, game);

			if (valid_move) {
				// Update game state
				(++game -> turn) %= game -> players.size();

				std::deque <struct card> cards_picked_up = update_game_state (&move, game);


				// Broadcast move to others
				for (int i = 0; i < game -> players.size() && game -> players[i] -> socket != rcv_sck; i++) {
					if (send (game -> players[i] -> socket, &msg_buffer, sizeof msg_buffer, 0) < 0) {
						perror ("Error sending MOVE to client socket");
						exit (EXIT_FAILURE);
					}
					else printf("Sent MOVE to client %s\n", game -> players[i] -> player_name);
				}

				// Send NEXT_TURN to all players, augmented with cards picked up for the sender
				for (int i = 0; i < game -> players.size(); i++) {
					struct game_msg next_turn_msg;
					next_turn_msg.msg_type = NEXT_TURN;
					struct next_turn_msg* next_turn = &next_turn_msg.message.next_turn;
					next_turn -> turn = game -> turn;
					strcpy(next_turn -> player_name, game -> players[game -> turn] -> player_name);
					next_turn -> cards_picked_up = cards_picked_up.size();
					for (int i = 0; i < cards_picked_up.size(); i++)
						next_turn -> cards[i] = cards_picked_up[i];

					if (send (game -> players[i] -> socket, &msg_buffer, sizeof msg_buffer, 0) < 0) {
						perror ("Error sending MOVE to client socket");
						exit (EXIT_FAILURE);
					}
					else printf("Sent MOVE to client %s\n", game -> players[i] -> player_name);
				}
			}
			else {
				struct game_msg invalid_move_msg;
				invalid_move_msg.msg_type = INVALID_MOVE;
				if (send (rcv_sck, &invalid_move_msg, sizeof invalid_move_msg, 0) < 0) {
					perror ("Error sending INVALID_MOVE to client socket");
					exit (EXIT_FAILURE);
				}
			}
		}
		break;

		case LEAVE_GAME: {
			printf("Received leave game message\n");
		}
		break;

		case READY: {

			printf("Received ready message\n");
			printf ("Game id = %hu\n", msg_buffer.game_id);

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
				}
			}
			if (invalid_player_token) {
				// TODO: send error message to client
				break;
			}

			pthread_mutex_lock(&games_lock);
			game -> players[player_index] -> ready = true;
			pthread_mutex_unlock(&games_lock);

			if (game -> players.size() >= 2) {
				short ready_players = 0;
				for (int i = 0; i < game -> players.size(); i++)
					if (game -> players[i] -> ready)
						++ready_players;
				if (ready_players == game -> players.size()) {
					// all players are ready, so deal the cards and start the game
					pthread_mutex_lock(&games_lock);
					deal_cards (game);
					pthread_mutex_unlock(&games_lock);

					// send 'Start game' to all players in game
					for (int i = 0; i < game -> players.size(); i++) {
						int player_sck = game -> players[i] -> socket;
						struct player_info* player = game -> players[i];

						struct game_msg start_game_msg;
						start_game_msg.msg_type = START_GAME;

						struct start_game_msg* start_game = &start_game_msg.message.start_game;
						for (int j = 0; j < player -> cards.size(); j++)
							start_game -> player_cards[j] = player -> cards[j];
						start_game -> first_card_in_stack = game -> deck.front();
						
						start_game -> turn = 0;

						if (send (player_sck, &start_game_msg, sizeof start_game_msg, 0) < 0) {
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
			pthread_mutex_lock(&games_lock);
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
			pthread_mutex_unlock(&games_lock);
			struct game_msg game_list_msg;
			game_list_msg.msg_type = GAME_LIST;
			game_list_msg.message.game_list = game_list;
			if (send (rcv_sck, &game_list_msg, sizeof game_list_msg, 0) < 0) {
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
}

void *main_loop(void *arg) {
	
	games = new struct game_info*[50];
	for (int i = 0; i < 50; i++) {
		games[i] = nullptr;
	}
	struct sockaddr_in srv_addr, cl_addr;

	srv_addr = local_address(service_port);
	int srv_socket = create_socket();
	bind_socket(srv_socket, srv_addr);

	if (listen (srv_socket, 200) == -1) {
		perror ("Cannot listen to main socket");
		exit (EXIT_FAILURE);
	}

	// Message loop
	while (true) {
		socklen_t fromlen = sizeof cl_addr;
		int rcv_sck;
		if ((rcv_sck = accept (srv_socket, (struct sockaddr*) &cl_addr, &fromlen)) == -1) {
			perror ("Main socket accept error \n");
			continue;
		}

		pthread_t client_thread;
		if (pthread_create (&client_thread, NULL, client_loop, &rcv_sck)) {
			perror ("Error creating client thread");
			exit (EXIT_FAILURE);
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