#pragma once

#include <iostream>
#include <string>
#include <regex>
#include "Matrix.hpp"

void parseHeader(std::istream& input, std::size_t& out_N, std::size_t& out_M);
void parseConflicts(std::istream& input, Matrix<bool>& out_Adj);
void parsePreferences(std::istream& input, Matrix<bool>& out_Pref);