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
    """Estrae i dati di performance dal JSON (multi-run data)"""
    results = []
    
    for test in data['tests']:
        if 'error' in test:
            continue
            
        test_name = test['test_name']
        config = test['configuration']
        
        # Estrae i dati SoA (usa i valori medi)
        soa_seq = test['results']['soa']['sequential']['mean_execution_time_ms']
        soa_seq_std = test['results']['soa']['sequential']['std_deviation_ms']
        soa_outer = test['results']['soa']['parallel_outer']['mean_execution_time_ms']
        soa_outer_std = test['results']['soa']['parallel_outer']['std_deviation_ms']
        soa_inner = test['results']['soa']['parallel_inner']['mean_execution_time_ms']
        soa_inner_std = test['results']['soa']['parallel_inner']['std_deviation_ms']
        soa_outer_speedup = test['results']['soa']['parallel_outer']['speedup']
        soa_inner_speedup = test['results']['soa']['parallel_inner']['speedup']
        
        # Estrae i dati AoS (usa i valori medi)
        aos_seq = test['results']['aos']['sequential']['mean_execution_time_ms']
        aos_seq_std = test['results']['aos']['sequential']['std_deviation_ms']
        aos_outer = test['results']['aos']['parallel_outer']['mean_execution_time_ms']
        aos_outer_std = test['results']['aos']['parallel_outer']['std_deviation_ms']
        aos_inner = test['results']['aos']['parallel_inner']['mean_execution_time_ms']
        aos_inner_std = test['results']['aos']['parallel_inner']['std_deviation_ms']
        aos_outer_speedup = test['results']['aos']['parallel_outer']['speedup']
        aos_inner_speedup = test['results']['aos']['parallel_inner']['speedup']
        
        # Analisi
        analysis = test['analysis']
        
        result = {
            'test_name': test_name,
            'num_series': config['num_series'],
            'series_length': config['series_length'],
            'query_length': config['query_length'],
            'num_runs': config['num_runs'],
            'soa_sequential': soa_seq,
            'soa_sequential_std': soa_seq_std,
            'soa_parallel_outer': soa_outer,
            'soa_parallel_outer_std': soa_outer_std,
            'soa_parallel_inner': soa_inner,
            'soa_parallel_inner_std': soa_inner_std,
            'soa_outer_speedup': soa_outer_speedup,
            'soa_inner_speedup': soa_inner_speedup,
            'aos_sequential': aos_seq,
            'aos_sequential_std': aos_seq_std,
            'aos_parallel_outer': aos_outer,
            'aos_parallel_outer_std': aos_outer_std,
            'aos_parallel_inner': aos_inner,
            'aos_parallel_inner_std': aos_inner_std,
            'aos_outer_speedup': aos_outer_speedup,
            'aos_inner_speedup': aos_inner_speedup,
            'soa_vs_aos_sequential': analysis['soa_vs_aos_sequential'],
            'soa_vs_aos_parallel_outer': analysis['soa_vs_aos_parallel_outer'],
            'soa_vs_aos_parallel_inner': analysis['soa_vs_aos_parallel_inner']
        }
        results.append(result)
    
    return results

def create_execution_time_comparison(results, output_dir):
    """Crea tre grafici separati per il confronto dei tempi di esecuzione con error bars"""
    
    test_names = [r['test_name'] for r in results]
    x = np.arange(len(test_names))
    width = 0.25
    
    colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd', '#8c564b']
    
    # Dati per i grafici (medie e deviazioni standard)
    soa_seq = [r['soa_sequential'] for r in results]
    soa_seq_std = [r['soa_sequential_std'] for r in results]
    soa_outer = [r['soa_parallel_outer'] for r in results]
    soa_outer_std = [r['soa_parallel_outer_std'] for r in results]
    soa_inner = [r['soa_parallel_inner'] for r in results]
    soa_inner_std = [r['soa_parallel_inner_std'] for r in results]
    
    aos_seq = [r['aos_sequential'] for r in results]
    aos_seq_std = [r['aos_sequential_std'] for r in results]
    aos_outer = [r['aos_parallel_outer'] for r in results]
    aos_outer_std = [r['aos_parallel_outer_std'] for r in results]
    aos_inner = [r['aos_parallel_inner'] for r in results]
    aos_inner_std = [r['aos_parallel_inner_std'] for r in results]
    
    # Aggiungi info sul numero di run al titolo
    num_runs = results[0]['num_runs'] if results else 1
    
    # GRAFICO 1: SoA Performance
    fig1, ax1 = plt.subplots(figsize=(12, 8))
    bars1 = ax1.bar(x - width, soa_seq, width, yerr=soa_seq_std, label='Sequential', alpha=0.8, color=colors[0], capsize=5)
    bars2 = ax1.bar(x, soa_outer, width, yerr=soa_outer_std, label='Parallel Outer', alpha=0.8, color=colors[1], capsize=5)
    bars3 = ax1.bar(x + width, soa_inner, width, yerr=soa_inner_std, label='Parallel Inner', alpha=0.8, color=colors[2], capsize=5)
    
    ax1.set_title(f'SoA (Structure of Arrays) Performance\n(Mean ± Std Dev over {num_runs} runs)', fontsize=16, fontweight='bold')
    ax1.set_ylabel('Execution Time (ms)', fontsize=12)
    ax1.set_xlabel('Test Configuration', fontsize=12)
    ax1.set_xticks(x)
    ax1.set_xticklabels(test_names, rotation=45, ha='right')
    ax1.legend()
    ax1.grid(True, alpha=0.3)
    
    # Aggiungi etichette con valori medi ± std dev
    for i, (seq, seq_std, outer, outer_std, inner, inner_std) in enumerate(zip(soa_seq, soa_seq_std, soa_outer, soa_outer_std, soa_inner, soa_inner_std)):
        max_val = max(soa_seq)
        ax1.text(i - width, seq + seq_std + max_val * 0.01, f'{seq:.1f}±{seq_std:.1f}', ha='center', va='bottom', fontsize=8)
        ax1.text(i, outer + outer_std + max_val * 0.01, f'{outer:.1f}±{outer_std:.1f}', ha='center', va='bottom', fontsize=8)
        ax1.text(i + width, inner + inner_std + max_val * 0.01, f'{inner:.1f}±{inner_std:.1f}', ha='center', va='bottom', fontsize=8)
    
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'soa_performance_comparison.png'), dpi=300, bbox_inches='tight')
    plt.close()
    
    # GRAFICO 2: AoS Performance
    fig2, ax2 = plt.subplots(figsize=(12, 8))
    ax2.bar(x - width, aos_seq, width, yerr=aos_seq_std, label='Sequential', alpha=0.8, color=colors[0], capsize=5)
    ax2.bar(x, aos_outer, width, yerr=aos_outer_std, label='Parallel Outer', alpha=0.8, color=colors[1], capsize=5)
    ax2.bar(x + width, aos_inner, width, yerr=aos_inner_std, label='Parallel Inner', alpha=0.8, color=colors[2], capsize=5)
    
    ax2.set_title(f'AoS (Array of Structures) Performance\n(Mean ± Std Dev over {num_runs} runs)', fontsize=16, fontweight='bold')
    ax2.set_ylabel('Execution Time (ms)', fontsize=12)
    ax2.set_xlabel('Test Configuration', fontsize=12)
    ax2.set_xticks(x)
    ax2.set_xticklabels(test_names, rotation=45, ha='right')
    ax2.legend()
    ax2.grid(True, alpha=0.3)
    
    # Aggiungi etichette con valori medi ± std dev
    for i, (seq, seq_std, outer, outer_std, inner, inner_std) in enumerate(zip(aos_seq, aos_seq_std, aos_outer, aos_outer_std, aos_inner, aos_inner_std)):
        max_val = max(aos_seq)
        ax2.text(i - width, seq + seq_std + max_val * 0.01, f'{seq:.1f}±{seq_std:.1f}', ha='center', va='bottom', fontsize=8)
        ax2.text(i, outer + outer_std + max_val * 0.01, f'{outer:.1f}±{outer_std:.1f}', ha='center', va='bottom', fontsize=8)
        ax2.text(i + width, inner + inner_std + max_val * 0.01, f'{inner:.1f}±{inner_std:.1f}', ha='center', va='bottom', fontsize=8)
    
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'aos_performance_comparison.png'), dpi=300, bbox_inches='tight')
    plt.close()
    
    # GRAFICO 3: SoA vs AoS Sequential Comparison
    fig3, ax3 = plt.subplots(figsize=(12, 8))
    ax3.bar(x - width/2, soa_seq, width, yerr=soa_seq_std, label='SoA Sequential', alpha=0.8, color=colors[3], capsize=5)
    ax3.bar(x + width/2, aos_seq, width, yerr=aos_seq_std, label='AoS Sequential', alpha=0.8, color=colors[4], capsize=5)
    
    ax3.set_title(f'Sequential Performance: SoA vs AoS\n(Mean ± Std Dev over {num_runs} runs)', fontsize=16, fontweight='bold')
    ax3.set_ylabel('Execution Time (ms)', fontsize=12)
    ax3.set_xlabel('Test Configuration', fontsize=12)
    ax3.set_xticks(x)
    ax3.set_xticklabels(test_names, rotation=45, ha='right')
    ax3.legend()
    ax3.grid(True, alpha=0.3)
    
    # Aggiungi etichette con valori medi ± std dev
    for i, (soa, soa_std, aos, aos_std) in enumerate(zip(soa_seq, soa_seq_std, aos_seq, aos_seq_std)):
        max_val = max(max(soa_seq), max(aos_seq))
        ax3.text(i - width/2, soa + soa_std + max_val * 0.01, f'{soa:.1f}±{soa_std:.1f}', ha='center', va='bottom', fontsize=8)
        ax3.text(i + width/2, aos + aos_std + max_val * 0.01, f'{aos:.1f}±{aos_std:.1f}', ha='center', va='bottom', fontsize=8)
    
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'soa_vs_aos_sequential.png'), dpi=300, bbox_inches='tight')
    plt.close()

def create_speedup_analysis(results, output_dir):
    """Crea grafico di analisi dello speedup"""
    fig, axes = plt.subplots(1, 2, figsize=(15, 6))  
    
    # Aggiungi info sui run nel titolo
    num_runs = results[0]['num_runs'] if results else 1
    fig.suptitle(f'Speedup Analysis: Parallel vs Sequential\n(Average over {num_runs} runs)', fontsize=16, fontweight='bold')
    
    test_names = [r['test_name'] for r in results]
    x = np.arange(len(test_names))
    width = 0.35
    
    colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd', '#8c564b']
    
    # SoA Speedup
    soa_outer_speedup = [r['soa_outer_speedup'] for r in results]
    soa_inner_speedup = [r['soa_inner_speedup'] for r in results]
    
    bars1 = axes[0].bar(x - width/2, soa_outer_speedup, width, label='Outer Parallelization', alpha=0.8, color=colors[0])
    bars2 = axes[0].bar(x + width/2, soa_inner_speedup, width, label='Inner Parallelization', alpha=0.8, color=colors[1])
    axes[0].set_title('SoA Speedup', fontweight='bold')
    axes[0].set_ylabel('Speedup Factor')
    axes[0].set_xticks(x)
    axes[0].set_xticklabels(test_names, rotation=45, ha='right')
    axes[0].legend()
    axes[0].grid(True, alpha=0.3)
    
    # Aggiungi etichette con valori di speedup
    for bar, speedup in zip(bars1, soa_outer_speedup):
        height = bar.get_height()
        axes[0].text(bar.get_x() + bar.get_width()/2., height + 0.1,
                    f'{speedup:.2f}x', ha='center', va='bottom', fontsize=9)
    for bar, speedup in zip(bars2, soa_inner_speedup):
        height = bar.get_height()
        axes[0].text(bar.get_x() + bar.get_width()/2., height + 0.1,
                    f'{speedup:.2f}x', ha='center', va='bottom', fontsize=9)
    
    # AoS Speedup
    aos_outer_speedup = [r['aos_outer_speedup'] for r in results]
    aos_inner_speedup = [r['aos_inner_speedup'] for r in results]
    
    bars3 = axes[1].bar(x - width/2, aos_outer_speedup, width, label='Outer Parallelization', alpha=0.8, color=colors[0])
    bars4 = axes[1].bar(x + width/2, aos_inner_speedup, width, label='Inner Parallelization', alpha=0.8, color=colors[1])
    axes[1].set_title('AoS Speedup', fontweight='bold')
    axes[1].set_ylabel('Speedup Factor')
    axes[1].set_xticks(x)
    axes[1].set_xticklabels(test_names, rotation=45, ha='right')
    axes[1].legend()
    axes[1].grid(True, alpha=0.3)
    
    # Aggiungi etichette con valori di speedup
    for bar, speedup in zip(bars3, aos_outer_speedup):
        height = bar.get_height()
        axes[1].text(bar.get_x() + bar.get_width()/2., height + 0.1,
                    f'{speedup:.2f}x', ha='center', va='bottom', fontsize=9)
    for bar, speedup in zip(bars4, aos_inner_speedup):
        height = bar.get_height()
        axes[1].text(bar.get_x() + bar.get_width()/2., height + 0.1,
                    f'{speedup:.2f}x', ha='center', va='bottom', fontsize=9)
    
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'speedup_analysis.png'), dpi=300, bbox_inches='tight')
    plt.close()

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
    """Crea tabella riassuntiva dei risultati con statistiche multi-run"""
    fig, ax = plt.subplots(figsize=(16, 10))
    ax.axis('tight')
    ax.axis('off')
    
    # Prepara i dati per la tabella
    table_data = []
    headers = ['Test', 'Runs', 'Series×Length', 'SoA Seq (ms)', 'SoA Best Par (ms)', 'SoA Speedup', 
               'AoS Seq (ms)', 'AoS Best Par (ms)', 'AoS Speedup', 'SoA Advantage']
    
    for r in results:
        soa_best_par = min(r['soa_parallel_outer'], r['soa_parallel_inner'])
        aos_best_par = min(r['aos_parallel_outer'], r['aos_parallel_inner'])
        soa_best_speedup = r['soa_sequential'] / soa_best_par
        aos_best_speedup = r['aos_sequential'] / aos_best_par
        
        row = [
            r['test_name'],
            f"{r['num_runs']}",
            f"{r['num_series']}×{r['series_length']}",
            f"{r['soa_sequential']:.1f}±{r['soa_sequential_std']:.1f}",
            f"{soa_best_par:.1f}",
            f"{soa_best_speedup:.2f}x",
            f"{r['aos_sequential']:.1f}±{r['aos_sequential_std']:.1f}",
            f"{aos_best_par:.1f}",
            f"{aos_best_speedup:.2f}x",
            f"{r['soa_vs_aos_sequential']:.2f}x"
        ]
        table_data.append(row)
    
    table = ax.table(cellText=table_data, colLabels=headers, cellLoc='center', loc='center')
    table.auto_set_font_size(False)
    table.set_fontsize(9)
    table.scale(1.2, 1.8)
    
    # Styling
    for i in range(len(headers)):
        table[(0, i)].set_facecolor('#4CAF50')
        table[(0, i)].set_text_props(weight='bold', color='white')
    
    # Aggiungi informazioni sui run
    num_runs = results[0]['num_runs'] if results else 1
    plt.title(f'Performance Summary Table\n(Statistics from {num_runs} runs per test)', fontsize=16, fontweight='bold', pad=20)
    plt.savefig(os.path.join(output_dir, 'summary_table.png'), dpi=300, bbox_inches='tight')
    plt.close()

def create_execution_time_distribution(data, output_dir):
    """Crea grafico delle distribuzioni dei tempi di esecuzione per tutti i run"""
    fig, axes = plt.subplots(2, 2, figsize=(16, 12))
    fig.suptitle('Distribution of Execution Times Across Multiple Runs', fontsize=16, fontweight='bold')
    
    test_configs = []
    for test in data['tests']:
        if 'error' not in test:
            test_configs.append(test)
    
    for idx, test in enumerate(test_configs):
        row = idx // 2
        col = idx % 2
        ax = axes[row, col]
        
        config = test['configuration']
        test_name = f"{config['num_series']}×{config['series_length']}"
        
        # Raccogli tutti i tempi di esecuzione per questo test
        all_times = {}
        
        for data_structure in ['soa', 'aos']:
            for parallel_type in ['sequential', 'parallel_outer', 'parallel_inner']:
                key = f"{data_structure.upper()} {parallel_type.replace('_', ' ').title()}"
                times = test['results'][data_structure][parallel_type]['all_execution_times']
                all_times[key] = times
        
        # Crea box plot
        labels = list(all_times.keys())
        values = [all_times[label] for label in labels]
        
        bp = ax.boxplot(values, labels=labels, patch_artist=True)
        
        # Colora le box
        colors = ['lightblue', 'lightgreen', 'lightcoral', 'lightyellow', 'lightpink', 'lightgray']
        for patch, color in zip(bp['boxes'], colors):
            patch.set_facecolor(color)
        
        ax.set_title(f'Test: {test_name} ({config["num_runs"]} runs)')
        ax.set_ylabel('Execution Time (ms)')
        ax.tick_params(axis='x', rotation=45)
        ax.grid(True, alpha=0.3)
    
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'execution_time_distributions.png'), dpi=300, bbox_inches='tight')
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
    
    if not results:
        print("No valid test results found in the JSON file.")
        return
    
    num_runs = results[0]['num_runs']
    print(f"Processing {len(results)} test results with {num_runs} runs each...")
    
    # Crea i grafici
    print("Creating execution time comparison plots...")
    create_execution_time_comparison(results, output_dir)
    
    print("Creating speedup analysis plot...")
    create_speedup_analysis(results, output_dir)
    
    print("Creating workload analysis plot...")
    create_workload_analysis(results, output_dir)
    
    print("Creating summary table...")
    create_summary_table(results, output_dir)
    
    print("Creating execution time distribution plot...")
    create_execution_time_distribution(data, output_dir)
    
    print(f"\nAll plots saved to {output_dir}/")
    print("Generated plots:")
    print("  - soa_performance_comparison.png")
    print("  - aos_performance_comparison.png") 
    print("  - soa_vs_aos_sequential.png")
    print("  - speedup_analysis.png")
    print("  - workload_analysis.png")
    print("  - execution_time_distributions.png")
    print("  - summary_table.png")


if __name__ == "__main__":
    main()