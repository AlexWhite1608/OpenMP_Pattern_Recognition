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

    std::cout << "\n=== DETAILED RESULTS SUMMARY ===" << std::endl;
    for (const auto &test : results["tests"])
    {
        if (test.contains("error"))
        {
            std::cout << "Test: " << test["test_name"] << " - ERROR: " << test["error"] << std::endl;
            continue;
        }

        std::cout << "\n"
                  << std::string(100, '=') << std::endl;
        std::cout << "TEST: " << test["test_name"] << std::endl;
        std::cout << "Configuration: " << test["configuration"]["num_series"] << " series × "
                  << test["configuration"]["series_length"] << " points" << std::endl;
        std::cout << std::string(100, '-') << std::endl;

        double soa_sequential = 0.0, aos_sequential = 0.0;
        if (test["thread_results"].contains("1"))
        {
            const auto &baseline = test["thread_results"]["1"];
            if (baseline.contains("soa") && baseline["soa"].contains("sequential"))
                soa_sequential = baseline["soa"]["sequential"]["mean_execution_time_ms"];
            if (baseline.contains("aos") && baseline["aos"].contains("sequential"))
                aos_sequential = baseline["aos"]["sequential"]["mean_execution_time_ms"];
        }

        std::cout << "\nSEQUENTIAL BASELINE RESULTS:" << std::endl;
        std::cout << "  SoA Sequential: " << std::fixed << std::setprecision(2) << soa_sequential << " ms" << std::endl;
        std::cout << "  AoS Sequential: " << std::fixed << std::setprecision(2) << aos_sequential << " ms" << std::endl;

        if (soa_sequential > 0 && aos_sequential > 0)
        {
            if (soa_sequential < aos_sequential)
                std::cout << "  → SoA is " << std::setprecision(2) << (aos_sequential / soa_sequential) << "x faster than AoS" << std::endl;
            else
                std::cout << "  → AoS is " << std::setprecision(2) << (soa_sequential / aos_sequential) << "x faster than SoA" << std::endl;
        }

        std::cout << "\nALL PARALLEL ALGORITHMS - DETAILED RESULTS:" << std::endl;
        std::cout << std::left
                  << std::setw(8) << "Threads"
                  << std::setw(20) << "Algorithm"
                  << std::setw(15) << "Time (ms)"
                  << std::setw(12) << "Speedup"
                  << std::setw(12) << "Efficiency"
                  << std::setw(15) << "vs Sequential" << std::endl;
        std::cout << std::string(82, '-') << std::endl;

        for (const auto &[thread_count_str, thread_data] : test["thread_results"].items())
        {
            int threads = std::stoi(thread_count_str);
            if (threads == 1)
                continue;

            bool first_for_thread = true;

            auto print_algorithm = [&](const std::string &algo_name, const nlohmann::json &algo_data)
            {
                if (algo_data.contains("mean_execution_time_ms") &&
                    algo_data.contains("speedup") &&
                    algo_data.contains("efficiency"))
                {

                    double time = algo_data["mean_execution_time_ms"];
                    double speedup = algo_data["speedup"];
                    double efficiency = algo_data["efficiency"];

                    double baseline = (algo_name.find("SoA") != std::string::npos) ? soa_sequential : aos_sequential;
                    double improvement = baseline / time;

                    std::cout << std::left
                              << std::setw(8) << (first_for_thread ? std::to_string(threads) : "")
                              << std::setw(20) << algo_name
                              << std::setw(15) << std::fixed << std::setprecision(2) << time
                              << std::setw(12) << std::setprecision(2) << speedup << "x"
                              << std::setw(12) << std::setprecision(1) << (efficiency * 100) << "%"
                              << std::setw(15) << std::setprecision(2) << improvement << "x" << std::endl;

                    first_for_thread = false;
                }
            };

            // SoA algorithms
            if (thread_data.contains("soa"))
            {
                if (thread_data["soa"].contains("parallel_outer"))
                    print_algorithm("SoA Outer", thread_data["soa"]["parallel_outer"]);

                if (thread_data["soa"].contains("parallel_inner"))
                    print_algorithm("SoA Inner", thread_data["soa"]["parallel_inner"]);

                if (thread_data["soa"].contains("parallel_simple"))
                    print_algorithm("SoA Simple", thread_data["soa"]["parallel_simple"]);
            }

            // AoS algorithms
            if (thread_data.contains("aos"))
            {
                if (thread_data["aos"].contains("parallel_outer"))
                    print_algorithm("AoS Outer", thread_data["aos"]["parallel_outer"]);

                if (thread_data["aos"].contains("parallel_inner"))
                    print_algorithm("AoS Inner", thread_data["aos"]["parallel_inner"]);

                if (thread_data["aos"].contains("parallel_simple"))
                    print_algorithm("AoS Simple", thread_data["aos"]["parallel_simple"]);
            }

            std::cout << std::string(82, '-') << std::endl;
        }

        std::cout << "\nBEST PERFORMANCE BY CATEGORY:" << std::endl;

        for (const auto &[thread_count_str, thread_data] : test["thread_results"].items())
        {
            int threads = std::stoi(thread_count_str);
            if (threads == 1)
                continue;

            std::cout << "\n"
                      << threads << " Threads:" << std::endl;

            double best_soa_time = 99999.0;
            std::string best_soa_name = "N/A";
            double best_soa_speedup = 0.0;

            if (thread_data.contains("soa"))
            {
                std::vector<std::pair<std::string, nlohmann::json>> soa_algos = {
                    {"SoA Outer", thread_data["soa"].value("parallel_outer", nlohmann::json{})},
                    {"SoA Inner", thread_data["soa"].value("parallel_inner", nlohmann::json{})},
                    {"SoA Simple", thread_data["soa"].value("parallel_simple", nlohmann::json{})}};

                for (const auto &[name, data] : soa_algos)
                {
                    if (data.contains("mean_execution_time_ms"))
                    {
                        double time = data["mean_execution_time_ms"];
                        if (time < best_soa_time)
                        {
                            best_soa_time = time;
                            best_soa_name = name;
                            best_soa_speedup = data["speedup"];
                        }
                    }
                }
            }

            double best_aos_time = 99999.0;
            std::string best_aos_name = "N/A";
            double best_aos_speedup = 0.0;

            if (thread_data.contains("aos"))
            {
                std::vector<std::pair<std::string, nlohmann::json>> aos_algos = {
                    {"AoS Outer", thread_data["aos"].value("parallel_outer", nlohmann::json{})},
                    {"AoS Inner", thread_data["aos"].value("parallel_inner", nlohmann::json{})},
                    {"AoS Simple", thread_data["aos"].value("parallel_simple", nlohmann::json{})}};

                for (const auto &[name, data] : aos_algos)
                {
                    if (data.contains("mean_execution_time_ms"))
                    {
                        double time = data["mean_execution_time_ms"];
                        if (time < best_aos_time)
                        {
                            best_aos_time = time;
                            best_aos_name = name;
                            best_aos_speedup = data["speedup"];
                        }
                    }
                }
            }

            std::cout << "  Best SoA: " << best_soa_name
                      << " → " << std::fixed << std::setprecision(2) << best_soa_time
                      << " ms (" << best_soa_speedup << "x speedup)" << std::endl;
            std::cout << "  Best AoS: " << best_aos_name
                      << " → " << std::fixed << std::setprecision(2) << best_aos_time
                      << " ms (" << best_aos_speedup << "x speedup)" << std::endl;

            if (best_soa_time < 99999.0 && best_aos_time < 99999.0)
            {
                if (best_soa_time < best_aos_time)
                {
                    double advantage = best_aos_time / best_soa_time;
                    std::cout << "  → " << best_soa_name << " is " << std::setprecision(2)
                              << advantage << "x faster than " << best_aos_name << std::endl;
                }
                else
                {
                    double advantage = best_soa_time / best_aos_time;
                    std::cout << "  → " << best_aos_name << " is " << std::setprecision(2)
                              << advantage << "x faster than " << best_soa_name << std::endl;
                }
            }
        }

        double absolute_best_time = 99999.0;
        double absolute_best_speedup = 0.0;
        int absolute_best_threads = 1;
        std::string absolute_best_name = "Sequential";

        for (const auto &[thread_count_str, thread_data] : test["thread_results"].items())
        {
            int threads = std::stoi(thread_count_str);
            if (threads == 1)
                continue;

            std::vector<std::tuple<std::string, nlohmann::json>> all_algos;

            if (thread_data.contains("soa"))
            {
                if (thread_data["soa"].contains("parallel_outer"))
                    all_algos.emplace_back("SoA Outer", thread_data["soa"]["parallel_outer"]);
                if (thread_data["soa"].contains("parallel_inner"))
                    all_algos.emplace_back("SoA Inner", thread_data["soa"]["parallel_inner"]);
                if (thread_data["soa"].contains("parallel_simple"))
                    all_algos.emplace_back("SoA Simple", thread_data["soa"]["parallel_simple"]);
            }

            if (thread_data.contains("aos"))
            {
                if (thread_data["aos"].contains("parallel_outer"))
                    all_algos.emplace_back("AoS Outer", thread_data["aos"]["parallel_outer"]);
                if (thread_data["aos"].contains("parallel_inner"))
                    all_algos.emplace_back("AoS Inner", thread_data["aos"]["parallel_inner"]);
                if (thread_data["aos"].contains("parallel_simple"))
                    all_algos.emplace_back("AoS Simple", thread_data["aos"]["parallel_simple"]);
            }

            for (const auto &[name, data] : all_algos)
            {
                if (data.contains("mean_execution_time_ms") && data.contains("speedup"))
                {
                    double time = data["mean_execution_time_ms"];
                    double speedup = data["speedup"];

                    if (speedup > absolute_best_speedup ||
                        (speedup == absolute_best_speedup && time < absolute_best_time))
                    {
                        absolute_best_time = time;
                        absolute_best_speedup = speedup;
                        absolute_best_threads = threads;
                        absolute_best_name = name;
                    }
                }
            }
        }

        std::cout << "\n"
                  << std::string(50, '=') << std::endl;
        std::cout << "ABSOLUTE BEST PERFORMANCE:" << std::endl;
        std::cout << "  Algorithm: " << absolute_best_name << std::endl;
        std::cout << "  Threads: " << absolute_best_threads << std::endl;
        std::cout << "  Time: " << std::fixed << std::setprecision(2) << absolute_best_time << " ms" << std::endl;
        std::cout << "  Speedup: " << std::setprecision(2) << absolute_best_speedup << "x" << std::endl;

        // Confronto con baseline
        double baseline_time = std::min(soa_sequential, aos_sequential);
        double total_improvement = baseline_time / absolute_best_time;
        std::cout << "  Total improvement: " << std::setprecision(2) << total_improvement << "x faster than best sequential" << std::endl;
        std::cout << std::string(50, '=') << std::endl;
    }

    std::cout << "\nResults saved to: " << output_filename << std::endl;
    return 0;
}