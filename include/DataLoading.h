#ifndef DATALOADING_H
#define DATALOADING_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "TimeSeries.h"
#include "TimeSeriesAoS.h"
#include "TimeSeriesSoA.h"


// Importa la timeseries dal csv (AoS)
std::vector<TimeSeries> loadTimeSeriesAoS(const std::string &filename);

// Importa la timeseries dal csv (SoA)
TimeSeriesSoA loadTimeSeriesSoA(const std::string &filename);

// Importa la query dal csv
TimeSeries loadQueryFromCSV(const std::string &filename);

#endif // DATALOADING_H