#include <sstream>
#include "board.h"
#include "bitboard.h"
#include "attacks.h"
#include "util.h"
#include "hash.h"

bool board_t::is_square_attacked(int square, int color) const
{
	if (color == white) {
		if (pawn_attacks<white>(squares<pawn>(white)) & (1ull << square))
			return true;
	} else {
		if (pawn_attacks<black>(squares<pawn>(black)) & (1ull << square))
			return true;
	}

	if (single_attacks<rook>(square, occupancy()) & squares<queen, rook>(color))
		return true;

	if (single_attacks<bishop>(square, occupancy()) & squares<queen, bishop>(color))
		return true;

	if (single_attacks<knight>(square) & squares<knight>(color))
		return true;

	if (single_attacks<king>(square) & squares<king>(color))
		return true;

	return false;
}

bool board_t::is_king_attacked(int color) const
{
	const int k = std::countr_zero(squares<king>(color));
	return is_square_attacked(k, !color);
}

bool board_t::in_check() const
{
	return is_king_attacked(stm());
}

int board_t::piece_at(int square) const
{
	for (int p = 0; p < 6; ++p) {
		if (pieces[p] & (1ull << square))
			return p;
	}

	return -1;
}

int board_t::stm() const
{
	return turn;
}

bool board_t::can_castle(int castling_right) const
{
	return castle & castling_right;
}

uint64_t board_t::ep_bb() const
{
	return ep_square > 0 ? (1ull << ep_square) : 0;
}

uint64_t board_t::occupancy() const
{
	return squares(white) | squares(black);
}

uint64_t board_t::empty() const
{
	return ~occupancy();
}

bool board_t::is_capture(move_t move)
{
	return occupancy() & (1ull << move.to());
}

uint64_t board_t::hash() const
{
	uint64_t key = 0;

	for (int color = 0; color < 2; ++color)
		for (int piece = 0; piece < 6; ++piece)
			for (uint64_t b = colors[color] & pieces[piece]; b; b &= b - 1)
				hash_piece(key, color, piece, std::countr_zero(b));

	hash_turn(key, turn);
	hash_castle(key, castle);
	hash_ep_square(key, ep_square);

	return key;
}

void board_t::parse_fen(const std::string& fen)
{
	std::stringstream ss(fen);
	std::string token;
	int square = 56;

	*this = {};

	ss >> token;
	for (auto c : token) {
		if (c == '/')
			square -= 16;
		else if (std::isdigit(c))
			square += c - '0';
		else {
			const int piece = ascii_to_piece(c);
			const int color = std::isupper(c) ? white : black;

			pieces[piece] |= 1ull << square;
			colors[color] |= 1ull << square;
			++square;
		}
	}

	ss >> token;
	turn = (token == "w") ? white : black;

	ss >> token;
	for (auto c : token) {
		if (c == 'K')
			castle |= wk_castle;
		else if (c == 'Q')
			castle |= wq_castle;
		else if (c == 'k')
			castle |= bk_castle;
		else if (c == 'q')
			castle |= bq_castle;
	}

	ss >> token;
	if (token == "-")
		ep_square = -1;
	else
		ep_square = string_to_square(token);
}

void board_t::execute(move_t move, int color, int mpiece, int cpiece)
{
	const int from = move.from();
	const int to = move.to();

	pieces[mpiece] ^= (1ull << from) ^ (1ull << to);
	colors[color ] ^= (1ull << from) ^ (1ull << to);

	if (cpiece >= 0) {
		pieces[cpiece] ^= 1ull << to;
		colors[!color] ^= 1ull << to;
	}

	if (mpiece == pawn) {
		const int ppiece = move.promo();

		if (ppiece) {
			pieces[pawn  ] ^= 1ull << to;
			pieces[ppiece] ^= 1ull << to;
		} else if (to == ep_square) {
			pieces[pawn  ] ^= 1ull << (to ^ 8);
			colors[!color] ^= 1ull << (to ^ 8);
		}
	} else if (mpiece == king) {
		if (std::abs(from - to) == 2) {
			const int rfrom = from < to ? (from + 3) : (from - 4);
			const int rto   = from < to ? (from + 1) : (from - 1);

			pieces[rook ] ^= (1ull << rfrom) ^ (1ull << rto);
			colors[color] ^= (1ull << rfrom) ^ (1ull << rto);
		}
	}
}

constexpr std::array<int, 64> castle_mask = {
	13, 15, 15, 15, 12, 15, 15, 14,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	7 , 15, 15, 15,  3, 15, 15, 11,
};

bool board_t::play(move_t move, undo_t& undo)
{
	const int from = move.from();
	const int to = move.to();

	const int mpiece = piece_at(from);
	const int cpiece = piece_at(to);

	execute(move, turn, mpiece, cpiece);

	undo.cpiece = cpiece;
	undo.ep_square = ep_square;
	undo.castle = castle;

	ep_square = -1;
	castle &= castle_mask[from];
	castle &= castle_mask[to];
	turn = !turn;

	if (mpiece == pawn) {
		if (std::abs(from - to) == 16)
			ep_square = to ^ 8;
	}

	return !is_king_attacked(!turn);
}

void board_t::takeback(move_t move, const undo_t& undo)
{
	const int to = move.to();

	const int mpiece = move.promo() ? pawn : piece_at(to);
	const int cpiece = undo.cpiece;

	ep_square = undo.ep_square;
	castle = undo.castle;
	turn = !turn;

	execute(move, turn, mpiece, cpiece);
}

void board_t::play_null(undo_t& undo)
{
	undo.ep_square = ep_square;
	ep_square = -1;
	turn = !turn;
}

void board_t::takeback_null(const undo_t& undo)
{
	ep_square = undo.ep_square;
	turn = !turn;
}
