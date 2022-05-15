#include <random>
#include <array>
#include "hash.h"

namespace {
	std::array<std::array<std::array<uint64_t, 64>, 6>, 2> piece_keys;
	std::array<uint64_t,  2> turn_keys;
	std::array<uint64_t, 16> castle_keys;
	std::array<uint64_t, 64> ep_square_keys;
}

void init_keys()
{
	std::mt19937_64 prng;

	for (auto& color : piece_keys)
		for (auto& piece : color)
			for (auto& key : piece)
				key = prng();

	for (auto& key : turn_keys)
		key = prng();

	for (auto& key : castle_keys)
		key = prng();

	for (auto& key : ep_square_keys)
		key = prng();
}

void hash_piece(uint64_t& key, int color, int piece, int square)
{
	key ^= piece_keys[color][piece][square];
}

void hash_turn(uint64_t& key, int turn)
{
	key ^= turn_keys[turn];
}

void hash_castle(uint64_t& key, int castle)
{
	key ^= castle_keys[castle];
}

void hash_ep_square(uint64_t& key, int ep_square)
{
	if (ep_square > 0)
		key ^= ep_square_keys[ep_square];
}
