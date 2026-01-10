import pandas as pd
import matplotlib.pyplot as plt

# Read the CSV file
df = pd.read_csv('camera_module_pwr_consum.csv')

# Convert current from uA to mA
df['Current(mA)'] = df['Current(uA)'] / 1000

# Convert timestamp to seconds (sampling frequency is 1000 Hz)
df['Time(s)'] = df['Timestamp(ms)'] / 1000

# Create the plot
plt.figure(figsize=(12, 6))
plt.plot(df['Time(s)'], df['Current(mA)'], linewidth=0.5)
plt.xlabel('Time (s)', fontsize=14)
plt.ylabel('Current (mA)', fontsize=14)
plt.title('Camera Module Power Consumption', fontsize=16)
plt.tick_params(axis='both', which='major', labelsize=12)
plt.grid(True, alpha=0.3)
plt.tight_layout()

# Show the plot
plt.show()

# Calculate power (voltage is 3.7V)
voltage = 3.7  # V
df['Power(mW)'] = voltage * df['Current(mA)']

# Calculate average power
avg_power = df['Power(mW)'].mean()

# Print some statistics
print(f"Statistics:")
print(f"Voltage: {voltage} V")
print(f"Mean current: {df['Current(mA)'].mean():.3f} mA")
print(f"Max current: {df['Current(mA)'].max():.3f} mA")
print(f"Min current: {df['Current(mA)'].min():.3f} mA")
print(f"Duration: {df['Time(s)'].max():.3f} s")
print(f"\nAverage Power: {avg_power:.3f} mW ({avg_power/1000:.6f} W)")
