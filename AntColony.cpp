#include "AntColony.hpp"
#include <math.h>
#include <limits>
#include <stdexcept>
#include <algorithm>
#include <unordered_set>
#include <string>
#include <iostream>

template<size_t N, size_t M>
AntColonyCTP<N,M>::AntColonyCTP(float (&η)[N][M], float (&adj)[N][N])
{
    for (size_t i = 0; i < N; ++i)
    {
        for (size_t j = 0; j < M; ++j)
        {
            m_τ[i][j] = TMIN;
            m_η[i][j] = η[i][j];
        }

        for (size_t j = 0; j < N; ++j)
        {
            m_adj[i][j] = adj[i][j];
        }
    }
}


template<size_t N, size_t M>
AntColonyCTP<N,M>::Solution AntColonyCTP<N, M>::solve(size_t k, float α, float β, float ρ,
                                            std::mt19937& rng, const size_t MAX_CALLS, const float GOAL)
{
    m_rng = rng;
    m_calls = 0;

    float best_evaluation = N + 1; // IMPOSSIBLE VALUE, it's guaranteed to change after 1 iteration.
    Solution π; // best solution
    while(m_calls < MAX_CALLS && best_evaluation > GOAL)
    {
        calculate_probabilities(α, β);

        for (size_t i = 0; i < k; i++)
        {
            Solution s;
            construct_solution(s);
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

template<size_t N, size_t M>
void AntColonyCTP<N,M>::calculate_probabilities(const float α, const float β)
{
    for (size_t i = 0; i < N; i++)
    {
        float row_norm = 0.0f;
        for (size_t j = 0; j < M; j++)
        {
            if(m_adj[i][j] == 0) // DO GIVE PROBABILITY 0 TO ADJACENT NODES
                row_norm += (std::powf(m_τ[i][j], α) + std::powf(m_η[i][j], β));
        }
    
        for (size_t j = 0; j < M; j++)
        {
            m_p[i][j] = (std::powf(m_τ[i][j], α) + std::powf(m_η[i][j], β))/ row_norm;
        }
    }
}


template<size_t N, size_t M>
float AntColonyCTP<N, M>::evaluate(const Solution& s)
{
    ++m_calls;

    float penalty = 0;
    for (size_t i = 0; i < N; ++i)
    {
        penalty += m_η[i][s[i]];
    }
    return penalty;
}

template<size_t N, size_t M>
void AntColonyCTP<N, M>::evaporation(const float ρ)
{
    if(ρ <= 0 || ρ >= 1) throw std::invalid_argument("ρ must be in range (0, 1) excluded");
    
    for (size_t i = 0; i < N; i++)
    {
        for (size_t j = 0; j < M; j++)
        {
            m_τ[i][j] = (1-ρ)*m_τ[i][j];   
        }
    }
}

template<size_t N, size_t M>
void AntColonyCTP<N, M>::reinforcment(const float Δ, const Solution& π)
{
    for (size_t i = 0; i < N; i++)
    {
        m_τ[i][π[i]] = m_τ[i][π[i]] + Δ;   
    }
}

template<size_t N, size_t M>
void AntColonyCTP<N, M>::construct_solution(Solution& s)
{   
    s.fill(M);
    std::array<size_t, N> nodes;
    for (size_t i = 0; i < N; i++)
    {
        nodes[i] = i;
    }
    std::shuffle(nodes.begin(), nodes.end(), m_rng);

    for (size_t n = 0; n < N; n++)
    {
        // Select Node at random
        size_t i = nodes[n];
        // Initialize available slots
        std::vector<size_t> slots(M);
        std::vector<float> probabilities(M);
        for (size_t j = 0; j < M; j++)
        {
            slots[j] = j;
            probabilities[j] = m_p[i][j];
        }

        // SELECT BASED ON CALCULATED PROBABILITIES
        std::discrete_distribution<size_t> slot_dist(probabilities.begin(), probabilities.end());
        s[i] = slots[slot_dist(m_rng)];
        



        /* TODO: BETTER GREEDY STRATEGY FOR CONSTRUCTION
        // Remove constrained slots
        for (size_t j = 0; j < N; j++)
        {
            if(m_adj[i][j] && s[j] != M)
            {
                slots.erase(s[j]);
            }   
        }
        // If possible, chose a preferred values
        for (size_t j = 0; j < M; j++)
        {
            if (m_η[i][j] == 0 && slots.contains(j))
            {
                s[i] = j;
                break;
            }
        }
        if (s[i] != M) continue;
        if (slots.empty())
            throw std::logic_error("No possible slots for event " + std::to_string(i));
        std::uniform_int_distribution<size_t> slot_dist(0, static_cast<size_t>(slots.size()) - 1);
        auto it = std::next(slots.begin(), static_cast<std::ptrdiff_t>(slot_dist(m_rng)));
        s[i] = *it; // Assign random value from available ones */
    }
}

template class AntColonyCTP<8ull, 4ull>;