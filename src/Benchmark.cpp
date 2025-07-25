#include "../include/Benchmark.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include "Benchmark.h"
#include <filesystem>
#include <DataLoading.h>
#include <numeric>
#include <algorithm>

namespace
{
    // Calcola la media
    double calculate_mean(const std::vector<double> &values)
    {
        if (values.empty())
            return 0.0;
        return std::accumulate(values.begin(), values.end(), 0.0) / values.size();
    }

    // Calcola la deviazione standard
    double calculate_std_deviation(const std::vector<double> &values, double mean)
    {
        if (values.size() <= 1)
            return 0.0;
        double variance = 0.0;
        for (double value : values)
        {
            variance += (value - mean) * (value - mean);
        }
        variance /= (values.size() - 1);
        return std::sqrt(variance);
    }

    // Riempie le statistiche nel BenchmarkResult
    void fill_statistics(BenchmarkResult &result)
    {
        if (result.execution_times_ms.empty())
            return;

        result.mean_execution_time_ms = calculate_mean(result.execution_times_ms);
        result.std_deviation_ms = calculate_std_deviation(result.execution_times_ms, result.mean_execution_time_ms);
        result.min_execution_time_ms = *std::min_element(result.execution_times_ms.begin(), result.execution_times_ms.end());
        result.max_execution_time_ms = *std::max_element(result.execution_times_ms.begin(), result.execution_times_ms.end());
        result.num_runs = result.execution_times_ms.size();
    }
}

BenchmarkResult Benchmark::benchmarkSequentialSoA(const TimeSeriesDataset &dataset,
                                                  const TimeSeries &query,
                                                  const std::string &test_name,
                                                  int num_runs)
{
    std::vector<double> execution_times;
    std::vector<double> sadValues;
    size_t bestIndex = 0;

    std::cout << "  Running " << num_runs << " iterations for Sequential SoA..." << std::flush;

    for (int run = 0; run < num_runs; ++run)
    {
        auto start = std::chrono::high_resolution_clock::now();

        auto [currentSadValues, currentBestIndex] = SearchEngine::searchSequentialSoA(dataset, query);

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        execution_times.push_back(duration.count() / 1000.0);

        // usa prima run per il valore ottimo
        if (run == 0)
        {
            sadValues = currentSadValues;
            bestIndex = currentBestIndex;
        }

        std::cout << "." << std::flush;
    }
    std::cout << " Done" << std::endl;

    BenchmarkResult result;
    result.algorithm_name = "Sequential_SoA_" + test_name;
    result.num_series = dataset.getNumSeries();
    result.series_length = dataset.getNumSeries() > 0 ? dataset.getSeries(0).size() : 0;
    result.query_length = query.getSize();
    result.execution_times_ms = execution_times;
    result.best_match_index = bestIndex;
    result.best_sad_value = sadValues[bestIndex];

    fill_statistics(result);

    return result;
}

BenchmarkResult Benchmark::benchmarkSoA_parallelOuter(const TimeSeriesDataset &dataset,
                                                      const TimeSeries &query,
                                                      const std::string &test_name,
                                                      int num_runs)
{
    std::vector<double> execution_times;
    std::vector<double> sadValues;
    size_t bestIndex = 0;

    std::cout << "  Running " << num_runs << " iterations for Parallel SoA Outer..." << std::flush;

    for (int run = 0; run < num_runs; ++run)
    {
        auto start = std::chrono::high_resolution_clock::now();

        auto [currentSadValues, currentBestIndex] = SearchEngine::searchParallelSoAOuter(dataset, query);

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        execution_times.push_back(duration.count() / 1000.0);

        if (run == 0)
        {
            sadValues = currentSadValues;
            bestIndex = currentBestIndex;
        }

        std::cout << "." << std::flush;
    }
    std::cout << " Done" << std::endl;

    BenchmarkResult result;
    result.algorithm_name = "Parallel_SoA_Outer_" + test_name;
    result.num_series = dataset.getNumSeries();
    result.series_length = dataset.getNumSeries() > 0 ? dataset.getSeries(0).size() : 0;
    result.query_length = query.getSize();
    result.execution_times_ms = execution_times;
    result.best_match_index = bestIndex;
    result.best_sad_value = sadValues[bestIndex];

    fill_statistics(result);

    return result;
}

BenchmarkResult Benchmark::benchmarkSoA_parallelInner(const TimeSeriesDataset &dataset,
                                                      const TimeSeries &query,
                                                      const std::string &test_name,
                                                      int num_runs)
{
    std::vector<double> execution_times;
    std::vector<double> sadValues;
    size_t bestIndex = 0;

    std::cout << "  Running " << num_runs << " iterations for Parallel SoA Inner..." << std::flush;

    for (int run = 0; run < num_runs; ++run)
    {
        auto start = std::chrono::high_resolution_clock::now();

        auto [currentSadValues, currentBestIndex] = SearchEngine::searchParallelSoAInner(dataset, query);

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        execution_times.push_back(duration.count() / 1000.0);

        if (run == 0)
        {
            sadValues = currentSadValues;
            bestIndex = currentBestIndex;
        }

        std::cout << "." << std::flush;
    }
    std::cout << " Done" << std::endl;

    BenchmarkResult result;
    result.algorithm_name = "Parallel_SoA_Inner_" + test_name;
    result.num_series = dataset.getNumSeries();
    result.series_length = dataset.getNumSeries() > 0 ? dataset.getSeries(0).size() : 0;
    result.query_length = query.getSize();
    result.execution_times_ms = execution_times;
    result.best_match_index = bestIndex;
    result.best_sad_value = sadValues[bestIndex];

    fill_statistics(result);

    return result;
}

BenchmarkResult Benchmark::benchmarkSequentialAoS(const std::vector<TimeSeries> &dataset,
                                                  const TimeSeries &query,
                                                  const std::string &test_name,
                                                  int num_runs)
{
    std::vector<double> execution_times;
    std::vector<double> sadValues;
    size_t bestIndex = 0;

    std::cout << "  Running " << num_runs << " iterations for Sequential AoS..." << std::flush;

    for (int run = 0; run < num_runs; ++run)
    {
        auto start = std::chrono::high_resolution_clock::now();

        auto [currentSadValues, currentBestIndex] = SearchEngine::searchSequentialAoS(dataset, query);

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        execution_times.push_back(duration.count() / 1000.0);

        if (run == 0)
        {
            sadValues = currentSadValues;
            bestIndex = currentBestIndex;
        }

        std::cout << "." << std::flush;
    }
    std::cout << " Done" << std::endl;

    BenchmarkResult result;
    result.algorithm_name = "Sequential_AoS_" + test_name;
    result.num_series = dataset.size();
    result.series_length = dataset.size() > 0 ? dataset[0].getSize() : 0;
    result.query_length = query.getSize();
    result.execution_times_ms = execution_times;
    result.best_match_index = bestIndex;
    result.best_sad_value = sadValues[bestIndex];

    fill_statistics(result);

    return result;
}

BenchmarkResult Benchmark::benchmarkAoS_parallelOuter(const std::vector<TimeSeries> &dataset,
                                                      const TimeSeries &query,
                                                      const std::string &test_name,
                                                      int num_runs)
{
    std::vector<double> execution_times;
    std::vector<double> sadValues;
    size_t bestIndex = 0;

    std::cout << "  Running " << num_runs << " iterations for Parallel AoS Outer..." << std::flush;

    for (int run = 0; run < num_runs; ++run)
    {
        auto start = std::chrono::high_resolution_clock::now();

        auto [currentSadValues, currentBestIndex] = SearchEngine::searchParallelAoSOuter(dataset, query);

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        execution_times.push_back(duration.count() / 1000.0);

        if (run == 0)
        {
            sadValues = currentSadValues;
            bestIndex = currentBestIndex;
        }

        std::cout << "." << std::flush;
    }
    std::cout << " Done" << std::endl;

    BenchmarkResult result;
    result.algorithm_name = "Parallel_AoS_Outer_" + test_name;
    result.num_series = dataset.size();
    result.series_length = dataset.size() > 0 ? dataset[0].getSize() : 0;
    result.query_length = query.getSize();
    result.execution_times_ms = execution_times;
    result.best_match_index = bestIndex;
    result.best_sad_value = sadValues[bestIndex];

    fill_statistics(result);

    return result;
}

BenchmarkResult Benchmark::benchmarkAoS_parallelInner(const std::vector<TimeSeries> &dataset,
                                                      const TimeSeries &query,
                                                      const std::string &test_name,
                                                      int num_runs)
{
    std::vector<double> execution_times;
    std::vector<double> sadValues;
    size_t bestIndex = 0;

    std::cout << "  Running " << num_runs << " iterations for Parallel AoS Inner..." << std::flush;

    for (int run = 0; run < num_runs; ++run)
    {
        auto start = std::chrono::high_resolution_clock::now();

        auto [currentSadValues, currentBestIndex] = SearchEngine::searchParallelAoSInner(dataset, query);

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        execution_times.push_back(duration.count() / 1000.0);

        if (run == 0)
        {
            sadValues = currentSadValues;
            bestIndex = currentBestIndex;
        }

        std::cout << "." << std::flush;
    }
    std::cout << " Done" << std::endl;

    BenchmarkResult result;
    result.algorithm_name = "Parallel_AoS_Inner_" + test_name;
    result.num_series = dataset.size();
    result.series_length = dataset.size() > 0 ? dataset[0].getSize() : 0;
    result.query_length = query.getSize();
    result.execution_times_ms = execution_times;
    result.best_match_index = bestIndex;
    result.best_sad_value = sadValues[bestIndex];

    fill_statistics(result);

    return result;
}

bool Benchmark::generateDataset(const TestConfiguration &config)
{
    std::filesystem::create_directories("src/utils/data/timeseries");
    std::filesystem::create_directories("src/utils/data/query");

    std::string dataset_filename = "src/utils/data/timeseries/timeseries_" +
                                   std::to_string(config.num_series) + "_" +
                                   std::to_string(config.series_length) + "_" +
                                   std::to_string(config.query_length) + ".csv";

    std::string query_filename = "src/utils/data/query/query_" +
                                 std::to_string(config.num_series) + "_" +
                                 std::to_string(config.series_length) + "_" +
                                 std::to_string(config.query_length) + ".csv";

    // controlla se i file esistono già
    if (std::filesystem::exists(dataset_filename) && std::filesystem::exists(query_filename))
    {
        std::cout << "Dataset already exists: " << dataset_filename << " and " << query_filename << std::endl;
        return true;
    }

    // comando per lanciare script python
    std::ostringstream cmd;
    cmd << "cd src/utils && python3 generate_timeseries.py "
        << config.num_series << " "
        << config.series_length << " "
        << config.query_length;

    std::cout << "Generating dataset: " << config.num_series << " series × "
              << config.series_length << " points, query length "
              << config.query_length << std::endl;

    int result = system(cmd.str().c_str());

    if (result != 0)
    {
        std::cerr << "Error generating dataset" << std::endl;
        return false;
    }

    // verifica che i file siano creati
    if (!std::filesystem::exists(dataset_filename) ||
        !std::filesystem::exists(query_filename))
    {
        std::cerr << "Dataset files not found after generation" << std::endl;
        std::cerr << "Expected dataset: " << dataset_filename << std::endl;
        std::cerr << "Expected query: " << query_filename << std::endl;
        return false;
    }

    return true;
}

nlohmann::json Benchmark::run_test(const TestConfiguration &config)
{
    nlohmann::json result;

    // genera dataset
    if (!generateDataset(config))
    {
        result["error"] = "Failed to generate dataset";
        return result;
    }

    std::string test_name = std::to_string(config.num_series) + "_" +
                            std::to_string(config.series_length) + "_" +
                            std::to_string(config.query_length);

    std::string dataset_path = "src/utils/data/timeseries/timeseries_" + test_name + ".csv";
    std::string query_path = "src/utils/data/query/query_" + test_name + ".csv";

    std::vector<TimeSeries> datasetAos = loadTimeSeriesAoS(dataset_path);
    TimeSeriesDataset datasetSoa = loadTimeSeriesDatasetSoA(dataset_path);
    std::vector<TimeSeries> query = loadTimeSeriesAoS(query_path);

    if (datasetAos.empty() || query.empty())
    {
        result["error"] = "Failed to load dataset or query";
        return result;
    }

    std::cout << "Running benchmark for " << test_name << " (" << config.num_runs << " runs each):" << std::endl;

    auto resultSoA_sequential = benchmarkSequentialSoA(datasetSoa, query[0], test_name, config.num_runs);
    auto resultSoA_parallelOuter = benchmarkSoA_parallelOuter(datasetSoa, query[0], test_name, config.num_runs);
    auto resultSoA_parallelInner = benchmarkSoA_parallelInner(datasetSoa, query[0], test_name, config.num_runs);
    auto resultAoS_sequential = benchmarkSequentialAoS(datasetAos, query[0], test_name, config.num_runs);
    auto resultAoS_parallelOuter = benchmarkAoS_parallelOuter(datasetAos, query[0], test_name, config.num_runs);
    auto resultAoS_parallelInner = benchmarkAoS_parallelInner(datasetAos, query[0], test_name, config.num_runs);

    // calcolo speedup rispetto alle versioni sequenziali (usa le medie)
    double soa_outer_speedup = resultSoA_sequential.mean_execution_time_ms / resultSoA_parallelOuter.mean_execution_time_ms;
    double soa_inner_speedup = resultSoA_sequential.mean_execution_time_ms / resultSoA_parallelInner.mean_execution_time_ms;
    double aos_outer_speedup = resultAoS_sequential.mean_execution_time_ms / resultAoS_parallelOuter.mean_execution_time_ms;
    double aos_inner_speedup = resultAoS_sequential.mean_execution_time_ms / resultAoS_parallelInner.mean_execution_time_ms;

    // confronto SoA vs AoS
    double soa_vs_aos_sequential = resultAoS_sequential.mean_execution_time_ms / resultSoA_sequential.mean_execution_time_ms;
    double soa_vs_aos_parallel_outer = resultAoS_parallelOuter.mean_execution_time_ms / resultSoA_parallelOuter.mean_execution_time_ms;
    double soa_vs_aos_parallel_inner = resultAoS_parallelInner.mean_execution_time_ms / resultSoA_parallelInner.mean_execution_time_ms;

    result["test_name"] = test_name;
    result["configuration"] = {
        {"num_series", config.num_series},
        {"series_length", config.series_length},
        {"query_length", config.query_length},
        {"num_runs", config.num_runs}};

    // arrotonda a due decimali
    auto round2 = [](double value)
    {
        return std::round(value * 100.0) / 100.0;
    };

    result["results"] = {
        {"soa", {{"sequential", {{"mean_execution_time_ms", round2(resultSoA_sequential.mean_execution_time_ms)}, {"std_deviation_ms", round2(resultSoA_sequential.std_deviation_ms)}, {"min_execution_time_ms", round2(resultSoA_sequential.min_execution_time_ms)}, {"max_execution_time_ms", round2(resultSoA_sequential.max_execution_time_ms)}, {"best_match_index", resultSoA_sequential.best_match_index}, {"best_sad_value", resultSoA_sequential.best_sad_value}, {"all_execution_times", resultSoA_sequential.execution_times_ms}}}, {"parallel_outer", {{"mean_execution_time_ms", round2(resultSoA_parallelOuter.mean_execution_time_ms)}, {"std_deviation_ms", round2(resultSoA_parallelOuter.std_deviation_ms)}, {"min_execution_time_ms", round2(resultSoA_parallelOuter.min_execution_time_ms)}, {"max_execution_time_ms", round2(resultSoA_parallelOuter.max_execution_time_ms)}, {"speedup", soa_outer_speedup}, {"best_match_index", resultSoA_parallelOuter.best_match_index}, {"best_sad_value", resultSoA_parallelOuter.best_sad_value}, {"results_match", resultSoA_parallelOuter.best_match_index == resultSoA_sequential.best_match_index}, {"all_execution_times", resultSoA_parallelOuter.execution_times_ms}}}, {"parallel_inner", {{"mean_execution_time_ms", round2(resultSoA_parallelInner.mean_execution_time_ms)}, {"std_deviation_ms", round2(resultSoA_parallelInner.std_deviation_ms)}, {"min_execution_time_ms", round2(resultSoA_parallelInner.min_execution_time_ms)}, {"max_execution_time_ms", round2(resultSoA_parallelInner.max_execution_time_ms)}, {"speedup", soa_inner_speedup}, {"best_match_index", resultSoA_parallelInner.best_match_index}, {"best_sad_value", resultSoA_parallelInner.best_sad_value}, {"results_match", resultSoA_parallelInner.best_match_index == resultSoA_sequential.best_match_index}, {"all_execution_times", resultSoA_parallelInner.execution_times_ms}}}}},
        {"aos", {{"sequential", {{"mean_execution_time_ms", round2(resultAoS_sequential.mean_execution_time_ms)}, {"std_deviation_ms", round2(resultAoS_sequential.std_deviation_ms)}, {"min_execution_time_ms", round2(resultAoS_sequential.min_execution_time_ms)}, {"max_execution_time_ms", round2(resultAoS_sequential.max_execution_time_ms)}, {"best_match_index", resultAoS_sequential.best_match_index}, {"best_sad_value", resultAoS_sequential.best_sad_value}, {"all_execution_times", resultAoS_sequential.execution_times_ms}}}, {"parallel_outer", {{"mean_execution_time_ms", round2(resultAoS_parallelOuter.mean_execution_time_ms)}, {"std_deviation_ms", round2(resultAoS_parallelOuter.std_deviation_ms)}, {"min_execution_time_ms", round2(resultAoS_parallelOuter.min_execution_time_ms)}, {"max_execution_time_ms", round2(resultAoS_parallelOuter.max_execution_time_ms)}, {"speedup", aos_outer_speedup}, {"best_match_index", resultAoS_parallelOuter.best_match_index}, {"best_sad_value", resultAoS_parallelOuter.best_sad_value}, {"results_match", resultAoS_parallelOuter.best_match_index == resultAoS_sequential.best_match_index}, {"all_execution_times", resultAoS_parallelOuter.execution_times_ms}}}, {"parallel_inner", {{"mean_execution_time_ms", round2(resultAoS_parallelInner.mean_execution_time_ms)}, {"std_deviation_ms", round2(resultAoS_parallelInner.std_deviation_ms)}, {"min_execution_time_ms", round2(resultAoS_parallelInner.min_execution_time_ms)}, {"max_execution_time_ms", round2(resultAoS_parallelInner.max_execution_time_ms)}, {"speedup", aos_inner_speedup}, {"best_match_index", resultAoS_parallelInner.best_match_index}, {"best_sad_value", resultAoS_parallelInner.best_sad_value}, {"results_match", resultAoS_parallelInner.best_match_index == resultAoS_sequential.best_match_index}, {"all_execution_times", resultAoS_parallelInner.execution_times_ms}}}}}};

    result["analysis"] = {
        {"soa_vs_aos_sequential", soa_vs_aos_sequential},
        {"soa_vs_aos_parallel_outer", soa_vs_aos_parallel_outer},
        {"soa_vs_aos_parallel_inner", soa_vs_aos_parallel_inner}};

    return result;
}

nlohmann::json Benchmark::run_multiple_tests(const std::vector<TestConfiguration> &configurations)
{
    nlohmann::json results;
    results["timestamp"] = std::chrono::duration_cast<std::chrono::seconds>(
                               std::chrono::system_clock::now().time_since_epoch())
                               .count();
    results["tests"] = nlohmann::json::array();

    for (const auto &config : configurations)
    {
        auto test_result = run_test(config);
        results["tests"].push_back(test_result);

        std::cout << "Completed test: " << test_result["test_name"] << std::endl;
    }

    return results;
}