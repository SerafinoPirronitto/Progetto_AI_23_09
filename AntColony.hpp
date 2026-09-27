#pragma once

#include "Matrix.hpp"
#include "Solution.hpp"
#include <random>


class AntColonyCTP
{
public:
    const float TMIN ;
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
    Matrix<float> m_τ; // τ[i][j] = pheromon on the vertex<x_i, v_j>

    // METHODS
    float evaluate(const Solution& s);                      // number of non-preferred time-slots
    void reinforcment(float Δ, const Solution& π);          // update pheromones
    void evaporation(float ρ);                              // update pheromones
    Solution construct_solution();                          // constructs valid solution

    float p(std::size_t i, std::size_t j, const Solution& s, float α, float β); // Transition probability for each vertex <x_i, v_j> at assignment s
    float η(std::size_t i, std::size_t j, const Solution& s); // Heuristic strenght of chosing vertex <x_i, v_j> for solution s

};