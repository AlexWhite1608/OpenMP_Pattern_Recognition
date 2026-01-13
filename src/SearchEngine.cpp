#include "../include/SearchEngine.h"
#include <cmath>
#include "SearchEngine.h"

std::pair<std::vector<double>, size_t> SearchEngine::searchSequentialSoA(const TimeSeriesSoA &dataset, const TimeSeries &query)
{
    std::vector<double> sadValues(dataset.getNumSeries(), std::numeric_limits<double>::max());
    size_t queryLength = query.getSize();
    size_t bestIndex = 0;
    double bestSad = std::numeric_limits<double>::max();
    const std::vector<double> &queryData = query.getData();

    for (size_t i = 0; i < dataset.getNumSeries(); ++i)
    {
        size_t seriesLength = dataset.getSeriesLength(i);
        double minSad = std::numeric_limits<double>::max();

        for (size_t j = 0; j <= seriesLength - queryLength; ++j)
        {
            double sad = 0.0;

            for (size_t k = 0; k < queryLength; ++k)
            {
                double seriesValue = dataset.getValue(i, j + k);
                sad += std::abs(seriesValue - queryData[k]);
            }

            if (sad < minSad)
            {
                minSad = sad;
            }
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

std::pair<std::vector<double>, size_t> SearchEngine::searchSequentialAoS(const TimeSeriesAoS &dataset, const TimeSeries &query)
{
    std::vector<double> sadValues(dataset.getNumSeries(), std::numeric_limits<double>::max());
    size_t queryLength = query.getSize();
    size_t bestIndex = 0;
    double bestSad = std::numeric_limits<double>::max();
    const std::vector<double> &queryData = query.getData();

    for (size_t i = 0; i < dataset.getNumSeries(); ++i)
    {
        const std::vector<double> &seriesData = dataset.getSeries(i);
        size_t seriesLength = seriesData.size();
        double minSad = std::numeric_limits<double>::max();

        for (size_t j = 0; j <= seriesLength - queryLength; ++j)
        {
            double sad = 0.0;

            for (size_t k = 0; k < queryLength; ++k)
            {
                sad += std::abs(seriesData[j + k] - queryData[k]);
            }

            if (sad < minSad)
            {
                minSad = sad;
            }
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

std::pair<std::vector<double>, size_t> SearchEngine::searchParallelAoSOuter(const TimeSeriesAoS &dataset, const TimeSeries &query)
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
            const std::vector<double> &seriesData = dataset.getSeries(i);
            size_t seriesLength = seriesData.size();

            double minSad = std::numeric_limits<double>::max();

            for (size_t j = 0; j <= seriesLength - queryLength; ++j)
            {
                double sad = 0.0;

#pragma omp simd reduction(+ : sad)
                for (size_t k = 0; k < queryLength; ++k)
                {
                    sad += std::abs(seriesData[j + k] - queryData[k]);
                }

                if (sad < minSad)
                {
                    minSad = sad;
                }
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

std::pair<std::vector<double>, size_t> SearchEngine::searchParallelAoSInner(const TimeSeriesAoS &dataset, const TimeSeries &query)
{
    size_t numSeries = dataset.getNumSeries();
    size_t queryLength = query.getSize();
    const auto &queryData = query.getData();

    std::vector<double> sadValues(numSeries, std::numeric_limits<double>::max());
    size_t bestIndex = 0;
    double bestSad = std::numeric_limits<double>::max();

    // Loop sequenziale sulle serie
    for (size_t i = 0; i < numSeries; ++i)
    {
        const std::vector<double> &seriesData = dataset.getSeries(i);
        size_t seriesLength = seriesData.size();

        double minSad = std::numeric_limits<double>::max();

        // Parallelizzazione sulle posizioni nella serie
#pragma omp parallel for reduction(min : minSad) schedule(static)
        for (size_t j = 0; j <= seriesLength - queryLength; ++j)
        {
            double sad = 0.0;

            // Vettorizzazione del calcolo SAD
#pragma omp simd reduction(+ : sad)
            for (size_t k = 0; k < queryLength; ++k)
            {
                sad += std::abs(seriesData[j + k] - queryData[k]);
            }

            if (sad < minSad)
            {
                minSad = sad;
            }
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

std::pair<std::vector<double>, size_t> SearchEngine::searchParallelSoAOuter(const TimeSeriesSoA &dataset, const TimeSeries &query)
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
            size_t seriesLength = dataset.getSeriesLength(i);

            double minSad = std::numeric_limits<double>::max();

            // Loop sequenziale sulle posizioni
            for (size_t j = 0; j <= seriesLength - queryLength; ++j)
            {
                double sad = 0.0;

                // Calcolo SAD con accesso SoA
#pragma omp simd reduction(+ : sad)
                for (size_t k = 0; k < queryLength; ++k)
                {
                    double seriesValue = dataset.getValue(i, j + k);
                    sad += std::abs(seriesValue - queryData[k]);
                }

                if (sad < minSad)
                {
                    minSad = sad;
                }
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

std::pair<std::vector<double>, size_t> SearchEngine::searchParallelSoAInner(const TimeSeriesSoA &dataset, const TimeSeries &query)
{
    size_t numSeries = dataset.getNumSeries();
    size_t queryLength = query.getSize();
    const auto &queryData = query.getData();

    std::vector<double> sadValues(numSeries, std::numeric_limits<double>::max());
    size_t bestIndex = 0;
    double bestSad = std::numeric_limits<double>::max();

    for (size_t i = 0; i < numSeries; ++i)
    {
        size_t seriesLength = dataset.getSeriesLength(i);

        double minSad = std::numeric_limits<double>::max();

#pragma omp parallel for reduction(min : minSad) schedule(static)
        for (size_t j = 0; j <= seriesLength - queryLength; ++j)
        {
            double sad = 0.0;

#pragma omp simd reduction(+ : sad)
            for (size_t k = 0; k < queryLength; ++k)
            {
                double seriesValue = dataset.getValue(i, j + k);
                sad += std::abs(seriesValue - queryData[k]);
            }

            if (sad < minSad)
            {
                minSad = sad;
            }
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
