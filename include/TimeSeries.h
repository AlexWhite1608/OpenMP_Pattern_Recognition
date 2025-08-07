#ifndef TIMESERIES_H
#define TIMESERIES_H

#include <vector>
#include <iostream>

class TimeSeries
{
public:
    TimeSeries(const std::vector<double> &values) : data(values) {}

    size_t getSize() const { return data.size(); }
    const std::vector<double> &getData() const { return data; }
    double getValue(size_t index) const { return data[index]; }

    void print() const
    {
        for (const auto &value : data)
        {
            std::cout << value << " ";
        }
        std::cout << std::endl;
    }

private:
    std::vector<double> data;
};

#endif // TIMESERIES_H