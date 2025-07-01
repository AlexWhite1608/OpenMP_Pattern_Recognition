#ifndef BENCHMARK_H
#define BENCHMARK_H

#include "SearchEngine.h"
#include <chrono>
#include <string>
#include <fstream>

struct BenchmarkResult
{
    std::string algorithm_name;
    int num_series;
    int series_length;
    int query_length;
    double execution_time_ms;
    size_t best_match_index;
    double best_sad_value;
    bool results_match_reference = true;
};

class Benchmark
{
public:
    static BenchmarkResult benchmarkSequentialSoA(const TimeSeriesDataset &dataset,
                                                  const TimeSeries &query,
                                                  const std::string &test_name);

    static BenchmarkResult benchmarkSequentialAoS(const std::vector<TimeSeries> &dataset,
                                                  const TimeSeries &query,
                                                  const std::string &test_name);

    static void saveResultsToCSV(const std::vector<BenchmarkResult> &results,
                                 const std::string &filename);

    static bool validateResults(const std::vector<double> &sadSoA,
                                const std::vector<double> &sadAoS,
                                size_t bestIndexSoA,
                                size_t bestIndexAoS);
};

#endif