#include <algorithm>
#include "seq.h"


sequencer_t::sequencer_t(const board_t& board, move_t pv_move, const history_t& history)
	: moves(generate_all(board))
	, it(moves.begin())
{
	for (auto& [move, score] : moves) {
		const int piece  = board.piece_at(move.from());
		const int victim = board.piece_at(move.to());

		if (move == pv_move)
			score = 1 << 20;
		else if (victim >= 0)
			score = (victim << 10) - piece;
		else
			score = history.score(board, move);
	}
}

sequencer_t::sequencer_t(const board_t& board)
	: moves(generate_noisy(board))
	, it(moves.begin())
{
	for (auto& [move, score] : moves) {
		const int piece  = board.piece_at(move.from());
		const int victim = board.piece_at(move.to());

		score = (victim << 10) - piece;
	}
}

bool sequencer_t::operator>>(move_t& move)
{
	if (it == moves.end())
		return false;

	std::iter_swap(it, std::max_element(it, moves.end(),
				[] (const auto& a, const auto& b) {
					return a.second < b.second;
				})
		);

	move = it->first;
	++it;

	return true;
}
