#include "games.h"
#include <cstdlib>
#include <algorithm>
#include <random>
#include <cstdio>
#include <cstring>

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

	return ret;
}

void player_join_game (struct player_info* player, struct game_info* game) {
	game->players.push_back(player);
}

void deal_cards (struct game_info* game) {

}