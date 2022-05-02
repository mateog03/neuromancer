#pragma once

#include <string>

int ascii_to_piece(char c);
char piece_to_ascii(int p);

int string_to_square(const std::string& s);
std::string square_to_string(int square);
