#ifndef BENCHMARK_H
#define BENCHMARK_H

#include "SearchEngine.h"
#include <chrono>
#include <string>
#include <fstream>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

struct BenchmarkResult
{
    std::string algorithm_name;
    std::string parallelization_type;
    int num_series;
    int series_length;
    int query_length;

    std::vector<double> execution_times_ms; // Tutti i tempi di ogni run
    double mean_execution_time_ms;
    double std_deviation_ms;
    double min_execution_time_ms;
    double max_execution_time_ms;

    size_t best_match_index;
    double best_sad_value;
    int num_runs;
    bool results_match_reference = true;
};

struct TestConfiguration
{
    int num_series;
    int series_length;
    int query_length;
    int num_runs = 10;
    std::vector<int> thread_counts;
    std::string dataset_path;
    std::string query_path;
};

class Benchmark
{
public:
    // SoA
    static BenchmarkResult benchmarkSequentialSoA(const TimeSeriesSoA &dataset,
                                                  const TimeSeries &query,
                                                  const std::string &test_name,
                                                  int num_runs = 1);
    static BenchmarkResult benchmarkSoA_parallelOuter(const TimeSeriesSoA &dataset,
                                                      const TimeSeries &query,
                                                      const std::string &test_name,
                                                      int num_runs = 1);
    static BenchmarkResult benchmarkSoA_parallelInner(const TimeSeriesSoA &dataset,
                                                      const TimeSeries &query,
                                                      const std::string &test_name,
                                                      int num_runs = 1);

    // AoS
    static BenchmarkResult benchmarkSequentialAoS(const TimeSeriesAoS &dataset,
                                                  const TimeSeries &query,
                                                  const std::string &test_name,
                                                  int num_runs = 1);
    static BenchmarkResult benchmarkAoS_parallelOuter(const TimeSeriesAoS &dataset,
                                                      const TimeSeries &query,
                                                      const std::string &test_name,
                                                      int num_runs = 1);
    static BenchmarkResult benchmarkAoS_parallelInner(const TimeSeriesAoS &dataset,
                                                      const TimeSeries &query,
                                                      const std::string &test_name,
                                                      int num_runs = 1);

    static bool generateDataset(const TestConfiguration &config);

    static nlohmann::json run_test(const TestConfiguration &config);

    static nlohmann::json run_multiple_tests(const std::vector<TestConfiguration> &configurations);
};

#endif