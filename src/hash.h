#pragma once

#include <cstdint>

void init_keys();

void hash_piece(uint64_t& key, int color, int piece, int square);
void hash_turn(uint64_t& key, int turn);
void hash_castle(uint64_t& key, int castle);
void hash_ep_square(uint64_t& key, int ep_square);
