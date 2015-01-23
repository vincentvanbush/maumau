#ifndef GAMES_H
#define GAMES_H

#include "messages.h"
#include <deque>
#include <vector>
#include <netinet/in.h>
#include <json/json.h>

struct card {
	unsigned value;
	unsigned color;
};

struct player_info {
	unsigned token;
	char player_name[30];
	bool ready;
	bool finished;
	bool left;
	std::vector<struct card> cards;
	int socket;
	short turns_to_miss;
};

struct game_info {
	unsigned game_token;
	unsigned game_id;
	std::map<int, struct player_info*> players;
	std::deque <struct card> played_cards;
	std::deque <struct card> deck;
	unsigned turn;
	unsigned color_request;
	unsigned value_request;
	unsigned turns_to_miss;
	unsigned cards_to_pick;
	unsigned request_ttl;
	bool started;
	bool player_connected[4];
	bool finished;
};

struct game_info* new_game (int);

struct player_info* new_player (char*);

short player_join_game (struct player_info*, struct game_info*);

void player_leave_game (struct player_info*, struct game_info*);

void deal_cards (struct game_info*);

bool validate_move (Json::Value, struct game_info*);

bool card_equals (struct card, struct card);

bool is_finished (struct game_info*);

std::deque <struct card> pick_n_cards (struct game_info*, short, short);

std::deque <struct card> update_game_state (Json::Value, struct game_info*);

short finished_players (struct game_info*);

short players_still_in_game (struct game_info*);

#endif
