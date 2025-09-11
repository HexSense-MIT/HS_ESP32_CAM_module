import matplotlib.pyplot as plt
import numpy as np

def plot_0x00_distances(file_path):
    try:
        with open(file_path, 'rb') as f:
            raw_data = f.read()

        # Find positions of 0x00
        positions = [i for i, byte in enumerate(raw_data) if byte == 0x00]

        # Calculate distances between consecutive positions
        distances = np.diff(positions)

        # Plot the distances
        plt.figure(figsize=(10, 6))
        plt.plot(distances, 'bo', markersize=2, label='Distances between 0x00')
        plt.title('Distances between consecutive 0x00 positions in raw_image_data.bin')
        plt.xlabel('Index')
        plt.ylabel('Distance')
        plt.legend()
        plt.grid(True)
        plt.show()

    except FileNotFoundError:
        print(f"File {file_path} not found.")
    except Exception as e:
        print(f"An error occurred: {e}")

if __name__ == "__main__":
    file_path = "raw_image_data.bin"
    plot_0x00_distances(file_path)