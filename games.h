#ifndef GAMES_H
#define GAMES_H

#include "messages.h"
#include <deque>
#include <vector>
#include <netinet/in.h>

struct player_info {
	int token;
	char player_name[30];
	bool ready;
	bool finished;
	bool left;
	std::vector<struct card> cards;
	int socket;
	short turns_to_miss;
};

struct game_info {
	int game_token;
	int game_id;
	std::vector <struct player_info*> players;
	std::deque <struct card> played_cards;
	std::deque <struct card> deck;
	short turn;
	short color_request;
	short value_request;
	short turns_to_miss;
	short cards_to_pick;
	bool started;
};

struct game_info* new_game (int);

struct player_info* new_player (char*);

short player_join_game (struct player_info*, struct game_info*);

void deal_cards (struct game_info*);

bool validate_move (struct game_msg*, struct game_info*);

bool card_equals (struct card, struct card);

bool is_finished (struct game_info*);

std::deque <struct card> pick_n_cards (struct game_info*, short, short);

std::deque <struct card> update_game_state (struct move_msg*, struct game_info*);

#endif