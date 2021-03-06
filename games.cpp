#include "games.h"
#include <cstdlib>
#include <algorithm>
#include <random>
#include <cstdio>
#include <cstring>
#include <deque>
#include <string>

struct game_info* new_game(int id) {
	struct game_info* ret = new struct game_info;

	ret->game_token = rand();
	ret->game_id = id;
	ret->turn = 0;
	ret->color_request = 0;
	ret->value_request = 0;
	ret->turns_to_miss = 0;
	ret->cards_to_pick = 0;
	ret->request_ttl = 0;
	for (int i = 0; i < 4; i++)
		ret->player_connected[i] = false;

	// shuffle the deck
	struct card card_array[52] = {
		{ KING, HEART }, { KING, TILE }, { KING, CLOVER }, { KING, PIKE },
		{ QUEEN, HEART }, { QUEEN, TILE }, { QUEEN, CLOVER }, { QUEEN, PIKE },
		{ ACE, HEART }, { ACE, TILE }, { ACE, CLOVER }, { ACE, PIKE },
		{ JACK, HEART }, { JACK, TILE }, { JACK, CLOVER }, { JACK, PIKE },
		{ 2, HEART }, { 2, TILE }, { 2, CLOVER }, { 2, PIKE },
		{ 3, HEART }, { 3, TILE }, { 3, CLOVER }, { 3, PIKE },
		{ 4, HEART }, { 4, TILE }, { 4, CLOVER }, { 4, PIKE },
		{ 5, HEART }, { 5, TILE }, { 5, CLOVER }, { 5, PIKE },
		{ 6, HEART }, { 6, TILE }, { 6, CLOVER }, { 6, PIKE },
		{ 7, HEART }, { 7, TILE }, { 7, CLOVER }, { 7, PIKE },
		{ 8, HEART }, { 8, TILE }, { 8, CLOVER }, { 8, PIKE },
		{ 9, HEART }, { 9, TILE }, { 9, CLOVER }, { 9, PIKE },
		{ 10, HEART }, { 10, TILE }, { 10, CLOVER }, { 10, PIKE }
	};

	std::deque <struct card> cards (card_array, card_array + 52);
	std::random_device rd;
    std::mt19937 g (rd());
    std::shuffle (cards.begin(), cards.end(), g);
    ret->deck = cards;
    ret->started = false;
    ret->finished = false;

	return ret;
}

struct player_info* new_player (char* player_name) {
	struct player_info* ret = new struct player_info;

	ret -> token = rand();
	strcpy (ret -> player_name, player_name);
	ret -> ready = false;
	ret -> left = false;
	ret -> finished = false;
	ret -> turns_to_miss = 0;

	return ret;
}

short player_join_game (struct player_info* player, struct game_info* game) {
	for (int i = 0; i < 4; i++) {
		if (!game->player_connected[i]) {
			game->players[i] = player;
			game->player_connected[i] = true;
			return i;
		}
	}
	return -1;
}

void player_leave_game (struct player_info* player, struct game_info* game) {
	for (int i = 0; i < 4; i++) {
		if (game->players[i] == player) {
			game->player_connected[i] = false;
			game->players.erase (i);
			return;
		}
	}
}

void deal_cards (struct game_info* game) {
	std::map <int, struct player_info*> players = game -> players;
	std::deque <struct card>* deck = &game -> deck;

	for (int i = 0; i < 5; i++) {
		for (unsigned j = 0; j < game -> players.size(); j++) {
			struct player_info* player = game -> players[j];
			struct card dealt_card = deck -> front();
			deck -> pop_front();
			player -> cards.push_back (dealt_card);
		}
	}

	struct card top = deck -> front();
	deck -> pop_front();
	game -> played_cards.push_back (top);
}

struct card top_card (struct game_info* game) {
	return game -> played_cards.front();
}

bool validate_move (Json::Value move_msg, struct game_info* game, std::string &cause) {
	int played_cards_count = move_msg["played_cards_count"].asInt();
	fprintf (stderr, "----------- (V) PLAYED CARDS COUNT: %d\n", played_cards_count);

	// if game is null
	if (game == nullptr) {
		cause = "Game is null";
		puts (cause.c_str());
		return false;
	}

	// if played cards count > 4, message is rubbish!
	if (move_msg["played_cards_count"] > 4) {
		cause = "Malformed message, cards count > 4";
		puts (cause.c_str());
		return false;
	}

	struct card top = top_card (game);
	std::vector <struct card> played_cards;
	for (int i = 0; i < move_msg["played_cards_count"].asInt(); i++) {
		struct card card;
		card.color = move_msg["played_cards"][i]["color"].asInt();
		card.value = move_msg["played_cards"][i]["value"].asInt();
		played_cards.push_back (card);
	}
	unsigned player_token = move_msg["player_token"].asInt();
	unsigned game_token = move_msg["game_token"].asInt();

	// Check if game is started
	if (!game -> started) {
		cause = "Game is not started";
		puts (cause.c_str());
		return false;
	}

	// Check game token
	if (game -> game_token != game_token) {
		cause = "Invalid game token";
		puts (cause.c_str());
		return false;
	}

	// Check if all played cards have the same value
	short value = 0;
	for (unsigned i = 0; i < played_cards.size(); i++) {
		short prev_value = value;
		value = played_cards[i].value;
		if (prev_value == 0) continue;
		if (prev_value != value) {
			cause = "Played cards have different values";
			puts (cause.c_str());
			return false;
		}
	}

	// Check if player's token equals the token of player at current turn slot
	if (player_token != game -> players[game -> turn] -> token) {
		printf ("Expected player token %d, got %d \n", game -> players[game -> turn] -> token, player_token);
		cause = "Wrong player token";
		puts (cause.c_str());
		return false;
	}

	// Check if first card played has the same color or value as top card...
	// or if first card conforms to the request
	if (played_cards.size() > 0) {

		// Check if the player actually does have the cards he sends

		struct player_info *player = game -> players[game -> turn];
		std::vector <struct card> &hand = player -> cards;

		for (unsigned i = 0; i < played_cards.size(); i++) {
			bool card_eq = false;
			for (unsigned j = 0; j < hand.size(); j++) {
				if (card_equals (hand[j], played_cards[i])) {
					card_eq = true;
					break;
				}
			}
			if (!card_eq) {
				cause = "Player tries to send card he doesn't have";
				puts (cause.c_str());
				return false;
			}
		}

		// No request: check top card
		if (game -> color_request == 0 && game -> value_request == 0 && game -> turns_to_miss == 0) {
			if (played_cards[0].color != top.color && played_cards[0].value != top.value) {
				cause = "Top card does not match first played card";
				puts (cause.c_str());
				return false;
			}
		}
		// Color request: check color or play an ace
		else if (game -> color_request != 0 && game -> color_request != played_cards[0].color) {
			if (played_cards[0].value != ACE) {
				cause = "There is a color request but the card is not right or an ace";
				puts (cause.c_str());
				return false;
			}
		}
		// Value request: check value or play a jack
		else if (game -> value_request != 0 && game -> value_request != played_cards[0].value) {
			if (played_cards[0].value != JACK) {
				cause = "There is a value request but the card is not right or a jack";
				puts (cause.c_str());
				return false;
			}
		}
		// If challenged to eat cards...
		else if (game -> cards_to_pick > 0) {
			// with a 2 or 3, one can respond with 2, 3 or king
			if (top.value <= 3 && played_cards[0].value != top.value && played_cards[0].color != top.color
				&& played_cards[0].value != 2 && played_cards[0].value != 3 && played_cards[0].value != KING) {
				cause = "There is a card pick challenge but no 2/3/K is played";
				puts (cause.c_str());
				return false;
			}
			// with a king, one can respond with ONLY ONE KING (pike or heart)
			else if (top.value == KING && played_cards.size() > 1 && played_cards[0].value != KING
				&& played_cards[0].color != HEART && played_cards[0].color != PIKE) {
				cause = "There is a card pick challenge with a king";
				puts (cause.c_str());
				return false;
			}
		}
		// If challenged to wait turns...
		else if (game -> turns_to_miss > 0) {
			// one can respond with 4s
			if (played_cards[0].value != 4) {
				cause = "There is a turn wait challenge but no 4 is played";
				puts (cause.c_str());
				return false;
			}
		}
		// If making a color request...
		else if (move_msg["color_request"] != 0) {
			// played card has to be ace
			if (played_cards[0].value != ACE) {
				cause = "Player requests a color but doesn't give an ace";
				puts (cause.c_str());
				return false;
			}
		}
		// If making a value request
		else if (move_msg["value_request"] != 0) {
			// played card has to be jack
			if (played_cards[0].value != JACK)
				cause = "Player requests a value but doesn't give a jack";
				puts (cause.c_str());
				return false;
		}
	}

	return true;
}

bool card_equals (struct card a, struct card b) {
	if (a.color == b.color && a.value == b.value)
		return true;
	return false;
}

std::deque <struct card> update_game_state(Json::Value move, struct game_info* game) {
	std::deque <struct card> picked_cards;

	// Get the turn of the move's player. Thus, the function should be executed
	// BEFORE the turn is incremented!
	short player_turn = game -> turn;
	struct player_info* player = game -> players[player_turn];

	if (game -> request_ttl > 0) game -> request_ttl -= 1;
	else {
		game -> value_request = 0;
		game -> color_request = 0;
	}

	int played_cards_count = move["played_cards_count"].asInt();
	fprintf (stderr, "----------- (U) PLAYED CARDS COUNT: %d\n", played_cards_count);


	// If no card played
	if (played_cards_count == 0) {
		short cards_to_pick = game -> cards_to_pick;
		short turns_to_miss = game -> turns_to_miss;
		short picked;

		if (player -> turns_to_miss > 0) {
			player -> turns_to_miss--;
		}
		else if (cards_to_pick == 0 && turns_to_miss == 0) {
			picked = 1;
		}
		else if (cards_to_pick != 0) {
			picked = cards_to_pick;
			game -> cards_to_pick = 0;
		}
		else if (turns_to_miss != 0) {
			player -> turns_to_miss = turns_to_miss;
			game -> turns_to_miss = 0;
		}
		picked_cards = pick_n_cards (game, picked, player_turn);
	}
	// If one or more cards played
	else {
		short value = move["played_cards"][0]["value"].asInt();
		short first_color = move["played_cards"][0]["color"].asInt();
		if (value == KING) { // eat 5 cards
			if (first_color == HEART) {
				game -> cards_to_pick += 5;
			}
			else if (first_color == PIKE) {
				game -> cards_to_pick += 5;
			}
		}
		else if (value == JACK) { // request value
			game -> value_request = move["value_request"].asInt();
			if (move["value_request"].asInt() != 0)
				game -> request_ttl = players_still_in_game (game);
		}
		else if (value == ACE) { // request color
			game -> color_request = move["color_request"].asInt();
			if (move["color_request"].asInt() != 0)
				game -> request_ttl = players_still_in_game (game);
		}
		else if (value == 4) { // miss a turn
			game -> turns_to_miss += played_cards_count;
		}
		else if (value == 3 || value == 2) { // eat 3 or 2 cards
			game -> cards_to_pick += value * played_cards_count;
		}
		else { // normal card

		}
	}


	// Increment the turn modulo 4. Repeat if the selected player has finished
	do
		if (move["played_cards_count"].asInt() > 0 && move["played_cards"][0]["value"].asInt() == KING
			&& move["played_cards"][0]["color"].asInt() == PIKE)
			(--game -> turn) %= game -> players.size();
		else
			(++game -> turn) %= game -> players.size();
	while (game -> players[game -> turn] -> finished);

	printf ("TRANSFERRING (player played %d cards) \n", played_cards_count);
	// Transfer move's played cards to the front of the structure in game
	std::deque <struct card> *cards_in_game = &game -> played_cards;
	std::deque <struct card> move_cards;
	for (int i = 0; i < played_cards_count; i++) {
		struct card card;
		card.color = move ["played_cards"][i]["color"].asInt();
		card.value = move ["played_cards"][i]["value"].asInt();
		printf ("Card %d %d \n", card.value, card.color);

		cards_in_game -> push_front(card);
		move_cards.push_back(card);
	}
	std::vector <struct card> &player_cards = player -> cards;

	for (unsigned i = 0; i < move_cards.size(); i++) {
		for (unsigned j = 0; j < player_cards.size(); j++) {
			if (card_equals (move_cards[i], player_cards[j])) {
				player_cards.erase(player_cards.begin() + j);
				break;
			}
		}
	}


	if (player -> cards.size() == 0) {
		player -> finished = true;
	}


	return picked_cards;
}

std::deque <struct card> pick_n_cards (struct game_info* game, short n, short player) {
	std::deque <struct card> ret;

	std::vector <struct card> &player_cards = game -> players[player] -> cards;
	std::deque <struct card> &deck = game -> deck;
	std::deque <struct card> &played_cards = game -> played_cards;
	for (int i = 0; i < n; i++) {
		if (deck.size() == 0) {
			deck = std::deque <struct card> (played_cards);
			played_cards.clear();
			played_cards.push_front(deck.front());
			deck.pop_front();
			std::random_device rd;
		    std::mt19937 g (rd());
		    std::shuffle (deck.begin(), deck.end(), g);
		}
		struct card card = deck.front();
		deck.pop_front();
		player_cards.push_back(card);
		ret.push_back(card);
	}

	return ret;
}

bool is_finished (struct game_info* game) {
	if (game -> players.size() == 0)
		return true;
	unsigned ret = finished_players (game);
	if (ret == game -> players.size() - 1)
		return true;
	return false;
}

short finished_players (struct game_info *game) {
	short ret = 0;
	for (unsigned i = 0; i < game -> players.size(); i++) {
		struct player_info* player = game -> players[i];
		if (!player -> finished) ++ret;
	}
	return game -> players.size() - ret;
}

short players_still_in_game (struct game_info *game) {
	return game -> players.size() - finished_players (game);
}
