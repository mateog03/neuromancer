#include <iostream>
#include "search.h"
#include "eval.h"
#include "seq.h"

void info_t::reset()
{
	start = tc_t::now();
	nodes = 0;
}

void info_t::report(const pv_t& pv, int depth, int score)
{
	const auto diff = tc_t::now() - start;
	const auto msec = 1 + diff / std::chrono::milliseconds(1);
	const auto nps  = (nodes * 1000) / msec;

	std::cout << "info depth " << depth;

	if (std::abs(score) >= min_mate)
		std::cout << " score mate " << (mate - std::abs(score) + 1) / 2;
	else
		std::cout << " score cp " << score;

	std::cout << " nps " << nps;
	std::cout << " nodes " << nodes;
	std::cout << " time " << msec;
	std::cout << " pv";

	for (auto move : pv)
		std::cout << ' ' << move;

	std::cout << std::endl;
}

void searcher_t::prepare(long msec, int depth)
{
	stop = false;
	max_time = tc_t::now() + std::chrono::milliseconds(msec);
	max_depth = depth + 1;
	infinite = msec == 0;
}

void searcher_t::request_stop()
{
	stop = true;
}

bool searcher_t::should_stop(int64_t nodes)
{
	if (!infinite) {
		if (nodes % 2048 == 0 && tc_t::now() >= max_time)
			request_stop();
	}

	return stop;
}

int searcher_t::quiesce(board_t& board, info_t& info, int alpha, int beta)
{
	if (should_stop(info.nodes))
		return 0;

	++info.nodes;

	const int stand_pat = evaluate(board);

	if (stand_pat >= beta)
		return beta;

	if (stand_pat > alpha)
		alpha = stand_pat;

	move_t move;
	undo_t undo;
	sequencer_t seq(board);

	int score;

	while (seq >> move) {
		if (!board.play(move, undo)) {
			board.takeback(move, undo);
			continue;
		}

		score = -quiesce(board, info, -beta, -alpha);

		board.takeback(move, undo);

		if (score >= beta)
			return beta;

		if (score > alpha)
			alpha = score;
	}

	return alpha;
}

int searcher_t::negamax(board_t& board, info_t& info, pv_t& pv, int alpha, int beta, int depth, int ply)
{
	if (should_stop(info.nodes))
		return 0;

	if (depth <= 0)
		return quiesce(board, info, alpha, beta);

	++info.nodes;

	move_t pv_move = (ply == 0 && depth > 1) ? info.prev_best : move_t();
	move_t move;
	undo_t undo;
	pv_t child_pv;
	sequencer_t seq(board, pv_move);

	int score, move_count = 0;

	while (seq >> move) {
		if (!board.play(move, undo)) {
			board.takeback(move, undo);
			continue;
		}

		++move_count;

		if (move_count == 1)
			score = -negamax(board, info, child_pv, -beta, -alpha, depth - 1, ply + 1);
		else {
			score = -negamax(board, info, child_pv, -alpha - 1, -alpha, depth - 1, ply + 1);

			if (score > alpha && score < beta)
				score = -negamax(board, info, child_pv, -beta, -alpha, depth - 1, ply + 1);
		}

		board.takeback(move, undo);

		if (score >= beta)
			return beta;

		if (score > alpha) {
			alpha = score;

			pv = std::move(child_pv);
			pv.insert(pv.begin(), move);
		}
	}

	if (move_count == 0)
		return board.in_check() ? -mate + ply : 0;

	return alpha;
}

void searcher_t::think(board_t& board)
{
	info_t info;
	pv_t pv;
	int score = -mate;

	for (int depth = 1; depth < max_depth; ++depth) {
		info.reset();

		score = negamax(board, info, pv, -mate, mate, depth, 0);

		if (stop)
			break;

		info.report(pv, depth, score);
		info.prev_best = pv.front();
	}

	std::cout << "bestmove " << info.prev_best << std::endl;
}
