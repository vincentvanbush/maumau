#include "games.h"
#include <cstdlib>
#include <algorithm>
#include <random>
#include <cstdio>
#include <cstring>
#include <deque>

struct game_info* new_game(int id) {
	struct game_info* ret = new struct game_info;

	ret->game_token = rand();
	ret->game_id = id;
	ret->turn = 0;

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

	return ret;
}

struct player_info* new_player (char* player_name) {
	struct player_info* ret = new struct player_info;

	ret -> token = rand();
	strcpy (ret -> player_name, player_name);
	ret -> ready = false;
	ret -> left = false;
	ret -> turns_to_miss = 0;

	return ret;
}

short player_join_game (struct player_info* player, struct game_info* game) {
	game->players.push_back(player);
	return (short) game->players.size() - 1;
}

void deal_cards (struct game_info* game) {
	std::vector <struct player_info*> players = game -> players;
	std::deque <struct card>* deck = &game -> deck;

	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < game -> players.size(); j++) {
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

bool validate_move (struct game_msg* move_msg, struct game_info* game) {
	// if played cards count > 4, message is rubbish!
	if (move_msg -> message.move.played_cards_count > 4) {
		puts ("Malformed message, cards count > 4");
		return false;
	}

	struct card top = top_card (game);
	struct move_msg &move = move_msg -> message.move;
	std::vector <struct card> played_cards (move.played_cards, move.played_cards + move.played_cards_count);
	int player_token = move_msg -> token;
	int game_token = move_msg -> game_token;

	// Check game token
	if (game -> game_token != game_token) {
		puts ("Invalid game token");
		return false;
	}

	// Check if all played cards have the same value
	short value = 0;
	for (int i = 0; i < played_cards.size(); i++) {
		short prev_value = value;
		value = played_cards[i].value;
		if (prev_value == 0) continue;
		if (prev_value != value) {
			puts ("Played cards have different values");
			return false;
		}
	}

	// Check if player's token equals the token of player at current turn slot
	if (player_token != game -> players[game -> turn] -> token) {
		puts ("Wrong player token");
		return false;
	}

	// Check if first card played has the same color or value as top card...
	// or if first card conforms to the request
	if (played_cards.size() > 0) {

		// Check if the player actually does have the cards he sends

		struct player_info *player = game -> players[game -> turn];
		std::vector <struct card> &hand = player -> cards;
		
		for (int i = 0; i < played_cards.size(); i++) {
			bool card_eq = false;
			for (int j = 0; j < hand.size(); j++) {
				if (card_equals (hand[j], played_cards[i])) {
					card_eq = true;
					break;
				}
			}
			if (!card_eq) {
				puts ("Player tries to send card he doesn't have");
				return false;
			}
		}

		// No request: check top card
		if (game -> color_request == 0 && game -> value_request == 0 && game -> turns_to_miss == 0) {
			if (played_cards[0].color != top.color && played_cards[0].value != top.value) {
				puts ("Top card does not match first played card");
				return false;
			}
		}
		// Color request: check color or play an ace
		else if (game -> color_request != 0 && game -> color_request != played_cards[0].color) {
			if (played_cards[0].value != ACE) {
				puts ("There is a color request but the card is not right or an ace");
				return false;
			}
		}
		// Value request: check value or play a jack
		else if (game -> value_request != 0 && game -> value_request != played_cards[0].value) {
			if (played_cards[0].value != JACK) {
				puts ("There is a value request but the card is not right or a jack");
				return false;
			}
		}
		// If challenged to eat cards...
		else if (game -> cards_to_pick > 0) {
			// with a 2 or 3, one can respond with 2, 3 or king	
			if (top.value <= 3 && played_cards[0].value != top.value && played_cards[0].color != top.color
				&& played_cards[0].value != 2 && played_cards[0].value != 3 && played_cards[0].value != KING) {
				puts ("There is a card pick challenge but no 2/3/K is played");
				return false;
			}
			// with a king, one can respond with ONLY ONE KING (pike or heart)
			else if (top.value == KING && played_cards.size() > 1 && played_cards[0].value != KING
				&& played_cards[0].color != HEART && played_cards[0].color != PIKE) {
				puts ("There is a card pick challenge with a king");
				return false;
			}
		}
		// If challenged to wait turns...
		else if (game -> turns_to_miss > 0) {
			// one can respond with 4s
			if (played_cards[0].value != 4) {
				puts ("There is a turn wait challenge but no 4 is played");
				return false;
			}
		}
		// If making a color request...
		else if (move.color_request != 0) {
			// played card has to be ace
			if (played_cards[0].value != ACE) {
				puts ("Player requests a color but doesn't give an ace");
				return false;
			}
		}
		// If making a value request
		else if (move.value_request != 0) {
			// played card has to be jack
			if (played_cards[0].value != JACK)
				puts ("Player requests a value but doesn't give a jack");
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

std::deque <struct card> update_game_state(struct move_msg* move, struct game_info* game) {
	std::deque <struct card> picked_cards;

	// Get the turn of the move's player. Thus, the function should be executed
	// BEFORE the turn is incremented!
	short player_turn = game -> turn;
	struct player_info* player = game -> players[player_turn];

	// If the player has to miss a turn, decrement the counter and don't change anything
	if (player -> turns_to_miss > 0) {
		player -> turns_to_miss--;
		return picked_cards;
	}

	// if game turns to miss > 0 and no card is played, the player misses n turns
	if (game -> turns_to_miss > 0 && move -> played_cards_count == 0) {
		player -> turns_to_miss = game -> turns_to_miss;
		game -> turns_to_miss = 0;
	}

	else { // if game turns to miss > 0 and 4's are played, add the number of played cards
		game -> turns_to_miss = move -> played_cards_count;
	}

	// Pick up n cards if move says so!
	short n = game -> cards_to_pick;
	if (n > 0) { // if has to pick up cards
		if (move -> played_cards_count == 0) {
			picked_cards = pick_n_cards (game, n, player_turn);
			game -> cards_to_pick = 0;
		}
		else if (move -> played_cards[0].value == 2) {
			game -> cards_to_pick += 2 * move -> played_cards_count;
		}
		else if (move -> played_cards[0].value == 3) {
			game -> cards_to_pick += 3 * move -> played_cards_count;
		}
		else if (move -> played_cards[0].value == KING) {
			game -> cards_to_pick += 5;
		}
	}

	// Transfer move's played cards to the front of the structure in game
	std::deque <struct card> *cards_in_game = &game -> played_cards;
	for (int i = 0; i < move -> played_cards_count; i++) {
		cards_in_game -> push_front(move -> played_cards[i]);
	}
	std::vector <struct card> &player_cards = player -> cards;
	std::deque <struct card> move_cards(move -> played_cards, move -> played_cards + move -> played_cards_count);
	for (int i = 0; i < move_cards.size(); i++) {
		for (int j = 0; j < player_cards.size(); j++) {
			if (card_equals (move_cards[i], player_cards[j])) {
				player_cards.erase(player_cards.begin() + j);
				break;
			}
		}
	}
	// Update color/value requests and turns to sit
	game -> color_request = move -> color_request;
	game -> value_request = move -> value_request;
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
	bool game_end = true;
	for (int i = 0; i < game -> players.size(); i++) {
		struct player_info* player = game -> players[i];
		if (!player -> finished) {
			game_end = false;
			break;
		}
	}
}