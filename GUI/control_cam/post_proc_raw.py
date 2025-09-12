from cobs import cobs
import os
import math

chunk_size = 20

def parse_image(image_data):
    print("length of image data: ", len(image_data))
    # print(image_data)
    with open("output_image.jpg", "wb") as image_file:
        image_file.write(image_data)

def process_raw_image_data(file_path, output_file):
    try:
        with open(file_path, 'rb') as f:
            raw_data = f.read()

        frames_cnt = math.floor(len(raw_data)/(chunk_size + 3)) + 1
        raw_frames = []

        for i in range(frames_cnt):
            start = i * (chunk_size + 3)
            end = start + (chunk_size + 3)
            raw_frames.append(raw_data[start:end])  # Append each frame to the list

        image_data = bytearray()

        for raw_frame in raw_frames:
            try:
                head = raw_frame[0]
                camera_number = raw_frame[1]
                sequence_number = raw_frame[2]
                # print(f"Head: {head}, Camera: {camera_number}, Sequence: {sequence_number}, Frame Length: {len(raw_frame)}")
                frame_data = raw_frame[3:]  # Assuming the rest is image data
                image_data.extend(frame_data)
            except Exception as e:
                print(f"Error decoding frame: {e}", " | len(raw_frame): ", len(raw_frame), "raw_frame: ", raw_frame)

        for byte in image_data:
            print(f"{byte:02X}", end='')

        parse_image(image_data)

    except Exception as e:
        print(f"Error processing file: {e}")

if __name__ == "__main__":
    input_file = "raw_image_data.bin"
    output_file = "processed_image_data.bin"

    if os.path.exists(input_file):
        process_raw_image_data(input_file, output_file)
    else:
        print(f"Input file {input_file} does not exist.")