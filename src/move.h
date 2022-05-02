#pragma once

#include <iostream>
#include <cstdint>

class move_t {
public:
	move_t();
	move_t(int from, int to);
	move_t(int from, int to, int promo);

	int from() const;
	int to() const;
	int promo() const;

	bool operator==(move_t other) const;
	bool operator!=(move_t other) const;

private:
	uint16_t move;
};

std::ostream& operator<<(std::ostream& os, move_t move);
std::istream& operator>>(std::istream& is, move_t& move);
