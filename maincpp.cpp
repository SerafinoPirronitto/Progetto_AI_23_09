#pragma once
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <print>
#include "AntColony.hpp"

static std::string trim(const std::string& s)
{
    const std::string whitespace = " \t\r\n";
    const size_t start = s.find_first_not_of(whitespace);
    if (start == std::string::npos)
        return "";

    const size_t end = s.find_last_not_of(whitespace);
    return s.substr(start, end - start + 1);
}

static std::vector<std::string> splitWords(const std::string& text)
{
    std::istringstream iss(text);
    std::vector<std::string> words;
    std::string word;
    while (iss >> word)
        words.push_back(word);
    return words;
}

static int parseIntegerValue(const std::string& line)
{
    const size_t pos = line.find('=');
    if (pos == std::string::npos)
        throw std::runtime_error("Invalid numeric line: " + line);

    const std::string value = trim(line.substr(pos + 1));
    return std::stoi(value);
}

// Matches the fixed-size matrix style of AntColonyCTP<N, M>.
template<size_t N, size_t M>
void parseCoursesFile(const std::string& filePath,
    float (&adjacency)[N][N],
    float (&preferences)[N][M])
{
    for (size_t i = 0; i < N; ++i)
    {
        for (size_t j = 0; j < N; ++j)
            adjacency[i][j] = 0.0f;
    }

    for (size_t i = 0; i < N; ++i)
    {
        for (size_t j = 0; j < M; ++j)
            preferences[i][j] = 1.0f;
    }

    std::ifstream input(filePath.c_str());
    if (!input)
        throw std::runtime_error("Unable to open file: " + filePath);

    std::vector<std::pair<std::string, std::string>> conflicts;
    std::map<std::string, std::vector<int> > preferredSlots;
    std::map<std::string, int> eventIndex;
    std::vector<std::string> events;

    int n = 0;
    int T = 0;
    bool inConflicts = false;
    bool inPreferences = false;

    std::string line;
    while (std::getline(input, line))
    {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        if (line == "conflicts:")
        {
            inConflicts = true;
            inPreferences = false;
            continue;
        }

        if (line == "preferences:")
        {
            inConflicts = false;
            inPreferences = true;
            continue;
        }

        if (line.rfind("n", 0) == 0 || line.rfind("N", 0) == 0)
        {
            n = parseIntegerValue(line);
            continue;
        }

        if (line.rfind("T", 0) == 0 || line.rfind("t", 0) == 0)
        {
            T = parseIntegerValue(line);
            continue;
        }

        if (inConflicts)
        {
            std::istringstream iss(line);
            std::string e1, e2;
            if (iss >> e1 >> e2)
            {
                conflicts.push_back(std::make_pair(e1, e2));

                if (eventIndex.find(e1) == eventIndex.end())
                {
                    eventIndex[e1] = static_cast<int>(events.size());
                    events.push_back(e1);
                }

                if (eventIndex.find(e2) == eventIndex.end())
                {
                    eventIndex[e2] = static_cast<int>(events.size());
                    events.push_back(e2);
                }
            }
            continue;
        }

        if (inPreferences)
        {
            const size_t colonPos = line.find(':');
            if (colonPos == std::string::npos)
                continue;

            const std::string event = trim(line.substr(0, colonPos));
            const std::string slotsText = trim(line.substr(colonPos + 1));

            if (eventIndex.find(event) == eventIndex.end())
            {
                eventIndex[event] = static_cast<int>(events.size());
                events.push_back(event);
            }

            preferredSlots[event] = std::vector<int>();
            std::vector<std::string> tokens = splitWords(slotsText);
            for (size_t k = 0; k < tokens.size(); ++k)
            {
                std::string slotName = tokens[k];
                for (size_t c = 0; c < slotName.size(); ++c)
                    slotName[c] = static_cast<char>(std::toupper(static_cast<unsigned char>(slotName[c])));

                if (slotName.size() > 1 && slotName.rfind("T", 0) == 0)
                {
                    try
                    {
                        const int slotIndex = std::stoi(slotName.substr(1));
                        if (slotIndex >= 1 && slotIndex <= T)
                            preferredSlots[event].push_back(slotIndex - 1);
                    }
                    catch (...) {}
                }
            }
        }
    }

    if (n <= 0)
        n = static_cast<int>(events.size());

    if (n > static_cast<int>(N))
        throw std::runtime_error("File contains more events than N in AntColonyCTP<N, M>.");

    if (T > static_cast<int>(M))
        throw std::runtime_error("File contains more time slots than M in AntColonyCTP<N, M>.");

    for (size_t i = 0; i < conflicts.size(); ++i)
    {
        const std::string& e1 = conflicts[i].first;
        const std::string& e2 = conflicts[i].second;

        std::map<std::string, int>::const_iterator it1 = eventIndex.find(e1);
        std::map<std::string, int>::const_iterator it2 = eventIndex.find(e2);
        if (it1 == eventIndex.end() || it2 == eventIndex.end())
            continue;

        const size_t i1 = static_cast<size_t>(it1->second);
        const size_t i2 = static_cast<size_t>(it2->second);
        adjacency[i1][i2] = 1.0f;
        adjacency[i2][i1] = 1.0f;
    }

    for (std::map<std::string, std::vector<int> >::const_iterator it = preferredSlots.begin(); it != preferredSlots.end(); ++it)
    {
        std::map<std::string, int>::const_iterator indexIt = eventIndex.find(it->first);
        if (indexIt == eventIndex.end())
            continue;

        const size_t row = static_cast<size_t>(indexIt->second);
        for (size_t j = 0; j < M; ++j)
            preferences[row][j] = 1.0f;

        for (size_t k = 0; k < it->second.size(); ++k)
        {
            const size_t col = static_cast<size_t>(it->second[k]);
            if (col < M)
                preferences[row][col] = 0.0f;
        }
    }
}

int main()
{
    const size_t N = 8;
    const size_t M = 4;
    float adjacency[N][N];
    float preferences[N][M];

    try
    {
        parseCoursesFile("courses.txt", adjacency, preferences);

        std::cout << "Conflict adjacency matrix:\n";
        for (size_t i = 0; i < N; ++i)
        {
            for (size_t j = 0; j < N; ++j)
                std::cout << adjacency[i][j] << " ";
            std::cout << "\n";
        }

        std::cout << "\nPenalty matrix (event x slot):\n";
        std::cout << "0 = preferred slot, 1 = non-preferred slot\n";
        for (size_t i = 0; i < N; ++i)
        {
            for (size_t j = 0; j < M; ++j)
                std::cout << preferences[i][j] << " ";
            std::cout << "\n";
        }
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    
    // Solve CTP
    std::mt19937 RNG{1};
    AntColonyCTP<N, M> solver(preferences, adjacency);
    AntColonyCTP<N, M>::Solution s = solver.solve(5, .5f, .5f, .5f, RNG);

    for (size_t i = 0; i < s.size(); ++i)
        std::cout << s[i] << " ";
    std::cout << "\n";

    return 0;
}