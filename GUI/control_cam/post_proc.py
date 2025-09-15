from cobs import cobs
import os

def parse_image(image_data):
    print("length of image data: ", len(image_data))
    # print(image_data)
    with open("output_image.jpg", "wb") as image_file:
        image_file.write(image_data)

def process_raw_image_data(file_path, output_file):
    try:
        with open(file_path, 'rb') as f:
            raw_data = f.read()

        # Split raw data into frames using 0x00 as the delimiter
        raw_frames = raw_data.split(b'\x00')
        frames = []
        image_data = bytearray()

        for raw_frame in raw_frames:
            if len(raw_frame) < 103:  # Combine with the next frame
                if raw_frames.index(raw_frame) < len(raw_frames) - 1:  # Ensure it's not the last frame
                    next_index = raw_frames.index(raw_frame) + 1
                    raw_frames[next_index] = raw_frame + raw_frames[next_index]
                continue

            else:  # Skip empty frames
                try:
                    decoded_frame = cobs.decode(raw_frame)
                    camera_number = decoded_frame[1]
                    sequence_number = decoded_frame[2]
                    print(f"Camera: {camera_number}, Sequence: {sequence_number}, Frame Length: {len(decoded_frame)}")
                    frame_data = decoded_frame[3:]  # Assuming the rest is image data
                    frames.append({
                        "camera_number": camera_number,
                        "sequence_number": sequence_number,
                        "frame_data": frame_data
                    })
                    image_data.extend(frame_data)
                except Exception as e:
                    print(f"Error decoding frame: {e}", " | len(raw_frame): ", len(raw_frame), "raw_frame: ", raw_frame)

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