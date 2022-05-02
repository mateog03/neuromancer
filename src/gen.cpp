#include "gen.h"
#include "attacks.h"

template<int piece, int color>
void serialize_piece(const board_t& board, uint64_t filter, movelist_t& moves)
{
	uint64_t att, b = board.squares<piece>(color);
	int from, to;

	for (; b; b &= b - 1) {
		from = std::countr_zero(b);
		att  = single_attacks<piece>(from, board.occupancy()) & filter;

		for (; att; att &= att - 1) {
			to = std::countr_zero(att);

			moves.emplace_back(move_t(from, to), 0);
		}
	}
}

template<int delta, bool promotions>
void serialize_pawns(uint64_t b, uint64_t filter, movelist_t& moves)
{
	constexpr uint64_t prank = promotions ? (rank_1 | rank_8) : ~(rank_1 | rank_8);

	uint64_t att = shift<delta>(b) & filter & prank;
	int from, to;

	for (; att; att &= att - 1) {
		to = std::countr_zero(att);
		from = to - delta;

		if (promotions) {
			moves.emplace_back(move_t(from, to, queen ), 0);
			moves.emplace_back(move_t(from, to, knight), 0);
			moves.emplace_back(move_t(from, to, rook  ), 0);
			moves.emplace_back(move_t(from, to, bishop), 0);
		} else
			moves.emplace_back(move_t(from, to), 0);
	}
}

template<int color>
void add_pawns_noisy(const board_t& board, movelist_t& moves)
{
	constexpr int no   = (color == white) ? north 	   : south;
	constexpr int noea = (color == white) ? north_east : south_west;
	constexpr int nowe = (color == white) ? north_west : south_east;

	const uint64_t pawns  = board.squares<pawn>(color);
	const uint64_t filter = board.squares(!color);

	serialize_pawns<noea, false>(pawns, filter | board.ep_bb(), moves);
	serialize_pawns<nowe, false>(pawns, filter | board.ep_bb(), moves);

	serialize_pawns<noea, true>(pawns, filter, moves);
	serialize_pawns<nowe, true>(pawns, filter, moves);
	serialize_pawns<no  , true>(pawns, board.empty(), moves);
}

template<int color>
void add_pawns_quiet(const board_t& board, movelist_t& moves)
{
	constexpr uint64_t drank = (color == white) ? rank_4 : rank_5;
	constexpr int no = (color == white) ? north : south;

	const uint64_t pawns  = board.squares<pawn>(color);
	const uint64_t empty  = board.empty();
	const uint64_t dempty = shift<no>(empty) & empty & drank;

	serialize_pawns<no	  , false>(pawns, empty , moves);
	serialize_pawns<no * 2, false>(pawns, dempty, moves);
}

template<int color>
void add_pieces(const board_t& board, uint64_t filter, movelist_t& moves)
{
	serialize_piece<king  , color>(board, filter, moves);
	serialize_piece<knight, color>(board, filter, moves);
	serialize_piece<rook  , color>(board, filter, moves);
	serialize_piece<bishop, color>(board, filter, moves);
	serialize_piece<queen , color>(board, filter, moves);
}

template<int color>
void add_castle(const board_t& board, movelist_t& moves)
{
	constexpr int kc = (color == white) ? wk_castle : bk_castle;
	constexpr int qc = (color == white) ? wq_castle : bq_castle;

	constexpr int kfrom  = (color == white) ? 4 : 60;
	constexpr int kto_k  = kfrom + 2;
	constexpr int kto_q  = kfrom - 2;

	if (!board.in_check()) {
		if (board.can_castle(kc)
		&& !board.is_square_attacked(kfrom + 1, !color)
		&& !board.is_square_attacked(kfrom + 2, !color)
		&& !(board.occupancy() & (1ull << (kfrom + 1) | 1ull << (kfrom + 2))))
			moves.emplace_back(move_t(kfrom, kto_k), 0);

		if (board.can_castle(qc)
		&& !board.is_square_attacked(kfrom - 1, !color)
		&& !board.is_square_attacked(kfrom - 2, !color)
		&& !(board.occupancy() & (1ull << (kfrom - 1) | 1ull << (kfrom - 2) | 1ull << (kfrom - 3))))
			moves.emplace_back(move_t(kfrom, kto_q), 0);
	}
}

template<int color, bool noisy>
movelist_t generate(const board_t& board)
{
	const uint64_t filter = noisy ? board.squares(!color) : ~board.squares(color);
	movelist_t moves;
	moves.reserve(100);

	add_pieces<color>(board, filter, moves);
	add_pawns_noisy<color>(board, moves);

	if (!noisy) {
		add_pawns_quiet<color>(board, moves);
		add_castle<color>(board, moves);
	}

	return moves;
}

movelist_t generate_all(const board_t& board)
{
	if (board.stm() == white)
		return generate<white, false>(board);
	else
		return generate<black, false>(board);
}

movelist_t generate_noisy(const board_t& board)
{
	if (board.stm() == white)
		return generate<white, true>(board);
	else
		return generate<black, true>(board);
}
