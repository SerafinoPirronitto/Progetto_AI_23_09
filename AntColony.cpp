#include "AntColony.hpp"
#include <math.h>
#include <limits>
#include <stdexcept>
#include <algorithm>
#include <string>
#include <iostream>
#include <cassert>
#include <ranges>



AntColonyCTP::AntColonyCTP(std::size_t N, std::size_t M,
                        const Matrix<bool>& Adj, const Matrix<bool>& Pref)
                        : N(N), M(M), m_adj(Adj), m_pref(Pref), m_τ(N,M), m_η(N,M), m_p(N,M)
{
    // HEURISTIC WEIGHTS
    const float w1 = 1.0f;
    const float w2 = 1.0f;

    for (size_t i = 0; i < N; i++)
    {
        for (size_t j = 0; j < M; j++)
        {
            m_τ[i, j] = TMIN;
            m_η[i, j] = w1*m_adj.degree(i)/m_pref.degree(i) + w2*m_pref[i, j];
        }
    }   
}


AntColonyCTP::Solution AntColonyCTP::solve(size_t k, float α, float β, float ρ,
                                            std::mt19937& rng, const size_t MAX_CALLS, const float GOAL)
{
    m_rng = rng;
    m_calls = 0;

    float best_evaluation = N + 1; // IMPOSSIBLE VALUE, it's guaranteed to change after 1 iteration.
    Solution π; // best solution so far
    while(m_calls < MAX_CALLS && best_evaluation > GOAL)
    {
        calculate_probabilities(α, β); // Based on pheromon's τ and heuristic η

        for (size_t i = 0; i < k; i++)
        {
            Solution s = construct_solution();
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

void AntColonyCTP::calculate_probabilities(float α, float β)
{
    for (size_t i = 0; i < N; i++)
    {
        float row_norm = 0.0f;
        for (size_t j = 0; j < M; j++)
        {
            row_norm += (std::powf(m_τ[i, j], α) * std::powf(m_η[i, j], β));
        }
    
        for (size_t j = 0; j < M; j++)
        {
            m_p[i, j] = (std::powf(m_τ[i, j], α) * std::powf(m_η[i, j], β))/ row_norm;
        }
    }
}


float AntColonyCTP::evaluate(const Solution& s)
{
    ++m_calls;

    float penalty = 0;
    for (size_t i = 0; i < N; ++i)
    {
        assert(s.degree(i) == 1 && "valid solutions only have one time slot for each event");
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
        }
    }
}

void AntColonyCTP::reinforcment(const float Δ, const Solution& π)
{
    for (size_t i = 0; i < N; i++)
    {
        size_t j = 0;
        for (size_t j = 0; j < M; j++)
            if (π[i , j]) break;
        m_τ[i, j] = m_τ[i, j] + Δ;   
    }
}


std::vector<float> AntColonyCTP::calculate_probability(const std::vector<size_t>& events)
{
    float nomr_factor = 0.0f;
    for(const auto& e : events)
    {
        nomr_factor += m_adj.degree(e);
    }
    std::vector<float> prob_dist(events.size());
    for(size_t i = 0; i < events.size(); i++)
    {
        prob_dist[i] = m_adj.degree(events[i])/nomr_factor;
    }
    return prob_dist;
}

AntColonyCTP::Solution AntColonyCTP::construct_solution()
{   
    Solution s(N, M);

    std::vector<size_t> unassigned_events(N);
    for (size_t i = 0; i < N; i++)
    {
        unassigned_events[i] = i;
    }

    for (size_t i = 0; i < N; i++)
    {
        // SELECT EVENT
        auto event_probabilities = calculate_probability(unassigned_events);
        std::discrete_distribution<size_t> prob_event_dist(event_probabilities.begin(), event_probabilities.end());
        size_t index = prob_event_dist(m_rng);
        size_t random_event = unassigned_events[index];
        unassigned_events.erase(unassigned_events.begin() + index);

        // ASSIGN TIME SLOT
        auto time_probabilities = m_p[random_event];
        std::discrete_distribution<size_t> prob_time_dist(time_probabilities.begin(), time_probabilities.end());
        size_t random_time = prob_time_dist(m_rng);

        s[random_event, random_time] = 1;
    }
    return s;
}
