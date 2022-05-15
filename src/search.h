#pragma once

#include <vector>
#include <chrono>
#include <limits>
#include <atomic>
#include "board.h"
#include "ttable.h"

constexpr int max_ply = 256;
constexpr int mate = std::numeric_limits<int>::max();
constexpr int min_mate = mate - max_ply;

using pv_t = std::vector<move_t>;
using tc_t = std::chrono::high_resolution_clock;

struct info_t {
	void reset();
	void report(const pv_t& pv, int depth, int score);

	tc_t::time_point start;
	move_t prev_best;
	int64_t nodes;
};

class searcher_t {
public:
	void prepare(long msec, int depth);
	void request_stop();
	bool should_stop(int64_t nodes);

	int negamax(board_t& board, info_t& info, pv_t& pv, int alpha, int beta, int depth, int ply);
	int quiesce(board_t& board, info_t& info, int alpha, int beta);
	void think(board_t& board);

private:
	std::atomic<bool> stop;
	tc_t::time_point max_time;
	int max_depth;
	bool infinite;
	
	ttable_t tt;
};
