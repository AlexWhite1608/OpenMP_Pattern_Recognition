#include "../include/SearchEngine.h"
#include <cmath>
#include "SearchEngine.h"

std::pair<std::vector<double>, size_t> SearchEngine::searchSequentialSoA(const TimeSeriesDataset &dataset, const TimeSeries &query)
{
    std::vector<double> sadValues(dataset.getNumSeries(), std::numeric_limits<double>::max());
    size_t queryLength = query.getSize();
    size_t bestIndex = 0;
    double bestSad = std::numeric_limits<double>::max();

    for (size_t i = 0; i < dataset.getNumSeries(); ++i)
    {
        const std::vector<double> &series = dataset.getSeries(i);
        size_t seriesLength = series.size();

        double minSad = std::numeric_limits<double>::max();

        for (size_t j = 0; j <= seriesLength - queryLength; ++j)
        {
            double sad = 0.0;
            for (size_t k = 0; k < queryLength; ++k)
            {
                sad += std::abs(series[j + k] - query.getData()[k]);
            }
            if (sad < minSad)
            {
                minSad = sad;
            }
        }

        sadValues[i] = minSad; // valore SAD per questa serie

        // si aggiorna la serie con la SAD migliore
        if (minSad < bestSad)
        {
            bestSad = minSad;
            bestIndex = i;
        }
    }

    return {sadValues, bestIndex};
}

std::pair<std::vector<double>, size_t> SearchEngine::searchSequentialAoS(const std::vector<TimeSeries> &dataset, const TimeSeries &query)
{
    std::vector<double> sadValues(dataset.size(), std::numeric_limits<double>::max());
    size_t queryLength = query.getSize();
    size_t bestIndex = 0;
    double bestSad = std::numeric_limits<double>::max();

    for (size_t i = 0; i < dataset.size(); ++i)
    {
        const TimeSeries &timeseries = dataset[i];
        size_t seriesLength = timeseries.getSize();

        double minSad = std::numeric_limits<double>::max();

        for (size_t j = 0; j <= seriesLength - queryLength; ++j)
        {
            double sad = 0.0;
            for (size_t k = 0; k < queryLength; ++k)
            {
                sad += std::abs(timeseries.getData()[j + k] - query.getData()[k]);
            }
            if (sad < minSad)
            {
                minSad = sad;
            }
        }

        sadValues[i] = minSad; // valore SAD per questa serie

        // si aggiorna la serie con la SAD migliore
        if (minSad < bestSad)
        {
            bestSad = minSad;
            bestIndex = i;
        }
    }

    return {sadValues, bestIndex};
}

std::pair<std::vector<double>, size_t> SearchEngine::searchParallelSoAOuter(const TimeSeriesDataset &dataset, const TimeSeries &query)
{
    size_t numSeries = dataset.getNumSeries();
    size_t queryLength = query.getSize();
    const auto &queryData = query.getData();

    std::vector<double> sadValues(numSeries, std::numeric_limits<double>::max());
    size_t bestIndex = 0;
    double bestSad = std::numeric_limits<double>::max();

#pragma omp parallel
    {
        double localBestSad = std::numeric_limits<double>::max();
        size_t localBestIndex = 0;

#pragma omp for schedule(dynamic)
        for (size_t i = 0; i < numSeries; ++i)
        {
            const auto &series = dataset.getSeries(i);
            size_t seriesLength = series.size();

            double minSad = std::numeric_limits<double>::max();

            for (size_t j = 0; j <= seriesLength - queryLength; ++j)
            {
                double sad = 0.0;

#pragma omp simd reduction(+ : sad)
                for (size_t k = 0; k < queryLength; ++k)
                    sad += std::abs(series[j + k] - queryData[k]);

                if (sad < minSad)
                    minSad = sad;
            }

            sadValues[i] = minSad;

            if (minSad < localBestSad)
            {
                localBestSad = minSad;
                localBestIndex = i;
            }
        }

#pragma omp critical
        {
            if (localBestSad < bestSad)
            {
                bestSad = localBestSad;
                bestIndex = localBestIndex;
            }
        }
    }

    return {sadValues, bestIndex};
}

std::pair<std::vector<double>, size_t> SearchEngine::searchParallelAoSOuter(const std::vector<TimeSeries> &dataset, const TimeSeries &query)
{
    size_t numSeries = dataset.size();
    size_t queryLength = query.getSize();
    const auto &queryData = query.getData();

    std::vector<double> sadValues(numSeries, std::numeric_limits<double>::max());
    size_t bestIndex = 0;
    double bestSad = std::numeric_limits<double>::max();

#pragma omp parallel
    {
        double localBestSad = std::numeric_limits<double>::max();
        size_t localBestIndex = 0;

#pragma omp for schedule(dynamic)
        for (size_t i = 0; i < numSeries; ++i)
        {
            const auto &timeseries = dataset[i];
            const auto &seriesData = timeseries.getData();
            size_t seriesLength = timeseries.getSize();

            double minSad = std::numeric_limits<double>::max();

            for (size_t j = 0; j <= seriesLength - queryLength; ++j)
            {
                double sad = 0.0;

#pragma omp simd reduction(+ : sad)
                for (size_t k = 0; k < queryLength; ++k)
                    sad += std::abs(seriesData[j + k] - queryData[k]);

                if (sad < minSad)
                    minSad = sad;
            }

            sadValues[i] = minSad;

            if (minSad < localBestSad)
            {
                localBestSad = minSad;
                localBestIndex = i;
            }
        }

#pragma omp critical
        {
            if (localBestSad < bestSad)
            {
                bestSad = localBestSad;
                bestIndex = localBestIndex;
            }
        }
    }

    return {sadValues, bestIndex};
}

std::pair<std::vector<double>, size_t> SearchEngine::searchParallelSoAInner(const TimeSeriesDataset &dataset, const TimeSeries &query)
{
    size_t numSeries = dataset.getNumSeries();
    size_t queryLength = query.getSize();
    const auto &queryData = query.getData();

    std::vector<double> sadValues(numSeries, std::numeric_limits<double>::max());
    size_t bestIndex = 0;
    double bestSad = std::numeric_limits<double>::max();

    for (size_t i = 0; i < numSeries; ++i)
    {
        const auto &series = dataset.getSeries(i);
        size_t seriesLength = series.size();

        double minSad = std::numeric_limits<double>::max();

#pragma omp parallel for reduction(min : minSad) schedule(static)
        for (size_t j = 0; j <= seriesLength - queryLength; ++j)
        {
            double sad = 0.0;
#pragma omp simd reduction(+ : sad)
            for (size_t k = 0; k < queryLength; ++k)
                sad += std::abs(series[j + k] - queryData[k]);

            if (sad < minSad)
                minSad = sad;
        }

        sadValues[i] = minSad;

        if (minSad < bestSad)
        {
            bestSad = minSad;
            bestIndex = i;
        }
    }

    return {sadValues, bestIndex};
}

std::pair<std::vector<double>, size_t> SearchEngine::searchParallelAoSInner(const std::vector<TimeSeries> &dataset, const TimeSeries &query)
{
    size_t numSeries = dataset.size();
    size_t queryLength = query.getSize();
    const auto &queryData = query.getData();

    std::vector<double> sadValues(numSeries, std::numeric_limits<double>::max());
    size_t bestIndex = 0;
    double bestSad = std::numeric_limits<double>::max();

    for (size_t i = 0; i < numSeries; ++i)
    {
        const auto &timeseries = dataset[i];
        const auto &seriesData = timeseries.getData();
        size_t seriesLength = timeseries.getSize();

        double minSad = std::numeric_limits<double>::max();

#pragma omp parallel for reduction(min : minSad) schedule(static)
        for (size_t j = 0; j <= seriesLength - queryLength; ++j)
        {
            double sad = 0.0;
#pragma omp simd reduction(+ : sad)
            for (size_t k = 0; k < queryLength; ++k)
                sad += std::abs(seriesData[j + k] - queryData[k]);

            if (sad < minSad)
                minSad = sad;
        }

        sadValues[i] = minSad;

        if (minSad < bestSad)
        {
            bestSad = minSad;
            bestIndex = i;
        }
    }

    return {sadValues, bestIndex};
}
