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

def extract_thread_scaling_data(data):
    """Estrae i dati di scaling per thread dal nuovo formato JSON"""
    results = []
    
    for test in data['tests']:
        if 'error' in test:
            continue
            
        test_name = test['test_name']
        config = test['configuration']
        thread_results = test['thread_results']  # Questa chiave esiste nel tuo JSON
        
        # Prepara struttura per questo test
        test_data = {
            'test_name': test_name,
            'config': config,
            'thread_counts': config['thread_counts'],
            'baseline_soa': test['summary']['baseline_soa_time_ms'],
            'baseline_aos': test['summary']['baseline_aos_time_ms'],
            'baseline_soa_vs_aos': test['summary']['baseline_soa_vs_aos'],
            'thread_data': {}
        }
        
        # Estrai dati per ogni thread count
        for thread_count_str, thread_data in thread_results.items():
            thread_count = int(thread_count_str)
            
            thread_info = {
                'thread_count': thread_count,
                'analysis': thread_data['analysis']
            }
            
            # Dati SoA
            if thread_count == 1:
                # Sequential
                soa_data = thread_data['soa']['sequential']
                thread_info['soa'] = {
                    'sequential': {
                        'mean_time': soa_data['mean_execution_time_ms'],
                        'std_time': soa_data['std_deviation_ms'],
                        'speedup': 1.0,
                        'efficiency': 1.0
                    }
                }
            else:
                # Parallel
                soa_outer = thread_data['soa']['parallel_outer']
                soa_inner = thread_data['soa']['parallel_inner']
                thread_info['soa'] = {
                    'parallel_outer': {
                        'mean_time': soa_outer['mean_execution_time_ms'],
                        'std_time': soa_outer['std_deviation_ms'],
                        'speedup': soa_outer['speedup'],
                        'efficiency': soa_outer['efficiency']
                    },
                    'parallel_inner': {
                        'mean_time': soa_inner['mean_execution_time_ms'],
                        'std_time': soa_inner['std_deviation_ms'],
                        'speedup': soa_inner['speedup'],
                        'efficiency': soa_inner['efficiency']
                    }
                }
            
            # Dati AoS
            if thread_count == 1:
                # Sequential
                aos_data = thread_data['aos']['sequential']
                thread_info['aos'] = {
                    'sequential': {
                        'mean_time': aos_data['mean_execution_time_ms'],
                        'std_time': aos_data['std_deviation_ms'],
                        'speedup': 1.0,
                        'efficiency': 1.0
                    }
                }
            else:
                # Parallel
                aos_outer = thread_data['aos']['parallel_outer']
                aos_inner = thread_data['aos']['parallel_inner']
                thread_info['aos'] = {
                    'parallel_outer': {
                        'mean_time': aos_outer['mean_execution_time_ms'],
                        'std_time': aos_outer['std_deviation_ms'],
                        'speedup': aos_outer['speedup'],
                        'efficiency': aos_outer['efficiency']
                    },
                    'parallel_inner': {
                        'mean_time': aos_inner['mean_execution_time_ms'],
                        'std_time': aos_inner['std_deviation_ms'],
                        'speedup': aos_inner['speedup'],
                        'efficiency': aos_inner['efficiency']
                    }
                }
            
            test_data['thread_data'][thread_count] = thread_info
        
        results.append(test_data)
    
    return results

def create_thread_scaling_plots(results, output_dir):
    """Crea grafici di scaling per thread per ogni test"""
    
    for test_data in results:
        test_name = test_data['test_name']
        config = test_data['config']
        thread_counts = sorted(test_data['thread_data'].keys())
        
        fig, axes = plt.subplots(2, 2, figsize=(16, 12))
        fig.suptitle(f'Thread Scaling Analysis - {test_name}\n'
                    f'{config["num_series"]} series × {config["series_length"]} points '
                    f'(query: {config["query_length"]}, runs: {config["num_runs"]})', 
                    fontsize=16, fontweight='bold')
        
        # Prepara i dati
        soa_outer_times = []
        soa_outer_speedups = []
        soa_outer_efficiencies = []
        soa_inner_times = []
        soa_inner_speedups = []
        soa_inner_efficiencies = []
        
        aos_outer_times = []
        aos_outer_speedups = []
        aos_outer_efficiencies = []
        aos_inner_times = []
        aos_inner_speedups = []
        aos_inner_efficiencies = []
        
        for tc in thread_counts:
            thread_info = test_data['thread_data'][tc]
            
            if tc == 1:
                # Sequential
                soa_time = thread_info['soa']['sequential']['mean_time']
                aos_time = thread_info['aos']['sequential']['mean_time']
                
                soa_outer_times.append(soa_time)
                soa_outer_speedups.append(1.0)
                soa_outer_efficiencies.append(1.0)
                soa_inner_times.append(soa_time)
                soa_inner_speedups.append(1.0)
                soa_inner_efficiencies.append(1.0)
                
                aos_outer_times.append(aos_time)
                aos_outer_speedups.append(1.0)
                aos_outer_efficiencies.append(1.0)
                aos_inner_times.append(aos_time)
                aos_inner_speedups.append(1.0)
                aos_inner_efficiencies.append(1.0)
            else:
                # Parallel
                soa_outer_times.append(thread_info['soa']['parallel_outer']['mean_time'])
                soa_outer_speedups.append(thread_info['soa']['parallel_outer']['speedup'])
                soa_outer_efficiencies.append(thread_info['soa']['parallel_outer']['efficiency'])
                
                soa_inner_times.append(thread_info['soa']['parallel_inner']['mean_time'])
                soa_inner_speedups.append(thread_info['soa']['parallel_inner']['speedup'])
                soa_inner_efficiencies.append(thread_info['soa']['parallel_inner']['efficiency'])
                
                aos_outer_times.append(thread_info['aos']['parallel_outer']['mean_time'])
                aos_outer_speedups.append(thread_info['aos']['parallel_outer']['speedup'])
                aos_outer_efficiencies.append(thread_info['aos']['parallel_outer']['efficiency'])
                
                aos_inner_times.append(thread_info['aos']['parallel_inner']['mean_time'])
                aos_inner_speedups.append(thread_info['aos']['parallel_inner']['speedup'])
                aos_inner_efficiencies.append(thread_info['aos']['parallel_inner']['efficiency'])
        
        # GRAFICO 1: Execution Time vs Thread Count
        ax1 = axes[0, 0]
        ax1.plot(thread_counts, soa_outer_times, 'bo-', label='SoA Outer', linewidth=2, markersize=8)
        ax1.plot(thread_counts, soa_inner_times, 'b^--', label='SoA Inner', linewidth=2, markersize=8)
        ax1.plot(thread_counts, aos_outer_times, 'ro-', label='AoS Outer', linewidth=2, markersize=8)
        ax1.plot(thread_counts, aos_inner_times, 'r^--', label='AoS Inner', linewidth=2, markersize=8)
        ax1.set_xlabel('Number of Threads')
        ax1.set_ylabel('Execution Time (ms)')
        ax1.set_title('Execution Time vs Thread Count')
        ax1.legend()
        ax1.grid(True, alpha=0.3)
        ax1.set_yscale('log')
        
        # GRAFICO 2: Speedup vs Thread Count
        ax2 = axes[0, 1]
        ax2.plot(thread_counts, soa_outer_speedups, 'bo-', label='SoA Outer', linewidth=2, markersize=8)
        ax2.plot(thread_counts, soa_inner_speedups, 'b^--', label='SoA Inner', linewidth=2, markersize=8)
        ax2.plot(thread_counts, aos_outer_speedups, 'ro-', label='AoS Outer', linewidth=2, markersize=8)
        ax2.plot(thread_counts, aos_inner_speedups, 'r^--', label='AoS Inner', linewidth=2, markersize=8)
        ax2.plot(thread_counts, thread_counts, 'k--', alpha=0.5, label='Linear Speedup')
        ax2.set_xlabel('Number of Threads')
        ax2.set_ylabel('Speedup')
        ax2.set_title('Speedup vs Thread Count')
        ax2.legend()
        ax2.grid(True, alpha=0.3)
        
        # GRAFICO 3: Efficiency vs Thread Count
        ax3 = axes[1, 0]
        ax3.plot(thread_counts, soa_outer_efficiencies, 'bo-', label='SoA Outer', linewidth=2, markersize=8)
        ax3.plot(thread_counts, soa_inner_efficiencies, 'b^--', label='SoA Inner', linewidth=2, markersize=8)
        ax3.plot(thread_counts, aos_outer_efficiencies, 'ro-', label='AoS Outer', linewidth=2, markersize=8)
        ax3.plot(thread_counts, aos_inner_efficiencies, 'r^--', label='AoS Inner', linewidth=2, markersize=8)
        ax3.axhline(y=1.0, color='k', linestyle='--', alpha=0.5, label='Perfect Efficiency')
        ax3.set_xlabel('Number of Threads')
        ax3.set_ylabel('Efficiency')
        ax3.set_title('Efficiency vs Thread Count')
        ax3.legend()
        ax3.grid(True, alpha=0.3)
        
        # GRAFICO 4: SoA vs AoS Comparison
        ax4 = axes[1, 1]
        best_soa_speedups = [max(soa_outer_speedups[i], soa_inner_speedups[i]) for i in range(len(thread_counts))]
        best_aos_speedups = [max(aos_outer_speedups[i], aos_inner_speedups[i]) for i in range(len(thread_counts))]
        
        width = 0.35
        x = np.arange(len(thread_counts))
        bars1 = ax4.bar(x - width/2, best_soa_speedups, width, label='SoA Best', alpha=0.8)
        bars2 = ax4.bar(x + width/2, best_aos_speedups, width, label='AoS Best', alpha=0.8)
        
        ax4.set_xlabel('Number of Threads')
        ax4.set_ylabel('Best Speedup')
        ax4.set_title('Best Speedup Comparison')
        ax4.set_xticks(x)
        ax4.set_xticklabels(thread_counts)
        ax4.legend()
        ax4.grid(True, alpha=0.3)
        
        # Aggiungi etichette sui bar
        for bar, speedup in zip(bars1, best_soa_speedups):
            height = bar.get_height()
            ax4.text(bar.get_x() + bar.get_width()/2., height + 0.05,
                    f'{speedup:.2f}x', ha='center', va='bottom', fontsize=8)
        for bar, speedup in zip(bars2, best_aos_speedups):
            height = bar.get_height()
            ax4.text(bar.get_x() + bar.get_width()/2., height + 0.05,
                    f'{speedup:.2f}x', ha='center', va='bottom', fontsize=8)
        
        plt.tight_layout()
        plt.savefig(os.path.join(output_dir, f'thread_scaling_{test_name}.png'), dpi=300, bbox_inches='tight')
        plt.close()

def create_comprehensive_speedup_comparison(results, output_dir):
    """Crea un grafico complessivo che confronta tutti i test per speedup vs thread count"""
    
    fig, axes = plt.subplots(2, 2, figsize=(16, 12))
    fig.suptitle('Comprehensive Thread Scaling Analysis - All Tests', fontsize=16, fontweight='bold')
    
    colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728']
    markers = ['o', 's', '^', 'D']
    
    # GRAFICO 1: SoA Outer Speedup
    ax1 = axes[0, 0]
    for idx, test_data in enumerate(results):
        test_name = test_data['test_name']
        thread_counts = sorted(test_data['thread_data'].keys())
        speedups = []
        
        for tc in thread_counts:
            if tc == 1:
                speedups.append(1.0)
            else:
                speedups.append(test_data['thread_data'][tc]['soa']['parallel_outer']['speedup'])
        
        ax1.plot(thread_counts, speedups, f'{markers[idx % len(markers)]}-', 
                label=test_name, color=colors[idx % len(colors)], linewidth=2, markersize=8)
    
    ax1.plot(thread_counts, thread_counts, 'k--', alpha=0.5, label='Linear Speedup')
    ax1.set_xlabel('Number of Threads')
    ax1.set_ylabel('Speedup')
    ax1.set_title('SoA Outer Parallelization Speedup')
    ax1.legend()
    ax1.grid(True, alpha=0.3)
    
    # GRAFICO 2: SoA Inner Speedup
    ax2 = axes[0, 1]
    for idx, test_data in enumerate(results):
        test_name = test_data['test_name']
        thread_counts = sorted(test_data['thread_data'].keys())
        speedups = []
        
        for tc in thread_counts:
            if tc == 1:
                speedups.append(1.0)
            else:
                speedups.append(test_data['thread_data'][tc]['soa']['parallel_inner']['speedup'])
        
        ax2.plot(thread_counts, speedups, f'{markers[idx % len(markers)]}-', 
                label=test_name, color=colors[idx % len(colors)], linewidth=2, markersize=8)
    
    ax2.plot(thread_counts, thread_counts, 'k--', alpha=0.5, label='Linear Speedup')
    ax2.set_xlabel('Number of Threads')
    ax2.set_ylabel('Speedup')
    ax2.set_title('SoA Inner Parallelization Speedup')
    ax2.legend()
    ax2.grid(True, alpha=0.3)
    
    # GRAFICO 3: AoS Outer Speedup
    ax3 = axes[1, 0]
    for idx, test_data in enumerate(results):
        test_name = test_data['test_name']
        thread_counts = sorted(test_data['thread_data'].keys())
        speedups = []
        
        for tc in thread_counts:
            if tc == 1:
                speedups.append(1.0)
            else:
                speedups.append(test_data['thread_data'][tc]['aos']['parallel_outer']['speedup'])
        
        ax3.plot(thread_counts, speedups, f'{markers[idx % len(markers)]}-', 
                label=test_name, color=colors[idx % len(colors)], linewidth=2, markersize=8)
    
    ax3.plot(thread_counts, thread_counts, 'k--', alpha=0.5, label='Linear Speedup')
    ax3.set_xlabel('Number of Threads')
    ax3.set_ylabel('Speedup')
    ax3.set_title('AoS Outer Parallelization Speedup')
    ax3.legend()
    ax3.grid(True, alpha=0.3)
    
    # GRAFICO 4: AoS Inner Speedup
    ax4 = axes[1, 1]
    for idx, test_data in enumerate(results):
        test_name = test_data['test_name']
        thread_counts = sorted(test_data['thread_data'].keys())
        speedups = []
        
        for tc in thread_counts:
            if tc == 1:
                speedups.append(1.0)
            else:
                speedups.append(test_data['thread_data'][tc]['aos']['parallel_inner']['speedup'])
        
        ax4.plot(thread_counts, speedups, f'{markers[idx % len(markers)]}-', 
                label=test_name, color=colors[idx % len(colors)], linewidth=2, markersize=8)
    
    ax4.plot(thread_counts, thread_counts, 'k--', alpha=0.5, label='Linear Speedup')
    ax4.set_xlabel('Number of Threads')
    ax4.set_ylabel('Speedup')
    ax4.set_title('AoS Inner Parallelization Speedup')
    ax4.legend()
    ax4.grid(True, alpha=0.3)
    
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'comprehensive_speedup_comparison.png'), dpi=300, bbox_inches='tight')
    plt.close()

def create_efficiency_heatmap(results, output_dir):
    """Crea heatmap delle efficienze per thread count e test"""
    
    # Prepara i dati per la heatmap
    test_names = [r['test_name'] for r in results]
    thread_counts = sorted(results[0]['thread_data'].keys())
    
    # Matrici per le efficienze
    soa_outer_eff = []
    soa_inner_eff = []
    aos_outer_eff = []
    aos_inner_eff = []
    
    for test_data in results:
        soa_outer_row = []
        soa_inner_row = []
        aos_outer_row = []
        aos_inner_row = []
        
        for tc in thread_counts:
            if tc == 1:
                soa_outer_row.append(1.0)
                soa_inner_row.append(1.0)
                aos_outer_row.append(1.0)
                aos_inner_row.append(1.0)
            else:
                soa_outer_row.append(test_data['thread_data'][tc]['soa']['parallel_outer']['efficiency'])
                soa_inner_row.append(test_data['thread_data'][tc]['soa']['parallel_inner']['efficiency'])
                aos_outer_row.append(test_data['thread_data'][tc]['aos']['parallel_outer']['efficiency'])
                aos_inner_row.append(test_data['thread_data'][tc]['aos']['parallel_inner']['efficiency'])
        
        soa_outer_eff.append(soa_outer_row)
        soa_inner_eff.append(soa_inner_row)
        aos_outer_eff.append(aos_outer_row)
        aos_inner_eff.append(aos_inner_row)
    
    # Crea la heatmap
    fig, axes = plt.subplots(2, 2, figsize=(16, 10))
    fig.suptitle('Efficiency Heatmaps - Thread Scaling', fontsize=16, fontweight='bold')
    
    # SoA Outer
    im1 = axes[0, 0].imshow(soa_outer_eff, cmap='RdYlGn', aspect='auto', vmin=0, vmax=1)
    axes[0, 0].set_title('SoA Outer Parallelization')
    axes[0, 0].set_xticks(range(len(thread_counts)))
    axes[0, 0].set_xticklabels(thread_counts)
    axes[0, 0].set_yticks(range(len(test_names)))
    axes[0, 0].set_yticklabels(test_names)
    axes[0, 0].set_xlabel('Number of Threads')
    axes[0, 0].set_ylabel('Test Configuration')
    
    # Aggiungi valori nelle celle
    for i in range(len(test_names)):
        for j in range(len(thread_counts)):
            text = axes[0, 0].text(j, i, f'{soa_outer_eff[i][j]:.2f}',
                                 ha="center", va="center", color="black", fontsize=8)
    
    # SoA Inner
    im2 = axes[0, 1].imshow(soa_inner_eff, cmap='RdYlGn', aspect='auto', vmin=0, vmax=1)
    axes[0, 1].set_title('SoA Inner Parallelization')
    axes[0, 1].set_xticks(range(len(thread_counts)))
    axes[0, 1].set_xticklabels(thread_counts)
    axes[0, 1].set_yticks(range(len(test_names)))
    axes[0, 1].set_yticklabels(test_names)
    axes[0, 1].set_xlabel('Number of Threads')
    
    for i in range(len(test_names)):
        for j in range(len(thread_counts)):
            text = axes[0, 1].text(j, i, f'{soa_inner_eff[i][j]:.2f}',
                                 ha="center", va="center", color="black", fontsize=8)
    
    # AoS Outer
    im3 = axes[1, 0].imshow(aos_outer_eff, cmap='RdYlGn', aspect='auto', vmin=0, vmax=1)
    axes[1, 0].set_title('AoS Outer Parallelization')
    axes[1, 0].set_xticks(range(len(thread_counts)))
    axes[1, 0].set_xticklabels(thread_counts)
    axes[1, 0].set_yticks(range(len(test_names)))
    axes[1, 0].set_yticklabels(test_names)
    axes[1, 0].set_xlabel('Number of Threads')
    axes[1, 0].set_ylabel('Test Configuration')
    
    for i in range(len(test_names)):
        for j in range(len(thread_counts)):
            text = axes[1, 0].text(j, i, f'{aos_outer_eff[i][j]:.2f}',
                                 ha="center", va="center", color="black", fontsize=8)
    
    # AoS Inner
    im4 = axes[1, 1].imshow(aos_inner_eff, cmap='RdYlGn', aspect='auto', vmin=0, vmax=1)
    axes[1, 1].set_title('AoS Inner Parallelization')
    axes[1, 1].set_xticks(range(len(thread_counts)))
    axes[1, 1].set_xticklabels(thread_counts)
    axes[1, 1].set_yticks(range(len(test_names)))
    axes[1, 1].set_yticklabels(test_names)
    axes[1, 1].set_xlabel('Number of Threads')
    
    for i in range(len(test_names)):
        for j in range(len(thread_counts)):
            text = axes[1, 1].text(j, i, f'{aos_inner_eff[i][j]:.2f}',
                                 ha="center", va="center", color="black", fontsize=8)
    
    # Aggiungi colorbar
    fig.colorbar(im1, ax=axes, shrink=0.6, label='Efficiency')
    
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'efficiency_heatmap.png'), dpi=300, bbox_inches='tight')
    plt.close()

def create_baseline_comparison(results, output_dir):
    """Crea grafico di confronto delle baseline sequenziali"""
    
    fig, axes = plt.subplots(1, 2, figsize=(15, 6))
    fig.suptitle('Sequential Baseline Comparison', fontsize=16, fontweight='bold')
    
    test_names = [r['test_name'] for r in results]
    x = np.arange(len(test_names))
    width = 0.35
    
    soa_baselines = [r['baseline_soa'] for r in results]
    aos_baselines = [r['baseline_aos'] for r in results]
    soa_advantages = [r['baseline_soa_vs_aos'] for r in results]
    
    # Grafico 1: Tempi baseline
    bars1 = axes[0].bar(x - width/2, soa_baselines, width, label='SoA Sequential', alpha=0.8)
    bars2 = axes[0].bar(x + width/2, aos_baselines, width, label='AoS Sequential', alpha=0.8)
    
    axes[0].set_xlabel('Test Configuration')
    axes[0].set_ylabel('Execution Time (ms)')
    axes[0].set_title('Sequential Baseline Times')
    axes[0].set_xticks(x)
    axes[0].set_xticklabels(test_names, rotation=45, ha='right')
    axes[0].legend()
    axes[0].grid(True, alpha=0.3)
    
    # Grafico 2: SoA Advantage
    bars3 = axes[1].bar(x, soa_advantages, width, alpha=0.8, color='green')
    axes[1].axhline(y=1.0, color='red', linestyle='--', alpha=0.7, label='No Advantage')
    
    axes[1].set_xlabel('Test Configuration')
    axes[1].set_ylabel('SoA Advantage (AoS_time / SoA_time)')
    axes[1].set_title('SoA vs AoS Sequential Performance')
    axes[1].set_xticks(x)
    axes[1].set_xticklabels(test_names, rotation=45, ha='right')
    axes[1].legend()
    axes[1].grid(True, alpha=0.3)
    
    # Aggiungi etichette
    for bar, adv in zip(bars3, soa_advantages):
        height = bar.get_height()
        axes[1].text(bar.get_x() + bar.get_width()/2., height + 0.01,
                    f'{adv:.2f}x', ha='center', va='bottom', fontsize=10)
    
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'baseline_comparison.png'), dpi=300, bbox_inches='tight')
    plt.close()

def create_summary_table(results, output_dir):
    """Crea tabella riassuntiva con focus su thread scaling"""
    
    fig, ax = plt.subplots(figsize=(20, 8))
    ax.axis('tight')
    ax.axis('off')
    
    # Prepara i dati per la tabella
    table_data = []
    headers = ['Test', 'Config', 'Baseline\nSoA (ms)', 'Baseline\nAoS (ms)', 'SoA\nAdvantage',
               'Best SoA\nSpeedup', 'Best SoA\nThreads', 'Best AoS\nSpeedup', 'Best AoS\nThreads']
    
    for test_data in results:
        config = test_data['config']
        
        # Trova migliori speedup
        best_soa_speedup = 1.0
        best_soa_threads = 1
        best_aos_speedup = 1.0
        best_aos_threads = 1
        
        for tc, thread_info in test_data['thread_data'].items():
            if tc > 1:
                soa_outer_speedup = thread_info['soa']['parallel_outer']['speedup']
                soa_inner_speedup = thread_info['soa']['parallel_inner']['speedup']
                aos_outer_speedup = thread_info['aos']['parallel_outer']['speedup']
                aos_inner_speedup = thread_info['aos']['parallel_inner']['speedup']
                
                best_soa_current = max(soa_outer_speedup, soa_inner_speedup)
                best_aos_current = max(aos_outer_speedup, aos_inner_speedup)
                
                if best_soa_current > best_soa_speedup:
                    best_soa_speedup = best_soa_current
                    best_soa_threads = tc
                
                if best_aos_current > best_aos_speedup:
                    best_aos_speedup = best_aos_current
                    best_aos_threads = tc
        
        row = [
            test_data['test_name'],
            f"{config['num_series']}×{config['series_length']}",
            f"{test_data['baseline_soa']:.1f}",
            f"{test_data['baseline_aos']:.1f}",
            f"{test_data['baseline_soa_vs_aos']:.2f}x",
            f"{best_soa_speedup:.2f}x",
            f"{best_soa_threads}",
            f"{best_aos_speedup:.2f}x",
            f"{best_aos_threads}"
        ]
        table_data.append(row)
    
    table = ax.table(cellText=table_data, colLabels=headers, cellLoc='center', loc='center')
    table.auto_set_font_size(False)
    table.set_fontsize(10)
    table.scale(1.2, 2.0)
    
    # Styling
    for i in range(len(headers)):
        table[(0, i)].set_facecolor('#4CAF50')
        table[(0, i)].set_text_props(weight='bold', color='white')
    
    plt.title('Thread Scaling Performance Summary', fontsize=16, fontweight='bold', pad=20)
    plt.savefig(os.path.join(output_dir, 'thread_scaling_summary_table.png'), dpi=300, bbox_inches='tight')
    plt.close()

def main():
    """Funzione principale"""
    # Configurazione paths
    json_file = 'output/benchmark_results/parallelization_analysis.json'
    output_dir = 'output/plots'
    
    # Crea directory output se non esiste
    Path(output_dir).mkdir(parents=True, exist_ok=True)
    
    # Carica e processa i dati
    print("Loading thread scaling benchmark data...")
    data = load_benchmark_data(json_file)
    results = extract_thread_scaling_data(data)
    
    if not results:
        print("No valid test results found in the JSON file.")
        return
    
    print(f"Processing {len(results)} test configurations with thread scaling...")
    
    # Crea i grafici
    print("Creating individual thread scaling plots for each test...")
    create_thread_scaling_plots(results, output_dir)
    
    print("Creating comprehensive speedup comparison...")
    create_comprehensive_speedup_comparison(results, output_dir)
    
    print("Creating efficiency heatmap...")
    create_efficiency_heatmap(results, output_dir)
    
    print("Creating baseline comparison...")
    create_baseline_comparison(results, output_dir)
    
    print("Creating summary table...")
    create_summary_table(results, output_dir)
    
    print(f"\nAll plots saved to {output_dir}/")
    print("Generated plots:")
    print("  - thread_scaling_[test_name].png (individual analysis)")
    print("  - comprehensive_speedup_comparison.png")
    print("  - efficiency_heatmap.png")
    print("  - baseline_comparison.png")
    print("  - thread_scaling_summary_table.png")


if __name__ == "__main__":
    main()