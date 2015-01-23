#ifndef GAMES_H
#define GAMES_H

#include "messages.h"
#include <deque>
#include <vector>
#include <string>
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

// Creates a structure representing a new game.
struct game_info* new_game (int);

// Creates a structure representinga a new player with a given name.
struct player_info* new_player (char*);

// Joins the given player to a given game.
short player_join_game (struct player_info*, struct game_info*);

// Removes a given player from a given game.
void player_leave_game (struct player_info*, struct game_info*);

// Deals cards in a given game when it starts.
void deal_cards (struct game_info*);

// Validates a move given by a JSON message in the context of a given game.
// Additionally, prints out the cause of validation failure into a string variable.
bool validate_move (Json::Value, struct game_info*, std::string&);

// Checks if two given cards are equal to each other.
bool card_equals (struct card, struct card);

// Checks if a given game is finished, that is when all but one players have no cards left.
bool is_finished (struct game_info*);

// Transfer a given number of cards from the deck to a player at a given slot in a game.
std::deque <struct card> pick_n_cards (struct game_info*, short, short);

// Updates game state with the results of a given move.
// Should only be executed after the move is validated.
std::deque <struct card> update_game_state (Json::Value, struct game_info*);

// Returns the number of players in a given game who have already finished.
short finished_players (struct game_info*);

// Returns the number of players that are still in game (haven't finished).
short players_still_in_game (struct game_info*);

#endif
