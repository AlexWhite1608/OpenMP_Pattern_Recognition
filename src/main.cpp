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

    std::vector<int> thread_counts = {1, 2, 4, 8, 16, 32};

    std::vector<TestConfiguration> configurations;

    // configurazione dei test: num_series, series_length, query_length
    std::vector<std::tuple<int, int, int>> test_cases = {
        // Casi estremi per outer parallelization (molte serie corte)
        {1000, 100, 50},
        {2000, 200, 50},

        // // Casi estremi per inner parallelization (poche serie lunghe)
        {10, 50000, 50},
        {5, 100000, 50},

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
    std::cout << std::endl;
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

        if (test.contains("thread_results") && test["thread_results"].contains("1"))
        {
            const auto &single_thread = test["thread_results"]["1"];

            std::cout << "  SoA Results (avg of " << test["configuration"]["num_runs"] << " runs):" << std::endl;

            if (single_thread.contains("soa") && single_thread["soa"].contains("sequential"))
            {
                const auto &soa_seq = single_thread["soa"]["sequential"];
                std::cout << "    Sequential: " << soa_seq["mean_execution_time_ms"]
                          << " ± " << soa_seq["std_deviation_ms"] << " ms" << std::endl;
            }

            // Per parallel outer/inner, usa i risultati con più thread (es. 16)
            if (test["thread_results"].contains("16"))
            {
                const auto &multi_thread = test["thread_results"]["16"];

                if (multi_thread.contains("soa") && multi_thread["soa"].contains("parallel_outer"))
                {
                    const auto &soa_outer = multi_thread["soa"]["parallel_outer"];
                    std::cout << "    Parallel Outer (16 threads): " << soa_outer["mean_execution_time_ms"]
                              << " ± " << soa_outer["std_deviation_ms"] << " ms"
                              << " (speedup: " << soa_outer["speedup"] << "x)" << std::endl;
                }

                if (multi_thread.contains("soa") && multi_thread["soa"].contains("parallel_inner"))
                {
                    const auto &soa_inner = multi_thread["soa"]["parallel_inner"];
                    std::cout << "    Parallel Inner (16 threads): " << soa_inner["mean_execution_time_ms"]
                              << " ± " << soa_inner["std_deviation_ms"] << " ms"
                              << " (speedup: " << soa_inner["speedup"] << "x)" << std::endl;
                }
            }

            // AoS Results
            std::cout << "  AoS Results (avg of " << test["configuration"]["num_runs"] << " runs):" << std::endl;

            if (single_thread.contains("aos") && single_thread["aos"].contains("sequential"))
            {
                const auto &aos_seq = single_thread["aos"]["sequential"];
                std::cout << "    Sequential: " << aos_seq["mean_execution_time_ms"]
                          << " ± " << aos_seq["std_deviation_ms"] << " ms" << std::endl;
            }

            if (test["thread_results"].contains("16"))
            {
                const auto &multi_thread = test["thread_results"]["16"];

                if (multi_thread.contains("aos") && multi_thread["aos"].contains("parallel_outer"))
                {
                    const auto &aos_outer = multi_thread["aos"]["parallel_outer"];
                    std::cout << "    Parallel Outer (16 threads): " << aos_outer["mean_execution_time_ms"]
                              << " ± " << aos_outer["std_deviation_ms"] << " ms"
                              << " (speedup: " << aos_outer["speedup"] << "x)" << std::endl;
                }

                if (multi_thread.contains("aos") && multi_thread["aos"].contains("parallel_inner"))
                {
                    const auto &aos_inner = multi_thread["aos"]["parallel_inner"];
                    std::cout << "    Parallel Inner (16 threads): " << aos_inner["mean_execution_time_ms"]
                              << " ± " << aos_inner["std_deviation_ms"] << " ms"
                              << " (speedup: " << aos_inner["speedup"] << "x)" << std::endl;
                }
            }

            // Analysis
            if (single_thread.contains("analysis") && single_thread["analysis"].contains("soa_vs_aos_sequential"))
            {
                std::cout << "  Analysis:" << std::endl;
                std::cout << "    SoA vs AoS (Sequential): " << single_thread["analysis"]["soa_vs_aos_sequential"] << "x" << std::endl;
            }
        }

        std::cout << std::endl;
    }

    return 0;
}