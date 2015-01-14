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

	struct card top_card = deck -> front();
	deck -> pop_front();
	game -> played_cards.push_back (top_card);
}

struct card top_card (struct game_info* game) {
	return game -> deck.front();
}

bool validate_move (struct move_msg* move, struct game_info* game) {
	// One hell of a TODO :)
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
	} else game -> turns_to_miss = move -> turns_for_next;

	// Pick up n cards if move says so!
	short n = move -> cards_picked_up_count;
	if (n > 0) {
		picked_cards = pick_n_cards (game, n, player_turn);
		game -> cards_to_pick = 0;
	} else game -> cards_to_pick = move -> cards_for_next;

	// Transfer move's played cards to the front of the structure in game
	std::deque <struct card> &cards_in_game = game -> played_cards;
	for (int i = 0; i < move -> played_cards_count; i++) {
		cards_in_game.push_front(move -> played_cards[i]);
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