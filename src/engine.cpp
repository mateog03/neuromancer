#include <iostream>
#include <thread>
#include "move.h"
#include "engine.h"
#include "gen.h"
#include "bitboard.h"
#include "hash.h"

constexpr auto name = "Neuromancer";
constexpr auto version = "1.1";
constexpr auto startpos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

int64_t perft_impl(board_t& board, int depth)
{
	if (depth <= 0)
		return 1;

	int64_t nodes = 0;
	undo_t undo;

	for (const auto& [move, score] : generate_all(board)) {
		if (board.play(move, undo))
			nodes += perft_impl(board, depth - 1);

		board.takeback(move, undo);
	}

	return nodes;
}

engine_t::engine_t()
{
	init_keys();

	board.parse_fen(startpos);
}

void engine_t::start()
{
	for (std::string line; std::getline(std::cin, line); ) {
		std::stringstream ss(line);
		std::string token;

		ss >> token;
		if (token == "quit") {
			searcher.request_stop();
			break;
		} else if (token == "stop")
			searcher.request_stop();
		else if (token == "isready")
			isready();
		else if (token == "uci")
			uci();
		else if (token == "position")
			position(ss);
		else if (token == "perft")
			perft(ss);
		else if (token == "go")
			go(ss);
	}
}

void engine_t::uci()
{
	std::cout << "id name " << name << " " << version << std::endl;
	std::cout << "id author Mateo Gjika" << std::endl;
	std::cout << "uciok" << std::endl;
}

void engine_t::isready()
{
	std::cout << "readyok" << std::endl;
}

void engine_t::position(std::stringstream& ss)
{
	std::string fen, token;

	ss >> token;
	if (token == "fen") {
		while (ss >> token && token != "moves")
			fen += token + " ";
	} else if (token == "startpos") {
		fen = startpos;
		ss >> token;
	}

	board.parse_fen(fen);

	if (token == "moves") {
		move_t move;
		undo_t undo;

		while (ss >> move)
			board.play(move, undo);
	}
}

void engine_t::perft(std::stringstream& ss)
{
	int depth;

	if (ss >> depth)
		std::cout << perft_impl(board, depth) << std::endl;
}

void engine_t::go(std::stringstream& ss)
{
	const char ccolor = (board.stm() == white) ? 'w' : 'b';

	long time = 0, inc = 0, mtg = 30;
	int depth = max_ply / 2;

	for (std::string token; ss >> token; ) {
		if (token.substr(1) == "time" && token.front() == ccolor )
			ss >> time;
		else if (token.substr(1) == "inc" && token.front() == ccolor )
			ss >> inc;
		else if (token == "depth")
			ss >> depth;
		else if (token == "movetime") {
			ss >> time;
			mtg = 1;
		}
	}

	searcher.prepare(time / mtg + inc, depth);

	std::thread([&] {
		searcher.think(board);
	}).detach();
}
