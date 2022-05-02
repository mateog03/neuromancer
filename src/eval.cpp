#include "eval.h"
#include "attacks.h"

constexpr std::array<int, 6> value  = {100, 300, 315, 500, 900, 0};
constexpr std::array<int, 6> weight = {0  , 5  , 4  , 3  , 1  , 0};
constexpr std::array<int, 6> danger = {0  , 16 , 8  , 12 , 20 , 0};

constexpr std::array<int, 64> bare_king = {
	200, 130, 110, 100, 100, 110, 130, 200,
	130,  90,  85,  80,  80,  85,  90, 130,
	110,  85,  70,  60,  60,  70,  85, 110,
	100,  80,  60,  30,  30,  60,  80, 100,
	100,  80,  60,  30,  30,  60,  80, 100,
	110,  85,  70,  60,  60,  70,  85, 110,
	130,  90,  85,  80,  80,  85,  90, 130,
	200, 130, 110, 100, 100, 110, 130, 200
};

template<int piece>
int mobility(uint64_t mob, uint64_t dan)
{
	return std::popcount(mob) * weight[piece] + value[piece]
		+  std::popcount(dan) * danger[piece];
}

template<int color>
int evaluate(const board_t& board)
{
	const int opk = std::countr_zero(board.squares<king>(!color));
	const uint64_t opkzone = single_attacks<king>(opk);

	uint64_t b, mob;
	int from, eval = 0;

	if (std::popcount(board.squares<knight, bishop, rook, queen>(!color)) < 3)
		eval += bare_king[opk];

	for (b = board.squares<knight>(color); b; b &= b - 1) {
		from = std::countr_zero(b);
		mob  = single_attacks<knight>(from);
		eval += mobility<knight>(mob, mob & opkzone);
	}

	for (b = board.squares<bishop>(color); b; b &= b - 1) {
		from = std::countr_zero(b);
		mob  = single_attacks<bishop>(from, board.occupancy());
		eval += mobility<bishop>(mob, mob & opkzone);
	}

	for (b = board.squares<rook>(color); b; b &= b - 1) {
		from = std::countr_zero(b);
		mob  = single_attacks<rook>(from, board.occupancy());
		eval += mobility<rook>(mob, mob & opkzone);
	}

	for (b = board.squares<queen>(color); b; b &= b - 1) {
		from = std::countr_zero(b);
		mob  = single_attacks<queen>(from, board.occupancy());
		eval += mobility<queen>(mob, mob & opkzone);
	}

	b = board.squares<pawn>(color);
	eval += std::popcount(b) * value[pawn];

	return eval;
}

int evaluate(const board_t& board)
{
	constexpr std::array<int, 2> sign = {1, -1};

	const int eval = evaluate<white>(board) - evaluate<black>(board);
	const int color = board.stm();

	return eval * sign[color];
}
