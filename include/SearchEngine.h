#ifndef SEARCHENGINE_H
#define SEARCHENGINE_H

#include "TimeSeries.h"
#include "TimeSeriesDataset.h"
#include <vector>
#include <utility>

class SearchEngine
{
public:
    // ricerca sequenziale
    static std::pair<std::vector<double>, size_t> searchSequentialSoA(const TimeSeriesDataset &dataset, const TimeSeries &query);
    static std::pair<std::vector<double>, size_t> searchSequentialAoS(const std::vector<TimeSeries> &dataset, const TimeSeries &query);

    // parallelizzazione loop esterno
    static std::pair<std::vector<double>, size_t> searchSequentialSoA_parallelOuter(const TimeSeriesDataset &dataset, const TimeSeries &query);
    static std::pair<std::vector<double>, size_t> searchSequentialAoS_parallelOuter(const std::vector<TimeSeries> &dataset, const TimeSeries &query);

    // parallelizzazione sliding window
    static std::pair<std::vector<double>, size_t> searchSequentialSoA_parallelInner(const TimeSeriesDataset &dataset, const TimeSeries &query);
    static std::pair<std::vector<double>, size_t> searchSequentialAoS_parallelInner(const std::vector<TimeSeries> &dataset, const TimeSeries &query);
};

#endif // SEARCHENGINE_H