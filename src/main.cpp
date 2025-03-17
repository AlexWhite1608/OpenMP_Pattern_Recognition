#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include "../include/DataLoading.h"

int main()
{

    std::string timeseriesPath = "src/utils/data/timeseries/timeseries_2_100.csv";

    // Carico in formato Aos
    std::vector<TimeSeries> datasetAos = loadTimeSeriesAoS(timeseriesPath);

    // Carico in formato SoA
    TimeSeriesDataset datasetSoa = loadTimeSeriesDatasetFromCSV(timeseriesPath);

    std::cout << "Numero di serie temporali nel dataset Aos: " << datasetAos.size() << std::endl;
    std::cout << "Numero di serie temporali nel dataset Soa: " << datasetSoa.getNumSeries() << std::endl;

    return 0;
}