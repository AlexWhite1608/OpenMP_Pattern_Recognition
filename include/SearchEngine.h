#ifndef SEARCHENGINE_H
#define SEARCHENGINE_H

#include "TimeSeries.h"
#include <vector>
#include <utility>
#include <omp.h>
#include "TimeSeriesAoS.h"
#include "TimeSeriesSoA.h"

class SearchEngine
{
public:
    static std::pair<std::vector<double>, size_t> searchSequentialSoA(
        const TimeSeriesSoA &dataset,
        const TimeSeries &query);
    static std::pair<std::vector<double>, size_t> searchSequentialAoS(
        const TimeSeriesAoS &dataset,
        const TimeSeries &query);

    static std::pair<std::vector<double>, size_t> searchParallelAoSOuter(
        const TimeSeriesAoS &dataset,
        const TimeSeries &query);
    static std::pair<std::vector<double>, size_t> searchParallelAoSInner(
        const TimeSeriesAoS &dataset,
        const TimeSeries &query);

    static std::pair<std::vector<double>, size_t> searchParallelSoAOuter(
        const TimeSeriesSoA &dataset,
        const TimeSeries &query);
    static std::pair<std::vector<double>, size_t> searchParallelSoAInner(
        const TimeSeriesSoA &dataset,
        const TimeSeries &query);
};

#endif // SEARCHENGINE_H