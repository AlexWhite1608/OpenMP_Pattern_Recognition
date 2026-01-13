#ifndef DATALOADING_H
#define DATALOADING_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "TimeSeries.h"
#include "TimeSeriesAoS.h"
#include "TimeSeriesSoA.h"


std::vector<TimeSeries> loadTimeSeriesAoS(const std::string &filename);

TimeSeriesSoA loadTimeSeriesSoA(const std::string &filename);

TimeSeries loadQueryFromCSV(const std::string &filename);

#endif // DATALOADING_H