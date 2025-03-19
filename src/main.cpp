#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include "../include/DataLoading.h"
#include "../include/SearchEngine.h"

int main()
{

    std::string timeseriesPath = "src/utils/data/timeseries/timeseries_2_100.csv";
    std::string queryPath = "src/utils/data/query/query_50.csv";

    // carico dati da csv
    std::vector<TimeSeries> datasetAos = loadTimeSeriesAoS(timeseriesPath);
    TimeSeriesDataset datasetSoa = loadTimeSeriesDatasetSoA(timeseriesPath);
    std::vector<TimeSeries> query = loadTimeSeriesAoS(queryPath);

    std::cout << "Numero di serie temporali nel dataset Aos: " << datasetAos.size() << std::endl;
    std::cout << "Numero di serie temporali nel dataset Soa: " << datasetSoa.getNumSeries() << std::endl;

    auto [sadValues, bestSadValueIndex] = SearchEngine::searchSequentialSoA(datasetSoa, query[0]);

    for (auto i = 0; i < sadValues.size(); i++)
    {
        std::cout << "Indice serie temporale SoA: " << i << " SAD: " << sadValues[i] << std::endl;
    }

    std::cout << "Indice serie temporale SoA con SAD minore: " << bestSadValueIndex << std::endl;

    return 0;
}