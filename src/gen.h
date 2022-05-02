#pragma once

#include <vector>
#include <utility>
#include "move.h"
#include "board.h"

using movelist_t = std::vector<std::pair<move_t, int>>;

movelist_t generate_all(const board_t& board);
movelist_t generate_noisy(const board_t& board);
