#include "history.h"

history_t::history_t()
{
	clear();
}

void history_t::clear()
{
	table = {};
}

void history_t::add(const board_t& board, move_t move, int depth)
{
	const int color = board.stm();
	const int from  = move.from();
	const int to    = move.to();

	table[color][from][to] += depth * depth;
}

int history_t::score(const board_t& board, move_t move) const
{
	const int color = board.stm();
	const int from  = move.from();
	const int to    = move.to();

	return table[color][from][to];
}
