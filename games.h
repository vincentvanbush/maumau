#ifndef GAMES_H
#define GAMES_H

#include "messages.h"
#include <stack>

struct player_info {
	int token;
	char player_name[30];
	bool ready;
	std::vector<struct card> cards;
};

struct game_info {
	int game_token;
	int game_id;
	struct player_info players[4];
	std::stack<struct card> played_cards;
	std::stack<struct card> deck;
	short turn;
	short color_request;
	short value_request;
	short turns_to_miss;
};

#endif