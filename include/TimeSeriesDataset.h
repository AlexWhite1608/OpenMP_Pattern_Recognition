#ifndef TIMESERIESDATASET_H
#define TIMESERIESDATASET_H

#include <vector>
#include <iostream>

// Implementazione SoA
class TimeSeriesDataset
{
public:
    void addSeries(const std::vector<double> &values)
    {
        data.push_back(values);
    }

    // ritorna il numero di serie temporali nel dataset
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

    // restituisce la serie temporale all'indice specificato
    const std::vector<double> &getSeries(size_t index) const
    {
        return data[index];
    }

    void print() const
    {
        for (const auto &series : data)
        {
            for (const auto &value : series)
            {
                std::cout << value << " ";
            }
            std::cout << std::endl;
        }
    }

private:
    std::vector<std::vector<double>> data;
};

#endif // TIMESERIESDATASET_H