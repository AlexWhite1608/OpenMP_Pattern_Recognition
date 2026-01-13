#ifndef TIMESERIESAOS_H
#define TIMESERIESAOS_H

#include <vector>
#include <iostream>

struct Sample
{
    double value;
    
    inline operator double() const { return value; }
    inline Sample& operator=(double val) { value = val; return *this; }
    inline Sample(double val = 0.0) : value(val) {}
};

class TimeSeriesAoS
{
public:
    void addSeries(const std::vector<double> &values)
    {
        std::vector<Sample> sampleSeries;
        sampleSeries.reserve(values.size());
        for (double val : values)
        {
            sampleSeries.emplace_back(val);
        }
        data.push_back(std::move(sampleSeries));
    }

    size_t getNumSeries() const
    {
        return data.size();
    }

    size_t getSeriesLength() const
    {
        if (data.empty())
            return 0;
        return data[0].size();
    }

    const std::vector<Sample> &getSeriesSamples(size_t index) const
    {
        return data[index];
    }

    const std::vector<double> getSeries(size_t index) const
    {
        const auto &samples = data[index];
        std::vector<double> result;
        result.reserve(samples.size());
        for (const auto &sample : samples)
        {
            result.push_back(sample.value);
        }
        return result;
    }

    inline const Sample &getSample(size_t seriesIndex, size_t timeIndex) const
    {
        return data[seriesIndex][timeIndex];
    }

    inline double getValue(size_t seriesIndex, size_t timeIndex) const
    {
        return data[seriesIndex][timeIndex].value;
    }

    void print() const
    {
        for (const auto &series : data)
        {
            for (const auto &sample : series)
            {
                std::cout << sample.value << " ";
            }
            std::cout << std::endl;
        }
    }

private:
    std::vector<std::vector<Sample>> data;
};

#endif // TIMESERIESAOS_H