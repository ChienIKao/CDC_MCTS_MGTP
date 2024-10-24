#ifndef MYAI_H
#define MYAI_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <string>

#include "DarkChess.h"
#include "MCTS.h"
#include "libchess.h"

class MyAI {
	public:
		MyAI();

		void InitBoard();
		void InitBoard(const char* data[]);
		void Move(int from, int to);
		void Flip(int sq, FIN f);
		void SetColor(COLOR c);
		void SetTime(COLOR c, int t);
		MOVE GenerateMove(int curr_color);

		std::string GetProtocolVersion() const;
		std::string GetAIName() const;
		std::string GetAIVersion() const;
		void Print() const;

	private:
		int color;
		int time[2];
		FIN board[BOARD_SIZE];
		int coverPieceCount[14];
		int allCoverCount;

		DarkChess_State curr_state;
};

#endif
