#pragma once
#include <stdlib.h>
#include <array>
#include <random>


template<size_t N, size_t M>
class AntColonyCTP
{
public:
    using Solution = std::array<size_t, N>;
    const float TMIN = 0.1f;

    AntColonyCTP(float (&η)[N][M], float (&adj)[N][N]);
    Solution solve(size_t k, float α, float β, float ρ, std::mt19937& rng, const size_t MAX_CALLS = 500, const float GOAL = 1);

private:
    // RNG
    std::mt19937 m_rng;

    // Stopping criteria
    size_t m_calls; // calls to evaluation function;
 
    // HARD-CONSTRAINT
    float m_adj[N][N] = {}; // adj[i][j] = 1 if (i,j) in E, else = 0

    // NxM MATRIXES
    float m_p[N][M] = {}; // p[i][j] = probability of selecting j from i 
    float m_τ[N][M] = {}; // τ[i][j] = pheromon on the arc (i, j)
    float m_η[N][M] = {}; // η[i][j] = heuristic strenght of a solution that has j after i

    // METHODS
    float evaluate(const Solution& s);                      // number of non-preferred time-slots
    void reinforcment(const float Δ, const Solution& π);    // update pheromones
    void evaporation(const float ρ);                        // update pheromones
    void construct_solution(Solution& s);                   // constructs valid solution
    void calculate_probabilities(const float α, const float β);
};