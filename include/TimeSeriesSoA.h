#ifndef TIMESERIESSOA_H
#define TIMESERIESSOA_H

#include <vector>
#include <iostream>

class TimeSeriesSoA
{
public:
    void addSeries(const std::vector<double> &values)
    {
        if (values.empty())
            return;

        // Se è la prima serie, inizializza gli array
        if (timePoints.empty())
        {
            timePoints.resize(values.size());
        }

        // Espandi gli array se necessario
        if (values.size() > timePoints.size())
        {
            timePoints.resize(values.size());
        }

        // Aggiungi i valori agli array separati per ogni punto temporale
        for (size_t t = 0; t < values.size(); ++t)
        {
            timePoints[t].push_back(values[t]);
        }

        seriesLengths.push_back(values.size());
        numSeries++;
    }

    size_t getNumSeries() const
    {
        return numSeries;
    }

    size_t getMaxTimePoints() const
    {
        return timePoints.size();
    }

    size_t getSeriesLength(size_t seriesIndex) const
    {
        return seriesLengths[seriesIndex];
    }

    // Accede al valore della serie i al tempo t
    double getValue(size_t seriesIndex, size_t timeIndex) const
    {
        if (timeIndex >= timePoints.size() ||
            seriesIndex >= timePoints[timeIndex].size() ||
            timeIndex >= seriesLengths[seriesIndex])
        {
            return 0.0; // o gestire l'errore diversamente
        }
        return timePoints[timeIndex][seriesIndex];
    }

    // Ottieni tutti i valori al tempo t (per tutte le serie)
    const std::vector<double> &getValuesAtTime(size_t timeIndex) const
    {
        return timePoints[timeIndex];
    }

    void print() const
    {
        for (size_t i = 0; i < numSeries; ++i)
        {
            std::cout << "Serie " << i << ": ";
            for (size_t t = 0; t < seriesLengths[i]; ++t)
            {
                std::cout << getValue(i, t) << " ";
            }
            std::cout << std::endl;
        }
    }

private:
    std::vector<std::vector<double>> timePoints; // Array di valori per ogni punto temporale
    std::vector<size_t> seriesLengths;           // Lunghezza di ogni serie
    size_t numSeries = 0;
};

#endif // TIMESERIESSOA_H