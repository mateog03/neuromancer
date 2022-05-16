#pragma once

#include <array>
#include "board.h"
#include "move.h"

class history_t {
public:
	history_t();

	void clear();
	void add(const board_t& board, move_t move, int depth);

	int score(const board_t& board, move_t move) const;

private:
	std::array<std::array<std::array<int, 64>, 64>, 2> table;
};
