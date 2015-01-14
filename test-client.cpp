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
#include <deque>

#include "utils.h"
#include "messages.h"

const int MAX_GAME_NUM = 50;

ushort client_port = 0;
ushort server_port = 1234;

struct sockaddr_in srv_addr = local_address(server_port);
int cl_socket = create_socket();

void *recv_loop(void *arg) {
	struct game_msg msg_buffer;
	while (1) {
		socklen_t fromlen = sizeof srv_addr;
		if (recvfrom (cl_socket, &msg_buffer, sizeof msg_buffer, 0, (struct sockaddr*) &srv_addr, &fromlen) < 0) {
			perror ("Error receiving data on socket");
			exit (EXIT_FAILURE);
		}
		else {
			printf("Received message from server, type: %d\n", msg_buffer.msg_type);
		}

		if (msg_buffer.msg_type == GAME_LIST) {
			printf("Game list...\n");
			struct game_list_msg game_list = msg_buffer.message.game_list;
			for (int i = 0; i < MAX_GAME_NUM; i++) {
				if (game_list . game_exists[i]) {
					printf("id=%d\tplayers_count=%d\tstarted=%s ",
						game_list . game_id[i],
						game_list . players_count[i],
						game_list . started[i] ? "aktywna" : "nieaktywna"
					);
					if (game_list.game_exists[i]) {
						for (int j = 0; j < game_list.players_count[i]; j++) {
							printf("%s ", game_list.player_nick[i][j]);
						}
						printf("\n");
					}
				}
			}
		}

		else if (msg_buffer.msg_type == JOIN_OK) {
			struct join_ok_msg join_ok = msg_buffer.message.join_ok;
			int player_token = join_ok.player_token;
			int game_token = join_ok.game_token;
			printf("Player token: %d\nGame token: %d\n", player_token, game_token);
		}

		else if (msg_buffer.msg_type == START_GAME) {
			struct start_game_msg start_game = msg_buffer.message.start_game;
			std::deque <struct card> cards(start_game.player_cards, start_game.player_cards + 5);
			printf("Your cards:\n");
			for (int i = 0; i < cards.size(); i++) {
				printf("\t%d %d\n", cards[i].value, cards[i].color);
			}
			printf("Top card:\n");
			printf("\t%d %d\n", start_game.first_card_in_stack.value, start_game.first_card_in_stack.color);
		}
	}
}

int main(int argc, char* argv[]) {
	

	// Create client address info
	struct sockaddr_in cl_addr = local_address(client_port);

	// Initialize UDP socket
	
/*
	// Bind UDP socket to client address
	bind_socket(cl_socket, cl_addr);
*/
	
	inet_aton("127.0.0.1", &srv_addr.sin_addr);

	if (connect (cl_socket, (struct sockaddr*) &srv_addr, sizeof srv_addr) < 0) {
		perror ("Brak polaczenia");
		exit (EXIT_FAILURE);
	}

	pthread_t recv_thread;
	pthread_create (&recv_thread, NULL, recv_loop, NULL);

	while (1) {
		// define message
		struct game_msg msg_buffer;
		printf("Enter message type: ");
		scanf("%hu", &msg_buffer.msg_type);

		if (msg_buffer.msg_type == JOIN_GAME) {
			printf("Enter player name ");
			scanf("%s", &msg_buffer.message.join_game.player_name);
			printf("Enter game id ");
			scanf("%d", &msg_buffer.message.join_game.game_id);
		}

		else if (msg_buffer.msg_type == READY) {
			printf("Enter your player token ");
			scanf("%d", &msg_buffer.token);
			printf("Enter your game token ");
			scanf("%d", &msg_buffer.game_token);
			printf("Enter your game id ");
			scanf("%d", &msg_buffer.game_id);
		}


		socklen_t tolen = sizeof srv_addr;
		int sent;
		if ((sent = sendto (cl_socket, &msg_buffer, sizeof msg_buffer, 0, (struct sockaddr*) &srv_addr, tolen)) < 0) {
			perror ("Error sending data to socket");
			exit (EXIT_FAILURE);
		}
		else printf("Sent %d bytes\n", sent);

	}

	

	// Close socket
	close_socket(cl_socket);

	exit (EXIT_SUCCESS);
}