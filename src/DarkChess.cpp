#include "DarkChess.h"

// std::mt19937 random{std::random_device{}()};

void DarkChess_State::InitBoard() {
	// 偶數(0 ~ 12): 帥 (K)、仕 (G)、相 (M)、俥 (R)、傌 (N)、炮 (C)、兵 (P)
	// 奇數(1 ~ 13): 將 (k)、士 (g)、象 (m)、車 (r)、馬 (n)、包 (c)、卒 (p)
	// 14: 未翻開 (COVER)
	// 15: 空格 (EMPTY)
	const int chess[16] = {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 5, 5, 32, 0};
	const int cover[14] = {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 5, 5};

	time[RED] = 0;
	time[BLK] = 0;
	memcpy(coverPieceCount, cover, sizeof(int) * 14);
	memcpy(chess_count, chess, sizeof(int) * 16);
	act_history.clear();

	for (int i = 0, sq = 0; i < ROW_COUNT; i++) {
		for (int j = 0; j < COL_COUNT; j++, sq++) {
			board[sq] = FIN_COVER;
		}
	}
}

bool DarkChess_State::isLegalAction(DarkChess_Action action) const {
	int from = ActionMap[action.getActionID()].first;
	int to = ActionMap[action.getActionID()].second;
	FIN src_chess = board[from]; // 起點的棋子
	FIN dst_chess = board[to];   // 終點的棋子

	if (from != to) { // 移動或吃子
		// 雙方顏色未知時只能翻棋
		if (action.getPlayer() == COLOR::UNKNOWN) {
			return false;
		}
		// 起點、終點不能是暗子，起點不能是空子
		else if (src_chess == FIN_COVER || src_chess == FIN_EMPTY ||
		         dst_chess == FIN_COVER) {
			return false;
		} else if (action.getPlayer() == COLOR::RED) { // 紅棋
			// 起點需為紅棋，終點需為黑棋或空格
			if (color_of(src_chess) && dst_chess == FIN_EMPTY &&
			    isNeighbor(action)) {
				return true; // 終點是空格可以直接移動
			} else if (color_of(src_chess) != COLOR::BLK ||
			           color_of(dst_chess) != COLOR::RED) {
				return false;
			} else if (src_chess == FIN_C) { // 炮要特殊判定
				return checkCannonCanEat(action);
			} else if (!isNeighbor(action)) {
				return false;
			} else if (!can_capture(src_chess, dst_chess)) {
				return false;
			}
		} else if (action.getPlayer() == COLOR::BLK) { // 黑棋
			// 起點需為黑棋，終點需為紅棋或空格
			if (color_of(src_chess) && dst_chess == FIN_EMPTY &&
			    isNeighbor(action)) {
				return true; // 終點是空格可以直接移動
			} else if (color_of(src_chess) != COLOR::RED ||
			           color_of(dst_chess) != COLOR::BLK) {
				return false;
			} else if (src_chess == FIN_C) { // 炮要特殊判定
				return checkCannonCanEat(action);
			} else if (!isNeighbor(action)) {
				return false;
			} else if (!can_capture(src_chess, dst_chess)) {
				return false;
			}
		}
	} else {
		if (src_chess != FIN_COVER) { // 要翻開的那格只能是暗子
			return false;
		}
	}
	return true; // 符合以上條件即為合法的 action
}

std::vector<DarkChess_Action> DarkChess_State::getAvailableActions() const {
	std::vector<DarkChess_Action> actions;
	for (int pos = 0; pos < ACTION_SIZE; ++pos) {
		DarkChess_Action action(curr_player, pos);
		if (!isLegalAction(action)) {
			continue;
		}
		actions.push_back(action);
	}
	return actions;
}

DarkChess_State DarkChess_State::applyAction(DarkChess_Action action) {
	DarkChess_State next_state(*this);
	int from = ActionMap[action.getActionID()].first;
	int to = ActionMap[action.getActionID()].second;
	FIN FIN_SRC = board[from];
	FIN FIN_DST = board[to];

	if (from != to) { // 移動或吃子
		next_state.board[to] = FIN_SRC;
		next_state.board[from] = FIN_EMPTY;
		next_state.curr_player = (curr_player == RED) ? BLK : RED;
		if (FIN_DST != FIN_EMPTY) { // 吃子
			next_state.chess_count[FIN_DST]--;
			next_state.chess_count[FIN_EMPTY]++;
			next_state.no_eat_flip = 0;
		} else { // 移動
			next_state.no_eat_flip++;
		}
	} else { // 翻棋
		int chess_id = getRandomChessId();
		FIN FIN_FLIP = FIN(chess_id);

		next_state.board[to] = FIN_FLIP;
		next_state.coverPieceCount[chess_id]--;
		next_state.chess_count[FIN_COVER]--;
		next_state.no_eat_flip = 0;

		if (action.getPlayer() == UNKNOWN) {
			next_state.curr_player = (color_of(FIN_FLIP) == RED) ? BLK : RED;
			next_state.my_color = color_of(FIN_FLIP);
			next_state.opp_color = (color_of(FIN_FLIP) == RED) ? BLK : RED;
		} else {
			next_state.curr_player = (curr_player == RED) ? BLK : RED;
		}
	}
	next_state.last_action = action;
	next_state.act_history.push_back(action);

	if (next_state.getAvailableActions().size() == 0) {
		next_state.winner = (curr_player == RED) ? BLK : RED;
	}

	return next_state;
}

bool DarkChess_State::isNeighbor(DarkChess_Action action) const {
	int from = ActionMap[action.getActionID()].first;
	int to = ActionMap[action.getActionID()].second;
	int src_x = from / ROW_COUNT;
	int src_y = from % ROW_COUNT;
	int dst_x = to / ROW_COUNT;
	int dst_y = to % ROW_COUNT;

	if (src_x == dst_x && abs(src_y - dst_y) == 1) {
		return true;
	} else if (src_y == dst_y && abs(src_x - dst_x) == 1) {
		return true;
	}
	return false;
}

bool DarkChess_State::checkCannonCanEat(DarkChess_Action action) const {
	int from = ActionMap[action.getActionID()].first;
	int to = ActionMap[action.getActionID()].second;
	int src = from, dst = to;
	int chess_cnt = 0;

	// 由於前面已經判定過移動到相鄰空格，因此這裡只會是距離一格以上的空格
	if (board[dst] == FIN_EMPTY) return false;
	// 炮/包必須隔著一顆棋吃
	if (isNeighbor(action)) return false;

	if (src > dst) std::swap(src, dst);

	int src_x = src / ROW_COUNT, src_y = src % ROW_COUNT;
	int dst_x = dst / ROW_COUNT, dst_y = dst % ROW_COUNT;

	if (src_y == dst_y) { // 兩顆棋在同一個 row
		for (int i = src + 8; i < dst; i += 8) {
			if (board[i] != FIN_EMPTY) chess_cnt++;
		}
	} else if (src_x == dst_x) { // 兩顆棋在同一個 column
		for (int i = src + 1; i < dst; i++) {
			if (board[i] != FIN_EMPTY) chess_cnt++;
		}
	}
	return (chess_cnt == 1);
}

int DarkChess_State::getRandomChessId() {
	int rand_num =
	    std::uniform_int_distribution<int>{0, chess_count[14] - 1}(random_);
	int rand_chess_id = 0;
	for (int i = 0; i <= 13; i++) {
		rand_num -= coverPieceCount[i];
		if (rand_num < 0) {
			rand_chess_id = i;
			break;
		}
	}
	return rand_chess_id;
}

bool DarkChess_State::isTerminal() const {
	if (winner != UNKNOWN) return true;

	// 超過一定步數無吃翻
	if (no_eat_flip >= NO_EAT_FLIP_LIMIT) return true;

	// 長捉 (4 步一循環)
	if (no_eat_flip >= LONG_CATCH_LIMIT * 4) {
		int act_hist_size = act_history.size();
		// 循環的 4 步
		int act1 = act_history[act_hist_size - 1].getActionID();
		int act2 = act_history[act_hist_size - 2].getActionID();
		int act3 = act_history[act_hist_size - 3].getActionID();
		int act4 = act_history[act_hist_size - 4].getActionID();

		// 若沒有連續循環指定次數則長捉不成立
		for (int i = 1; i < LONG_CATCH_LIMIT; i++) {
			if (act_history[act_hist_size - i * 4 - 1].getActionID() != act1 ||
			    act_history[act_hist_size - i * 4 - 2].getActionID() != act2 ||
			    act_history[act_hist_size - i * 4 - 3].getActionID() != act3 ||
			    act_history[act_hist_size - i * 4 - 4].getActionID() != act4) {
				return false;
			}
		}
		return true;
	}
	return false;
}

double DarkChess_State::getResult() const {
	if (isTerminal()) {
		if (winner == my_color) {
			return 1.0;
		} else if (winner == opp_color) {
			return -1.0;
		} else {
			return 0.0;
		}
	}
	return 0.0;
}