#include "ttable.h"

ttable_t::ttable_t()
{
	table.resize(mb_to_size(128));
	clear();
}

size_t ttable_t::mb_to_size(int mb) const
{
	return (mb << 20) / sizeof(slot_t);
}

size_t ttable_t::index(uint64_t hash) const
{
	return hash % table.size();
}

void ttable_t::clear()
{
	for (auto& s : table)
		s = {};
}

slot_t& ttable_t::get(uint64_t hash)
{
	return table[index(hash)];
}

void ttable_t::store(uint64_t hash, move_t move, int score, int depth, int bound)
{
	get(hash) = { hash, move, score, uint8_t(depth), uint8_t(bound) };
}
