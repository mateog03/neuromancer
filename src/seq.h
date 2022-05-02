#pragma once

#include "board.h"
#include "gen.h"

class sequencer_t {
public:
	sequencer_t(const board_t& board, move_t pv_move);
	sequencer_t(const board_t& board);

	void score(const board_t& board, move_t pv_move);
	bool operator>>(move_t& move);

private:
	movelist_t moves;
	movelist_t::iterator it;
};
