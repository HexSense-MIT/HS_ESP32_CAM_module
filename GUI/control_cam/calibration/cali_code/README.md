# Fisheye Camera Calibration

This directory contains scripts for calibrating a fisheye camera using chessboard calibration images.

## Requirements

```bash
pip install opencv-python numpy
```

## Configuration

Before running the calibration, adjust these parameters in `fisheye_calibration.py`:

- **CHECKERBOARD**: Number of inner corners (columns, rows) in your chessboard
  - Default: `(6, 9)` - adjust based on your actual chessboard
  - Count the number of internal corner intersections, NOT the squares
- **SQUARE_SIZE**: Size of each square in cm
  - Default: `2.5` cm
- **IMAGE_PATH**: Path pattern to your calibration images
  - Default: `"./*.jpg"`

## Usage

### 1. Capture Calibration Images

Take 10-20 photos of your chessboard from different angles and distances:
- Hold the chessboard at different orientations
- Move it to different positions in the frame
- Ensure the entire chessboard is visible in each image
- Use good lighting conditions

Place all images in this directory.

### 2. Run Calibration

```bash
python fisheye_calibration.py
```

This will:
- Detect chessboard corners in each image
- Calculate the fisheye camera calibration matrix
- Save calibration data to `fisheye_calibration_data.npz`
- Create visualization images showing detected corners
- Generate an example undistorted image comparison

### 3. Undistort New Images

Use the calibration data to undistort new images:

```bash
python undistort_image.py <image_path> [balance]
```

**Parameters:**
- `image_path`: Path to the image you want to undistort
- `balance`: (Optional) Balance parameter between 0.0 and 1.0
  - `0.0`: All pixels in output are valid (some cropping may occur)
  - `1.0`: All source pixels are visible (may have black borders)
  - Default: `0.0`

**Example:**
```bash
python undistort_image.py my_photo.jpg 0.5
```

## Output Files

After calibration, you'll get:
- `fisheye_calibration_data.npz`: Calibration data (camera matrix, distortion coefficients)
- `detected_corners_*.jpg`: Visualization of detected chessboard corners
- `undistortion_comparison.jpg`: Side-by-side comparison of original vs undistorted

## Calibration Quality

- **RMS Error**: The calibration script reports the RMS re-projection error
  - Good calibration: < 1.0 pixel
  - Acceptable: < 2.0 pixels
  - If error is high, try:
    - Capturing more images
    - Ensuring better lighting
    - Using a larger/flatter chessboard

## Tips for Better Calibration

1. Use a **rigid, flat chessboard** (printed and mounted on board)
2. Take images covering the **entire field of view**
3. Include images at **various angles** and distances
4. Ensure **sharp, well-lit images**
5. Aim for **at least 10-15 successful detections**

## Troubleshooting

**No chessboard detected:**
- Check if the CHECKERBOARD size matches your actual board
- Ensure the entire board is visible in the image
- Improve lighting conditions
- Try a larger chessboard pattern

**High calibration error:**
- Capture more calibration images
- Ensure the chessboard is perfectly flat
- Check for motion blur in images

## Calibration Data Format

The `fisheye_calibration_data.npz` file contains:
- `camera_matrix`: 3x3 intrinsic camera matrix (K)
- `dist_coeffs`: 4x1 fisheye distortion coefficients (k1, k2, k3, k4)
- `rvecs`: Rotation vectors for each calibration image
- `tvecs`: Translation vectors for each calibration image
- `image_shape`: Image dimensions (width, height)
- `rms_error`: RMS re-projection error
- `successful_images`: List of images used in calibration
