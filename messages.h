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
const short CANNOT_READY = 14;
const short CANNOT_LEAVE = 15;

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
	short played_cards_count;
	struct card played_cards[4];

	short color_request;
	short value_request;

	short cards_for_next;
	short turns_for_next;

	short cards_picked_up_count;
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
	short slot;
};

struct request_game_list_msg {
	short dummy;
};

// Sent by server

struct cannot_join_msg {
	short dummy;
};

struct cannot_ready_msg {
	short dummy;
};

struct cannot_leave_msg {
	short dummy;
};

struct join_ok_msg {
	short slot_number;
	int player_token;
	int game_token;
	char player_name[4][30];
	bool slot_taken[4];
};

struct player_joined_msg {
	char player_name[30];
	short slot_number;
};

struct start_game_msg {
	struct card player_cards[52];
	struct card first_card_in_stack;
	short turn;
};

struct next_turn_msg {
	short turn;
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
	short slot;
};

struct game_list_msg {
	bool game_exists[50];
	int game_id[50];
	short players_count[50];
	char player_nick[50][4][30];
	bool started[50];
};

struct game_msg {
	short msg_type;
	int token;
	int game_token;
	int game_id;
	union msg {
		struct join_game_msg		join_game;
		struct ready_msg			ready;
		struct leave_game_msg		leave_game;
		struct cannot_join_msg		cannot_join;
		struct cannot_ready_msg		cannot_ready;
		struct cannot_leave_msg		cannot_leave;
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