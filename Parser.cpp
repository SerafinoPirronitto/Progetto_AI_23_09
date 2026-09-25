#include "Parser.hpp"
#include <sstream>

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


void parseConflicts(std::istream& input, Matrix<bool>& out_Adj)
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

            out_Adj[i - 1, j - 1] = 1;
            out_Adj[j - 1, i - 1] = 1;


        } else throw std::invalid_argument("Bad header: expected `Ei Ej`");
    }
}
void parsePreferences(std::istream& input, Matrix<bool>& out_Pref)
{
    std::regex line_pattern(R"(^\s*(\w+)(\d+)\s*:\s*(.+?)\s*$)");
    std::regex token_pattern(R"(^T(\d+)$)");
    std::smatch matches;

    std::string line;
    while (std::getline(input, line))
    {
        if (line.empty()) continue;
        if (line.starts_with("#")) continue;

        if (std::regex_match(line, matches, line_pattern))
        {
            const std::size_t i = std::stoull(matches[2].str());
            std::istringstream token_stream(matches[3].str());
            std::string token;

            while (token_stream >> token)
            {
                std::smatch token_match;
                if (!std::regex_match(token, token_match, token_pattern))
                    throw std::invalid_argument("Bad preference: expected `Tj` tokens after the colon.");

                const std::size_t j = std::stoull(token_match[1].str());
                out_Pref[i - 1, j - 1] = 1;
            }
        }
        else
        {
            throw std::invalid_argument("Bad header: expected `Ei: Tj ...`.");
        }
    }
}