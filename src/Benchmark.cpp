#include "../include/Benchmark.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include "Benchmark.h"
#include <filesystem>
#include <DataLoading.h>

BenchmarkResult Benchmark::benchmarkSequentialSoA(const TimeSeriesDataset &dataset,
                                                  const TimeSeries &query,
                                                  const std::string &test_name)
{
    auto start = std::chrono::high_resolution_clock::now();

    auto [sadValues, bestIndex] = SearchEngine::searchSequentialSoA(dataset, query);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    BenchmarkResult result;
    result.algorithm_name = "Sequential_SoA_" + test_name;
    result.num_series = dataset.getNumSeries();
    result.series_length = dataset.getNumSeries() > 0 ? dataset.getSeries(0).size() : 0;
    result.query_length = query.getSize();
    result.execution_time_ms = duration.count() / 1000.0; // millisecondi
    result.best_match_index = bestIndex;
    result.best_sad_value = sadValues[bestIndex];

    return result;
}

BenchmarkResult Benchmark::benchmarkSoA_parallelOuter(const TimeSeriesDataset &dataset, const TimeSeries &query, const std::string &test_name)
{
    auto start = std::chrono::high_resolution_clock::now();

    auto [sadValues, bestIndex] = SearchEngine::searchParallelSoAOuter(dataset, query);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    BenchmarkResult result;
    result.algorithm_name = "Parallel_SoA_Outer" + test_name;
    result.num_series = dataset.getNumSeries();
    result.series_length = dataset.getNumSeries() > 0 ? dataset.getSeries(0).size() : 0;
    result.query_length = query.getSize();
    result.execution_time_ms = duration.count() / 1000.0; // millisecondi
    result.best_match_index = bestIndex;
    result.best_sad_value = sadValues[bestIndex];

    return result;
}

BenchmarkResult Benchmark::benchmarkSoA_parallelInner(const TimeSeriesDataset &dataset, const TimeSeries &query, const std::string &test_name)
{
    auto start = std::chrono::high_resolution_clock::now();

    auto [sadValues, bestIndex] = SearchEngine::searchParallelSoAInner(dataset, query);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    BenchmarkResult result;
    result.algorithm_name = "Parallel_SoA_Inner" + test_name;
    result.num_series = dataset.getNumSeries();
    result.series_length = dataset.getNumSeries() > 0 ? dataset.getSeries(0).size() : 0;
    result.query_length = query.getSize();
    result.execution_time_ms = duration.count() / 1000.0; // millisecondi
    result.best_match_index = bestIndex;
    result.best_sad_value = sadValues[bestIndex];

    return result;
}

BenchmarkResult Benchmark::benchmarkSequentialAoS(const std::vector<TimeSeries> &dataset,
                                                  const TimeSeries &query,
                                                  const std::string &test_name)
{
    auto start = std::chrono::high_resolution_clock::now();

    auto [sadValues, bestIndex] = SearchEngine::searchSequentialAoS(dataset, query);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    BenchmarkResult result;
    result.algorithm_name = "Sequential_AoS_" + test_name;
    result.num_series = dataset.size();
    result.series_length = dataset.size() > 0 ? dataset[0].getSize() : 0;
    result.query_length = query.getSize();
    result.execution_time_ms = duration.count() / 1000.0;
    result.best_match_index = bestIndex;
    result.best_sad_value = sadValues[bestIndex];

    return result;
}

BenchmarkResult Benchmark::benchmarkAoS_parallelOuter(const std::vector<TimeSeries> &dataset, const TimeSeries &query, const std::string &test_name)
{
    auto start = std::chrono::high_resolution_clock::now();

    auto [sadValues, bestIndex] = SearchEngine::searchParallelAoSOuter(dataset, query);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    BenchmarkResult result;
    result.algorithm_name = "Parallel_AoS_Outer" + test_name;
    result.num_series = dataset.size();
    result.series_length = dataset.size() > 0 ? dataset[0].getSize() : 0;
    result.query_length = query.getSize();
    result.execution_time_ms = duration.count() / 1000.0; // millisecondi
    result.best_match_index = bestIndex;
    result.best_sad_value = sadValues[bestIndex];

    return result;
}

BenchmarkResult Benchmark::benchmarkAoS_parallelInner(const std::vector<TimeSeries> &dataset, const TimeSeries &query, const std::string &test_name)
{
    auto start = std::chrono::high_resolution_clock::now();

    auto [sadValues, bestIndex] = SearchEngine::searchParallelAoSInner(dataset, query);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    BenchmarkResult result;
    result.algorithm_name = "Parallel_AoS_Inner" + test_name;
    result.num_series = dataset.size();
    result.series_length = dataset.size() > 0 ? dataset[0].getSize() : 0;
    result.query_length = query.getSize();
    result.execution_time_ms = duration.count() / 1000.0; // millisecondi
    result.best_match_index = bestIndex;
    result.best_sad_value = sadValues[bestIndex];

    return result;
}

bool Benchmark::generateDataset(const TestConfiguration &config)
{
    std::filesystem::create_directories("src/utils/data/timeseries");
    std::filesystem::create_directories("src/utils/data/query");

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

    std::string dataset_filename = "src/utils/data/timeseries/timeseries_" +
                                   std::to_string(config.num_series) + "_" +
                                   std::to_string(config.series_length) + "_" +
                                   std::to_string(config.query_length) + ".csv";

    std::string query_filename = "src/utils/data/query/query_" +
                                 std::to_string(config.num_series) + "_" +
                                 std::to_string(config.series_length) + "_" +
                                 std::to_string(config.query_length) + ".csv";

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

    // lancia i benchmark
    auto resultSoA_sequential = benchmarkSequentialSoA(datasetSoa, query[0], test_name);
    auto resultSoA_parallelOuter = benchmarkSoA_parallelOuter(datasetSoa, query[0], test_name);
    auto resultSoA_parallelInner = benchmarkSoA_parallelInner(datasetSoa, query[0], test_name);
    auto resultAoS_sequential = benchmarkSequentialAoS(datasetAos, query[0], test_name);
    auto resultAoS_parallelOuter = benchmarkAoS_parallelOuter(datasetAos, query[0], test_name);
    auto resultAoS_parallelInner = benchmarkAoS_parallelInner(datasetAos, query[0], test_name);

    // calcolo speedup rispetto alle versioni sequenziali
    double soa_outer_speedup = resultSoA_sequential.execution_time_ms / resultSoA_parallelOuter.execution_time_ms;
    double soa_inner_speedup = resultSoA_sequential.execution_time_ms / resultSoA_parallelInner.execution_time_ms;
    double aos_outer_speedup = resultAoS_sequential.execution_time_ms / resultAoS_parallelOuter.execution_time_ms;
    double aos_inner_speedup = resultAoS_sequential.execution_time_ms / resultAoS_parallelInner.execution_time_ms;

    // confronto SoA vs AoS
    double soa_vs_aos_sequential = resultAoS_sequential.execution_time_ms / resultSoA_sequential.execution_time_ms;
    double soa_vs_aos_parallel_outer = resultAoS_parallelOuter.execution_time_ms / resultSoA_parallelOuter.execution_time_ms;
    double soa_vs_aos_parallel_inner = resultAoS_parallelInner.execution_time_ms / resultSoA_parallelInner.execution_time_ms;

    // FIXME: dovrei includere anche il numero di thread usati?
    result["test_name"] = test_name;
    result["configuration"] = {
        {"num_series", config.num_series},
        {"series_length", config.series_length},
        {"query_length", config.query_length}};

    result["results"] = {
        {"soa", {{"sequential", {{"execution_time_ms", resultSoA_sequential.execution_time_ms}, {"best_match_index", resultSoA_sequential.best_match_index}, {"best_sad_value", resultSoA_sequential.best_sad_value}}}, {"parallel_outer", {{"execution_time_ms", resultSoA_parallelOuter.execution_time_ms}, {"speedup", soa_outer_speedup}, {"best_match_index", resultSoA_parallelOuter.best_match_index}, {"best_sad_value", resultSoA_parallelOuter.best_sad_value}, {"results_match", resultSoA_parallelOuter.best_match_index == resultSoA_sequential.best_match_index}}}, {"parallel_inner", {{"execution_time_ms", resultSoA_parallelInner.execution_time_ms}, {"speedup", soa_inner_speedup}, {"best_match_index", resultSoA_parallelInner.best_match_index}, {"best_sad_value", resultSoA_parallelInner.best_sad_value}, {"results_match", resultSoA_parallelInner.best_match_index == resultSoA_sequential.best_match_index}}}}},
        {"aos", {{"sequential", {{"execution_time_ms", resultAoS_sequential.execution_time_ms}, {"best_match_index", resultAoS_sequential.best_match_index}, {"best_sad_value", resultAoS_sequential.best_sad_value}}}, {"parallel_outer", {{"execution_time_ms", resultAoS_parallelOuter.execution_time_ms}, {"speedup", aos_outer_speedup}, {"best_match_index", resultAoS_parallelOuter.best_match_index}, {"best_sad_value", resultAoS_parallelOuter.best_sad_value}, {"results_match", resultAoS_parallelOuter.best_match_index == resultAoS_sequential.best_match_index}}}, {"parallel_inner", {{"execution_time_ms", resultAoS_parallelInner.execution_time_ms}, {"speedup", aos_inner_speedup}, {"best_match_index", resultAoS_parallelInner.best_match_index}, {"best_sad_value", resultAoS_parallelInner.best_sad_value}, {"results_match", resultAoS_parallelInner.best_match_index == resultAoS_sequential.best_match_index}}}}}};

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