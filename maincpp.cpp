//#include "AntColony.hpp"
#include "Matrix.hpp"
#include "Parser.hpp"
#include "AntColony.hpp"
#include <fstream>

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
        std::size_t N = 0;
        std::size_t M = 0;
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

        

        std::cout << "Matrice di Adiacenza:" << Adj;
        std::cout << "Matrice delle Preferenze:" << Pref << std::endl;
        
        AntColonyCTP solver(N, M, Adj, Pref);
        std::cout << "Inizio Generazione soluzione basata su ACO..." << std::endl;
        std::mt19937 RNG{1};
        Matrix<bool> solution = solver.solve(3, 1.0f, 1.0f, 0.1f, RNG);
        std::cout << "Soluzione Generata per l'istanza in input:" << solution << std::endl;
    }

    return 0;


    
    


    /* const size_t N = 8;
    const size_t M = 4;
    float adjacency[N][N];
    float preferences[N][M];

    try
    {
        parseCoursesFile(v[1], adjacency, preferences);

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
 */
    return 0;
}