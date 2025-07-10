import mockseries as ms
import pandas as pd
import sys
import os
from datetime import timedelta
from datetime import datetime
from mockseries.trend import LinearTrend
from mockseries.seasonality import SinusoidalSeasonality
from mockseries.noise import GaussianNoise
from mockseries.utils import datetime_range

os.makedirs("data/timeseries", exist_ok=True)
os.makedirs("data/query", exist_ok=True)

NUM_SERIES = int(sys.argv[1])       # numero di serie 
SERIES_LENGTH = int(sys.argv[2])    # lunghezza di ogni serie
QUERY_LENGTH = int(sys.argv[3])     # lunghezza query

CSV_FILENAME = f"data/timeseries/timeseries.csv"
QUERY_FILENAME = f"data/query/query.csv"

# Generazione del dataset a partire dai dati ricevuti in input dove il trend è casuale
# con stagionalità sinusoidale e rumore gaussiano
def generate_series(length):
    trend = LinearTrend(coefficient=2, time_unit=timedelta(days=4), flat_base=100)
    seasonality = SinusoidalSeasonality(amplitude=20, period=timedelta(days=7))
    noise = GaussianNoise(mean=0, std=5)
    
    series = trend + seasonality + noise
    
    # calcola un intervallo di tempo adeguato per garantire abbastanza punti
    required_days = (length // 24) + 2  # +2 per sicurezza
    
    time_points = datetime_range(
        granularity=timedelta(hours=1),
        start_time=datetime(2021, 1, 1),
        end_time=datetime(2021, 1, 1) + timedelta(days=required_days)
    )
    ts_values = series.generate(time_points=time_points)
    
    # prende solo i primi 'length' valori
    if len(ts_values) < length:
        print(f"richiesti {length} punti, ma ne sono disponibili solo {len(ts_values)}")
        return ts_values  # restituisce comunque i valori disponibili
    
    return ts_values[:length]

# genera dataset
dataset = [generate_series(SERIES_LENGTH) for _ in range(NUM_SERIES)]
df_dataset = pd.DataFrame(dataset)
df_dataset.to_csv(CSV_FILENAME, index=False, header=False)

# genera query
query_series = generate_series(QUERY_LENGTH)
df_query = pd.DataFrame([query_series])
df_query.to_csv(QUERY_FILENAME, index=False, header=False)
