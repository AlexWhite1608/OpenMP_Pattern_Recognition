#ifndef TIMESERIESSOA_H
#define TIMESERIESSOA_H

#include <vector>
#include <iostream>

class TimeSeriesSoA
{
public:
    void addSeries(const std::vector<double> &values)
    {
        if (values.empty())
            return;

        if (timePoints.empty())
        {
            timePoints.resize(values.size());
        }

        if (values.size() > timePoints.size())
        {
            timePoints.resize(values.size());
        }

        for (size_t t = 0; t < values.size(); ++t)
        {
            timePoints[t].push_back(values[t]);
        }

        seriesLengths.push_back(values.size());
        numSeries++;
    }

    size_t getNumSeries() const
    {
        return numSeries;
    }

    size_t getMaxTimePoints() const
    {
        return timePoints.size();
    }

    size_t getSeriesLength(size_t seriesIndex) const
    {
        return seriesLengths[seriesIndex];
    }

    inline double getValue(size_t seriesIndex, size_t timeIndex) const
    {
        return timePoints[timeIndex][seriesIndex];
    }

private:
    std::vector<std::vector<double>> timePoints;
    std::vector<size_t> seriesLengths;           
    size_t numSeries = 0;
};

#endif // TIMESERIESSOA_H