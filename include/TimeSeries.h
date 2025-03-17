#include <vector>
#include <iostream>

// Implementazione AoS
class TimeSeries
{
public:
    TimeSeries(const std::vector<double> &values) : data(values) {}

    size_t getSize() const { return data.size(); }

    const std::vector<double> &getData() const { return data; }

    void print() const
    {
        for (const auto &value : data)
        {
            std::cout << value << "\n";
        }
        std::cout << std::endl;
    }

private:
    std::vector<double> data;
};
