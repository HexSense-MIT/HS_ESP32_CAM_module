from PIL import Image

# Example byte string and image dimensions
data = b'...'  # Your grayscale byte string
width, height = 160, 120  # Replace with your image's width and height

# Create a grayscale ('L' mode) image from the byte string
image = Image.frombytes('L', (width, height), data)

# Display the image
image.show()

# Optionally, save the image to a file
image.save('grayscale_image.png')
