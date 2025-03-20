#include "../include/SearchEngine.h"
#include <cmath>

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

        // se la serie è più corta della query
        if (seriesLength < queryLength)
        {
            std::cerr << "la serie temporale è più corta della query" << std::endl;
            continue;
        }

        double minSad = std::numeric_limits<double>::max();

        // query e timeseries hanno la stessa lunghezza
        if (seriesLength == queryLength)
        {
            double sad = 0.0;
            for (size_t k = 0; k < queryLength; ++k)
            {
                sad += std::abs(series[k] - query.getData()[k]);
            }
            minSad = sad;
        }
        // la query è più corta della timeseries quindi faccio una sliding window
        else
        {
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

        // se la serie è più corta della query
        if (seriesLength < queryLength)
        {
            std::cerr << "la serie temporale è più corta della query" << std::endl;
            continue;
        }

        double minSad = std::numeric_limits<double>::max();

        // query e timeseries hanno la stessa lunghezza
        if (seriesLength == queryLength)
        {
            double sad = 0.0;
            for (size_t k = 0; k < queryLength; ++k)
            {
                sad += std::abs(timeseries.getData()[k] - query.getData()[k]);
            }
            minSad = sad;
        }
        // la query è più corta della timeseries quindi faccio una sliding window
        else
        {
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
