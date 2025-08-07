#include "../include/DataLoading.h"

// Importa la timeseries dal csv (AoS)
std::vector<TimeSeries> loadTimeSeriesAoS(const std::string &filename) {
    std::vector<TimeSeries> timeSeriesList;
    std::ifstream file(filename);
    std::string line;
    
    if (!file.is_open()) {
        std::cerr << "Errore: impossibile aprire il file " << filename << std::endl;
        return timeSeriesList;
    }
    
    while (std::getline(file, line)) {
        std::vector<double> values;
        std::stringstream ss(line);
        std::string value;
        
        while (std::getline(ss, value, ',')) {
            values.push_back(std::stod(value));
        }
        
        if (!values.empty()) {
            timeSeriesList.push_back(TimeSeries(values));
        }
    }
    
    file.close();
    return timeSeriesList;
}

// Importa la timeseries dal csv (SoA)
TimeSeriesSoA loadTimeSeriesSoA(const std::string &filename) {
    TimeSeriesSoA dataset;
    std::ifstream file(filename);
    std::string line;
    
    if (!file.is_open()) {
        std::cerr << "Errore: impossibile aprire il file " << filename << std::endl;
        return dataset;
    }
    
    while (std::getline(file, line)) {
        std::vector<double> values;
        std::stringstream ss(line);
        std::string value;
        
        while (std::getline(ss, value, ',')) {
            values.push_back(std::stod(value));
        }
        
        if (!values.empty()) {
            dataset.addSeries(values);
        }
    }
    
    file.close();
    return dataset;
}

// Importa la query dal csv
TimeSeries loadQueryFromCSV(const std::string &filename) {
    std::ifstream file(filename);
    std::string line;
    std::vector<double> values;
    
    if (!file.is_open()) {
        std::cerr << "Errore: impossibile aprire il file " << filename << std::endl;
        return TimeSeries(values);
    }
    
    if (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string value;
        
        while (std::getline(ss, value, ',')) {
            values.push_back(std::stod(value));
        }
    }
    
    file.close();
    return TimeSeries(values);
}