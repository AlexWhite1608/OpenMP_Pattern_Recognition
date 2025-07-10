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
    double execution_time_ms;
    size_t best_match_index;
    double best_sad_value;
    double parallel_speedup = 1.0;
    bool results_match_reference = true;
};

struct TestConfiguration
{
    int num_series;
    int series_length;
    int query_length;
    std::string dataset_path;
    std::string query_path;
};

class Benchmark
{
public:
    // SoA
    static BenchmarkResult benchmarkSequentialSoA(const TimeSeriesDataset &dataset,
                                                  const TimeSeries &query,
                                                  const std::string &test_name);
    static BenchmarkResult benchmarkSoA_parallelOuter(const TimeSeriesDataset &dataset, const TimeSeries &query, const std::string &test_name);
    static BenchmarkResult benchmarkSoA_parallelInner(const TimeSeriesDataset &dataset, const TimeSeries &query, const std::string &test_name);

    // AoS
    static BenchmarkResult benchmarkSequentialAoS(const std::vector<TimeSeries> &dataset,
                                                  const TimeSeries &query,
                                                  const std::string &test_name);
    static BenchmarkResult benchmarkAoS_parallelOuter(const std::vector<TimeSeries> &dataset, const TimeSeries &query, const std::string &test_name);
    static BenchmarkResult benchmarkAoS_parallelInner(const std::vector<TimeSeries> &dataset, const TimeSeries &query, const std::string &test_name);

    // static bool validateResults(const std::vector<double> &sadSoA,
    //                             const std::vector<double> &sadAoS,
    //                             size_t bestIndexSoA,
    //                             size_t bestIndexAoS);

    static bool generateDataset(const TestConfiguration &config);

    static nlohmann::json run_test(const TestConfiguration &config);

    static nlohmann::json run_multiple_tests(const std::vector<TestConfiguration> &configurations);
};

#endif