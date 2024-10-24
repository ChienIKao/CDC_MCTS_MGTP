#ifndef LIBCHESS_H
#define LIBCHESS_H

#include <array>
#include <string.h>

static const int BOARD_SIZE = 32;
static const int ROW_COUNT = 8;
static const int COL_COUNT = 4;

static const char finEN[] = "KkGgMmRrNnCcPpX-";

/// Move turn
enum COLOR : int {
	RED,
	BLK,
	UNKNOWN,
};

/// Structure of a move
/// source square 10 ~ 6 bit | destination squrare 5 ~ 1 bit
enum MOVE : int {
	MOVE_NULL = 1024,
};

/// Piece type
enum FIN : int {
	FIN_K = 0,
	FIN_k = 1,
	FIN_G = 2,
	FIN_g = 3,
	FIN_M = 4,
	FIN_m = 5,
	FIN_R = 6,
	FIN_r = 7,
	FIN_N = 8,
	FIN_n = 9,
	FIN_C = 10,
	FIN_c = 11,
	FIN_P = 12,
	FIN_p = 13,
	FIN_COVER = 14,
	FIN_EMPTY = 15,

	FIN_COUNT = 16,
};

inline COLOR color_of(FIN f) {
	if (f == FIN_COVER || f == FIN_EMPTY) {
		return UNKNOWN;
	}
	return COLOR(f % 2);
}

inline int from_square(MOVE m) { return m >> 5; }

inline int to_square(MOVE m) { return m & 0x1F; }

inline MOVE make_move(int from, int to) { return MOVE((from << 5) | to); }

inline std::string to_string(MOVE m) {
	if (m == MOVE_NULL) {
		return "a0 a0"; /// Resign move
	}
	int from = from_square(m), to = to_square(m);
	return std::string() + char('a' + from / ROW_COUNT) +
	       char('1' + from % ROW_COUNT) + " " + char('a' + to / ROW_COUNT) +
	       char('1' + to % ROW_COUNT);
}

inline int string2square(const char *str) {
	return (str[0] - 'a') * ROW_COUNT + str[1] - '1';
}

inline FIN char2fin(char c) {
	for (int i = 0; i < FIN_COUNT; i++) {
		if (c == finEN[i]) {
			return FIN(i);
		}
	}
	return FIN_COUNT;
}

inline FIN type_of(FIN f) { return FIN(f & 0xE); }

inline bool can_capture(FIN attacker, FIN victim) {
	if (attacker == FIN_COVER || attacker == FIN_EMPTY || victim == FIN_COVER) {
		return false;
	}

	if (victim == FIN_EMPTY) {
		return true;
	}

	if (color_of(attacker) == color_of(victim)) {
		return false;
	}

	attacker = type_of(attacker);
	victim = type_of(victim);

	switch (attacker) {
		case FIN_K:
			return victim != FIN_P;
		case FIN_G:
			return victim != FIN_K;
		case FIN_M:
			return victim != FIN_K && victim != FIN_G;
		case FIN_R:
			return victim != FIN_K && victim != FIN_G && victim != FIN_M;
		case FIN_N:
			return victim == FIN_N || victim == FIN_C || victim == FIN_P;
		case FIN_C:
			return false;
		case FIN_P:
			return victim == FIN_K || victim == FIN_P;
		default:
			return false;
	}
}

#define ACTION_SIZE 352
#define NO_EAT_FLIP_LIMIT 180
#define LONG_CATCH_LIMIT 3
const std::array<std::pair<int, int>, 352> ActionMap = {
    {{0, 0},   {0, 1},   {0, 2},   {0, 3},   {0, 4},   {0, 5},   {0, 6},
     {0, 7},   {0, 8},   {0, 16},  {0, 24},  {1, 0},   {1, 1},   {1, 2},
     {1, 3},   {1, 4},   {1, 5},   {1, 6},   {1, 7},   {1, 9},   {1, 17},
     {1, 25},  {2, 0},   {2, 1},   {2, 2},   {2, 3},   {2, 4},   {2, 5},
     {2, 6},   {2, 7},   {2, 10},  {2, 18},  {2, 26},  {3, 0},   {3, 1},
     {3, 2},   {3, 3},   {3, 4},   {3, 5},   {3, 6},   {3, 7},   {3, 11},
     {3, 19},  {3, 27},  {4, 0},   {4, 1},   {4, 2},   {4, 3},   {4, 4},
     {4, 5},   {4, 6},   {4, 7},   {4, 12},  {4, 20},  {4, 28},  {5, 0},
     {5, 1},   {5, 2},   {5, 3},   {5, 4},   {5, 5},   {5, 6},   {5, 7},
     {5, 13},  {5, 21},  {5, 29},  {6, 0},   {6, 1},   {6, 2},   {6, 3},
     {6, 4},   {6, 5},   {6, 6},   {6, 7},   {6, 14},  {6, 22},  {6, 30},
     {7, 0},   {7, 1},   {7, 2},   {7, 3},   {7, 4},   {7, 5},   {7, 6},
     {7, 7},   {7, 15},  {7, 23},  {7, 31},  {8, 8},   {8, 9},   {8, 10},
     {8, 11},  {8, 12},  {8, 13},  {8, 14},  {8, 15},  {8, 0},   {8, 16},
     {8, 24},  {9, 8},   {9, 9},   {9, 10},  {9, 11},  {9, 12},  {9, 13},
     {9, 14},  {9, 15},  {9, 1},   {9, 17},  {9, 25},  {10, 8},  {10, 9},
     {10, 10}, {10, 11}, {10, 12}, {10, 13}, {10, 14}, {10, 15}, {10, 2},
     {10, 18}, {10, 26}, {11, 8},  {11, 9},  {11, 10}, {11, 11}, {11, 12},
     {11, 13}, {11, 14}, {11, 15}, {11, 3},  {11, 19}, {11, 27}, {12, 8},
     {12, 9},  {12, 10}, {12, 11}, {12, 12}, {12, 13}, {12, 14}, {12, 15},
     {12, 4},  {12, 20}, {12, 28}, {13, 8},  {13, 9},  {13, 10}, {13, 11},
     {13, 12}, {13, 13}, {13, 14}, {13, 15}, {13, 5},  {13, 21}, {13, 29},
     {14, 8},  {14, 9},  {14, 10}, {14, 11}, {14, 12}, {14, 13}, {14, 14},
     {14, 15}, {14, 6},  {14, 22}, {14, 30}, {15, 8},  {15, 9},  {15, 10},
     {15, 11}, {15, 12}, {15, 13}, {15, 14}, {15, 15}, {15, 7},  {15, 23},
     {15, 31}, {16, 16}, {16, 17}, {16, 18}, {16, 19}, {16, 20}, {16, 21},
     {16, 22}, {16, 23}, {16, 0},  {16, 8},  {16, 24}, {17, 16}, {17, 17},
     {17, 18}, {17, 19}, {17, 20}, {17, 21}, {17, 22}, {17, 23}, {17, 1},
     {17, 9},  {17, 25}, {18, 16}, {18, 17}, {18, 18}, {18, 19}, {18, 20},
     {18, 21}, {18, 22}, {18, 23}, {18, 2},  {18, 10}, {18, 26}, {19, 16},
     {19, 17}, {19, 18}, {19, 19}, {19, 20}, {19, 21}, {19, 22}, {19, 23},
     {19, 3},  {19, 11}, {19, 27}, {20, 16}, {20, 17}, {20, 18}, {20, 19},
     {20, 20}, {20, 21}, {20, 22}, {20, 23}, {20, 4},  {20, 12}, {20, 28},
     {21, 16}, {21, 17}, {21, 18}, {21, 19}, {21, 20}, {21, 21}, {21, 22},
     {21, 23}, {21, 5},  {21, 13}, {21, 29}, {22, 16}, {22, 17}, {22, 18},
     {22, 19}, {22, 20}, {22, 21}, {22, 22}, {22, 23}, {22, 6},  {22, 14},
     {22, 30}, {23, 16}, {23, 17}, {23, 18}, {23, 19}, {23, 20}, {23, 21},
     {23, 22}, {23, 23}, {23, 7},  {23, 15}, {23, 31}, {24, 24}, {24, 25},
     {24, 26}, {24, 27}, {24, 28}, {24, 29}, {24, 30}, {24, 31}, {24, 0},
     {24, 8},  {24, 16}, {25, 24}, {25, 25}, {25, 26}, {25, 27}, {25, 28},
     {25, 29}, {25, 30}, {25, 31}, {25, 1},  {25, 9},  {25, 17}, {26, 24},
     {26, 25}, {26, 26}, {26, 27}, {26, 28}, {26, 29}, {26, 30}, {26, 31},
     {26, 2},  {26, 10}, {26, 18}, {27, 24}, {27, 25}, {27, 26}, {27, 27},
     {27, 28}, {27, 29}, {27, 30}, {27, 31}, {27, 3},  {27, 11}, {27, 19},
     {28, 24}, {28, 25}, {28, 26}, {28, 27}, {28, 28}, {28, 29}, {28, 30},
     {28, 31}, {28, 4},  {28, 12}, {28, 20}, {29, 24}, {29, 25}, {29, 26},
     {29, 27}, {29, 28}, {29, 29}, {29, 30}, {29, 31}, {29, 5},  {29, 13},
     {29, 21}, {30, 24}, {30, 25}, {30, 26}, {30, 27}, {30, 28}, {30, 29},
     {30, 30}, {30, 31}, {30, 6},  {30, 14}, {30, 22}, {31, 24}, {31, 25},
     {31, 26}, {31, 27}, {31, 28}, {31, 29}, {31, 30}, {31, 31}, {31, 7},
     {31, 15}, {31, 23}}};

#endif