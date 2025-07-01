#include "../include/Benchmark.h"
#include <iostream>
#include <iomanip>
#include <cmath>

BenchmarkResult Benchmark::benchmarkSequentialSoA(const TimeSeriesDataset& dataset, 
                                                   const TimeSeries& query,
                                                   const std::string& test_name) {
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

BenchmarkResult Benchmark::benchmarkSequentialAoS(const std::vector<TimeSeries>& dataset,
                                                   const TimeSeries& query, 
                                                   const std::string& test_name) {
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

bool Benchmark::validateResults(const std::vector<double>& sadSoA, 
                              const std::vector<double>& sadAoS,
                              size_t bestIndexSoA, 
                              size_t bestIndexAoS) {
    if (sadSoA.size() != sadAoS.size()) return false;
    if (bestIndexSoA != bestIndexAoS) return false;
    
    const double EPSILON = 1e-10;
    for (size_t i = 0; i < sadSoA.size(); ++i) {
        if (std::abs(sadSoA[i] - sadAoS[i]) > EPSILON) {
            return false;
        }
    }
    return true;
}

//TODO:
void Benchmark::saveResultsToCSV(const std::vector<BenchmarkResult>& results, 
                                const std::string& filename) {
    std::ofstream file(filename);
    
    // Header
    file << "Algorithm,NumSeries,SeriesLength,QueryLength,ExecutionTime_ms,BestIndex,BestSAD,ResultsMatch\n";
    
    // Data
    for (const auto& result : results) {
        file << result.algorithm_name << ","
             << result.num_series << ","
             << result.series_length << ","
             << result.query_length << ","
             << std::fixed << std::setprecision(3) << result.execution_time_ms << ","
             << result.best_match_index << ","
             << std::fixed << std::setprecision(6) << result.best_sad_value << ","
             << (result.results_match_reference ? "true" : "false") << "\n";
    }
}