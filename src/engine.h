#pragma once

#include <sstream>
#include "board.h"
#include "search.h"

class engine_t {
public:
	engine_t();

	void start();
	void uci();
	void isready();
	void position(std::stringstream& ss);
	void perft(std::stringstream& ss);
	void go(std::stringstream& ss);

private:
	board_t board;
	searcher_t searcher;
};
