#ifndef MESSAGES_H
#define MESSAGES_H

const short JOIN_GAME = 0;
const short READY = 1;
const short LEAVE_GAME = 2;
const short CANNOT_JOIN = 3;
const short JOIN_OK = 4;
const short PLAYER_JOINED = 5;
const short START_GAME = 6;
const short NEXT_TURN = 7;
const short INVALID_MOVE = 8;
const short GAME_END = 9;
const short PLAYER_LEFT = 10;
const short MOVE = 11;
const short GAME_LIST = 12;
const short REQUEST_GAME_LIST = 13;

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

// Sent by both client and server

struct move_msg {
	struct card played_cards[4];
	short color_request;
	short value_request;
	short cards_to_pick_up;
	short turns_to_miss;
	struct card cards_picked_up[20];
};

// Sent by client

struct join_game_msg {
	char player_name[30];
	int game_id;
};

struct ready_msg {
	short dummy;
};

struct leave_game_msg {
	short dummy;
};

struct request_game_list_msg {
	short dummy;
};

// Sent by server

struct cannot_join_msg {
	short dummy;
};

struct join_ok_msg {
	short slot_number;
	int player_token;
	int game_token;
};

struct player_joined_msg {
	char player_name[30];
	short slot_number;
};

struct start_game_msg {
	struct card player_cards[52];
	card first_card_in_stack;
};

struct next_turn_msg {
	char player_name[30];
	short cards_picked_up;
	struct card cards[30];
};

struct invalid_move_msg {
	short dummy;
};

struct game_end_msg {
	short dummy;
};

struct player_left_msg {
	char player_name[30];
};

struct game_list_msg {
	int game_id[50];
	char player_nick[30][50];
	bool started[50];
};

struct game_msg {
	short msg_type;
	int token;
	int game_token;
	union msg {
		struct join_game_msg		join_game;
		struct ready_msg			ready;
		struct leave_game_msg		leave_game;
		struct cannot_join_msg		cannot_join;
		struct join_ok_msg			join_ok;
		struct player_joined_msg	player_joined;
		struct start_game_msg		start_game;
		struct next_turn_msg		next_turn;
		struct invalid_move_msg		invalid_move;
		struct game_end_msg			game_end;
		struct player_left_msg		player_left;
		struct move_msg 			move;
		struct game_list_msg		game_list;
		struct request_game_list_msg request_game_list;
	} message;
};

#endif