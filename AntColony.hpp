#pragma once

#include "Matrix.hpp"
#include "Solution.hpp"
#include <random>


class AntColonyCTP
{
public:
    const float TMIN = 0.01f;
    const float TMAX = 1.0f;
    const size_t N;
    const size_t M;
    // Stopping criteria
    size_t m_calls; // calls to evaluation function;

    /* Bulilds An instance of CTP, where Adj(NxN) and Pref(NxM)
       express HARD and SOFT constraint respectively*/
    AntColonyCTP(size_t N, size_t M, const Matrix<bool>& Adj, const Matrix<bool>& Pref);
    // Solves the instance of CTP using ACO with the provided parameter and random generator
    Solution solve(size_t k, float α, float β, float ρ, std::mt19937& rng,
                     const size_t MAX_CALLS = 500, const float GOAL = 1);
    float evaluate(const Solution& s);                       // number of non-preferred time-slots

private:
    // RNG
    std::mt19937 m_rng;

   
 
    // CONSTRAINTS
    Matrix<bool> m_adj;  // HARD Constraint Table
    Matrix<bool> m_pref; // SOFT Constraint Table

    // PHEROMONS
    Matrix<float> m_τ_time;      // τ[i][j] = pheromon on the arc <x_i, v_j>
    Matrix<float> m_τ_event;     // τx[i][j] = pheromon on the vertex x_j at stage i

    // METHODS
    void reinforcment(float Δ, const Solution& π);           // update pheromones
    void evaporation(float ρ);                               // update pheromones
    Solution* construct_solution(float α, float β);          // constructs valid solution
    void reset();                                            // resets internal state of the solver between runs

    float p(size_t i, size_t j, const Solution& s, float α, float β); // Transition probability for each vertex <x_i, v_j> at assignment s
    float η(size_t i, size_t j, const Solution& s); // Heuristic strenght of chosing vertex <x_i, v_j> for solution s
    float px(size_t i, size_t j, const Solution& s, float α, float β); // Transition probability from x_i to x_j at assignment s
    float ηx(size_t i, size_t j, const Solution& s); // Heuristic strenght of chosing vertex x_j at stage i for solution s

};