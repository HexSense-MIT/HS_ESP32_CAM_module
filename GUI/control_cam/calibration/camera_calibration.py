"""Camera matrix:
 [[549.91254035   0.         802.57313647]
 [  0.         551.38505012 607.02397446]
 [  0.           0.           1.        ]]
Distortion coefficients:
 [[ 0.19067352 -0.20602461  0.00023991 -0.00069994  0.04950833]]
"""
import cv2
import numpy as np

mtx = np.array([[468.59748972, 0, 659.10020194],
                [0, 469.30862959, 497.8638885],
                [0, 0, 1]], dtype=np.float32)
dist = np.array([0.2089946, -0.22271615, -0.00487632, 0.00075366, 0.05415361], dtype=np.float32)

img = cv2.imread('./cali_code/HFoV_109_4_degree.jpg')
h, w = img.shape[:2]
new_mtx, roi = cv2.getOptimalNewCameraMatrix(mtx, dist, (w,h), 1, (w,h))

# Undistort the image
dst = cv2.undistort(img, mtx, dist, None, new_mtx)

# Optionally crop the image
x, y, w, h = roi
dst = dst[y:y+h, x:x+w]
cv2.imwrite('calibrated_image/5_normalized_image.jpg', dst)