#include <iostream>
#include <vector>
#include <string>
#include "../include/Benchmark.h"
#include <fstream>
#include <filesystem>

int main()
{
    const int NUM_RUNS = 5;
    const int NUM_THREADS = 8;
    omp_set_num_threads(NUM_THREADS);

    std::vector<TestConfiguration> configurations;

    // configurazione dei test: num_series, series_length, query_length
    std::vector<std::tuple<int, int, int>> test_cases = {
        // Casi estremi per outer parallelization (molte serie corte)
        {10000, 150, 50}, 
        {20000, 200, 50}, 

        // Casi estremi per inner parallelization (poche serie lunghe)
        {10, 50000, 100}, 
        {5, 100000, 200}, 
    };

    for (const auto &[num_series, series_length, query_length] : test_cases)
    {
        TestConfiguration config;
        config.num_series = num_series;
        config.series_length = series_length;
        config.query_length = query_length;
        config.num_runs = NUM_RUNS;
        configurations.push_back(config);
    }

    std::cout << "=== BENCHMARK PERFORMANCE COMPARISON ===" << std::endl;
    std::cout << "Running " << configurations.size() << " tests..." << std::endl;
    std::cout << "Benchmark Configuration:" << std::endl;
    std::cout << "  Number of runs per test: " << NUM_RUNS << std::endl;
    std::cout << "  OpenMP max threads: " << omp_get_max_threads() << std::endl;
    std::cout << "  Number of CPU cores: " << omp_get_num_procs() << std::endl;
    std::cout << "=========================================" << std::endl;

    auto results = Benchmark::run_multiple_tests(configurations);

    std::filesystem::create_directories("output/benchmark_results");
    std::string output_filename = "output/benchmark_results/parallelization_analysis.json";
    std::ofstream output_file(output_filename);
    output_file << results.dump(2);
    output_file.close();

    std::cout << "\n=== RESULTS SUMMARY ===" << std::endl;
    for (const auto &test : results["tests"])
    {
        if (test.contains("error"))
        {
            std::cout << "Test: " << test["test_name"] << " - ERROR: " << test["error"] << std::endl;
            continue;
        }

        std::cout << "Test: " << test["test_name"] << std::endl;
        std::cout << "Configuration: " << test["configuration"]["num_series"] << " series × "
                  << test["configuration"]["series_length"] << " points" << std::endl;

        // SoA
        std::cout << "  SoA Results (avg of " << test["configuration"]["num_runs"] << " runs):" << std::endl;
        std::cout << "    Sequential: " << test["results"]["soa"]["sequential"]["mean_execution_time_ms"]
                  << " ± " << test["results"]["soa"]["sequential"]["std_deviation_ms"] << " ms" << std::endl;
        std::cout << "    Parallel Outer: " << test["results"]["soa"]["parallel_outer"]["mean_execution_time_ms"]
                  << " ± " << test["results"]["soa"]["parallel_outer"]["std_deviation_ms"] << " ms"
                  << " (speedup: " << test["results"]["soa"]["parallel_outer"]["speedup"] << "x)" << std::endl;
        std::cout << "    Parallel Inner: " << test["results"]["soa"]["parallel_inner"]["mean_execution_time_ms"]
                  << " ± " << test["results"]["soa"]["parallel_inner"]["std_deviation_ms"] << " ms"
                  << " (speedup: " << test["results"]["soa"]["parallel_inner"]["speedup"] << "x)" << std::endl;

        // AoS
        std::cout << "  AoS Results (avg of " << test["configuration"]["num_runs"] << " runs):" << std::endl;
        std::cout << "    Sequential: " << test["results"]["aos"]["sequential"]["mean_execution_time_ms"]
                  << " ± " << test["results"]["aos"]["sequential"]["std_deviation_ms"] << " ms" << std::endl;
        std::cout << "    Parallel Outer: " << test["results"]["aos"]["parallel_outer"]["mean_execution_time_ms"]
                  << " ± " << test["results"]["aos"]["parallel_outer"]["std_deviation_ms"] << " ms"
                  << " (speedup: " << test["results"]["aos"]["parallel_outer"]["speedup"] << "x)" << std::endl;
        std::cout << "    Parallel Inner: " << test["results"]["aos"]["parallel_inner"]["mean_execution_time_ms"]
                  << " ± " << test["results"]["aos"]["parallel_inner"]["std_deviation_ms"] << " ms"
                  << " (speedup: " << test["results"]["aos"]["parallel_inner"]["speedup"] << "x)" << std::endl;

        std::cout << "  Analysis:" << std::endl;
        std::cout << "    SoA vs AoS (Sequential): " << test["analysis"]["soa_vs_aos_sequential"] << "x" << std::endl;

        std::cout << std::endl;
    }

    std::cout << "Results saved to: " << output_filename << std::endl;

    return 0;
}