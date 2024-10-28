import matplotlib.pyplot as plt
import re

# Read the gem5 output file (assuming it's named 'm5out.txt')
file_path = 'stats.txt'

# Initialize dictionaries to store stats for decode and execute buffers
cc_buffer_cycles = 0
decode_buffer_occupancy_histogram = {}
decode_buffer_mean = 0.0
decode_buffer_stdev = 0.0

execute_buffer_occupancy_histogram = {}
execute_buffer_mean = 0.0
execute_buffer_stdev = 0.0

# Regex patterns to match relevant stats
patterns = {
    'cc_buffer_cycles': r'board\.processor\.cores0\.core\.cc_buffer\.cc_buffer_cycles\s+(\d+)',
    'decode_buffer_mean': r'board\.processor\.cores0\.core\.cc_buffer\.decode_buffer_occupancy_histogram::mean\s+([\d\.]+)',
    'decode_buffer_stdev': r'board\.processor\.cores0\.core\.cc_buffer\.decode_buffer_occupancy_histogram::stdev\s+([\d\.]+)',
    'decode_buffer_occupancy': r'board\.processor\.cores0\.core\.cc_buffer\.decode_buffer_occupancy_histogram::(\d+)\s+(\d+)',
    'execute_buffer_mean': r'board\.processor\.cores0\.core\.cc_buffer\.execute_buffer_occupancy_histogram::mean\s+([\d\.]+)',
    'execute_buffer_stdev': r'board\.processor\.cores0\.core\.cc_buffer\.execute_buffer_occupancy_histogram::stdev\s+([\d\.]+)',
    'execute_buffer_occupancy': r'board\.processor\.cores0\.core\.cc_buffer\.execute_buffer_occupancy_histogram::(\d+)\s+(\d+)'
}

# Read file and extract stats
with open(file_path, 'r') as f:
    for line in f:
        # Match cc_buffer_cycles
        match = re.search(patterns['cc_buffer_cycles'], line)
        if match:
            cc_buffer_cycles = int(match.group(1))
        
        # Match decode buffer mean and stdev
        match = re.search(patterns['decode_buffer_mean'], line)
        if match:
            decode_buffer_mean = float(match.group(1))
        
        match = re.search(patterns['decode_buffer_stdev'], line)
        if match:
            decode_buffer_stdev = float(match.group(1))

        # Match decode buffer histogram values
        match = re.search(patterns['decode_buffer_occupancy'], line)
        if match:
            bin_value = int(match.group(1))
            count = int(match.group(2))
            decode_buffer_occupancy_histogram[bin_value] = count

        # Match execute buffer mean and stdev
        match = re.search(patterns['execute_buffer_mean'], line)
        if match:
            execute_buffer_mean = float(match.group(1))
        
        match = re.search(patterns['execute_buffer_stdev'], line)
        if match:
            execute_buffer_stdev = float(match.group(1))

        # Match execute buffer histogram values
        match = re.search(patterns['execute_buffer_occupancy'], line)
        if match:
            bin_value = int(match.group(1))
            count = int(match.group(2))
            execute_buffer_occupancy_histogram[bin_value] = count

# Plot decode buffer occupancy histogram
bins_decode = list(decode_buffer_occupancy_histogram.keys())
counts_decode = list(decode_buffer_occupancy_histogram.values())

plt.figure(figsize=(10, 6))
plt.bar(bins_decode, counts_decode, color='blue')
plt.xlabel('Decode Buffer Occupancy')
plt.ylabel('Number of Samples')
plt.title('Decode Buffer Occupancy Histogram')

# Annotate the plot with mean and standard deviation
plt.text(max(bins_decode) * 0.7, max(counts_decode) * 0.9, f'Mean: {decode_buffer_mean:.2f}', fontsize=12)
plt.text(max(bins_decode) * 0.7, max(counts_decode) * 0.8, f'Std Dev: {decode_buffer_stdev:.2f}', fontsize=12)

# Save the decode buffer occupancy plot
output_file_decode = 'decode_buffer_histogram.png'
plt.savefig(output_file_decode)

# Plot execute buffer occupancy histogram
bins_execute = list(execute_buffer_occupancy_histogram.keys())
counts_execute = list(execute_buffer_occupancy_histogram.values())

plt.figure(figsize=(10, 6))
plt.bar(bins_execute, counts_execute, color='green')
plt.xlabel('Execute Buffer Occupancy')
plt.ylabel('Number of Samples')
plt.title('Execute Buffer Occupancy Histogram')

# Annotate the plot with mean and standard deviation
plt.text(max(bins_execute) * 0.7, max(counts_execute) * 0.9, f'Mean: {execute_buffer_mean:.2f}', fontsize=12)
plt.text(max(bins_execute) * 0.7, max(counts_execute) * 0.8, f'Std Dev: {execute_buffer_stdev:.2f}', fontsize=12)

# Save the execute buffer occupancy plot
output_file_execute = 'execute_buffer_histogram.png'
plt.savefig(output_file_execute)

# Optionally, print extracted stats
print(f"cc_buffer_cycles: {cc_buffer_cycles}")
print(f"decode_buffer_mean: {decode_buffer_mean}")
print(f"decode_buffer_stdev: {decode_buffer_stdev}")
print(f"Plot saved as {output_file_decode}")

print(f"execute_buffer_mean: {execute_buffer_mean}")
print(f"execute_buffer_stdev: {execute_buffer_stdev}")
print(f"Plot saved as {output_file_execute}")
