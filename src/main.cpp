#include <iostream>
#include <vector>
#include <string>
#include "../include/DataLoading.h"
#include "../include/SearchEngine.h"
#include "../include/Benchmark.h"
#include <iomanip>

// parametri: num_series, series_length, query_length
int main(int argc, char *argv[])
{
    int num_series = std::stoi(argv[1]);
    int series_length = std::stoi(argv[2]);
    int query_length = std::stoi(argv[3]);

    std::string test_name = std::to_string(num_series) + "_" +
                            std::to_string(series_length) + "_" +
                            std::to_string(query_length);

    // Carica dataset
    std::string timeseriesPath = "src/utils/data/timeseries/timeseries.csv";
    std::string queryPath = "src/utils/data/query/query.csv";

    std::vector<TimeSeries> datasetAos = loadTimeSeriesAoS(timeseriesPath);
    TimeSeriesDataset datasetSoa = loadTimeSeriesDatasetSoA(timeseriesPath);
    std::vector<TimeSeries> query = loadTimeSeriesAoS(queryPath);

    if (datasetAos.empty() || query.empty())
    {
        std::cerr << "Errore nel caricamento dei dati!" << std::endl;
        return 1;
    }

    std::cout << "=== BENCHMARK PERFORMANCE ===" << std::endl;
    std::cout << "Dataset: " << num_series << " series × " << series_length << " points" << std::endl;
    std::cout << "Query length: " << query_length << std::endl;
    std::cout << "================================" << std::endl;

    std::vector<BenchmarkResult> results;

    // Benchmark Sequential SoA
    auto resultSoA = Benchmark::benchmarkSequentialSoA(datasetSoa, query[0], test_name);
    results.push_back(resultSoA);

    // Benchmark Sequential AoS
    auto resultAoS = Benchmark::benchmarkSequentialAoS(datasetAos, query[0], test_name);
    results.push_back(resultAoS);

    // Validazione risultati
    auto [sadSoA, bestIndexSoA] = SearchEngine::searchSequentialSoA(datasetSoa, query[0]);
    auto [sadAoS, bestIndexAoS] = SearchEngine::searchSequentialAoS(datasetAos, query[0]);

    bool results_match = Benchmark::validateResults(sadSoA, sadAoS, bestIndexSoA, bestIndexAoS);

    std::cout << "\n=== RESULTS ===" << std::endl;
    std::cout << "Sequential SoA: " << resultSoA.execution_time_ms << " ms" << std::endl;
    std::cout << "Sequential AoS: " << resultAoS.execution_time_ms << " ms" << std::endl;

    double speedup = resultAoS.execution_time_ms / resultSoA.execution_time_ms;
    std::cout << "SoA Speedup: " << std::fixed << std::setprecision(2) << speedup << "x" << std::endl;
    std::cout << "Results match: " << (results_match ? "✓ YES" : "✗ NO") << std::endl;
    std::cout << "Best match index: " << bestIndexSoA << std::endl;
    std::cout << "Best SAD value: " << std::fixed << std::setprecision(6) << resultSoA.best_sad_value << std::endl;

    return 0;
}