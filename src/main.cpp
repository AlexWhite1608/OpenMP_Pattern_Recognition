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

    std::vector<int> thread_counts = {1, 2, 4, 8, 16};

    std::vector<TestConfiguration> configurations;

    // configurazione dei test: num_series, series_length, query_length
    std::vector<std::tuple<int, int, int>> test_cases = {
        // Casi estremi per outer parallelization (molte serie corte)
        {1000, 100, 50},
        {2000, 200, 50},

        // Casi estremi per inner parallelization (poche serie lunghe)
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

        std::cout << "\n"
                  << std::string(60, '=') << std::endl;
        std::cout << "Test: " << test["test_name"] << std::endl;
        std::cout << "Config: " << test["configuration"]["num_series"] << " series × "
                  << test["configuration"]["series_length"] << " points" << std::endl;
        std::cout << std::string(60, '-') << std::endl;

        // Ottieni baseline sequenziali (1 thread)
        double soa_sequential = 0.0, aos_sequential = 0.0;
        if (test["thread_results"].contains("1"))
        {
            const auto &baseline = test["thread_results"]["1"];
            if (baseline.contains("soa") && baseline["soa"].contains("sequential"))
                soa_sequential = baseline["soa"]["sequential"]["mean_execution_time_ms"];
            if (baseline.contains("aos") && baseline["aos"].contains("sequential"))
                aos_sequential = baseline["aos"]["sequential"]["mean_execution_time_ms"];
        }

        // Stampa baseline sequenziali
        std::cout << "SEQUENTIAL:" << std::endl;
        std::cout << "  SoA: " << std::fixed << std::setprecision(1) << soa_sequential << " ms" << std::endl;
        std::cout << "  AoS: " << std::fixed << std::setprecision(1) << aos_sequential << " ms" << std::endl;

        if (soa_sequential > 0 && aos_sequential > 0)
        {
            double advantage = aos_sequential / soa_sequential;
            std::cout << "  → SoA is " << std::setprecision(2) << advantage << "x faster" << std::endl;
        }
        std::cout << std::endl;

        // Trova i migliori risultati paralleli per ogni thread count
        std::cout << "PARALLEL RESULTS:" << std::endl;
        std::cout << std::left << std::setw(8) << "Threads"
                  << std::setw(15) << "SoA Best (ms)"
                  << std::setw(12) << "Speedup"
                  << std::setw(15) << "AoS Best (ms)"
                  << std::setw(12) << "Speedup" << std::endl;
        std::cout << std::string(62, '-') << std::endl;

        for (const auto &[thread_count_str, thread_data] : test["thread_results"].items())
        {
            int threads = std::stoi(thread_count_str);
            if (threads == 1)
                continue; // Skip sequential già mostrato

            // Trova il migliore SoA (outer vs inner)
            double soa_best_time = 99999.0;
            double soa_best_speedup = 1.0;

            if (thread_data.contains("soa"))
            {
                if (thread_data["soa"].contains("parallel_outer"))
                {
                    double time = thread_data["soa"]["parallel_outer"]["mean_execution_time_ms"];
                    double speedup = thread_data["soa"]["parallel_outer"]["speedup"];
                    if (time < soa_best_time)
                    {
                        soa_best_time = time;
                        soa_best_speedup = speedup;
                    }
                }
                if (thread_data["soa"].contains("parallel_inner"))
                {
                    double time = thread_data["soa"]["parallel_inner"]["mean_execution_time_ms"];
                    double speedup = thread_data["soa"]["parallel_inner"]["speedup"];
                    if (time < soa_best_time)
                    {
                        soa_best_time = time;
                        soa_best_speedup = speedup;
                    }
                }
            }

            // Trova il migliore AoS (outer vs inner)
            double aos_best_time = 99999.0;
            double aos_best_speedup = 1.0;

            if (thread_data.contains("aos"))
            {
                if (thread_data["aos"].contains("parallel_outer"))
                {
                    double time = thread_data["aos"]["parallel_outer"]["mean_execution_time_ms"];
                    double speedup = thread_data["aos"]["parallel_outer"]["speedup"];
                    if (time < aos_best_time)
                    {
                        aos_best_time = time;
                        aos_best_speedup = speedup;
                    }
                }
                if (thread_data["aos"].contains("parallel_inner"))
                {
                    double time = thread_data["aos"]["parallel_inner"]["mean_execution_time_ms"];
                    double speedup = thread_data["aos"]["parallel_inner"]["speedup"];
                    if (time < aos_best_time)
                    {
                        aos_best_time = time;
                        aos_best_speedup = speedup;
                    }
                }
            }

            // Stampa risultati con formattazione corretta
            std::cout << std::left
                      << std::setw(8) << threads
                      << std::setw(15) << std::fixed << std::setprecision(1) << soa_best_time
                      << std::setw(12) << std::setprecision(2) << soa_best_speedup
                      << std::setw(15) << std::setprecision(1) << aos_best_time
                      << std::setw(12) << std::setprecision(2) << aos_best_speedup << std::endl;
        }

        // Trova il migliore assoluto
        double best_speedup = 1.0;
        int best_threads = 1;
        std::string best_type = "Sequential";

        for (const auto &[thread_count_str, thread_data] : test["thread_results"].items())
        {
            int threads = std::stoi(thread_count_str);
            if (threads == 1)
                continue;

            std::vector<std::pair<std::string, double>> candidates = {
                {"SoA-Outer", 0}, {"SoA-Inner", 0}, {"AoS-Outer", 0}, {"AoS-Inner", 0}};

            if (thread_data.contains("soa"))
            {
                if (thread_data["soa"].contains("parallel_outer"))
                    candidates[0].second = thread_data["soa"]["parallel_outer"]["speedup"];
                if (thread_data["soa"].contains("parallel_inner"))
                    candidates[1].second = thread_data["soa"]["parallel_inner"]["speedup"];
            }
            if (thread_data.contains("aos"))
            {
                if (thread_data["aos"].contains("parallel_outer"))
                    candidates[2].second = thread_data["aos"]["parallel_outer"]["speedup"];
                if (thread_data["aos"].contains("parallel_inner"))
                    candidates[3].second = thread_data["aos"]["parallel_inner"]["speedup"];
            }

            for (const auto &[type, speedup] : candidates)
            {
                if (speedup > best_speedup)
                {
                    best_speedup = speedup;
                    best_threads = threads;
                    best_type = type;
                }
            }
        }

        std::cout << "\nBEST OVERALL: " << best_type << " with " << best_threads
                  << " threads → " << std::fixed << std::setprecision(2) << best_speedup << "x speedup" << std::endl;
        std::cout << std::endl;
    }

    std::cout << "Results saved to: " << output_filename << std::endl;
    return 0;
}