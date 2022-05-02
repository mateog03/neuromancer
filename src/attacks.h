#pragma once

#include "bitboard.h"

template<int color>
constexpr uint64_t pawn_attacks(uint64_t b)
{
	if (color == white)
		return shift<north_east, north_west>(b);
	else
		return shift<south_east, south_west>(b);
}

template<int piece>
constexpr uint64_t multiple_attacks(uint64_t b, [[ maybe_unused ]] uint64_t o = 0)
{
	if (piece == rook)
		return fill<north, south, east, west>(b, o);
	else if (piece == bishop)
		return fill<north_east, north_west, south_east, south_west>(b, o);
	else if (piece == queen)
		return multiple_attacks<bishop>(b, o) | multiple_attacks<rook>(b, o);
	else {
		const uint64_t side = shift<east, west>(b);

		if (piece == king)
			return shift<north, south>(b | side) | side;
		else
			return shift<north, south>(b ^ shift<east, west>(side))
				|  shift<north * 2, south * 2>(side);
	}
}

inline constexpr std::array<std::array<uint64_t, 64>, 6> empty_attacks = {
	make_table([] (uint64_t  ) { return 0; }),
	make_table([] (uint64_t b) { return multiple_attacks<knight>(b); }),
	make_table([] (uint64_t b) { return multiple_attacks<bishop>(b); }),
	make_table([] (uint64_t b) { return multiple_attacks<rook  >(b); }),
	make_table([] (uint64_t b) { return multiple_attacks<queen >(b); }),
	make_table([] (uint64_t b) { return multiple_attacks<king  >(b); }),
};

inline constexpr std::array<std::array<uint64_t, 64>, 4> dir_masks = {
	make_table([] (uint64_t b) { return fill<north_east, south_west>(b); }),
	make_table([] (uint64_t b) { return fill<north_west, south_east>(b); }),

	make_table([] (uint64_t b) { return fill<north, south>(b); }),
	make_table([] (uint64_t b) { return fill<east , west >(b); }),
};

template<int n_mask>
constexpr uint64_t sliding_attack(int s, uint64_t o)
{
	o &= dir_masks[n_mask][s];

	const uint64_t lower = o & ((1ull << s) - 1);
	const uint64_t upper = o & ~lower;
	const uint64_t msb = 0x8000000000000000 >> std::countl_zero(lower | 1);

	return (upper ^ (upper - msb)) & dir_masks[n_mask][s];
}

template<int piece>
constexpr uint64_t single_attacks(int s)
{
	return empty_attacks[piece][s];
}

template<int piece>
constexpr uint64_t single_attacks(int s, [[ maybe_unused ]] uint64_t o)
{
	if (piece == bishop)
		return sliding_attack<0>(s, o) | sliding_attack<1>(s, o);
	else if (piece == rook)
		return sliding_attack<2>(s, o) | sliding_attack<3>(s, o);
	else if (piece == queen)
		return single_attacks<bishop>(s, o) | single_attacks<rook>(s, o);
	else
		return single_attacks<piece>(s);
}
