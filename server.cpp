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

std::vector<int> all_sockets;
pthread_mutex_t sockets_lock = PTHREAD_MUTEX_INITIALIZER;

int games_num = 0;

void broadcast_game_list (int rcv_sck = -1) {
	Json::Value game_list_msg;
	game_list_msg["msg_type"] = GAME_LIST;
	game_list_msg["games"] = Json::Value(Json::arrayValue);
	pthread_mutex_lock(&games_lock);
	for (int i = 0; i < MAX_GAME_NUM; i++) {
		if (games[i] != nullptr && !games[i] -> finished) {
			game_list_msg["games"][i]["id"] = games[i] -> game_id;
			game_list_msg["games"][i]["players_count"] = (int) games[i] -> players.size();
			for (int j = 0; j < 4; j++) {
				if (games[i] -> player_connected[j])
					game_list_msg["games"][i]["player_nicks"][j] = games[i] -> players[j] -> player_name;
				else
					game_list_msg["games"][i]["player_nicks"][j] = Json::Value(Json::nullValue);
			}
			game_list_msg["games"][i]["started"] = games[i] -> started;
		}
	}
	pthread_mutex_unlock(&games_lock);

	if (rcv_sck == -1) {
		pthread_mutex_lock (&sockets_lock);
		for (int i = 0; i < all_sockets.size(); i++) {
			send_message (all_sockets[i], game_list_msg);
		}
		pthread_mutex_unlock (&sockets_lock);		
	}
	else send_message (rcv_sck, game_list_msg);
	
}

void *client_loop(void *arg) {
	int rcv_sck = *(int*)arg;

	printf ("Starting client thread on socket %d\n", rcv_sck);
	struct game_msg msg_buffer;
	struct sockaddr_in cl_addr;

	int msg_len;


	while (recv (rcv_sck, &msg_len, sizeof msg_len, 0) > 0) {
		// int msg_type = msg_buffer.msg_type;
		// printf("(%d) ", msg_type);
		fprintf (stderr, "[Socket %d] Receiving message of length %d \n", rcv_sck, msg_len);
		Json::Value json_buf;
		json_buf = recv_message(rcv_sck, msg_len);

		int msg_type = json_buf["msg_type"].asInt();
		fprintf (stderr, "[Socket %d] Message type: %d \n", rcv_sck, msg_type);

		switch (msg_type) {

		case JOIN_GAME: {
			fprintf (stderr, "[Socket %d] Received join game message from %s\n", rcv_sck, json_buf["player_name"].asCString());
			struct game_info* game;

			int game_id = json_buf["game_id"].asInt();
			const char *player_name = json_buf["player_name"].asCString();

			if (game_id == -1) {
				// Game does not exist yet, we have to create it
				fprintf (stderr, "[Socket %d] Game %d does not exist yet\n", rcv_sck, game_id);
				if (games_num > MAX_GAME_NUM - 1 || game_id < 0) {
					Json::Value error_msg;
					error_msg["msg_type"] = CANNOT_JOIN;
					send_message (rcv_sck, error_msg);
				}
				int i;
				for (i = 0; i < MAX_GAME_NUM; i++) {
					if (games[i] == nullptr || games[i]->finished) break;
				}
				game_id = i;
				game = new_game(i);
				pthread_mutex_lock(&games_lock);
				games[i] = game;
				games_num++;
				pthread_mutex_unlock(&games_lock);
			}
			else if (games[game_id] == nullptr || games[game_id]->finished) {
				Json::Value error_msg;
				error_msg["msg_type"] = CANNOT_JOIN;
				send_message (rcv_sck, error_msg);
				break;
			}
			else {
				// Game exists. Let the player join or send him an error.
				game = games[game_id];

				printf("--- %s wants to join game %d\n", player_name, game_id);
				printf("Game id %d has token %d\n", game -> game_id, game -> game_token);
				if (game -> started || game -> players.size() >= 4) {
					puts("--- Cannot join this game");
					Json::Value cannot_join_msg;
					cannot_join_msg["msg_type"] = CANNOT_JOIN;
					send_message (rcv_sck, cannot_join_msg);
					break;
				}
			}

			struct player_info* player = new_player ((char*) player_name);
			pthread_mutex_lock(&games_lock);
			player_join_game (player, game);
			pthread_mutex_unlock(&games_lock);
			player -> socket = rcv_sck;

			Json::Value player_joined_msg;
			player_joined_msg["msg_type"] = PLAYER_JOINED;
			player_joined_msg["player_name"] = json_buf["player_name"].asString();
			player_joined_msg["slot_number"] = (int) game -> players.size() - 1;
			for (int i = 0; i < game -> players.size(); i++) {
				if (game -> players[i] -> socket != rcv_sck) {
					send_message (game -> players[i] -> socket, player_joined_msg);
				}
			}

			Json::Value join_ok_msg;
			join_ok_msg["msg_type"] = JOIN_OK;
			join_ok_msg["game_id"] = game_id;
			join_ok_msg["player_token"] = player -> token;
			join_ok_msg["game_token"] = game -> game_token;
			join_ok_msg["slot_number"] = (int) game -> players.size() - 1;
			join_ok_msg["player_names"] = Json::Value();
			for (int i = 0; i < game -> players.size(); i++) {
				join_ok_msg["player_names"][i] = Json::Value(game -> players[i] -> player_name);
			}

			puts("--- Join OK");
			send_message (rcv_sck, join_ok_msg);

			broadcast_game_list ();

		}
		break;

		case MOVE: {
			printf("Received move message\n");

			int player_token = json_buf["player_token"].asInt();
			int game_token = json_buf["game_token"].asInt();
			int game_id = json_buf["game_id"].asInt();

			struct game_info* game;
			bool valid_move;

			if (game_id < 0 || game_id > 49 || games[game_id] == nullptr
				|| games[game_id] -> finished || !games[game_id] -> started) {
				printf("Game id out of bounds or game does not exist\n");
				valid_move = false;
			}
			else {
				game = games[game_id];
				valid_move = validate_move (json_buf, games[game_id]);
			}

			if (valid_move) {
				// Update game state

				std::deque <struct card> cards_picked_up;
				short sender_turn = game -> turn;
				cards_picked_up = update_game_state (json_buf, games[game_id]);

				// Broadcast move to ALL players including sender
				for (int i = 0; i < game -> players.size(); i++) {
					send_message (game -> players[i] -> socket, json_buf);
				}

				// If sender has to pick up any cards, broadcast PICK_CARDS to all players.
				// For the sender only, augment the message with the cards he picks.
				if (cards_picked_up.size() > 0) for (int i = 0; i < game -> players.size(); i++) {
					Json::Value pick_cards_msg;

					pick_cards_msg["msg_type"] = PICK_CARDS;

					pick_cards_msg["slot"] = sender_turn;
					pick_cards_msg["count"] = (int) cards_picked_up.size();

					pick_cards_msg["cards"] = Json::Value();
					if (player_token == game -> players[sender_turn] -> token) {
						for (int i = 0; i < cards_picked_up.size(); i++) {
							pick_cards_msg["cards"][i]["value"] = cards_picked_up[i].value;
							pick_cards_msg["cards"][i]["color"] = cards_picked_up[i].color;
						}
					}

					send_message (game -> players[i] -> socket, pick_cards_msg);
				}

				// Send NEXT_TURN to all players
				for (int i = 0; i < game -> players.size(); i++) {
					Json::Value next_turn_msg;
					next_turn_msg["msg_type"] = NEXT_TURN;
					next_turn_msg["turns_for_next"] = game -> turns_to_miss;
					next_turn_msg["cards_for_next"] = game -> cards_to_pick;

					next_turn_msg["turn"] = game -> turn;
					send_message (game -> players[i] -> socket, next_turn_msg);
				}

				bool game_end = is_finished (game);
				if (game_end) {
					int game_id = json_buf["game_id"].asInt();
					game -> started = false;
					Json::Value game_end_msg;
					game_end_msg["msg_type"] = GAME_END;

					for (int i = 0; i < game -> players.size() && !game -> players[i] -> left; i++) {
						send_message (game -> players[i] -> socket, game_end_msg);
					}

					games[game_id] -> started = false;
					games[game_id] -> finished = true;
				}

			}
			else {
				Json::Value invalid_move_msg;
				invalid_move_msg["msg_type"] = INVALID_MOVE;
				send_message (rcv_sck, invalid_move_msg);
			}
		}
		break;

		case LEAVE_GAME: {
			printf("Received leave game message\n");
			int player_token = json_buf["player_token"].asInt();
			int game_token = json_buf["game_token"].asInt();
			int game_id = json_buf["game_id"].asInt();
			short slot = json_buf["slot"].asInt();

			bool invalid_message = false;
			struct game_info* game;

			pthread_mutex_lock(&games_lock);
			if (slot < 0 || slot > 3 || game_id < 0 || game_id > 49 || games[game_id] == nullptr
				|| games[game_id] -> finished || games[game_id] -> game_token != game_token) {
				puts ("Checkpoint 1");
				printf("No such game or invalid slot\n");
				invalid_message = true;
			}
			else {
				game = games[game_id];
				if (game -> players[slot] -> token != player_token) { // wrong turn
					printf("Slot number doesn't match player token at given slot\n");
					invalid_message = true;
				}
				puts ("Checkpoint 2");
			}

			if (invalid_message) {
				puts ("Invalid leave message");
			}
			else {
				struct player_info *player = game -> players[slot];
				puts ("Checkpoint 4");
				player_leave_game (player, game);
				puts ("Checkpoint 5");
				if (game -> players.size() > 0) { // only if any player is in game
					// Broadcast message
					Json::Value player_left_msg;
					player_left_msg["msg_type"] = PLAYER_LEFT;
					player_left_msg["slot"] = slot;
				
					for (std::map<int, struct player_info*>::iterator it = game -> players.begin();
							it != game -> players.end();
							it++) {
						if ((*it).second -> token != player_token) {
							puts ("Sending player left message");
							send_message((*it).second -> socket, player_left_msg);
						}

					}
					puts ("Checkpoint 6");

					if (game -> started && !player -> finished) {
						puts ("Checkpoint 7");
						Json::Value game_end_msg;
						game_end_msg["msg_type"] = GAME_END;
						game_end_msg["game_token"] = game_token;
						game_end_msg["game_id"] = game_id;

						for (std::map<int, struct player_info*>::iterator it = game -> players.begin();
							it != game -> players.end();
							it++) {
							puts ("Checkpoint 8");
							if ((*it).second -> token != player_token)
								send_message((*it).second -> socket, game_end_msg);
							
						}

						games[game_id] -> started = false;
						games[game_id] -> finished = true;

					}
				}
				else {
					games[game_id] -> started = false;
					games[game_id] -> finished = true;
				}
				puts ("Checkpoint 9");

			}
			pthread_mutex_unlock(&games_lock);

			broadcast_game_list();

		}
		break;

		case READY: {

			printf("Received ready message\n");
			int game_id = json_buf["game_id"].asInt();
			printf ("Game id = %hu\n", game_id);

			int player_token = json_buf["player_token"].asInt();
			int game_token = json_buf["game_token"].asInt();

			//struct ready_msg ready_msg = msg_buffer.message.ready;
			bool invalid_player_token;
			struct game_info* game;
			short player_index = -1;

			if (game_id < 0 || game_id > MAX_GAME_NUM - 1) {
				invalid_player_token = true;
			}
			else {
				game = games[game_id];
				// check if received player token is valid
				invalid_player_token = true;

				if (game == nullptr || game -> finished || game -> game_token != game_token) {
					// if received game token is different to the actual one, send error
					invalid_player_token = true;
				}
				else for (int i = 0; i < game -> players.size(); i++) {
					if (game -> players[i] -> token == player_token) {
						invalid_player_token = false;
						player_index = i;
					}
				}
			}

			if (invalid_player_token) {
				// send error message to client
				Json::Value cannot_ready_msg;
				cannot_ready_msg["msg_type"] = CANNOT_READY;
				send_message (rcv_sck, cannot_ready_msg);
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
					game -> started = true;
					pthread_mutex_unlock(&games_lock);

					// send 'Start game' to all players in game
					for (int i = 0; i < game -> players.size(); i++) {
						int player_sck = game -> players[i] -> socket;
						struct player_info* player = game -> players[i];

						Json::Value start_game_msg;
						start_game_msg["msg_type"] = START_GAME;

						for (int j = 0; j < player -> cards.size(); j++) {
							start_game_msg["player_cards"][j]["value"] = player -> cards[j].value;
							start_game_msg["player_cards"][j]["color"] = player -> cards[j].color;
						}

						start_game_msg["first_card_in_stack"]["value"] = game -> played_cards.front().value;
						start_game_msg["first_card_in_stack"]["color"] = game -> played_cards.front().color;


						start_game_msg["turn"] = 0;

						send_message (player_sck, start_game_msg);

						broadcast_game_list ();

					}

				}
			}


		}
		break;

		case REQUEST_GAME_LIST: {
			printf("Received request game list message\n");

			broadcast_game_list (rcv_sck);
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

		pthread_mutex_lock (&sockets_lock);
		all_sockets.push_back (rcv_sck);
		pthread_mutex_unlock (&sockets_lock);

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

	while (true) {
		int id;
		printf ("Enter game id to show state ");
		scanf ("%d", &id);

		if (games[id] == nullptr || games[id] -> finished) {
			puts("Game doesn't exist.");
			continue;
		}

		pthread_mutex_lock(&games_lock);
		printf ("Game %d\n", id);
		for (int i = 0; i < games[id] -> players.size(); i++) {
			printf("%d: %s ", i, games[id] -> players[i] -> player_name);
			printf("Cards: ");
			std::vector <struct card> &cards = games[id] -> players[i] -> cards;
			for (int j = 0; j < cards.size(); j++) {
				printf("%d %d\t", cards[j].value, cards[j].color);
			}
			printf("\n");
		}

		printf("Cards on table:\n");
		for (int i = 0; i < games[id] -> played_cards.size(); i++) {
			printf("%d %d, ", games[id] -> played_cards[i].value, games[id] -> played_cards[i].color);
		}

		printf("Deck:\n");
		for (int i = 0; i < games[id] -> deck.size(); i++) {
			printf("%d %d, ", games[id] -> deck[i].value, games[id] -> deck[i].color);
		}
		printf("\n\n");

		printf("Turn: %d\n", games[id] -> turn);
		printf("Request TTL: %d\n", games[id] -> request_ttl);
		printf("Color request: %d\n", games[id] -> color_request);
		printf("Value request: %d\n", games[id] -> value_request);
		printf("Turns to miss: %d\n", games[id] -> turns_to_miss);
		printf("Cards to pick: %d\n", games[id] -> cards_to_pick);
		printf("Players still in game: %d, finished players: %d\n", players_still_in_game(games[id]), finished_players(games[id]));
		pthread_mutex_unlock(&games_lock);

		printf("\n\n");
	}

	printf ("Press ENTER to exit server\n");
	getc (stdin);

	exit (EXIT_SUCCESS);
}
