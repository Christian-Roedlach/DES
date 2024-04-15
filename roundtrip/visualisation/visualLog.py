import pandas as pd
import matplotlib.pyplot as plt
from statistics import mean
import sys
from pathlib import Path

# command line arguments
args_n = len(sys.argv)

print("Usage:   python3 visualLog.py <Filename>")
print("Example: python3 visualLog.py ../log/pi4_to_pi3_no_load_500000.csv")
print()

if args_n > 1:
    file_path = sys.argv[1]
else:
    file_path = "test_localhost_debug.csv"


# cvs input
data = pd.read_csv(file_path)

# Count occurrences of each unique value and sort by index
value_counts = data['roundtrip_time [s]'].value_counts()
sorted_counts = value_counts.sort_index()

# calc mean and get max and min values
data_mean = mean(sorted_counts.index)
data_max = sorted_counts.index.max()
data_min = sorted_counts.index.min()

print("Arithmetisches Mittel (Mean) :", data_mean)
print("Maximum value:", data_max)
print("Minimum value:", data_min)

# mit der eigenen histogram funktion plt.hist()
# Read in the DataFrame
df = pd.read_csv(file_path)
data = df['roundtrip_time [s]']

# Set figure size
plt.figure(figsize=(18,8)) # figure size in pixels * 100 (e.g. 1800x800px)

# Plot the histogram
plt.hist(data, bins=1200, range=(data_min, data_max), log=True, bottom="scalar")  # bins is nmbr of divisions

# Add labels and title
plt.xlabel('roundtrip_time [s]')
plt.ylabel('Frequency')
plt.title('Histogram of ' + Path(file_path).name)
plt.text(data_max,10,"min = {:.6f} s, mean = {:.6f} s, max = {:.6f} s".format(data_min, data_mean, data_max), horizontalalignment="right")

plt.savefig('../log/sum/logarithmic/' + Path(file_path).stem + '.svg')

# Show the plot
plt.show()
