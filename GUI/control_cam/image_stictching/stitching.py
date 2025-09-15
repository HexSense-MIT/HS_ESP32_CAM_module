# from imgstitch import stitch_images_and_save
#
# stitch_images_and_save("../images/", ["1.jpg", "2.jpg", "3.jpg", "4.jpg", "5.jpg", "6.jpg"], 1,"./output/")

# # # Alternative stitching method using OpenCV
import cv2

image_paths = [
    "../images/2.jpg",
    "../images/3.jpg",
    "../images/4.jpg"
    # "../images/4.jpg",
    # "../images/5.jpg",
    # "./stitched234.jpg"
    # "../images/5.jpg",
    # "../images/6.jpg"
    # "../calibration/calibrated_image/1_normalized_image.jpg",
    # "../calibration/calibrated_image/2_normalized_image.jpg",
    # "../calibration/calibrated_image/3_normalized_image.jpg",
    # "../calibration/calibrated_image/4_normalized_image.jpg",
    # "../calibration/calibrated_image/5_normalized_image.jpg",
    # "../calibration/calibrated_image/6_normalized_image.jpg"
]

imgs = [cv2.imread(p) for p in image_paths if cv2.imread(p) is not None]
stitcher = cv2.Stitcher_create()
status, result = stitcher.stitch(imgs)

if status == cv2.Stitcher_OK:
    cv2.imwrite("stitched.jpg", result)
else:
    print("❌ Stitching failed with status code:", status)