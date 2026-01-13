#include <iostream>
#include <vector>
#include <string>
#include "../include/Benchmark.h"
#include <fstream>
#include <filesystem>
#include <iomanip>

int main()
{
    const int NUM_RUNS = 10;

    std::vector<int> thread_counts = {2, 4, 8, 16, 24, 32, 48, 64};

    std::vector<TestConfiguration> configurations;

    // configurazione dei test: num_series, series_length, query_length
    std::vector<std::tuple<int, int, int>> test_cases = {
        {1000, 100, 50},
        {5000, 100, 50},

        {10, 5000, 50},
        {5, 10000, 50},
    };

    for (const auto &[num_series, series_length, query_length] : test_cases)
    {
        TestConfiguration config;
        config.num_series = num_series;
        config.series_length = series_length;
        config.query_length = query_length;
        config.num_runs = NUM_RUNS;
        config.thread_counts = thread_counts;
        configurations.push_back(config);
    }

    auto results = Benchmark::run_multiple_tests(configurations);

    std::filesystem::create_directories("output/benchmark_results");
    std::string output_filename = "output/benchmark_results/parallelization_analysis.json";
    std::ofstream output_file(output_filename);
    output_file << results.dump(2);
    output_file.close();

    std::cout << "\nResults saved to: " << output_filename << std::endl;
    return 0;
}