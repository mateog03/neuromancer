#include "move.h"
#include "util.h"

move_t::move_t()
	: move(0)
{
}

move_t::move_t(int from, int to)
	: move(from | to << 6)
{
}

move_t::move_t(int from, int to, int promo)
	: move(from | to << 6 | promo << 12)
{
}

int move_t::from() const
{
	return move & 63;
}

int move_t::to() const
{
	return (move >> 6) & 63;
}

int move_t::promo() const
{
	return move >> 12;
}

bool move_t::operator==(move_t other) const
{
	return move == other.move;
}

bool move_t::operator!=(move_t other) const
{
	return move != other.move;
}

std::ostream& operator<<(std::ostream& os, move_t move)
{
	os << square_to_string(move.from());
	os << square_to_string(move.to());

	if (const int promo = move.promo())
		os << piece_to_ascii(promo);

	return os;
}

std::istream& operator>>(std::istream& is, move_t& move)
{
	int from, to, promo;
	std::string token;

	if (is >> token) {
		from = string_to_square(token.substr(0, 2));
		to = string_to_square(token.substr(2, 2));

		if (token.size() > 4)
			promo = ascii_to_piece(token[4]);
		else
			promo = 0;

		move = { from, to, promo };
	}

	return is;
}
