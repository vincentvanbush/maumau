#ifndef MESSAGES_H
#define MESSAGES_H

#define JOIN_MESSAGE 100

#define KING 200
#define QUEEN 201
#define ACE 202
#define JACK 203

#define HEART 300
#define TILE 301
#define CLOVER 302
#define PIKE 303

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

struct game_msg {
	int msg_type;
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
	} message;
};

#endif