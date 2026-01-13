import json
import matplotlib.pyplot as plt
import numpy as np
import os
from pathlib import Path
import seaborn as sns

plt.style.use('seaborn-v0_8')
sns.set_palette("husl")

def load_benchmark_data(json_file):
    with open(json_file, 'r') as f:
        data = json.load(f)
    return data

def extract_thread_scaling_data(data):
    results = []
    
    for test in data['tests']:
        if 'error' in test:
            continue
            
        test_name = test['test_name']
        config = test['configuration']
        thread_results = test['thread_results']
        
        test_data = {
            'test_name': test_name,
            'config': config,
            'thread_counts': config['thread_counts'],
            'baseline_soa': test['summary']['baseline_soa_time_ms'],
            'baseline_aos': test['summary']['baseline_aos_time_ms'],
            'baseline_soa_vs_aos': test['summary']['baseline_soa_vs_aos'],
            'thread_data': {}
        }
        
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
    for test_data in results:
        test_name = test_data['test_name']
        config = test_data['config']
        thread_counts = sorted(test_data['thread_data'].keys())
        
        fig, ax = plt.subplots(figsize=(12, 6))
        
        fig.suptitle(f'Thread Scaling Analysis - {test_name}\n'
                    f'{config["num_series"]} series × {config["series_length"]} points '
                    f'(query: {config["query_length"]}, runs: {config["num_runs"]})', 
                    fontsize=16, fontweight='bold')
        
        soa_outer_speedups = []
        soa_inner_speedups = []
        aos_outer_speedups = []
        aos_inner_speedups = []
        
        for tc in thread_counts:
            thread_info = test_data['thread_data'][tc]
            
            if tc == 1:
                # Sequential
                soa_outer_speedups.append(1.0)
                soa_inner_speedups.append(1.0)
                aos_outer_speedups.append(1.0)
                aos_inner_speedups.append(1.0)
            else:
                # Parallel
                soa_outer_speedups.append(thread_info['soa']['parallel_outer']['speedup'])
                soa_inner_speedups.append(thread_info['soa']['parallel_inner']['speedup'])
                aos_outer_speedups.append(thread_info['aos']['parallel_outer']['speedup'])
                aos_inner_speedups.append(thread_info['aos']['parallel_inner']['speedup'])
        
        best_soa_speedups = [max(soa_outer_speedups[i], soa_inner_speedups[i]) for i in range(len(thread_counts))]
        best_aos_speedups = [max(aos_outer_speedups[i], aos_inner_speedups[i]) for i in range(len(thread_counts))]
        
        width = 0.35
        x = np.arange(len(thread_counts))
        bars1 = ax.bar(x - width/2, best_soa_speedups, width, label='SoA Best', alpha=0.8, color='blue')
        bars2 = ax.bar(x + width/2, best_aos_speedups, width, label='AoS Best', alpha=0.8, color='red')
        
        ax.set_xlabel('Number of Threads')
        ax.set_ylabel('Best Speedup')
        ax.set_title('Best Speedup Comparison: SoA vs AoS')
        ax.set_xticks(x)
        ax.set_xticklabels(thread_counts)
        ax.legend()
        ax.grid(True, alpha=0.3)
        
        for bar, speedup in zip(bars1, best_soa_speedups):
            height = bar.get_height()
            ax.text(bar.get_x() + bar.get_width()/2., height + 0.05,
                    f'{speedup:.2f}x', ha='center', va='bottom', fontsize=8)
        for bar, speedup in zip(bars2, best_aos_speedups):
            height = bar.get_height()
            ax.text(bar.get_x() + bar.get_width()/2., height + 0.05,
                    f'{speedup:.2f}x', ha='center', va='bottom', fontsize=8)
        
        plt.tight_layout()
        plt.savefig(os.path.join(output_dir, f'thread_scaling_{test_name}.png'), dpi=300, bbox_inches='tight')
        plt.close()

def create_comprehensive_speedup_comparison(results, output_dir):
    if not results:
        return

    all_thread_counts = sorted({tc for td in results for tc in td['thread_data'].keys()})
    if not all_thread_counts:
        return

    fig, axes = plt.subplots(2, 2, figsize=(16, 12), constrained_layout=True)
    fig.suptitle('Thread Scaling Analysis', fontsize=16, fontweight='bold')

    colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728']
    markers = ['o', 's', '^', 'D']

    x_min = min(all_thread_counts) - 1
    x_max = max(all_thread_counts) + 2

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
    
    ax1.set_xlabel('Number of Threads')
    ax1.set_ylabel('Speedup')
    ax1.set_title('SoA Outer Parallelization Speedup')
    ax1.legend()
    ax1.grid(True, alpha=0.3)
    ax1.set_xlim(x_min, x_max)
    ax1.set_ylim(0, 10)
    ax1.set_xticks(all_thread_counts)
    
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
    
    ax2.set_xlabel('Number of Threads')
    ax2.set_ylabel('Speedup')
    ax2.set_title('SoA Inner Parallelization Speedup')
    ax2.legend()
    ax2.grid(True, alpha=0.3)
    ax2.set_xlim(x_min, x_max)
    ax2.set_ylim(0, 10)
    ax2.set_xticks(all_thread_counts)
    
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
    
    ax3.set_xlabel('Number of Threads')
    ax3.set_ylabel('Speedup')
    ax3.set_title('AoS Outer Parallelization Speedup')
    ax3.legend()
    ax3.grid(True, alpha=0.3)
    ax3.set_xlim(x_min, x_max)
    ax3.set_ylim(0, 10)
    ax3.set_xticks(all_thread_counts)
    
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
    
    ax4.set_xlabel('Number of Threads')
    ax4.set_ylabel('Speedup')
    ax4.set_title('AoS Inner Parallelization Speedup')
    ax4.legend()
    ax4.grid(True, alpha=0.3)
    ax4.set_xlim(x_min, x_max)
    ax4.set_ylim(0, 10)
    ax4.set_xticks(all_thread_counts)
    
    plt.savefig(os.path.join(output_dir, 'comprehensive_speedup_comparison.png'), dpi=300, bbox_inches='tight')
    plt.close()
    
def create_baseline_comparison(results, output_dir):
    
    fig, ax = plt.subplots(figsize=(12, 6))
    fig.suptitle('Sequential Baseline Comparison', fontsize=16, fontweight='bold')
    
    test_names = [r['test_name'] for r in results]
    x = np.arange(len(test_names))
    width = 0.35
    
    soa_baselines = [r['baseline_soa'] for r in results]
    aos_baselines = [r['baseline_aos'] for r in results]
    
    bars1 = ax.bar(x - width/2, soa_baselines, width, label='SoA Sequential', alpha=0.8, color='blue')
    bars2 = ax.bar(x + width/2, aos_baselines, width, label='AoS Sequential', alpha=0.8, color='orange')
    
    ax.set_xlabel('Test Configuration')
    ax.set_ylabel('Execution Time (ms)')
    ax.set_title('Sequential Baseline Times')
    ax.set_xticks(x)
    ax.set_xticklabels(test_names, rotation=45, ha='right')
    ax.legend()
    ax.grid(True, alpha=0.3)
    
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'baseline_comparison.png'), dpi=300, bbox_inches='tight')
    plt.close()

def create_summary_table(results, output_dir):
    
    fig, ax = plt.subplots(figsize=(20, 8))
    ax.axis('tight')
    ax.axis('off')
    
    table_data = []
    headers = ['Test', 'Config', 'Baseline\nSoA (ms)', 'Baseline\nAoS (ms)', 'SoA\nAdvantage',
               'Best SoA\nSpeedup', 'Best SoA\nThreads', 'Best AoS\nSpeedup', 'Best AoS\nThreads']
    
    for test_data in results:
        config = test_data['config']
        
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
    
    for i in range(len(headers)):
        table[(0, i)].set_facecolor('#4CAF50')
        table[(0, i)].set_text_props(weight='bold', color='white')
    
    plt.title('Thread Scaling Performance Summary', fontsize=16, fontweight='bold', pad=20)
    plt.savefig(os.path.join(output_dir, 'thread_scaling_summary_table.png'), dpi=300, bbox_inches='tight')
    plt.close()

def main():
    json_file = 'output/benchmark_results/parallelization_analysis.json'
    output_dir = 'output/plots'
    
    Path(output_dir).mkdir(parents=True, exist_ok=True)
    
    print("Loading thread scaling benchmark data...")
    data = load_benchmark_data(json_file)
    results = extract_thread_scaling_data(data)
    
    if not results:
        print("No valid test results found in the JSON file.")
        return
    
    print(f"Processing {len(results)} test configurations with thread scaling...")
    
    print("Creating individual thread scaling plots for each test...")
    create_thread_scaling_plots(results, output_dir)
    
    print("Creating comprehensive speedup comparison...")
    create_comprehensive_speedup_comparison(results, output_dir)
        
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