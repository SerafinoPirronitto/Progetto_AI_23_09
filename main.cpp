//#include "AntColony.hpp"
#include "Matrix.hpp"
#include "Parser.hpp"
#include "AntColony.hpp"
#include <fstream>
#include <stdlib.h>

int main(int c, char** v)
{
    if (c != 2)
    {
        std::cout << "Usage: " << v[0] << " [PATH TO FILE]" << std::endl;
        return -1;
    }

    std::ifstream file(v[1]);   
    if (!file.is_open()) {
        std::cerr << "Cannot open file\n";
        exit(-1);
    }
    else
    {
        size_t N = 0;
        size_t M = 0;
        try
        {
            parseHeader(file, N, M);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            exit(-1);
        }

        Matrix<bool> Adj(N, N);
        Matrix<bool> Pref(N, M);

        try{
            parseConflicts(file, Adj);
            parsePreferences(file, Pref);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            exit(-1);
        }

        

        std::cout << "Adjacency Matrix:" << Adj;
        std::cout << "Preferences Matrix:" << Pref << std::endl;
        AntColonyCTP solver(N, M, Adj, Pref);

        for (uint32_t i = 1; i < 6; i++)
        {
            std::cout << std::format("\033[3{}m", i);
            std::cout << "Generating slution based on ACO..." << std::endl;
            std::cout << "Setting random seed to " << i << std::endl;
            std::mt19937 RNG{i};
            Solution solution = solver.solve(8, 2.0f, 1.5f, 0.1f, RNG, 500, 1);
            std::cout << "Solution Found:" << solution.data() << std::endl;
        }
        std::cout << "\033[30m";
    }

    return 0;
}