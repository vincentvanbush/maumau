#ifndef MESSAGES_H
#define MESSAGES_H

const short JOIN = 10;
const short WAITING_FOR_OPPONENT = 11;
const short START_GAME = 12;
const short GAME_FULL = 13;
const short MOVE = 14;
const short OPPONENT_TURN = 15;
const short INVALID_MOVE = 16;
const short GAME_END = 17;
const short OPPONENT_LEFT = 18;

const short KING = 20;
const short QUEEN = 21;
const short ACE = 22;
const short JACK = 23;

const short HEART = 30;
const short TILE = 31;
const short CLOVER = 32;
const short PIKE = 33;

struct card {
	short value;
	short color;
};

struct join_game_msg {
	char player_name[30];
};

struct waiting_for_opponent_msg {
	short dummy;
};

struct start_game_msg {
	struct card player_cards[52];
	card first_card_in_stack;
};

struct game_full_msg {
	short dummy;
};

struct move_msg {
	struct card played_cards[4];
	short color_request;
	short value_request;
	short cards_to_pick_up;
	short turns_to_miss;
	struct card cards_picked_up[20];
};

struct opponent_turn_msg {
	struct card cards_picked_up[20];
};

struct invalid_move_msg {
	short dummy;
};

struct game_end_msg {
	char winner_name[30];
};

struct opponent_left_msg {
	short dummy;
};

struct leave_game_msg {
	short dummy;
};

struct game_msg {
	short msg_type;
	union msg {
		struct join_game_msg join_game;
		struct waiting_for_opponent_msg waiting_for_opponent;
		struct start_game_msg start_game;
		struct game_full_msg game_full;
		struct move_msg move;
		struct opponent_turn_msg opponent_turn;
		struct invalid_move_msg invalid_move;
		struct game_end_msg game_end;
		struct opponent_left_msg opponent_left;
		struct leave_game_msg leave_game;
	} message;
};

#endif