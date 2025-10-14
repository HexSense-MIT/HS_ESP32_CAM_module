import cv2
import numpy as np
import sys

# Configuration
CALIBRATION_FILE = "fisheye_calibration_data.npz"

def load_calibration_data(filename):
    """Load calibration data from file"""
    try:
        data = np.load(filename)
        return data['camera_matrix'], data['dist_coeffs']
    except FileNotFoundError:
        print(f"Error: Calibration file '{filename}' not found.")
        print("Please run fisheye_calibration.py first.")
        sys.exit(1)

def undistort_image(image_path, K, D, balance=0.0):
    """
    Undistort a fisheye image using calibration data

    Parameters:
    - image_path: Path to the image to undistort
    - K: Camera matrix
    - D: Distortion coefficients
    - balance: Balance parameter (0.0 to 1.0)
             0.0: All pixels in undistorted image are valid (some cropping)
             1.0: All source pixels are visible (some invalid/black areas)
    """
    # Load image
    img = cv2.imread(image_path)
    if img is None:
        print(f"Error: Could not read image '{image_path}'")
        return None

    h, w = img.shape[:2]

    # Calculate new camera matrix
    new_K = cv2.fisheye.estimateNewCameraMatrixForUndistortRectify(
        K, D, (w, h), np.eye(3), balance=balance
    )

    # Create undistortion maps
    map1, map2 = cv2.fisheye.initUndistortRectifyMap(
        K, D, np.eye(3), new_K, (w, h), cv2.CV_16SC2
    )

    # Apply undistortion
    undistorted = cv2.remap(
        img, map1, map2,
        interpolation=cv2.INTER_LINEAR,
        borderMode=cv2.BORDER_CONSTANT
    )

    return undistorted

if __name__ == "__main__":
    # Check command line arguments
    if len(sys.argv) < 2:
        print("Usage: python undistort_image.py <image_path> [balance]")
        print("  image_path: Path to the image to undistort")
        print("  balance: Optional, 0.0-1.0 (default: 0.0)")
        print("\nExample: python undistort_image.py input.jpg 0.5")
        sys.exit(1)

    image_path = sys.argv[1]
    balance = float(sys.argv[2]) if len(sys.argv) > 2 else 0.0

    # Validate balance parameter
    if not 0.0 <= balance <= 1.0:
        print("Error: Balance must be between 0.0 and 1.0")
        sys.exit(1)

    # Load calibration data
    print("Loading calibration data...")
    K, D = load_calibration_data(CALIBRATION_FILE)

    print(f"Undistorting image: {image_path}")
    print(f"Balance parameter: {balance}")

    # Undistort the image
    undistorted = undistort_image(image_path, K, D, balance)

    if undistorted is not None:
        # Generate output filename
        output_path = image_path.rsplit('.', 1)[0] + "_undistorted.jpg"
        cv2.imwrite(output_path, undistorted)
        print(f"Undistorted image saved to: {output_path}")

        # Also save a side-by-side comparison
        original = cv2.imread(image_path)
        comparison = np.hstack((original, undistorted))
        comparison_path = image_path.rsplit('.', 1)[0] + "_comparison.jpg"
        cv2.imwrite(comparison_path, comparison)
        print(f"Comparison image saved to: {comparison_path}")
        print("\nUndistortion complete!")
