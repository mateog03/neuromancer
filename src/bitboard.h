#pragma once

#include <array>
#include <bit>
#include <cstdint>

constexpr int white = 0;
constexpr int black = 1;

constexpr int pawn   = 0;
constexpr int knight = 1;
constexpr int bishop = 2;
constexpr int rook   = 3;
constexpr int queen  = 4;
constexpr int king   = 5;

constexpr int north =  8;
constexpr int south = -8;
constexpr int east  =  1;
constexpr int west  = -1;
constexpr int north_east = north + east;
constexpr int north_west = north + west;
constexpr int south_east = south + east;
constexpr int south_west = south + west;

constexpr uint64_t file_a = 0x0101010101010101;
constexpr uint64_t file_b = file_a << 1;
constexpr uint64_t file_c = file_a << 2;
constexpr uint64_t file_d = file_a << 3;
constexpr uint64_t file_e = file_a << 4;
constexpr uint64_t file_f = file_a << 5;
constexpr uint64_t file_g = file_a << 6;
constexpr uint64_t file_h = file_a << 7;

constexpr uint64_t rank_1 = 0xff;
constexpr uint64_t rank_2 = rank_1 << (1 * 8);
constexpr uint64_t rank_3 = rank_1 << (2 * 8);
constexpr uint64_t rank_4 = rank_1 << (3 * 8);
constexpr uint64_t rank_5 = rank_1 << (4 * 8);
constexpr uint64_t rank_6 = rank_1 << (5 * 8);
constexpr uint64_t rank_7 = rank_1 << (6 * 8);
constexpr uint64_t rank_8 = rank_1 << (7 * 8);

template<int delta>
constexpr uint64_t bit_shift(uint64_t b)
{
	if (delta > 0)
		return b << delta;
	else
		return b >> -delta;
}

template<int delta>
constexpr uint64_t remove_wrap(uint64_t b)
{
	if (delta == east || delta == north_east || delta == south_east)
		return b & ~file_a;
	else if (delta == west || delta == north_west || delta == south_west)
		return b & ~file_h;
	else
		return b;
}

template<int delta>
constexpr uint64_t occluded_fill(uint64_t b, uint64_t o)
{
	o = remove_wrap<delta>(~o);

	b |= bit_shift<delta>(b) & o;
	o &= bit_shift<delta>(o);
	b |= bit_shift<delta * 2>(b) & o;
	o &= bit_shift<delta * 2>(o);
	b |= bit_shift<delta * 4>(b) & o;

	return b;
}

template<int...delta>
constexpr uint64_t shift(uint64_t b)
{
	return (... | remove_wrap<delta>(bit_shift<delta>(b)));
}

template<int...delta>
constexpr uint64_t fill(uint64_t b, uint64_t o = 0)
{
	return (... | shift<delta>(occluded_fill<delta>(b, o)));
}

consteval auto make_table(auto&& func)
{
	std::array<uint64_t, 64> ret = {};

	for (size_t i = 0; i < 64; ++i)
		ret[i] = func(1ull << i);

	return ret;
}
