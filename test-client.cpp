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

#include "utils.h"
#include "messages.h"

const int MAX_GAME_NUM = 50;

ushort client_port = 0;
ushort server_port = 1234;

int main(int argc, char* argv[]) {
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
	

	// Create client address info
	struct sockaddr_in cl_addr = local_address(client_port);

	// Initialize UDP socket
	int cl_socket = create_socket();

	// Bind UDP socket to client address
	bind_socket(cl_socket, cl_addr);

	struct sockaddr_in srv_addr = local_address(server_port);
	inet_aton("127.0.0.1", &srv_addr.sin_addr);

	socklen_t tolen = sizeof srv_addr;
	int sent;
	if ((sent = sendto (cl_socket, &msg_buffer, sizeof msg_buffer, 0, (struct sockaddr*) &srv_addr, tolen)) < 0) {
		perror ("Error sending data to socket");
		exit (EXIT_FAILURE);
	}
	else printf("Sent %d bytes\n", sent);

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


	// Close socket
	close_socket(cl_socket);

	exit (EXIT_SUCCESS);
}