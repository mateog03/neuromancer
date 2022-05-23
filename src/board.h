#pragma once

#include <string>
#include <cstdint>
#include <array>
#include "move.h"

constexpr int wk_castle = 1;
constexpr int wq_castle = 2;
constexpr int bk_castle = 4;
constexpr int bq_castle = 8;

struct undo_t {
	int8_t cpiece;
	int8_t ep_square;
	int8_t castle;
};

class board_t {
public:
	void parse_fen(const std::string& fen);

	bool is_square_attacked(int square, int color) const;
	bool is_king_attacked(int color) const;
	bool in_check() const;

	void execute(move_t move, int color, int mpiece, int cpiece);
	bool play(move_t move, undo_t& undo);
	void takeback(move_t move, const undo_t& undo);

	void play_null(undo_t& undo);
	void takeback_null(const undo_t& undo);

	bool is_capture(move_t move);

	int piece_at(int square) const;
	int stm() const;
	bool can_castle(int castling_right) const;

	uint64_t ep_bb() const;
	uint64_t occupancy() const;
	uint64_t empty() const;
	uint64_t hash() const;

	template<int...piece> uint64_t squares() const;
	template<int...piece> uint64_t squares(int color) const;

private:
	std::array<uint64_t, 6> pieces;
	std::array<uint64_t, 2> colors;
	int8_t turn;
	int8_t ep_square;
	int8_t castle;
};

template<int...piece>
uint64_t board_t::squares() const
{
	return (... | pieces[piece]);
}

template<int...piece>
uint64_t board_t::squares(int color) const
{
	if constexpr (sizeof...(piece) > 0)
		return squares<piece...>() & colors[color];
	else
		return colors[color];
}
