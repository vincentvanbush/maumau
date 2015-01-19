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

int game_token, player_token, game_id, slot_number;

void *recv_loop(void *arg) {
	int msg_len, msg_type;
	Json::Value msg_buffer;

	while (recv (cl_socket, &msg_len, sizeof msg_len, 0) > 0) {
		fprintf (stderr, "[Socket %d] Receiving message of length %d \n", cl_socket, msg_len);
		msg_buffer = recv_message(cl_socket, msg_len);


		if (msg_buffer["msg_type"] == GAME_LIST) {
			printf("Game list...\n");

			for (int i = 0; i < msg_buffer["games"].size(); i++) {
				if (!msg_buffer["games"][i].isNull()) {
					printf ("id = %d \t active = %s \t players_count = %d \t ",
						msg_buffer["games"][i]["id"].asInt(),
						msg_buffer["games"][i]["started"].asBool() ? "yes" : "no",
						msg_buffer["games"][i]["players_count"].asInt()
					);
					for (int j = 0; j < msg_buffer["games"][i]["players_count"].asInt(); j++) {
						printf("%s ", msg_buffer["games"][i]["player_nicks"][j].asCString());
					}
					printf ("\n");
				}

			}

		}


		if (msg_buffer["msg_type"] == JOIN_OK) {
			player_token = msg_buffer["player_token"].asInt();
			game_token = msg_buffer["game_token"].asInt();
			slot_number = msg_buffer["slot_number"].asInt();
			printf("Player token: %d\nGame token: %d\nSlot: %d\n", player_token, game_token, slot_number);
		}

		else if (msg_buffer["msg_type"] == START_GAME) {
			Json::Value cards_node = msg_buffer["player_cards"];
			std::deque <struct card> cards;
			for (int i = 0; i < cards_node.size(); i++) {
				Json::Value card_node = cards_node[i];
				struct card card;
				card.color = card_node["color"].asInt();
				card.value = card_node["value"].asInt();
				cards.push_back(card);
			}

			printf("Your cards:\n");
			for (int i = 0; i < cards.size(); i++) {
				printf("\t%d %d\n", cards[i].value, cards[i].color);
			}
			printf("Top card:\n");
			printf("\t%d %d\n", msg_buffer["first_card_in_stack"]["value"].asInt(), msg_buffer["first_card_in_stack"]["color"].asInt());
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
		Json::Value msg_buffer;
		printf("Enter message type: ");
		int msg_type;
		scanf("%d", &msg_type);
		msg_buffer["msg_type"] = msg_type;

		if (msg_buffer["msg_type"] == JOIN_GAME) {
			char player_name[30];
			int game_id;
			printf("Enter player name ");
			scanf("%s", player_name);
			printf("Enter game id ");
			scanf("%d", &game_id);
			msg_buffer["player_name"] = player_name;
			msg_buffer["game_id"] = game_id;
		}



		else if (msg_buffer["msg_type"] == READY) {
			msg_buffer["player_token"] = player_token;
			msg_buffer["game_token"] = game_token;
			msg_buffer["game_id"] = game_id;
		}


		else if (msg_buffer["msg_type"] == LEAVE_GAME) {
			msg_buffer["player_token"] = player_token;
			msg_buffer["game_token"] = game_token;
			msg_buffer["game_id"] = game_id;
			msg_buffer["slot"] = slot_number;
		}



		else if (msg_buffer["msg_type"] == MOVE) {
			msg_buffer["player_token"] = player_token;
			msg_buffer["game_token"] = game_token;
			msg_buffer["game_id"] = game_id;

			int played_cards_count;
			printf("Enter number of played cards ");
			scanf("%d", &played_cards_count);
			msg_buffer["played_cards_count"] = played_cards_count;

			for (int i = 0; i < played_cards_count; i++) {
				printf("Enter card %d value and color ", i);
				int value, color;
				scanf("%d %d",&value, &color);
				msg_buffer["played_cards"][i]["value"] = value;
				msg_buffer["played_cards"][i]["color"] = color;

			}

			int color_request, value_request;
			printf("Enter color request ");
			scanf("%d", &color_request);
			printf("Enter value request ");
			scanf("%d", &value_request);
			msg_buffer["color_request"] = color_request;
			msg_buffer["value_request"] = value_request;

		}


		send_message (cl_socket, msg_buffer);

	}



	// Close socket
	close_socket(cl_socket);

	exit (EXIT_SUCCESS);
}
