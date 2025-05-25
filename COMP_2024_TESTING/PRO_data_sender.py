import socket
import time
import depthai as dai
import cv2
import numpy as np

# Configure socket
HOST = '10.0.0.198'  # Replace with the server's IP address
PORT = 65432

# Function to detect faces and send coordinates
def detect_faces_and_send(frame, face_cascade):
    gray_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    faces = face_cascade.detectMultiScale(gray_frame, scaleFactor=1.1, minNeighbors=5, minSize=(30, 30))
    if len(faces) == 0:
        send_coordinates("no face detected")
    else:
        for (x, y, w, h) in faces:
            # Draw a rectangle around the face
            cv2.rectangle(frame, (x, y), (x + w, y + h), (255, 0, 0), 2)
            # Get the center point of the face
            center_x, center_y = x + w // 2, y + h // 2
            send_coordinates(f"{center_x},{center_y}")
            break  # Only consider the first detected face

def send_coordinates(message):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, PORT))
        s.sendall(message.encode())

def main():
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
        
        while True:
            in_rgb = q_rgb.get()

            # Retrieve 'bgr' frame
            frame_rgb = in_rgb.getCvFrame()

            # Detect faces and send coordinates
            detect_faces_and_send(frame_rgb, face_cascade)

            # Show the frame with the face box
            cv2.imshow("RGB", frame_rgb)
            if cv2.waitKey(1) == ord('q'):
                break
            
            time.sleep(0.5)  # Send coordinates every half second

    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
