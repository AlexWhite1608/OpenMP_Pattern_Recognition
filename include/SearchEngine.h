#ifndef SEARCHENGINE_H
#define SEARCHENGINE_H

#include "TimeSeries.h"
#include "TimeSeriesDataset.h"
#include <vector>
#include <utility> 

class SearchEngine
{
public:
    static std::pair<std::vector<double>, size_t> searchSequentialSoA(const TimeSeriesDataset &dataset, const TimeSeries &query);

    // static std::pair<std::vector<double>, size_t> searchSequentialAoS(const std::vector<TimeSeries> &dataset, const TimeSeries &query);
};

#endif // SEARCHENGINE_H