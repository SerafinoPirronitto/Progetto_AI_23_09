#pragma once

#include <iostream>
#include <string>
#include <regex>
#include "Matrix.hpp"


void parseHeader(std::istream& input, std::size_t& out_N, std::size_t& out_M)
{   
    std::regex pattern(R"((\w+)\s*=\s*(\d+))");
    std::smatch matches;

    std::string line;
    while (std::getline(input, line))
    {
        if (line.empty()) continue;
        if (line.starts_with("#")) continue;
        if (line.contains("conflicts:")) break;

        if (std::regex_match(line, matches, pattern))
        {
            std::string key = matches[1].str();
            std::string value_str = matches[2].str();

            if (key == "n")
                out_N = std::stoull(value_str);
            else if (key == "T")
                out_M = std::stoull(value_str);
            else
                throw std::invalid_argument("Bad header: " + key + ".\nExpected: n or T.");
        } else throw std::invalid_argument("Bad header: expected `n = d` or `T = d`.");
    }
}

template<std::size_t N>
void parseConflicts(std::istream& input, Matrix<bool, N, N>& out_Adj)
{
    std::regex pattern(R"(\w+(\d+)\s+\w+(\d+))");
    std::smatch matches;

    std::string line;
    while (std::getline(input, line))
    {
        if (line.empty()) continue;
        if (line.starts_with("#")) continue;
        if (line.contains("preferences:")) break;

        if (std::regex_match(line, matches, pattern))
        {
            std::size_t i = std::stoull(matches[1].str());
            std::size_t j = std::stoull(matches[2].str());

            out_Adj[i-1,j-1] = 1;

        } else throw std::invalid_argument("Bad header: expected `Ei Ej`");
    }
}

template<std::size_t N, std::size_t M>
void parsePreferences(std::istream& input, Matrix<bool, N, M>& out_Pref)
{
    std::regex pattern(R"(\w+(\d+):\s*(.+))");
    std::smatch matches;

    std::string line;
    while (std::getline(input, line))
    {
        if (line.empty()) continue;
        if (line.starts_with("#")) continue;

        if (std::regex_match(line, matches, pattern))
        {
            std::size_t i = std::stoull(matches[1].str()); // vertex index
            std::regex pref_pattern(R"(\w+(\d+))");
            auto begin = std::sregex_iterator(matches[2].str().begin(), matches[2].str().end(), pref_pattern);
            auto end = std::sregex_iterator():
            
            for(auto k = begin; preference < end; ++k)
            {
                std::smatch preference = *k; 
                std::size_t j = std::stoull(preference[1].str()); // time slot index
                out_Pref[i,j] = 1;
            }
        } else throw std::invalid_argument("Bad header: expected `Ei: Tj ...`.");
    }
}