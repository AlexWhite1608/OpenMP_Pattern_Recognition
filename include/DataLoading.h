#ifndef DATALOADING_H
#define DATALOADING_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "TimeSeries.h"
#include "TimeSeriesDataset.h"

// Importa la timeseries dal csv (AoS)
std::vector<TimeSeries> loadTimeSeriesAoS(const std::string &filename);

// Importa la timeseries dal csv (SoA)
TimeSeriesDataset loadTimeSeriesDatasetSoA(const std::string &filename);

// Importa la query dal csv
TimeSeries loadQueryFromCSV(const std::string &filename);

#endif // DATALOADING_H