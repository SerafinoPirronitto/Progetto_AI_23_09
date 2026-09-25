#pragma once

#include "Matrix.hpp"
#include <random>


class AntColonyCTP
{
public:
    using Solution = Matrix<bool>;
    const float TMIN = 0.1f;
    const std::size_t N;
    const std::size_t M;

    /* Bulilds An instance of CTP, where Adj(NxN) and Pref(NxM)
       express HARD and SOFT constraint respectively*/
    AntColonyCTP(std::size_t N, std::size_t M, const Matrix<bool>& Adj, const Matrix<bool>& Pref);
    // Solves the instance of CTP using ACO with the provided parameter and random generator
    Solution solve(size_t k, float α, float β, float ρ, std::mt19937& rng,
                     const size_t MAX_CALLS = 500, const float GOAL = 1);

private:
    // RNG
    std::mt19937 m_rng;

    // Stopping criteria
    size_t m_calls; // calls to evaluation function;
 
    // HARD-CONSTRAINT
    Matrix<bool> m_adj;  // HARD Constraint Table
    Matrix<bool> m_pref; // SOFT Constraint Table

    // NxM MATRIXES
    Matrix<float> m_p; // p[i][j] = probability of selecting j from i 
    Matrix<float> m_τ; // τ[i][j] = pheromon on the arc (i, j)
    Matrix<float> m_η; // η[i][j] = heuristic strenght of a solution that has j after i

    // METHODS
    float evaluate(const Solution& s);                      // number of non-preferred time-slots
    void reinforcment(float Δ, const Solution& π);          // update pheromones
    void evaporation(float ρ);                              // update pheromones
    Solution construct_solution();                   // constructs valid solution
    void calculate_probabilities(float α, float β);         // updated probability matrix
    std::vector<float> calculate_probability(const std::vector<size_t>& events); // calculates the probability of choosing each event for assignment
};