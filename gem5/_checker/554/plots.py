import matplotlib.pyplot as plt
import re

# Read the gem5 output file (assuming it's named 'm5out.txt')
file_path = 'lbm_s_stats.txt'
# file_path = 'saved_stats/iteration1.txt'

# Initialize dictionaries to store stats for each core and bank
cc_buffer_cycles = 0
decode_buffer_occupancy_histograms = {}
decode_buffer_means = {}
decode_buffer_stdevs = {}

execute_buffer_occupancy_histograms = {}
execute_buffer_means = {}
execute_buffer_stdevs = {}

# Initialize dictionaries to store stats for cc_buffer_test_bank_test (for each bank)
cc_buffer_bank_histograms = {}
cc_buffer_bank_means = {}
cc_buffer_bank_stdevs = {}

# Regex patterns to match relevant stats for multiple cores and banks
patterns = {
    # Patterns for cc_buffer_cycles (supporting multiple cores)
    'cc_buffer_cycles': r'board\.processor\.cores(\d+)\.core\.cc_buffer\.cc_buffer_cycles\s+(\d+)',

    # Patterns for decode buffer (supporting multiple cores)
    'decode_buffer_mean': r'board\.processor\.cores(\d+)\.core\.cc_buffer\.decode_buffer_occupancy_histogram::mean\s+([\d\.]+)',
    'decode_buffer_stdev': r'board\.processor\.cores(\d+)\.core\.cc_buffer\.decode_buffer_occupancy_histogram::stdev\s+([\d\.]+)',
    'decode_buffer_occupancy': r'board\.processor\.cores(\d+)\.core\.cc_buffer\.decode_buffer_occupancy_histogram::(\d+)\s+(\d+)',

    # Patterns for execute buffer (supporting multiple cores)
    'execute_buffer_mean': r'board\.processor\.cores(\d+)\.core\.cc_buffer\.execute_buffer_occupancy_histogram::mean\s+([\d\.]+)',
    'execute_buffer_stdev': r'board\.processor\.cores(\d+)\.core\.cc_buffer\.execute_buffer_occupancy_histogram::stdev\s+([\d\.]+)',
    'execute_buffer_occupancy': r'board\.processor\.cores(\d+)\.core\.cc_buffer\.execute_buffer_occupancy_histogram::(\d+)\s+(\d+)',

    # Patterns for cc_buffer_test_bank_test (for multiple banks)
    'cc_buffer_test_bank_test_samples': r'board\.cache_hierarchy\.cc_l3cache\.bank(\d+)\.cc_buffer_test_bank_test::samples\s+(\d+)',
    'cc_buffer_test_bank_test_mean': r'board\.cache_hierarchy\.cc_l3cache\.bank(\d+)\.cc_buffer_test_bank_test::mean\s+([\d\.]+)',
    'cc_buffer_test_bank_test_stdev': r'board\.cache_hierarchy\.cc_l3cache\.bank(\d+)\.cc_buffer_test_bank_test::stdev\s+([\d\.]+)',
    'cc_buffer_test_bank_test_occupancy': r'board\.cache_hierarchy\.cc_l3cache\.bank(\d+)\.cc_buffer_test_bank_test::(\d+)\s+(\d+)'
}

# Read file and extract stats
with open(file_path, 'r') as f:
    for line in f:
        # Match cc_buffer_cycles for each core
        match = re.search(patterns['cc_buffer_cycles'], line)
        if match:
            core_id = int(match.group(1))
            cc_buffer_cycles = int(match.group(2))

        # Match decode buffer mean and stdev for each core
        match = re.search(patterns['decode_buffer_mean'], line)
        if match:
            core_id = int(match.group(1))
            decode_buffer_mean = float(match.group(2))
            if core_id not in decode_buffer_means:
                decode_buffer_means[core_id] = decode_buffer_mean
        
        match = re.search(patterns['decode_buffer_stdev'], line)
        if match:
            core_id = int(match.group(1))
            decode_buffer_stdev = float(match.group(2))
            if core_id not in decode_buffer_stdevs:
                decode_buffer_stdevs[core_id] = decode_buffer_stdev

        # Match decode buffer histogram values for each core
        match = re.search(patterns['decode_buffer_occupancy'], line)
        if match:
            core_id = int(match.group(1))
            bin_value = int(match.group(2))
            count = int(match.group(3))
            if core_id not in decode_buffer_occupancy_histograms:
                decode_buffer_occupancy_histograms[core_id] = {}
            decode_buffer_occupancy_histograms[core_id][bin_value] = count

        # Match execute buffer mean and stdev for each core
        match = re.search(patterns['execute_buffer_mean'], line)
        if match:
            core_id = int(match.group(1))
            execute_buffer_mean = float(match.group(2))
            if core_id not in execute_buffer_means:
                execute_buffer_means[core_id] = execute_buffer_mean
        
        match = re.search(patterns['execute_buffer_stdev'], line)
        if match:
            core_id = int(match.group(1))
            execute_buffer_stdev = float(match.group(2))
            if core_id not in execute_buffer_stdevs:
                execute_buffer_stdevs[core_id] = execute_buffer_stdev

        # Match execute buffer histogram values for each core
        match = re.search(patterns['execute_buffer_occupancy'], line)
        if match:
            core_id = int(match.group(1))
            bin_value = int(match.group(2))
            count = int(match.group(3))
            if core_id not in execute_buffer_occupancy_histograms:
                execute_buffer_occupancy_histograms[core_id] = {}
            execute_buffer_occupancy_histograms[core_id][bin_value] = count

        # Match cc_buffer_test_bank_test samples, mean, and stdev for each bank
        match = re.search(patterns['cc_buffer_test_bank_test_samples'], line)
        if match:
            bank_id = int(match.group(1))
            samples = int(match.group(2))
            if bank_id not in cc_buffer_bank_histograms:
                cc_buffer_bank_histograms[bank_id] = {}
            cc_buffer_bank_histograms[bank_id]['samples'] = samples
        
        match = re.search(patterns['cc_buffer_test_bank_test_mean'], line)
        if match:
            bank_id = int(match.group(1))
            mean = float(match.group(2))
            cc_buffer_bank_means[bank_id] = mean
        
        match = re.search(patterns['cc_buffer_test_bank_test_stdev'], line)
        if match:
            bank_id = int(match.group(1))
            stdev = float(match.group(2))
            cc_buffer_bank_stdevs[bank_id] = stdev

        # Match cc_buffer_test_bank_test occupancy for each bank
        match = re.search(patterns['cc_buffer_test_bank_test_occupancy'], line)
        if match:
            bank_id = int(match.group(1))
            bin_value = int(match.group(2))
            count = int(match.group(3))
            if bank_id not in cc_buffer_bank_histograms:
                cc_buffer_bank_histograms[bank_id] = {}
            cc_buffer_bank_histograms[bank_id][bin_value] = count

# Plot decode buffer occupancy histogram for each core
for core_id, histogram in decode_buffer_occupancy_histograms.items():
    bins_decode = list(histogram.keys())
    counts_decode = list(histogram.values())

    plt.figure(figsize=(10, 6))
    plt.bar(bins_decode, counts_decode, color='blue')
    plt.xlabel(f'Decode Buffer Occupancy (Core {core_id})')
    plt.ylabel('Number of Samples')
    plt.title(f'Decode Buffer Occupancy Histogram for Core {core_id}')

    # Annotate the plot with mean and standard deviation
    if core_id in decode_buffer_means and core_id in decode_buffer_stdevs:
        plt.text(max(bins_decode) * 0.7, max(counts_decode) * 0.9, f'Mean: {decode_buffer_means[core_id]:.2f}', fontsize=12)
        plt.text(max(bins_decode) * 0.7, max(counts_decode) * 0.8, f'Std Dev: {decode_buffer_stdevs[core_id]:.2f}', fontsize=12)

    # Save the plot for this core
    output_file_decode = f'plots/decode_buffer/decode_buffer_histogram_core_{core_id}.png'
    plt.savefig(output_file_decode)

    print(f"Decode Buffer Core {core_id} done")

    plt.close()

# Plot execute buffer occupancy histogram for each core
for core_id, histogram in execute_buffer_occupancy_histograms.items():
    bins_execute = list(histogram.keys())
    counts_execute = list(histogram.values())

    plt.figure(figsize=(10, 6))
    plt.bar(bins_execute, counts_execute, color='green')
    plt.xlabel(f'Execute Buffer Occupancy (Core {core_id})')
    plt.ylabel('Number of Samples')
    plt.title(f'Execute Buffer Occupancy Histogram for Core {core_id}')

    # Annotate the plot with mean and standard deviation
    if core_id in execute_buffer_means and core_id in execute_buffer_stdevs:
        plt.text(max(bins_execute) * 0.7, max(counts_execute) * 0.9, f'Mean: {execute_buffer_means[core_id]:.2f}', fontsize=12)
        plt.text(max(bins_execute) * 0.7, max(counts_execute) * 0.8, f'Std Dev: {execute_buffer_stdevs[core_id]:.2f}', fontsize=12)

    # Save the plot for this core
    output_file_execute = f'plots/execute_buffer/execute_buffer_histogram_core_{core_id}.png'
    plt.savefig(output_file_execute)

    print(f"Execute Buffer Core {core_id} done")

    plt.close()

# Now plot the histograms for each bank's cc_buffer_test_bank_test data
for bank_id, histogram in cc_buffer_bank_histograms.items():
    if 'samples' in histogram:  # Only plot if there are samples
        bins_bank = list(histogram.keys())
        bins_bank_str = [str(bin_val) for bin_val in bins_bank]  # Convert to strings
        
        counts_bank = [histogram[bin_val] for bin_val in bins_bank]

        plt.figure(figsize=(10, 6))
        plt.bar(bins_bank_str, counts_bank, color='purple')  # Plot using string bins
        
        plt.xlabel(f'Bank {bank_id} Occupancy')
        plt.ylabel('Number of Samples')
        plt.title(f'cc_buffer_test_bank_test Occupancy for Bank {bank_id}')
        
        # Annotate the plot with mean and standard deviation if available
        if bank_id in cc_buffer_bank_means and bank_id in cc_buffer_bank_stdevs:
            plt.text(len(bins_bank) * 0.7, max(counts_bank) * 0.9, f'Mean: {cc_buffer_bank_means[bank_id]:.2f}', fontsize=12)
            plt.text(len(bins_bank) * 0.7, max(counts_bank) * 0.8, f'Std Dev: {cc_buffer_bank_stdevs[bank_id]:.2f}', fontsize=12)

        # Save the plot for this bank
        output_file_bank = f'plots/banked_cache/cc_buffer_test_bank_test_histogram_bank_{bank_id}.png'
        plt.savefig(output_file_bank)

        if bank_id % 8 == 0:
            print(f"Bank {bank_id} done")

        plt.close()

# Optionally, print extracted stats
print(f"cc_buffer_cycles: {cc_buffer_cycles}")
print(f"Plots saved for each core and bank.")

# import matplotlib.pyplot as plt
# import re

# Path to the stats file
# file_path = '../../m5out/stats.txt'

# Initialize dictionaries to store stats for each core
core_metrics = {}

# Regex patterns to capture the relevant stats for each core
patterns = {
    'ooo_stall_signals': r'board\.processor\.cores(\d+)\.core\.cc_buffer\.ooo_stall_signals\s+(\d+)',
    'execute_old_inst_not_finished': r'board\.processor\.cores(\d+)\.core\.cc_buffer\.execute_old_inst_not_finished\s+(\d+)',
    'decode_execute_full_stalls': r'board\.processor\.cores(\d+)\.core\.cc_buffer\.decode_execute_full_stalls\s+(\d+)',
    'regfile_bandwidth_reached': r'board\.processor\.cores(\d+)\.core\.cc_buffer\.regfile_bandwidth_reached\s+(\d+)',
    'decode_bandwidth_stalls': r'board\.processor\.cores(\d+)\.core\.cc_buffer\.decode_bandwidth_stalls\s+(\d+)',
    'bank_queue_full_block': r'board\.processor\.cores(\d+)\.core\.cc_buffer\.bank_queue_full_block\s+(\d+)',
    'num_fault_insts': r'board\.processor\.cores(\d+)\.core\.cc_buffer\.num_fault_insts\s+(\d+)',
    'num_unknown_insts': r'board\.processor\.cores(\d+)\.core\.cc_buffer\.num_unknown_insts\s+(\d+)',
}

# Read the file and extract stats
# file_path = 'your_log_file.log'  # Specify the path to your log file
with open(file_path, 'r') as f:
    for line in f:
        # Check each pattern
        for key, pattern in patterns.items():
            match = re.search(pattern, line)
            if match:
                core_id = int(match.group(1))
                value = int(match.group(2))
                
                # If the core is not in the dictionary, add it
                if core_id not in core_metrics:
                    core_metrics[core_id] = {
                        'ooo_stall_signals': 0,
                        'execute_old_inst_not_finished': 0,
                        'decode_execute_full_stalls': 0,
                        'regfile_bandwidth_reached': 0,
                        'decode_bandwidth_stalls': 0,
                        'bank_queue_full_block': 0,
                        'num_fault_insts': 0,
                        'num_unknown_insts': 0,
                    }
                
                # Update the appropriate metric for the core
                core_metrics[core_id][key] = value

# Now we create a bar chart for each core
for core_id, metrics in core_metrics.items():
    # Bar labels and values
    labels = list(metrics.keys())
    values = list(metrics.values())
    
    # Create a bar chart
    plt.figure(figsize=(10, 6))
    bars = plt.bar(labels, values)
    
    # Add title and labels
    plt.title(f"Core {core_id} - Buffer Metrics")
    plt.xlabel('Metric')
    plt.ylabel('Count')
    
    # Rotate x-axis labels for better readability
    plt.xticks(rotation=45, ha='right')
    
    # Annotate each bar with the value
    for bar in bars:
        height = bar.get_height()
        plt.text(bar.get_x() + bar.get_width() / 2, height + 0.1,  # Adjust vertical positioning
                 f'{height}', ha='center', va='bottom', fontsize=10)
    
    # Show the plot
    plt.tight_layout()
    output_path = f'plots/core_stats/core_{core_id}_stats.png'
    plt.savefig(output_path)

    plt.close()