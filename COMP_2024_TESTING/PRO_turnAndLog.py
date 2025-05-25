import time
import depthai as dai
import cv2
import numpy as np
from datetime import datetime
import threading

# Function to detect faces, send coordinates, and determine status
def detect_faces_and_process(frame, face_cascade):
    gray_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    faces = face_cascade.detectMultiScale(gray_frame, scaleFactor=1.1, minNeighbors=5, minSize=(30, 30))
    if len(faces) == 0:
        process_data("no face detected")
    else:
        for (x, y, w, h) in faces:
            # Draw a rectangle around the face
            cv2.rectangle(frame, (x, y), (x + w, y + h), (255, 0, 0), 2)
            # Get the center point of the face
            center_x, center_y = x + w // 2, y + h // 2
            process_data(f"{center_x},{center_y}")
            break  # Only consider the first detected face

def process_data(message):
    global status, recent_data
    if message == "no face detected":
        status = "No face detected"
    else:
        x, y = map(int, message.split(','))
        status = determine_status(y)
    recent_data.append((datetime.now(), message, status))
    print(f"Received coordinates: {message}, status={status}")

def determine_status(distance_mm):
    if distance_mm > 400:
        return "Turning left " + str(distance_mm) +"mm"
    else:
        return "Turning right " + str(distance_mm) +"mm"

def log_data():
    global recent_data
    while True:
        time.sleep(5)
        if recent_data:
            with open("turtle_log.txt", "a") as file:
                for data in recent_data:
                    timestamp, coordinates, status = data
                    file.write(f"{timestamp} - coordinates: {coordinates}, status: {status}\n")
            recent_data.clear()

def display_status():
    global status
    while True:
        print(f"Current status: {status}")
        time.sleep(0.1)

def main():
    global status, recent_data
    status = ""
    recent_data = []

    # Load face detection model
    face_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + 'haarcascade_frontalface_default.xml')

    # Create pipeline
    pipeline = dai.Pipeline()

    # Define sources and outputs
    cam_rgb = pipeline.create(dai.node.ColorCamera)
    xout_rgb = pipeline.create(dai.node.XLinkOut)

    cam_rgb.setBoardSocket(dai.CameraBoardSocket.CAM_A)
    cam_rgb.setResolution(dai.ColorCameraProperties.SensorResolution.THE_800_P)
    xout_rgb.setStreamName("rgb")

    cam_rgb.video.link(xout_rgb.input)

    # Connect to device and start pipeline
    with dai.Device(pipeline) as device:
        # Get output queue
        q_rgb = device.getOutputQueue(name="rgb", maxSize=4, blocking=False)

        # Start logging and status display threads
        threading.Thread(target=log_data, daemon=True).start()
        threading.Thread(target=display_status, daemon=True).start()
        
        while True:
            in_rgb = q_rgb.get()

            # Retrieve 'bgr' frame
            frame_rgb = in_rgb.getCvFrame()

            # Detect faces and process data
            detect_faces_and_process(frame_rgb, face_cascade)

            # Show the frame with the face box
            cv2.imshow("RGB", frame_rgb)
            if cv2.waitKey(1) == ord('q'):
                break

    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
