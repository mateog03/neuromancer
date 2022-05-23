#include <iostream>
#include "search.h"
#include "eval.h"
#include "seq.h"
#include "bitboard.h"

int score_to_tt(int score, int ply)
{
	if (score >= min_mate)
		score += ply;
	else if (score <= -min_mate)
		score -= ply;

	return score;
}

int score_from_tt(int score, int ply)
{
	if (score >= min_mate)
		score -= ply;
	else if (score <= -min_mate)
		score += ply;

	return score;
}

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
	
	tt.clear();
	history.clear();
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

int searcher_t::negamax(board_t& board, info_t& info, pv_t& pv, int alpha, int beta, int depth, int ply, bool do_null)
{
	if (should_stop(info.nodes))
		return 0;

	if (depth <= 0)
		return quiesce(board, info, alpha, beta);

	++info.nodes;

	const bool in_pv = alpha != beta - 1;
	const bool in_root = ply == 0;
	const bool in_check = board.in_check();

	const uint64_t hash = board.hash();
	const slot_t& s = tt.get(hash);

	int score, move_count = 0, bound = upper_bound;

	if (!in_root && !in_pv && s.hash == hash && s.depth >= depth) {
		score = score_from_tt(s.score, ply);

		if (s.bound <= exact_bound && score >= beta)
			return beta;

		if (s.bound >= exact_bound && score <= alpha)
			return alpha;

		if (s.bound == exact_bound)
			return score;
	}

	move_t pv_move = (in_root && depth > 1) ? info.prev_best : s.move;
	move_t move;
	move_t best_move;
	undo_t undo;
	pv_t child_pv;

	if (do_null
	&&  depth >= 3
	&&  !(in_root || in_pv || in_check)
	&&  board.squares<knight, bishop, rook, queen>(board.stm())) {
		board.play_null(undo);

		score = -negamax(board, info, child_pv, -beta, -beta + 1, depth - 3, ply + 1, false);

		board.takeback_null(undo);

		if (score >= beta)
			return beta;
	}

	sequencer_t seq(board, pv_move, history);

	while (seq >> move) {
		if (!board.play(move, undo)) {
			board.takeback(move, undo);
			continue;
		}

		++move_count;

		if (move_count == 1)
			score = -negamax(board, info, child_pv, -beta, -alpha, depth - 1, ply + 1, true);
		else {
			score = -negamax(board, info, child_pv, -alpha - 1, -alpha, depth - 1, ply + 1, true);

			if (score > alpha && score < beta)
				score = -negamax(board, info, child_pv, -beta, -alpha, depth - 1, ply + 1, true);
		}

		board.takeback(move, undo);

		if (score >= beta) {
			tt.store(hash, move, score_to_tt(beta, ply), depth, lower_bound);

			if (!board.is_capture(move))
				history.add(board, move, depth);

			return beta;
		}

		if (score > alpha) {
			alpha = score;
			bound = exact_bound;
			best_move = move;

			pv = std::move(child_pv);
			pv.insert(pv.begin(), move);
		}
	}

	if (move_count == 0)
		return board.in_check() ? -mate + ply : 0;

	tt.store(hash, best_move, score_to_tt(alpha, ply), depth, bound);

	return alpha;
}

void searcher_t::think(board_t& board)
{
	info_t info;
	pv_t pv;
	int score = -mate;

	for (int depth = 1; depth < max_depth; ++depth) {
		info.reset();

		score = negamax(board, info, pv, -mate, mate, depth, 0, true);

		if (stop)
			break;

		info.report(pv, depth, score);
		info.prev_best = pv.front();
	}

	std::cout << "bestmove " << info.prev_best << std::endl;
}
