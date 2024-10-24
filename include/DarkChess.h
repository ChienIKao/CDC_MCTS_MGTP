#ifndef DARKCHESS_H
#define DARKCHESS_H

#include <random>
#include <vector>

#include "libchess.h"

class DarkChess_Action {
	public:
		DarkChess_Action() : player(UNKNOWN), actionID(-1) {}
		DarkChess_Action(int player, int actionID)
		    : player(player), actionID(actionID) {}
		DarkChess_Action(const DarkChess_Action& action)
		    : player(action.player), actionID(action.actionID) {}

		int getPlayer() const { return player; }
		int getActionID() const { return actionID; }

	private:
		int player;   // 什麼玩家的 action
		int actionID; // action 的 index
};

class DarkChess_State {
	public:
		DarkChess_State()
		    : curr_player(UNKNOWN),
		      my_color(UNKNOWN),
		      opp_color(UNKNOWN),
		      winner(UNKNOWN),
		      no_eat_flip(0),
		      last_action(DarkChess_Action(UNKNOWN, -1)) {
			InitBoard();
		}
		DarkChess_State(const DarkChess_State& state) {
			memcpy(board, state.board, sizeof(FIN) * BOARD_SIZE);
			memcpy(time, state.time, sizeof(int) * 2);
			memcpy(coverPieceCount, state.coverPieceCount, sizeof(int) * 14);
			memcpy(chess_count, state.chess_count, sizeof(int) * 16);
			last_action = state.last_action;
			curr_player = state.curr_player;
			my_color = state.my_color;
			opp_color = state.opp_color;
			winner = state.winner;
			no_eat_flip = state.no_eat_flip;
			act_history = state.act_history;
		}

		// 判斷當前遊戲狀態是否已經結束。
		bool isTerminal() const;

		// 如果遊戲結束，返回當前狀態的結果，1（勝利）、0（平局）、-1（失敗）。
		double getResult() const;

		// 返回當前狀態下可執行的所有動作。
		std::vector<DarkChess_Action> getAvailableActions() const;

		// 這個函數會根據 Action 對當前狀態進行更新，並返回新的狀態。
		DarkChess_State applyAction(DarkChess_Action action);

		// 返回狀態在上一步的動作（用於最後確定選擇的最佳動作）。
		DarkChess_Action getLastAction() const { return last_action; }

		// 初始化盤面
		void InitBoard();

		// 判斷是否為合法動作
		bool isLegalAction(DarkChess_Action action) const;

		// 判斷是否為上下左右相鄰的位置
		bool isNeighbor(DarkChess_Action action) const;

		// 判斷炮/包是否可以吃子
		bool checkCannonCanEat(DarkChess_Action action) const;

		int getRandomChessId();

		void applyFlip(int sq, FIN f) {
			board[sq] = f;
			coverPieceCount[f]--;
			chess_count[14]--;

			no_eat_flip = 0;
			for (int i = 0; i < ActionMap.size(); i++) {
				if (ActionMap[i].first == sq && ActionMap[i].second == sq) {
					act_history.push_back(DarkChess_Action(curr_player, i));
					break;
				}
			}
		}

		void applyMoveEat(int from, int to) {
			FIN FIN_SRC = board[from];
			FIN FIN_DST = board[to];
			board[to] = FIN_SRC;
			board[from] = FIN_EMPTY;
			chess_count[FIN_DST]--;
			chess_count[FIN_EMPTY]++;
			if (FIN_DST != FIN_EMPTY) no_eat_flip = 0;
			for (int i = 0; i < ActionMap.size(); i++) {
				if (ActionMap[i].first == from && ActionMap[i].second == to) {
					act_history.push_back(DarkChess_Action(curr_player, i));
					break;
				}
			}
		}

		// BOARD FORMAT:
		// 8 | 7 15 23 31
		// 7 | 6 14 22 30
		// 6 | 5 13 21 29
		// 5 | 4 12 20 28
		// 4 | 3 11 19 27
		// 3 | 2 10 18 26
		// 2 | 1  9 17 25
		// 1 | 0  8 16 24
		//   +-----------
		//     a  b  c  d
		//
		// 偶數(0 ~ 12): 帥 (K)、仕 (G)、相 (M)、俥 (R)、傌 (N)、炮 (C)、兵 (P)
		// 奇數(1 ~ 13): 將 (k)、士 (g)、象 (m)、車 (r)、馬 (n)、包 (c)、卒 (p)
		// 14: 未翻開 (COVER)
		// 15: 空格 (EMPTY)

		int getCurrColor() const { return curr_player; }
		int getMyColor() const { return my_color; }
		int getOppColor() const { return opp_color; }

		void setCurrPlayer(int player) { curr_player = player; }
		void setMyColor(int color) { my_color = color; }
		void setOppColor(int color) { opp_color = color; }

	private:
		FIN board[BOARD_SIZE];
		int time[2];                  // 玩家的剩餘時間
		int coverPieceCount[14];      // 各類棋子的覆蓋數量
		int chess_count[16];          // 每種棋子剩餘的數量
		int curr_player;              // 當前玩家顏色
		int my_color;                 // 我的顏色
		int opp_color;                // 對手的顏色
		int winner;                   // 獲勝者顏色
		int no_eat_flip = 0;          // 無吃翻次數
		DarkChess_Action last_action; // 上一步的動作
		std::vector<DarkChess_Action> act_history; // 歷史動作

		std::mt19937 random_;
};

#endif