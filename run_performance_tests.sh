#!/bin/bash

echo "=== PERFORMANCE TESTING ==="
echo "Date: $(date)"
echo "=========================================="

PROJECT_ROOT=$(pwd)
mkdir -p output/benchmark_results

# File di output
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
OUTPUT_FILE="output/benchmark_results/performance_results_$TIMESTAMP.txt"
CSV_FILE="output/benchmark_results/performance_summary_$TIMESTAMP.csv"

# Valori da testare
series_counts=(10 50 100 500)
series_lengths=(100 500 1000 2000)
query_lengths=(10 50 100)

total_tests=$((${#series_counts[@]} * ${#series_lengths[@]} * ${#query_lengths[@]}))

# Inizializza files
cat > "$OUTPUT_FILE" << EOF
Performance Benchmark Results - $(date)
Total Tests: $total_tests
================================================================================

EOF

# Legenda colonne dei valori CSV
echo "NumSeries,SeriesLength,QueryLength,SoA_Time_ms,AoS_Time_ms,Speedup,ResultsMatch" > "$CSV_FILE"

current_test=0

for num_series in "${series_counts[@]}"; do
    for series_length in "${series_lengths[@]}"; do
        for query_length in "${query_lengths[@]}"; do
            current_test=$((current_test + 1))
            
            echo "[$current_test/$total_tests] Testing: $num_series × $series_length (query: $query_length)"
            
            # Genera dataset 
            cd "$PROJECT_ROOT/src/utils"
            python generate_timeseries.py $num_series $series_length $query_length > /dev/null 2>&1
            cd "$PROJECT_ROOT"
            
            # Header per questo test
            {
                echo "Test $current_test/$total_tests: $num_series × $series_length (query: $query_length)"
                echo "Time: $(date)"
                echo "----------------------------------------"
            } >> "$OUTPUT_FILE"
            
            # Esegue benchmark
            if [ -f "build/Pattern_Recognition" ]; then
                temp_output=$(mktemp)
                ./build/Pattern_Recognition $num_series $series_length $query_length | tee "$temp_output"
                
                cat "$temp_output" >> "$OUTPUT_FILE"
                echo "" >> "$OUTPUT_FILE"
                
                # Estrai dati per CSV
                soa_time=$(grep "Sequential SoA:" "$temp_output" | sed 's/.*Sequential SoA: \([0-9.]*\) ms.*/\1/')
                aos_time=$(grep "Sequential AoS:" "$temp_output" | sed 's/.*Sequential AoS: \([0-9.]*\) ms.*/\1/')
                speedup=$(grep "SoA Speedup:" "$temp_output" | sed 's/.*SoA Speedup: \([0-9.]*\)x.*/\1/')
                results_match=$(grep "Results match:" "$temp_output" | sed 's/.*Results match: \([^[:space:]]*\).*/\1/')
                
                echo "$num_series,$series_length,$query_length,$soa_time,$aos_time,$speedup,$results_match" >> "$CSV_FILE"
                
                rm "$temp_output"
            else
                echo "ERROR: Pattern_Recognition not found" | tee -a "$OUTPUT_FILE"
                exit 1
            fi
        done
    done
done

echo "All tests completed!"