import time  # Standard library for time-related functions
import depthai as dai  # Library for DepthAI camera control
import cv2  # OpenCV library for computer vision tasks
import numpy as np  # NumPy library for numerical operations
from datetime import datetime  # Standard library for datetime operations
import threading  # Standard library for multi-threading

def detect_green_and_process(frame, depth_frame):
    # Convert the frame from BGR (Blue, Green, Red) to HSV (Hue, Saturation, Value) color space using OpenCV
    hsv_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    # Define the lower and upper bounds for the green color in HSV space
    lower_green = np.array([40, 40, 40])
    upper_green = np.array([80, 255, 255])

    # Create a mask that captures only the green areas within the specified bounds
    mask = cv2.inRange(hsv_frame, lower_green, upper_green)

    # Find contours in the mask using OpenCV
    contours, _ = cv2.findContours(mask, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)

    # If no contours are found, report "no green detected"
    if len(contours) == 0:
        process_data("no green detected")
    else:
        # Find the largest contour based on the area
        largest_contour = max(contours, key=cv2.contourArea)
        (x, y, w, h) = cv2.boundingRect(largest_contour)  # Get bounding box for the largest contour

        # Draw a rectangle around the largest green area
        cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)

        # Calculate the center of the bounding box
        center_x, center_y = x + w // 2, y + h // 2

        # Get the distance from the depth frame at the center of the bounding box
        distance = depth_frame[center_y, center_x]

        # Map the x-coordinate to the range 0 to 180
        frame_width = frame.shape[1]
        mapped_x = int((center_x / frame_width) * 180)

        # Process the data with the detected green object's coordinates and distance
        process_data(f"{center_x},{center_y},{distance},{mapped_x}")

def process_data(message):
    global status, recent_data  # Access global variables

    # Check if no green object is detected
    if message == "no green detected":
        status = "No green detected"
    else:
        # Extract coordinates, distance, and mapped_x from the message
        x, y, distance, mapped_x = map(int, message.split(','))
        
        # Update the status message with the distance information
        status = determine_status(distance)
        
        # Rotate the arrow based on the distance (or y-coordinate in this context)
        rotate_arrow(distance)
    
    # Append the new data entry with a timestamp to the recent_data list
    recent_data.append((datetime.now(), message, status))

def determine_status(distance_mm):
    # Return a formatted string indicating the distance to the nearest green object
    return f"Nearest Green is {distance_mm} mm away from the camera!"

def rotate_arrow(y_coordinate):
    # Load an arrow image using OpenCV
    arrow = cv2.imread('arrow.png', cv2.IMREAD_UNCHANGED)

    # Resize the arrow image for display
    arrow_resized = cv2.resize(arrow, (arrow.shape[1] // 32, arrow.shape[0] // 32), interpolation=cv2.INTER_AREA)
    
    # Rotate the arrow based on the y-coordinate (distance) thresholds
    if y_coordinate < 200:
        # Rotate 90 degrees counterclockwise if distance is less than 200mm
        rotated_arrow = cv2.rotate(arrow_resized, cv2.ROTATE_90_COUNTERCLOCKWISE)
    elif y_coordinate > 400:
        # Rotate 90 degrees clockwise if distance is greater than 400mm
        rotated_arrow = cv2.rotate(arrow_resized, cv2.ROTATE_90_CLOCKWISE)
    else:
        # Point straight up if distance is between 200mm and 400mm
        rotated_arrow = arrow_resized  # No rotation needed for pointing straight up
    
    # Display the rotated arrow image using OpenCV
    cv2.imshow("Rotated Arrow", rotated_arrow)
    cv2.waitKey(1)  # Display the image for 1 ms

def display_status():
    global status  # Access the global status variable

    # Continuously print the current status every 0.1 seconds
    while True:
        print(f"Current status: {status}")
        time.sleep(0.1)

def main():
    global status, recent_data  # Define global variables for status and recent data
    status = ""  # Initialize status
    recent_data = []  # Initialize recent data list

    pipeline = dai.Pipeline()  # Create a DepthAI pipeline

    # Configure the RGB camera node
    cam_rgb = pipeline.create(dai.node.ColorCamera)
    cam_rgb.setBoardSocket(dai.CameraBoardSocket.CAM_A)  # Use the CAM_A socket
    cam_rgb.setResolution(dai.ColorCameraProperties.SensorResolution.THE_800_P)  # Set resolution to 800P
    cam_rgb.setFps(30)  # Set frames per second to 30

    # Configure the left mono camera for the stereo setup
    mono_left = pipeline.create(dai.node.MonoCamera)
    mono_left.setBoardSocket(dai.CameraBoardSocket.LEFT)  # Use the LEFT socket
    mono_left.setResolution(dai.MonoCameraProperties.SensorResolution.THE_400_P)  # Set resolution to 400P

    # Configure the right mono camera for the stereo setup
    mono_right = pipeline.create(dai.node.MonoCamera)
    mono_right.setBoardSocket(dai.CameraBoardSocket.RIGHT)  # Use the RIGHT socket
    mono_right.setResolution(dai.MonoCameraProperties.SensorResolution.THE_400_P)  # Set resolution to 400P

    # Configure the stereo depth node
    stereo = pipeline.create(dai.node.StereoDepth)
    stereo.setDefaultProfilePreset(dai.node.StereoDepth.PresetMode.HIGH_DENSITY)  # Set high-density preset for stereo depth
    stereo.setDepthAlign(dai.CameraBoardSocket.CAM_A)  # Align depth map to the RGB camera
    stereo.initialConfig.setConfidenceThreshold(200)  # Set confidence threshold for depth calculations

    # Link the mono camera outputs to the stereo depth node inputs
    mono_left.out.link(stereo.left)
    mono_right.out.link(stereo.right)

    # Create output nodes for RGB and depth data
    xout_rgb = pipeline.create(dai.node.XLinkOut)
    xout_rgb.setStreamName("rgb")  # Name the RGB output stream
    cam_rgb.video.link(xout_rgb.input)  # Link RGB video output to the XLinkOut node

    xout_depth = pipeline.create(dai.node.XLinkOut)
    xout_depth.setStreamName("depth")  # Name the depth output stream
    stereo.depth.link(xout_depth.input)  # Link depth output to the XLinkOut node

    # Start the device with the pipeline
    with dai.Device(pipeline) as device:
        # Get output queues for RGB and depth streams
        q_rgb = device.getOutputQueue(name="rgb", maxSize=4, blocking=False)
        q_depth = device.getOutputQueue(name="depth", maxSize=4, blocking=False)
        
        # Start a thread to continuously display the status
        threading.Thread(target=display_status, daemon=True).start()

        while True:
            # Retrieve the latest RGB frame
            in_rgb = q_rgb.get()
            frame_rgb = in_rgb.getCvFrame()  # Convert to OpenCV format

            # Retrieve the latest depth frame
            in_depth = q_depth.get()
            depth_frame = in_depth.getFrame()  # Get depth data

            # Process the frames to detect green objects and their distances
            detect_green_and_process(frame_rgb, depth_frame)

            # Display the RGB frame with any annotations
            cv2.imshow("RGB", frame_rgb)
            
            # Break the loop if 'q' key is pressed
            if cv2.waitKey(1) == ord('q'):
                break

    # Destroy all OpenCV windows
    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()  # Execute the main function
