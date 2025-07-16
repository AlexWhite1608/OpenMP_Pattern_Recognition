import json
import matplotlib.pyplot as plt
import numpy as np
import os
from pathlib import Path
import seaborn as sns

# Configura lo stile dei grafici
plt.style.use('seaborn-v0_8')
sns.set_palette("husl")

def load_benchmark_data(json_file):
    """Carica i dati dal file JSON"""
    with open(json_file, 'r') as f:
        data = json.load(f)
    return data

def extract_performance_data(data):
    """Estrae i dati di performance dal JSON"""
    results = []
    
    for test in data['tests']:
        test_name = test['test_name']
        config = test['configuration']
        
        # Estrae i dati SoA
        soa_seq = test['results']['soa']['sequential']['execution_time_ms']
        soa_outer = test['results']['soa']['parallel_outer']['execution_time_ms']
        soa_inner = test['results']['soa']['parallel_inner']['execution_time_ms']
        soa_outer_speedup = test['results']['soa']['parallel_outer']['speedup']
        soa_inner_speedup = test['results']['soa']['parallel_inner']['speedup']
        
        # Estrae i dati AoS
        aos_seq = test['results']['aos']['sequential']['execution_time_ms']
        aos_outer = test['results']['aos']['parallel_outer']['execution_time_ms']
        aos_inner = test['results']['aos']['parallel_inner']['execution_time_ms']
        aos_outer_speedup = test['results']['aos']['parallel_outer']['speedup']
        aos_inner_speedup = test['results']['aos']['parallel_inner']['speedup']
        
        # Analisi
        analysis = test['analysis']
        
        result = {
            'test_name': test_name,
            'num_series': config['num_series'],
            'series_length': config['series_length'],
            'query_length': config['query_length'],
            'soa_sequential': soa_seq,
            'soa_parallel_outer': soa_outer,
            'soa_parallel_inner': soa_inner,
            'soa_outer_speedup': soa_outer_speedup,
            'soa_inner_speedup': soa_inner_speedup,
            'aos_sequential': aos_seq,
            'aos_parallel_outer': aos_outer,
            'aos_parallel_inner': aos_inner,
            'aos_outer_speedup': aos_outer_speedup,
            'aos_inner_speedup': aos_inner_speedup,
            'soa_vs_aos_sequential': analysis['soa_vs_aos_sequential'],
            'soa_vs_aos_parallel_outer': analysis['soa_vs_aos_parallel_outer'],
            'soa_vs_aos_parallel_inner': analysis['soa_vs_aos_parallel_inner']
        }
        results.append(result)
    
    return results

def create_execution_time_comparison(results, output_dir):
    """Crea tre grafici separati per il confronto dei tempi di esecuzione"""
    
    test_names = [r['test_name'] for r in results]
    x = np.arange(len(test_names))
    width = 0.25
    
    colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd', '#8c564b']
    
    # Dati per i grafici
    soa_seq = [r['soa_sequential'] for r in results]
    soa_outer = [r['soa_parallel_outer'] for r in results]
    soa_inner = [r['soa_parallel_inner'] for r in results]
    
    aos_seq = [r['aos_sequential'] for r in results]
    aos_outer = [r['aos_parallel_outer'] for r in results]
    aos_inner = [r['aos_parallel_inner'] for r in results]
    
    # GRAFICO 1: SoA Performance
    fig1, ax1 = plt.subplots(figsize=(12, 8))
    ax1.bar(x - width, soa_seq, width, label='Sequential', alpha=0.8, color=colors[0])
    ax1.bar(x, soa_outer, width, label='Parallel Outer', alpha=0.8, color=colors[1])
    ax1.bar(x + width, soa_inner, width, label='Parallel Inner', alpha=0.8, color=colors[2])
    ax1.set_title('SoA (Structure of Arrays) Performance', fontsize=16, fontweight='bold')
    ax1.set_ylabel('Execution Time (ms)', fontsize=12)
    ax1.set_xlabel('Test Configuration', fontsize=12)
    ax1.set_xticks(x)
    ax1.set_xticklabels(test_names, rotation=45, ha='right')
    ax1.legend()
    ax1.grid(True, alpha=0.3)
    
    for i, (seq, outer, inner) in enumerate(zip(soa_seq, soa_outer, soa_inner)):
        ax1.text(i - width, seq + max(soa_seq) * 0.01, f'{seq:.1f}', ha='center', va='bottom', fontsize=9)
        ax1.text(i, outer + max(soa_seq) * 0.01, f'{outer:.1f}', ha='center', va='bottom', fontsize=9)
        ax1.text(i + width, inner + max(soa_seq) * 0.01, f'{inner:.1f}', ha='center', va='bottom', fontsize=9)
    
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'soa_performance_comparison.png'), dpi=300, bbox_inches='tight')
    plt.close()
    
    # GRAFICO 2: AoS Performance
    fig2, ax2 = plt.subplots(figsize=(12, 8))
    ax2.bar(x - width, aos_seq, width, label='Sequential', alpha=0.8, color=colors[0])
    ax2.bar(x, aos_outer, width, label='Parallel Outer', alpha=0.8, color=colors[1])
    ax2.bar(x + width, aos_inner, width, label='Parallel Inner', alpha=0.8, color=colors[2])
    ax2.set_title('AoS (Array of Structures) Performance', fontsize=16, fontweight='bold')
    ax2.set_ylabel('Execution Time (ms)', fontsize=12)
    ax2.set_xlabel('Test Configuration', fontsize=12)
    ax2.set_xticks(x)
    ax2.set_xticklabels(test_names, rotation=45, ha='right')
    ax2.legend()
    ax2.grid(True, alpha=0.3)
    
    for i, (seq, outer, inner) in enumerate(zip(aos_seq, aos_outer, aos_inner)):
        ax2.text(i - width, seq + max(aos_seq) * 0.01, f'{seq:.1f}', ha='center', va='bottom', fontsize=9)
        ax2.text(i, outer + max(aos_seq) * 0.01, f'{outer:.1f}', ha='center', va='bottom', fontsize=9)
        ax2.text(i + width, inner + max(aos_seq) * 0.01, f'{inner:.1f}', ha='center', va='bottom', fontsize=9)
    
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'aos_performance_comparison.png'), dpi=300, bbox_inches='tight')
    plt.close()
    
    # GRAFICO 3: SoA vs AoS Sequential Comparison
    fig3, ax3 = plt.subplots(figsize=(12, 8))
    ax3.bar(x - width/2, soa_seq, width, label='SoA Sequential', alpha=0.8, color=colors[3])
    ax3.bar(x + width/2, aos_seq, width, label='AoS Sequential', alpha=0.8, color=colors[4])
    ax3.set_title('Sequential Performance: SoA vs AoS', fontsize=16, fontweight='bold')
    ax3.set_ylabel('Execution Time (ms)', fontsize=12)
    ax3.set_xlabel('Test Configuration', fontsize=12)
    ax3.set_xticks(x)
    ax3.set_xticklabels(test_names, rotation=45, ha='right')
    ax3.legend()
    ax3.grid(True, alpha=0.3)
    
    for i, (soa, aos) in enumerate(zip(soa_seq, aos_seq)):
        ax3.text(i - width/2, soa + max(max(soa_seq), max(aos_seq)) * 0.01, f'{soa:.1f}', ha='center', va='bottom', fontsize=9)
        ax3.text(i + width/2, aos + max(max(soa_seq), max(aos_seq)) * 0.01, f'{aos:.1f}', ha='center', va='bottom', fontsize=9)
    
    
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'soa_vs_aos_sequential.png'), dpi=300, bbox_inches='tight')
    plt.close()

def create_speedup_analysis(results, output_dir):
    """Crea grafico di analisi dello speedup"""
    fig, axes = plt.subplots(1, 2, figsize=(15, 6))  
    fig.suptitle('Speedup Analysis: Parallel vs Sequential', fontsize=16, fontweight='bold')
    
    test_names = [r['test_name'] for r in results]
    x = np.arange(len(test_names))
    width = 0.35
    
    colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd', '#8c564b']
    
    # SoA Speedup
    soa_outer_speedup = [r['soa_outer_speedup'] for r in results]
    soa_inner_speedup = [r['soa_inner_speedup'] for r in results]
    
    axes[0].bar(x - width/2, soa_outer_speedup, width, label='Outer Parallelization', alpha=0.8, color=colors[0])
    axes[0].bar(x + width/2, soa_inner_speedup, width, label='Inner Parallelization', alpha=0.8, color=colors[1])
    axes[0].set_title('SoA Speedup', fontweight='bold')
    axes[0].set_ylabel('Speedup Factor')
    axes[0].set_xticks(x)
    axes[0].set_xticklabels(test_names, rotation=45, ha='right')
    axes[0].legend()
    axes[0].grid(True, alpha=0.3)
    
    # AoS Speedup
    aos_outer_speedup = [r['aos_outer_speedup'] for r in results]
    aos_inner_speedup = [r['aos_inner_speedup'] for r in results]
    
    axes[1].bar(x - width/2, aos_outer_speedup, width, label='Outer Parallelization', alpha=0.8, color=colors[0])
    axes[1].bar(x + width/2, aos_inner_speedup, width, label='Inner Parallelization', alpha=0.8, color=colors[1])
    axes[1].axhline(y=4, color='red', linestyle='--', alpha=0.7, label='Theoretical Max (4 threads)')
    axes[1].set_title('AoS Speedup', fontweight='bold')
    axes[1].set_ylabel('Speedup Factor')
    axes[1].set_xticks(x)
    axes[1].set_xticklabels(test_names, rotation=45, ha='right')
    axes[1].legend()
    axes[1].grid(True, alpha=0.3)
    
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'speedup_analysis.png'), dpi=300, bbox_inches='tight')
    plt.close()
    """Crea grafico di analisi dello speedup"""
    fig, axes = plt.subplots(2, 2, figsize=(15, 12))
    fig.suptitle('Speedup Analysis: Parallel vs Sequential', fontsize=16, fontweight='bold')
    
    test_names = [r['test_name'] for r in results]
    x = np.arange(len(test_names))
    width = 0.35
    
    # SoA Speedup
    soa_outer_speedup = [r['soa_outer_speedup'] for r in results]
    soa_inner_speedup = [r['soa_inner_speedup'] for r in results]
    
    axes[0, 0].bar(x - width/2, soa_outer_speedup, width, label='Outer Parallelization', alpha=0.8)
    axes[0, 0].bar(x + width/2, soa_inner_speedup, width, label='Inner Parallelization', alpha=0.8)
    axes[0, 0].axhline(y=4, color='red', linestyle='--', alpha=0.7, label='Theoretical Max (4 threads)')
    axes[0, 0].set_title('SoA Speedup', fontweight='bold')
    axes[0, 0].set_ylabel('Speedup Factor')
    axes[0, 0].set_xticks(x)
    axes[0, 0].set_xticklabels(test_names, rotation=45, ha='right')
    axes[0, 0].legend()
    axes[0, 0].grid(True, alpha=0.3)
    
    # AoS Speedup
    aos_outer_speedup = [r['aos_outer_speedup'] for r in results]
    aos_inner_speedup = [r['aos_inner_speedup'] for r in results]
    
    axes[0, 1].bar(x - width/2, aos_outer_speedup, width, label='Outer Parallelization', alpha=0.8)
    axes[0, 1].bar(x + width/2, aos_inner_speedup, width, label='Inner Parallelization', alpha=0.8)
    axes[0, 1].axhline(y=4, color='red', linestyle='--', alpha=0.7, label='Theoretical Max (4 threads)')
    axes[0, 1].set_title('AoS Speedup', fontweight='bold')
    axes[0, 1].set_ylabel('Speedup Factor')
    axes[0, 1].set_xticks(x)
    axes[0, 1].set_xticklabels(test_names, rotation=45, ha='right')
    axes[0, 1].legend()
    axes[0, 1].grid(True, alpha=0.3)

def create_workload_analysis(results, output_dir):
    """Crea grafico di analisi del carico di lavoro"""
    fig, axes = plt.subplots(1, 2, figsize=(15, 6))
    fig.suptitle('Workload Analysis: Performance vs Dataset Characteristics', fontsize=16, fontweight='bold')
    
    # Estrae caratteristiche del dataset
    num_series = [r['num_series'] for r in results]
    series_length = [r['series_length'] for r in results]
    
    # Performance metrics
    soa_best_parallel = [min(r['soa_parallel_outer'], r['soa_parallel_inner']) for r in results]
    aos_best_parallel = [min(r['aos_parallel_outer'], r['aos_parallel_inner']) for r in results]
    
    # Scatter plot: Series count vs Performance
    axes[0].scatter(num_series, soa_best_parallel, label='SoA Best Parallel', alpha=0.8, s=100)
    axes[0].scatter(num_series, aos_best_parallel, label='AoS Best Parallel', alpha=0.8, s=100)
    axes[0].set_xlabel('Number of Series')
    axes[0].set_ylabel('Best Parallel Time (ms)')
    axes[0].set_title('Performance vs Number of Series', fontweight='bold')
    axes[0].legend()
    axes[0].grid(True, alpha=0.3)
    
    # Scatter plot: Series length vs Performance
    axes[1].scatter(series_length, soa_best_parallel, label='SoA Best Parallel', alpha=0.8, s=100)
    axes[1].scatter(series_length, aos_best_parallel, label='AoS Best Parallel', alpha=0.8, s=100)
    axes[1].set_xlabel('Series Length')
    axes[1].set_ylabel('Best Parallel Time (ms)')
    axes[1].set_title('Performance vs Series Length', fontweight='bold')
    axes[1].legend()
    axes[1].grid(True, alpha=0.3)
    
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'workload_analysis.png'), dpi=300, bbox_inches='tight')
    plt.close()

def create_summary_table(results, output_dir):
    """Crea tabella riassuntiva dei risultati"""
    fig, ax = plt.subplots(figsize=(14, 8))
    ax.axis('tight')
    ax.axis('off')
    
    # Prepara i dati per la tabella
    table_data = []
    headers = ['Test', 'Series×Length', 'SoA Seq (ms)', 'SoA Best Par (ms)', 'SoA Speedup', 
               'AoS Seq (ms)', 'AoS Best Par (ms)', 'AoS Speedup', 'SoA Advantage']
    
    for r in results:
        soa_best_par = min(r['soa_parallel_outer'], r['soa_parallel_inner'])
        aos_best_par = min(r['aos_parallel_outer'], r['aos_parallel_inner'])
        soa_best_speedup = r['soa_sequential'] / soa_best_par
        aos_best_speedup = r['aos_sequential'] / aos_best_par
        
        row = [
            r['test_name'],
            f"{r['num_series']}×{r['series_length']}",
            f"{r['soa_sequential']:.1f}",
            f"{soa_best_par:.1f}",
            f"{soa_best_speedup:.2f}x",
            f"{r['aos_sequential']:.1f}",
            f"{aos_best_par:.1f}",
            f"{aos_best_speedup:.2f}x",
            f"{r['soa_vs_aos_sequential']:.2f}x"
        ]
        table_data.append(row)
    
    table = ax.table(cellText=table_data, colLabels=headers, cellLoc='center', loc='center')
    table.auto_set_font_size(False)
    table.set_fontsize(10)
    table.scale(1.2, 1.5)
    
    # Styling
    for i in range(len(headers)):
        table[(0, i)].set_facecolor('#4CAF50')
        table[(0, i)].set_text_props(weight='bold', color='white')
    
    plt.title('Performance Summary Table', fontsize=16, fontweight='bold', pad=20)
    plt.savefig(os.path.join(output_dir, 'summary_table.png'), dpi=300, bbox_inches='tight')
    plt.close()

def main():
    """Funzione principale"""
    # Configurazione paths
    json_file = 'output/benchmark_results/parallelization_analysis.json'
    output_dir = 'output/plots'
    
    # Crea directory output se non esiste
    Path(output_dir).mkdir(parents=True, exist_ok=True)
    
    # Carica e processa i dati
    print("Loading benchmark data...")
    data = load_benchmark_data(json_file)
    results = extract_performance_data(data)
    
    print(f"Processing {len(results)} test results...")
    
    # Crea i grafici
    print("Creating execution time comparison plot...")
    create_execution_time_comparison(results, output_dir)
    
    print("Creating speedup analysis plot...")
    create_speedup_analysis(results, output_dir)
    
    print("Creating workload analysis plot...")
    create_workload_analysis(results, output_dir)
    
    print("Creating summary table...")
    create_summary_table(results, output_dir)
    
    print(f"All plots saved to {output_dir}/")


if __name__ == "__main__":
    main()