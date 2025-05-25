import socket
import depthai as dai
import cv2
import numpy as np

# Configure socket
HOST = '10.0.0.198'  # Replace with the server's IP address
PORT = 65432

# Function to detect faces and translate movement into commands
def detect_faces_and_translate(frame, depth_frame, face_cascade):
    gray_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    faces = face_cascade.detectMultiScale(gray_frame, scaleFactor=1.1, minNeighbors=5, minSize=(30, 30))
    for (x, y, w, h) in faces:
        # Get the center point of the face
        center_x, center_y = x + w // 2, y + h // 2
        # Check if the center point is within the depth frame bounds
        if 0 <= center_x < depth_frame.shape[1] and 0 <= center_y < depth_frame.shape[0]:
            # Get the distance from the depth frame
            distance_mm = depth_frame[center_y, center_x]
            # Translate face movement into commands
            if center_x < frame.shape[1] // 3:
                send_command('a')
            elif center_x > 2 * frame.shape[1] // 3:
                send_command('d')
            if distance_mm > 1500:  # Move forward if far
                send_command('w')
            elif distance_mm < 1000:  # Move backward if close
                send_command('s')
            break

def send_command(command):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, PORT))
        s.sendall(command.encode())

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

    # Depth source
    mono_left = pipeline.create(dai.node.MonoCamera)
    mono_right = pipeline.create(dai.node.MonoCamera)
    stereo = pipeline.create(dai.node.StereoDepth)
    xout_depth = pipeline.create(dai.node.XLinkOut)

    mono_left.setBoardSocket(dai.CameraBoardSocket.CAM_B)
    mono_right.setBoardSocket(dai.CameraBoardSocket.CAM_C)

    stereo.setLeftRightCheck(True)
    stereo.setExtendedDisparity(True)

    mono_left.out.link(stereo.left)
    mono_right.out.link(stereo.right)
    stereo.depth.link(xout_depth.input)

    xout_depth.setStreamName("depth")

    # Connect to device and start pipeline
    with dai.Device(pipeline) as device:
        # Get output queues
        q_rgb = device.getOutputQueue(name="rgb", maxSize=4, blocking=False)
        q_depth = device.getOutputQueue(name="depth", maxSize=4, blocking=False)
        
        while True:
            in_rgb = q_rgb.get()
            in_depth = q_depth.get()

            # Retrieve 'bgr' frame
            frame_rgb = in_rgb.getCvFrame()

            # Retrieve depth map frame
            frame_depth = in_depth.getFrame()

            # Detect faces and translate movements
            detect_faces_and_translate(frame_rgb, frame_depth, face_cascade)

            # Show the frames for debugging purposes
            cv2.imshow("RGB", frame_rgb)
            if cv2.waitKey(1) == ord('q'):
                break

    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
