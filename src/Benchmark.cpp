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

bool Benchmark::validateResults(const std::vector<double> &sadSoA,
                                const std::vector<double> &sadAoS,
                                size_t bestIndexSoA,
                                size_t bestIndexAoS)
{
    if (sadSoA.size() != sadAoS.size())
        return false;
    if (bestIndexSoA != bestIndexAoS)
        return false;

    const double EPSILON = 1e-10;
    for (size_t i = 0; i < sadSoA.size(); ++i)
    {
        if (std::abs(sadSoA[i] - sadAoS[i]) > EPSILON)
        {
            return false;
        }
    }
    return true;
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

    // verifica che i file siano creati
    if (!std::filesystem::exists(config.dataset_path) ||
        !std::filesystem::exists(config.query_path))
    {
        std::cerr << "Dataset files not found after generation" << std::endl;
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

    std::vector<TimeSeries> datasetAos = loadTimeSeriesAoS(config.dataset_path);
    TimeSeriesDataset datasetSoa = loadTimeSeriesDatasetSoA(config.dataset_path);
    std::vector<TimeSeries> query = loadTimeSeriesAoS(config.query_path);

    if (datasetAos.empty() || query.empty())
    {
        result["error"] = "Failed to load dataset or query";
        return result;
    }

    // lancia il benchmark
    auto resultSoA = benchmarkSequentialSoA(datasetSoa, query[0], test_name);
    auto resultAoS = benchmarkSequentialAoS(datasetAos, query[0], test_name);

    // validazione dei risultati
    auto [sadSoA, bestIndexSoA] = SearchEngine::searchSequentialSoA(datasetSoa, query[0]);
    auto [sadAoS, bestIndexAoS] = SearchEngine::searchSequentialAoS(datasetAos, query[0]);
    bool results_match = validateResults(sadSoA, sadAoS, bestIndexSoA, bestIndexAoS);

    double speedup = resultAoS.execution_time_ms / resultSoA.execution_time_ms;

    result["test_name"] = test_name;
    result["configuration"] = {
        {"num_series", config.num_series},
        {"series_length", config.series_length},
        {"query_length", config.query_length}};

    result["results"] = {
        {"soa", {{"execution_time_ms", resultSoA.execution_time_ms}, {"best_match_index", resultSoA.best_match_index}, {"best_sad_value", resultSoA.best_sad_value}}},
        {"aos", {{"execution_time_ms", resultAoS.execution_time_ms}, {"best_match_index", resultAoS.best_match_index}, {"best_sad_value", resultAoS.best_sad_value}}}};

    result["performance"] = {
        {"speedup", speedup},
        {"results_match", results_match}};

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