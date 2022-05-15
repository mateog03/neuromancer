#pragma once

#include <vector>
#include <cstdint>
#include "move.h"

constexpr int lower_bound = 0;
constexpr int exact_bound = 1;
constexpr int upper_bound = 2;

struct slot_t {
	uint64_t hash;
	move_t move;
	int32_t score;
	uint8_t depth;
	uint8_t bound;
};

class ttable_t {
public:
	ttable_t();

	size_t mb_to_size(int mb) const;
	size_t index(uint64_t hash) const;

	void clear();
	void store(uint64_t hash, move_t move, int score, int depth, int bound);
	slot_t& get(uint64_t hash);

private:
	std::vector<slot_t> table;
};
