#include <iostream>
#include <vector>
#include <string>
#include "../include/Benchmark.h"
#include <fstream>
#include <filesystem>

int main()
{
    const int NUM_RUNS = 10;

    std::vector<int> thread_counts = {1, 2, 4, 8, 16};

    std::vector<TestConfiguration> configurations;

    // configurazione dei test: num_series, series_length, query_length
    std::vector<std::tuple<int, int, int>> test_cases = {
        // Casi estremi per outer parallelization (molte serie corte)
        {1000, 150, 50},
        {2000, 200, 50},

        // // Casi estremi per inner parallelization (poche serie lunghe)
        {10, 50000, 50},
        {5, 100000, 50},

        // {1, 1000000, 100},
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

    std::cout << "=== BENCHMARK PERFORMANCE COMPARISON ===" << std::endl;
    std::cout << "Running " << configurations.size() << " tests..." << std::endl;
    std::cout << "Benchmark Configuration:" << std::endl;
    std::cout << "  Number of runs per test: " << NUM_RUNS << std::endl;
    std::cout << "  Thread counts to test: ";
    for (int t : thread_counts)
        std::cout << t << " ";
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

        std::cout << "\n=== THREAD SCALING SUMMARY ===" << std::endl;
        for (const auto &test : results["tests"])
        {
            if (test.contains("error"))
            {
                std::cout << "Test: " << test["test_name"] << " - ERROR: " << test["error"] << std::endl;
                continue;
            }

            std::cout << "\n"
                      << std::string(80, '=') << std::endl;
            std::cout << "Test: " << test["test_name"] << std::endl;
            std::cout << "Configuration: " << test["configuration"]["num_series"] << " series × "
                      << test["configuration"]["series_length"] << " points (query: "
                      << test["configuration"]["query_length"] << ")" << std::endl;
            std::cout << std::string(80, '-') << std::endl;

            // Summary baseline
            std::cout << "Sequential Baseline:" << std::endl;
            std::cout << "  SoA: " << test["summary"]["baseline_soa_time_ms"] << " ms" << std::endl;
            std::cout << "  AoS: " << test["summary"]["baseline_aos_time_ms"] << " ms" << std::endl;
            std::cout << "  SoA Advantage: " << test["summary"]["baseline_soa_vs_aos"] << "x" << std::endl;
            std::cout << std::endl;

            // Thread scaling results
            std::cout << std::left << std::setw(8) << "Threads"
                      << std::setw(15) << "SoA Outer (ms)"
                      << std::setw(12) << "Speedup"
                      << std::setw(12) << "Efficiency"
                      << std::setw(15) << "AoS Outer (ms)"
                      << std::setw(12) << "Speedup"
                      << std::setw(12) << "Efficiency" << std::endl;
            std::cout << std::string(80, '-') << std::endl;

            for (const auto &[thread_count, thread_data] : test["thread_results"].items())
            {
                int threads = std::stoi(thread_count);

                if (threads == 1)
                {
                    // Sequential
                    double soa_time = thread_data["soa"]["sequential"]["mean_execution_time_ms"];
                    double aos_time = thread_data["aos"]["sequential"]["mean_execution_time_ms"];

                    std::cout << std::left << std::setw(8) << threads
                              << std::setw(15) << std::fixed << std::setprecision(1) << soa_time
                              << std::setw(12) << "1.00x"
                              << std::setw(12) << "100.0%"
                              << std::setw(15) << std::fixed << std::setprecision(1) << aos_time
                              << std::setw(12) << "1.00x"
                              << std::setw(12) << "100.0%" << std::endl;
                }
                else
                {
                    // Parallel
                    double soa_time = thread_data["soa"]["parallel_outer"]["mean_execution_time_ms"];
                    double soa_speedup = thread_data["soa"]["parallel_outer"]["speedup"];

                    double aos_time = thread_data["aos"]["parallel_outer"]["mean_execution_time_ms"];
                    double aos_speedup = thread_data["aos"]["parallel_outer"]["speedup"];

                    std::cout << std::left << std::setw(8) << threads
                              << std::setw(15) << std::fixed << std::setprecision(1) << soa_time
                              << std::setw(12) << std::setprecision(2) << soa_speedup << "x"
                              << std::setw(15) << std::setprecision(1) << aos_time
                              << std::setw(12) << std::setprecision(2) << aos_speedup << "x";
                }
            }
        }

        std::cout << "Results saved to: " << output_filename << std::endl;

        return 0;
    }
}