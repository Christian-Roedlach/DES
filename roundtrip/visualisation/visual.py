import pandas as pd
import matplotlib.pyplot as plt
from statistics import mean

# cvs input
file_path = "test_localhost_debug.csv"
data = pd.read_csv(file_path)

# Count occurrences of each unique value and sort by index
value_counts = data['roundtrip_time [s]'].value_counts()
sorted_counts = value_counts.sort_index()


# cvs input
#file_path = "Data-norm-index.csv"
#data = pd.read_csv(file_path)

# Count occurrences of each unique value and sort by index
#value_counts = data['Data'].value_counts()
#sorted_counts = value_counts.sort_index()



# calc mean and get max and min values
data_mean = mean(sorted_counts.index)
data_max = sorted_counts.index.max()
data_min = sorted_counts.index.min()

print("Arithmetisches Mittel (Mean) :", data_mean)
print("Maximum value:", data_max)
print("Minimum value:", data_min)

# Plotting the data as a bar chart
plt.figure(figsize=(10, 10))
plt.bar(sorted_counts.index, sorted_counts.values, color='skyblue')
plt.xlabel('Round Trip time (s)')
plt.ylabel('Frequency')
plt.title('Frequency of Data Points')

# Set x-axis limits
#plt.ylim(0, 2000)
#plt.xlim(0, 5000)

plt.show()


#mit der eigenen histogram funktion plt.hist()
# Read in the DataFrame
df = pd.read_csv('test_localhost_debug.csv')
data = df['roundtrip_time [s]']

# Plot the histogram
plt.hist(data, bins=1000, range=(6.91e-07, 20.91e-07))  # bins is nmbr of divisions

# Add labels and title
plt.xlabel('Values')
plt.ylabel('Frequency')
plt.title('Histogram of Values')

# Show the plot
plt.show()

