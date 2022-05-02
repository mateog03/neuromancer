#include <cctype>
#include "util.h"
#include "bitboard.h"

int ascii_to_piece(char c)
{
	switch (std::tolower(c)) {
		case 'p': return pawn;
		case 'n': return knight;
		case 'b': return bishop;
		case 'r': return rook;
		case 'q': return queen;
		case 'k': return king;
	}

	return -1;
}

char piece_to_ascii(int p)
{
	switch (p) {
		case pawn  : return 'p';
		case knight: return 'n';
		case bishop: return 'b';
		case rook  : return 'r';
		case queen : return 'q';
		case king  : return 'k';
	}

	return -1;
}

int string_to_square(const std::string& s)
{
	const int f = s[0] - 'a';
	const int r = s[1] - '1';

	return f + r * 8;
}

std::string square_to_string(int square)
{
	return {
		char('a' + square % 8),
		char('1' + square / 8)
	};
}
