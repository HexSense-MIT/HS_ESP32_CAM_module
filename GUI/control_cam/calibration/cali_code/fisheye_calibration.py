import cv2
import numpy as np
import glob
import os

# Configuration
CHECKERBOARD = (6, 9)  # Number of inner corners (columns, rows) - adjust based on your chessboard
SQUARE_SIZE = 2.5  # Size of a square in cm
IMAGE_PATH = "./*.jpg"  # Path to calibration images
OUTPUT_FILE = "fisheye_calibration_data.npz"

# Termination criteria for corner sub-pixel accuracy
criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 0.001)

# Prepare object points based on the actual chessboard dimensions
objp = np.zeros((CHECKERBOARD[0] * CHECKERBOARD[1], 3), np.float32)
objp[:, :2] = np.mgrid[0:CHECKERBOARD[0], 0:CHECKERBOARD[1]].T.reshape(-1, 2)
objp = objp * SQUARE_SIZE  # Scale by square size

# Arrays to store object points and image points from all images
objpoints = []  # 3D points in real world space
imgpoints = []  # 2D points in image plane

# Get list of calibration images
images = glob.glob(IMAGE_PATH)

if not images:
    print(f"No images found at path: {IMAGE_PATH}")
    print("Please ensure calibration images are in the current directory.")
    exit(1)

print(f"Found {len(images)} images for calibration")
print(f"Chessboard pattern: {CHECKERBOARD[0]}x{CHECKERBOARD[1]} inner corners")
print(f"Square size: {SQUARE_SIZE} cm\n")

successful_images = []
img_shape = None

# Process each image
for idx, fname in enumerate(images):
    img = cv2.imread(fname)
    if img is None:
        print(f"Warning: Could not read image {fname}")
        continue

    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    img_shape = gray.shape[::-1]  # (width, height)

    print(f"Processing image {idx + 1}/{len(images)}: {os.path.basename(fname)}")

    # Find the chessboard corners
    ret, corners = cv2.findChessboardCorners(
        gray,
        CHECKERBOARD,
        cv2.CALIB_CB_ADAPTIVE_THRESH + cv2.CALIB_CB_FAST_CHECK + cv2.CALIB_CB_NORMALIZE_IMAGE
    )

    # If found, add object points and image points after refining them
    if ret:
        objpoints.append(objp)

        # Refine corner locations to sub-pixel accuracy
        corners_refined = cv2.cornerSubPix(gray, corners, (3, 3), (-1, -1), criteria)
        imgpoints.append(corners_refined)

        successful_images.append(fname)
        print(f"  ✓ Chessboard detected successfully")

        # Draw and display the corners (optional - for visualization)
        img_with_corners = cv2.drawChessboardCorners(img, CHECKERBOARD, corners_refined, ret)

        # Optionally save visualization
        output_vis_path = f"detected_corners_{idx + 1}.jpg"
        cv2.imwrite(output_vis_path, img_with_corners)

    else:
        print(f"  ✗ Chessboard not detected")

print(f"\n{'='*60}")
print(f"Successfully processed {len(successful_images)}/{len(images)} images")

if len(objpoints) < 3:
    print("Error: Need at least 3 images with detected chessboards for calibration")
    exit(1)

# Perform fisheye camera calibration
print(f"\nPerforming fisheye camera calibration...")

# Initialize camera matrix and distortion coefficients
K = np.zeros((3, 3))
D = np.zeros((4, 1))
rvecs = [np.zeros((1, 1, 3), dtype=np.float64) for _ in range(len(objpoints))]
tvecs = [np.zeros((1, 1, 3), dtype=np.float64) for _ in range(len(objpoints))]

# Calibration flags for fisheye model
calibration_flags = (
    cv2.fisheye.CALIB_RECOMPUTE_EXTRINSIC +
    cv2.fisheye.CALIB_CHECK_COND +
    cv2.fisheye.CALIB_FIX_SKEW
)

# Perform calibration
ret, K, D, rvecs, tvecs = cv2.fisheye.calibrate(
    objpoints,
    imgpoints,
    img_shape,
    K,
    D,
    rvecs,
    tvecs,
    calibration_flags,
    criteria
)

print(f"{'='*60}")
print(f"Calibration completed!")
print(f"RMS re-projection error: {ret:.4f} pixels\n")

# Display calibration results
print("Camera Matrix (K):")
print(K)
print("\nDistortion Coefficients (D):")
print(D.ravel())
print(f"\nFocal length: fx={K[0,0]:.2f}, fy={K[1,1]:.2f}")
print(f"Principal point: cx={K[0,2]:.2f}, cy={K[1,2]:.2f}")

# Save calibration data
np.savez(
    OUTPUT_FILE,
    camera_matrix=K,
    dist_coeffs=D,
    rvecs=rvecs,
    tvecs=tvecs,
    image_shape=img_shape,
    rms_error=ret,
    successful_images=successful_images
)

print(f"\nCalibration data saved to: {OUTPUT_FILE}")

# Create an example of undistortion
if successful_images:
    print("\nCreating undistorted example image...")
    example_img = cv2.imread(successful_images[0])
    h, w = example_img.shape[:2]

    # Calculate new camera matrix for undistortion
    new_K = cv2.fisheye.estimateNewCameraMatrixForUndistortRectify(
        K, D, (w, h), np.eye(3), balance=0.0
    )

    # Create undistortion maps
    map1, map2 = cv2.fisheye.initUndistortRectifyMap(
        K, D, np.eye(3), new_K, (w, h), cv2.CV_16SC2
    )

    # Apply undistortion
    undistorted_img = cv2.remap(
        example_img, map1, map2,
        interpolation=cv2.INTER_LINEAR,
        borderMode=cv2.BORDER_CONSTANT
    )

    # Save comparison
    comparison = np.hstack((example_img, undistorted_img))
    cv2.imwrite("undistortion_comparison.jpg", comparison)
    print("Saved undistortion comparison to: undistortion_comparison.jpg")

print("\nCalibration complete!")
