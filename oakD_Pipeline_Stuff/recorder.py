#!/usr/bin/env python3
"""
recorder.py

Record MP4 video from OAK-D LR by piping H.264 bitstream into FFmpeg,
and display a live preview window.
Requires:
  • Python 3.7+
  • depthai (`pip install depthai`)
  • OpenCV (`pip install opencv-python`)
  • FFmpeg installed and on PATH
"""

import argparse
import subprocess
import sys
import signal
import cv2
import depthai as dai

def build_pipeline(width, height, fps):
    pipeline    = dai.Pipeline()
    cam         = pipeline.create(dai.node.ColorCamera)
    enc         = pipeline.create(dai.node.VideoEncoder)
    xout_h264   = pipeline.create(dai.node.XLinkOut)
    xout_preview= pipeline.create(dai.node.XLinkOut)

    # camera setup (unchanged)
    cam.setBoardSocket(dai.CameraBoardSocket.CAM_A)
    cam.setResolution(dai.ColorCameraProperties.SensorResolution.THE_1080_P)
    cam.setVideoSize(width, height)
    cam.setFps(fps)

    # encoder setup (unchanged)
    enc.setDefaultProfilePreset(fps, dai.VideoEncoderProperties.Profile.H264_HIGH)
    cam.video.link(enc.input)
    enc.bitstream.link(xout_h264.input)
    xout_h264.setStreamName("h264")

    # ——— ADDED: preview output for live display ———
    cam.preview.link(xout_preview.input)
    xout_preview.setStreamName("rgb")

    return pipeline

def start_ffmpeg(out_file, fps):
    """Spawn FFmpeg to read raw H264 from stdin and wrap into a fragmented MP4."""
    cmd = [
        'ffmpeg',
        '-y',
        '-f', 'h264',
        '-framerate', str(fps),
        '-i', 'pipe:0',
        '-c', 'copy',
        '-movflags', '+frag_keyframe+empty_moov+faststart',
        out_file
    ]
    return subprocess.Popen(cmd, stdin=subprocess.PIPE)

def main():
    parser = argparse.ArgumentParser(description="Record & preview MP4 from OAK-D LR")
    parser.add_argument('-o','--output', default='recorded.mp4',
                        help='Output MP4 filename')
    parser.add_argument('--width',  type=int, default=1920,
                        help='Capture width')
    parser.add_argument('--height', type=int, default=1080,
                        help='Capture height')
    parser.add_argument('--fps',    type=int, default=30,
                        help='Frames per second')
    args = parser.parse_args()

    pipeline = build_pipeline(args.width, args.height, args.fps)
    ffmpeg   = start_ffmpeg(args.output, args.fps)

    # Graceful shutdown
    def signal_handler(sig, frame):
        print("\nStopping recording...")
        ffmpeg.stdin.close()
        ffmpeg.wait()
        cv2.destroyAllWindows()
        sys.exit(0)
    signal.signal(signal.SIGINT, signal_handler)

    with dai.Device(pipeline) as device:
        q_h264  = device.getOutputQueue(name="h264", maxSize=30, blocking=True)
        q_rgb   = device.getOutputQueue(name="rgb",  maxSize=30, blocking=False)
        print(f"Recording to {args.output} ({args.width}×{args.height}@{args.fps}fps).")
        print("Preview window: press 'q' to stop, or Ctrl+C to quit.")

        while True:
            # 1) Write H264 to FFmpeg
            packet = q_h264.get()
            ffmpeg.stdin.write(packet.getData())

            # 2) Grab preview frames (non-blocking) and display
            frame = q_rgb.tryGet()
            if frame:
                img = frame.getCvFrame()
                cv2.imshow("OAK‑D LR Preview", img)
                # stop if user hits 'q'
                if cv2.waitKey(1) & 0xFF == ord('q'):
                    signal_handler(None, None)

if __name__ == '__main__':
    main()
