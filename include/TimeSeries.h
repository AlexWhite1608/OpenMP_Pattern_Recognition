#include <vector>
#include <iostream>

// Implementazione AoS
class TimeSeries
{
public:
    TimeSeries(const std::vector<double> &values) : data(values) {}

    size_t getSize() const { return data.size(); }

    const std::vector<double> &getData() const { return data; }

private:
    std::vector<double> data;
};
