#include "AntColony.hpp"
#include <math.h>
#include <limits>
#include <stdexcept>
#include <algorithm>
#include <string>
#include <iostream>
#include <cassert>
#include <ranges>
#include <numeric>



AntColonyCTP::AntColonyCTP(std::size_t N, std::size_t M,
                        const Matrix<bool>& Adj, const Matrix<bool>& Pref)
                        : N(N), M(M), m_adj(Adj), m_pref(Pref), m_τ(N,M), TMIN(0.01f)
{

    for (size_t i = 0; i < N; i++)
    {
        for (size_t j = 0; j < M; j++)
        {
            m_τ[i, j] = 1;
        }
    }   
}

float AntColonyCTP::η(std::size_t i, std::size_t j, const Solution& s)
{
    if (s[i,j])
        return 1;
    
    return 1.0f/(1 + 1 - m_pref[i, j]);    
}

float AntColonyCTP::p(std::size_t i, std::size_t j, const Solution& s, float α, float β)
{
    if(s.domain()[i, j] == 0) return 0;

    return  std::powf(m_τ[i, j], α) *std::powf(η(i, j, s), β); ;
}



Solution AntColonyCTP::solve(size_t k, float α, float β, float ρ,
                            std::mt19937& rng, const size_t MAX_CALLS, const float GOAL)
{
    m_rng = rng;
    m_calls = 0;

    float best_evaluation = N + 1; // IMPOSSIBLE VALUE, it's guaranteed to change after 1 iteration.
    Solution π(N, M, m_adj); // best solution so far
    while(m_calls < MAX_CALLS && best_evaluation > GOAL)
    {
        for (size_t ant = 0; ant < k; ++ant)
        {
            Solution s(N, M, m_adj);
            std::uniform_int_distribution<size_t> event_dist(0, N-1);
            std::size_t current_event = event_dist(m_rng); // SELECT RANDOM EVENT
            bool invalid = false;
            for (size_t i = 0; i < N; i++) // CONSTRUCT SOLUTION
            {
                std::vector<float> probabilities(M, 0);
                for (size_t j = 0; j < M; j++)
                {
                    probabilities[j] = p(current_event, j, s, α, β);
                }
                if (std::accumulate(probabilities.begin(), probabilities.end(), 0.0f) == 0.0f)
                {
                    std::cerr << "Impossible assignment for partial solution " << ant << " at event " << current_event+1 <<  s.domain();
                    invalid = true;
                    break; 
                }
                // SELECT TIME SLOT BASED ON PHEROMON AND HEURISTIC
                std::discrete_distribution<size_t> time_dist(probabilities.begin(), probabilities.end());
                std::size_t time_slot = time_dist(m_rng);
                s[current_event, time_slot] = 1;  
                
                // SELECT NEXT UNASSIGNED EVENT
                std::size_t next_event;
                do { 
                    next_event = event_dist(m_rng);
                }  while (s.data().degree(next_event) != 0 && i != 7);
                current_event = next_event;
            }
            if (invalid)
            {
                continue; // IMPOSSIBLE ASSIGNMENT, THROW AWAY SOLUTION
            }
            

            float score = evaluate(s);
            if (score < best_evaluation) 
            {
                π = s;
                best_evaluation = score;
            }
        }

        evaporation(ρ);
        reinforcment(1/best_evaluation, π);
    }
    
    std::cout << "Solved after " << m_calls << " calls to evaluation function \n";
    std::cout << "Best solution found has score: " << best_evaluation << std::endl;
    return π;
}


float AntColonyCTP::evaluate(const Solution& s)
{
    ++m_calls;

    float penalty = 0;
    for (size_t i = 0; i < N; ++i)
    {
        size_t j = 0;
        for(; j < M; j++)
        {
            if (s[i, j]) break; // j = Time slot chosen for event i
        }

        penalty += 1 - m_pref[i, j]; // If it's preferred, give no penalty
    }
    return penalty;
}


void AntColonyCTP::evaporation(float ρ)
{
    if(ρ <= 0 || ρ >= 1) throw std::invalid_argument("ρ must be in range (0, 1) excluded");
    
    for (size_t i = 0; i < N; i++)
    {
        for (size_t j = 0; j < M; j++)
        {
            m_τ[i, j] = (1-ρ)*m_τ[i, j];
            if(m_τ[i, j] < TMIN) m_τ[i, j] = TMIN;   
        }
    }
}

void AntColonyCTP::reinforcment(float Δ, const Solution& π)
{
    for (size_t i = 0; i < N; i++)
    {
        size_t j = 0;
        for (size_t j = 0; j < M; j++)
            if (π[i , j]) break;
        m_τ[i, j] = m_τ[i, j] + Δ;   
    }
}


Solution AntColonyCTP::construct_solution()
{   
    Solution s(N, M, m_adj);
    return s;
}
