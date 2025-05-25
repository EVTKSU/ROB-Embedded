import depthai as dai
import cv2
import numpy as np

# Function to detect faces and display distance using OpenCV
def detect_faces_and_display_distance(frame, depth_frame, face_cascade):
    gray_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    faces = face_cascade.detectMultiScale(gray_frame, scaleFactor=1.1, minNeighbors=5, minSize=(30, 30))
    for (x, y, w, h) in faces:
        cv2.rectangle(frame, (x, y), (x+w, y+h), (255, 0, 0), 2)
        # Get the center point of the face
        center_x, center_y = x + w // 2, y + h // 2
        # Check if the center point is within the depth frame bounds
        if 0 <= center_x < depth_frame.shape[1] and 0 <= center_y < depth_frame.shape[0]:
            # Get the distance from the depth frame
            distance_mm = depth_frame[center_y, center_x]
            distance_ft = distance_mm / 304.8  # Convert mm to feet
            cv2.putText(frame, f"{distance_ft:.2f} ft", (x, y-10), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 0, 0), 2)
    return frame

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

        # Detect faces and display distance
        frame_rgb = detect_faces_and_display_distance(frame_rgb, frame_depth, face_cascade)

        # Normalize depth map for display
        max_depth = np.max(frame_depth)
        if max_depth > 0:
            frame_depth_normalized = (frame_depth * (255 / max_depth)).astype(np.uint8)
        else:
            frame_depth_normalized = np.zeros_like(frame_depth, dtype=np.uint8)
        frame_depth_colorized = cv2.applyColorMap(frame_depth_normalized, cv2.COLORMAP_JET)

        # Resize the frames to 1/4 size
        frame_rgb_resized = cv2.resize(frame_rgb, (frame_rgb.shape[1] // 2, frame_rgb.shape[0] // 2))
        frame_depth_resized = cv2.resize(frame_depth_colorized, (frame_depth_colorized.shape[1] // 2, frame_depth_colorized.shape[0] // 2))

        # Show the frames
        cv2.imshow("RGB", frame_rgb_resized)
        cv2.imshow("Depth", frame_depth_resized)

        if cv2.waitKey(1) == ord('q'):
            break

cv2.destroyAllWindows()