#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "TimeSeries.h"
#include "TimeSeriesDataset.h"

// Importa la timeseries dal csv (AoS)
std::vector<TimeSeries> loadTimeSeriesAoS(const std::string &filename)
{
    std::vector<TimeSeries> dataset;
    std::ifstream file(filename);

    if (!file.is_open())
    {
        std::cerr << "Errore: impossibile aprire il file " << filename << std::endl;
        return dataset;
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::vector<double> values;
        std::stringstream ss(line);
        std::string cell;

        while (std::getline(ss, cell, ','))
        {
            values.push_back(std::stod(cell)); // Converte stringa in double
        }

        dataset.emplace_back(values);
    }

    file.close();
    return dataset;
}

// Importa la timeseries dal csv (SoA)
TimeSeriesDataset loadTimeSeriesDatasetFromCSV(const std::string &filename)
{
    TimeSeriesDataset dataset;
    std::ifstream file(filename);

    if (!file.is_open())
    {
        std::cerr << "Errore: impossibile aprire il file " << filename << std::endl;
        return dataset;
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::vector<double> values;
        std::stringstream ss(line);
        std::string cell;

        while (std::getline(ss, cell, ','))
        {
            values.push_back(std::stod(cell)); 
        }

        dataset.addSeries(values); 
    }

    file.close();
    return dataset;
}

// Importa la query dal csv
TimeSeries loadQueryFromCSV(const std::string &filename)
{
    std::ifstream file(filename);
    std::string line;

    if (!file.is_open() || !std::getline(file, line))
    {
        std::cerr << "Errore: impossibile aprire il file " << filename << std::endl;
        return TimeSeries({});
    }

    std::vector<double> values;
    std::stringstream ss(line);
    std::string cell;

    while (std::getline(ss, cell, ','))
    {
        values.push_back(std::stod(cell));
    }

    return TimeSeries(values);
}
