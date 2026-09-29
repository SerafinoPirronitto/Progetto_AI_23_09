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



AntColonyCTP::AntColonyCTP(size_t N, size_t M,
                        const Matrix<bool>& Adj, const Matrix<bool>& Pref)
                        : N(N), M(M), m_adj(Adj), m_pref(Pref), m_τ(N,M), m_τx(N, N)
{}

float AntColonyCTP::η(size_t i, size_t j, const Solution& s)
{
    if (s[i,j]) return 1;
    
    return 1.0f/(1 + 1 - m_pref[i, j]);    
}

float AntColonyCTP::p(size_t i, size_t j, const Solution& s, float α, float β)
{
    if(s.domain()[i, j] == 0) return 0;

    return  std::powf(m_τ[i, j], α) *std::powf(η(i, j, s), β);
}

float AntColonyCTP::ηx(size_t i, size_t j, const Solution& s)
{
    return 1.0f/(1 + 1.0f*m_adj.count_ones(j)/s.domain().count_ones(j));
}
float AntColonyCTP::px(size_t i, size_t j, const Solution& s, float α, float β)
{
    
    return std::powf(m_τx[i, j], α) *std::powf(ηx(i, j, s), β);
}

void AntColonyCTP::reset()
{
    m_calls = 0;

    for (size_t i = 0; i < N; i++)
    {
        for (size_t j = 0; j < M; j++)
        {
            m_τ[i, j] = TMAX;
        }
        for (size_t j = 0; j < N; j++)
        {
            m_τx[i, j] = TMAX;
        }
    }   
}

Solution* AntColonyCTP::construct_solution(float α, float β)
{
    Solution* s = new Solution(N, M, m_adj);
    size_t current_event = std::uniform_int_distribution<size_t>(0, N-1)(m_rng); // SELECT RANDOM EVENT

    // Helper function
    auto construct_prob_dist = [&] (auto prob_func, std::vector<float>& out_prob, bool selecting_event)
    {
        for (size_t j = 0; j < out_prob.size(); j++)
        {
            const bool is_valid_choice = selecting_event
                ? s->data().is_row_zero(j)
                : s->domain()[current_event, j];
            if(is_valid_choice)
            {
                out_prob[j] = (this->*prob_func)(current_event, j, *s, α, β);
            }
        }
        return std::accumulate(out_prob.begin(), out_prob.end(), 0.0f) != 0.0f;
    };


    while(!s->is_complete()) // CONSTRUCT SOLUTION
    {
        // SELECT NEXT UNASSIGNED EVENT
        std::vector<float> event_prob(N, 0);
        if (!construct_prob_dist(px, event_prob, true)) return nullptr;

        std::discrete_distribution<size_t> event_dist = std::discrete_distribution<size_t>(event_prob.begin(), event_prob.end());
        current_event = event_dist(m_rng);


        // SELECT TIME SLOT BASED ON PHEROMON AND HEURISTIC
        std::vector<float> time_prob(M, 0);
        if (!construct_prob_dist(p, time_prob, false)) return nullptr;

        std::discrete_distribution<size_t> time_dist(time_prob.begin(), time_prob.end());
        size_t time_slot = time_dist(m_rng);

        // MAKE THE ASSIGNMENT
        (*s)[current_event, time_slot] = 1;  
    }
    
    return s;
}



Solution AntColonyCTP::solve(size_t k, float α, float β, float ρ,
                            std::mt19937& rng, const size_t MAX_CALLS, const float GOAL)
{
    m_rng = rng;
    reset(); // clean-up pheromons trails and set m_calls to 0

    float best_evaluation = N + 1; // IMPOSSIBLE VALUE, it's guaranteed to change after 1 admissable solution is found.
    Solution* π = nullptr; // best solution so far
    while(m_calls < MAX_CALLS && best_evaluation > GOAL)
    {
        for (size_t ant = 0; ant < k; ++ant)
        {
            Solution* s = construct_solution(α, β);
            if (s == nullptr) continue;

            float score = evaluate(*s);
            if (score < best_evaluation) 
            {
                π = s;
                best_evaluation = score;
            }
        }

        if(π == nullptr)
        {
            std::cerr << "Impossible to construct a valid solution after " << k << " tries.\n Try changing parameters.\n";
            throw new std::runtime_error("No admissable solution found"); 
        }

        evaporation(ρ);
        reinforcment(1 / (1 + best_evaluation), *π);
    }
    
    std::cout << "Solved after " << m_calls << " calls to evaluation function \n";
    std::cout << "Timeslots Pheromones Matrix" << m_τ;
    std::cout << "Events Pheromones Matrix" << m_τx;
    std::cout << "Best solution found has score: " << best_evaluation << std::endl;
    return *π;
}


float AntColonyCTP::evaluate(const Solution& s)
{
    ++m_calls;

    float penalty = 0;
    for (size_t i = 0; i < N; ++i)
    {
        size_t j = s.data().first_one(i);
        if (j >= M)
            throw std::invalid_argument("Cannot evaluate an incomplete solution");
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
            m_τx[i, j] = (1-ρ)*m_τx[i, j];
            if(m_τx[i, j] < TMIN) m_τx[i, j] = TMIN;
        }
    }
}

void AntColonyCTP::reinforcment(float Δ, const Solution& π)
{
    for (size_t i = 0; i < N; i++)
    {
        size_t j = π.data().first_one(i);
        if (j >= M)
            throw std::invalid_argument("Cannot reinforce an incomplete solution");

        m_τ[i, j] += Δ;
        if (m_τ[i, j] > TMAX)
            m_τ[i, j] = TMAX;

        if (i > 0)
        {
            m_τx[π.assignment_order[i-1], π.assignment_order[i]] += Δ;
            if (m_τx[π.assignment_order[i-1], π.assignment_order[i]] > TMAX)
                m_τx[π.assignment_order[i-1], π.assignment_order[i]] = TMAX;   
        }
    }    
}