#include <iostream>
#include <vector>
#include <string>
#include "../include/Benchmark.h"
#include <fstream>

int main()
{
    std::vector<TestConfiguration> configurations;

    // configurazione dei test: num_series, series_length, query_length
    std::vector<std::tuple<int, int, int>> test_cases = {
        {100, 500, 100},
        {500, 1000, 100},
        {1000, 5000, 100}};

    for (const auto &[num_series, series_length, query_length] : test_cases)
    {
        TestConfiguration config;
        config.num_series = num_series;
        config.series_length = series_length;
        config.query_length = query_length;
        config.dataset_path = "src/utils/data/timeseries/timeseries.csv";
        config.query_path = "src/utils/data/query/query.csv";
        configurations.push_back(config);
    }

    std::cout << "=== BENCHMARK PERFORMANCE COMPARISON ===" << std::endl;
    std::cout << "Running " << configurations.size() << " tests..." << std::endl;
    std::cout << "=========================================" << std::endl;

    auto results = Benchmark::run_multiple_tests(configurations);

    std::filesystem::create_directories("output/benchmark_results");
    std::string output_filename = "output/benchmark_results/sequential.json";
    std::ofstream output_file(output_filename);
    output_file << results.dump(2);
    output_file.close();


    std::cout << "\n=== RESULTS SUMMARY ===" << std::endl;
    for (const auto &test : results["tests"])
    {
        std::cout << "Test: " << test["test_name"] << std::endl;
        std::cout << "  SoA: " << test["results"]["soa"]["execution_time_ms"] << " ms" << std::endl;
        std::cout << "  AoS: " << test["results"]["aos"]["execution_time_ms"] << " ms" << std::endl;
        std::cout << "  Speedup: " << test["performance"]["speedup"] << "x" << std::endl;
        std::cout << "  Match: " << (test["performance"]["results_match"] ? "true" : "false") << std::endl;
        std::cout << std::endl;
    }

    std::cout << "Results saved to: " << output_filename << std::endl;

    return 0;
}