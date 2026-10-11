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


    struct parameters
    {
        float α;
        float β;
        float ρ;
    };

    std::vector<float> α_values = {0.5f, 1.0f, 2.0f, 5.0f};
    std::vector<float> β_values = {1.0f, 2.0f, 5.0f, 10.0f};
    std::vector<float> ρ_values = {0.1f, 0.3f, 0.5f, 0.7f};

    float best_overall_evaluation = std::numeric_limits<float>::max();
    float best_num_calls = std::numeric_limits<float>::max();
    parameters best_parameters;
    for (auto α : α_values)
    {
        for (auto β : β_values)
        {
            for (auto ρ : ρ_values)
            {
                float best_evaluation = std::numeric_limits<float>::max();
                float evaluation_sum = 0.0f;
                float num_calls = 0.0f;
                std::cout << "Running ACO with α = " << α << ", β = " << β << ", ρ = " << ρ << std::endl;
                for (uint32_t i = 1; i < 6; i++)
                {
                    std::cout << "Generating solution based on ACO..." << std::endl;
                    std::cout << "Setting random seed to " << i << std::endl;
                    std::mt19937 RNG{i};
                    Solution solution = solver.solve(8, α, β, ρ, RNG, 500, 1);
                    std::cout << "Solution Found:" << solution.data() << std::endl;
                    float score = solver.evaluate(solution);
                    if (score < best_evaluation)
                    {
                        best_evaluation = score;
                    }
                    evaluation_sum += score;
                    num_calls += solver.m_calls;
                }
                float avarage = evaluation_sum / 5.0f;
                float avarege_calls = num_calls / 5.0f;
                std::cout << "Best evaluation for α = " << α << ", β = " << β << ", ρ = " << ρ << " is: " << best_evaluation << std::endl;
                std::cout << "Avarage evaluation for α = " << α << ", β = " << β << ", ρ = " << ρ << " is: " << avarage << std::endl;
                std::cout << "Average number of calls for α = " << α << ", β = " << β << ", ρ = " << ρ << " is: " << avarege_calls << std::endl;

                if (avarage <= best_overall_evaluation && avarege_calls < best_num_calls)
                {
                    best_overall_evaluation = avarage;
                    best_num_calls = avarege_calls;
                    best_parameters = {α, β, ρ};
                }
            }
        }
    }
    std::cout << "Best overall evaluation is: " << best_overall_evaluation << std::endl;
    std::cout << "Best number of calls is: " << best_num_calls << std::endl;
    std::cout << "Best parameters are: α = " << best_parameters.α << ", β = " << best_parameters.β << ", ρ = " << best_parameters.ρ << std::endl;       

    return 0;
}