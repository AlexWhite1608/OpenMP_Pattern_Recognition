import csv
import matplotlib.pyplot as plt
import numpy as np

FILE_PATH = "data/timeseries/timeseries.csv"      

def load_and_display_timeseries():
    timeseries_data = []
    
    try:
        with open(FILE_PATH, mode='r') as file:    
            csv_reader = csv.reader(file)
            
            for i, line in enumerate(csv_reader):
                values = [float(val) for val in line if val.strip()]
                timeseries_data.append(values)
                print(f"Timeseries #{i+1}: ha {len(values)} punti: \n")
                
                #print(values)
                
        return timeseries_data
        
    except FileNotFoundError:
        print(f"Errore: File non trovato nel percorso {FILE_PATH}")
        return []
    except Exception as e:
        print(f"Errore caricamento file: {str(e)}")
        return []

def plot_timeseries(timeseries_data):
    if not timeseries_data:
        return
        
    plt.figure(figsize=(12, 8))
    for i, ts in enumerate(timeseries_data):
        plt.plot(ts, label=f"Series {i+1}")
    
    plt.title("Timeseries Data")
    plt.xlabel("Time point")
    plt.ylabel("Value")
    
    if len(timeseries_data) <= 10:  
        plt.legend()
    
    plt.tight_layout()
    plt.show()

data = load_and_display_timeseries()
if data:
    plot_timeseries(data)