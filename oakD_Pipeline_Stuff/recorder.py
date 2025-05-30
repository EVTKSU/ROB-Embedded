#!/usr/bin/env python3
"""
recorder.py

Record MP4 video from OAK-D LR by piping H.264 bitstream into FFmpeg.
Requires:
  • Python 3.7+
  • depthai (`pip install depthai`)
  • FFmpeg installed and on PATH
"""

import argparse
import subprocess
import sys
import depthai as dai
import signal

def build_pipeline(width, height, fps):
    pipeline = dai.Pipeline()
    cam      = pipeline.create(dai.node.ColorCamera)
    enc      = pipeline.create(dai.node.VideoEncoder)
    xout     = pipeline.create(dai.node.XLinkOut)

    cam.setBoardSocket(dai.CameraBoardSocket.CAM_A)
    cam.setResolution(dai.ColorCameraProperties.SensorResolution.THE_1080_P)
    cam.setVideoSize(width, height)
    cam.setFps(fps)

    enc.setDefaultProfilePreset(fps, dai.VideoEncoderProperties.Profile.H264_HIGH)
    cam.video.link(enc.input)
    enc.bitstream.link(xout.input)
    xout.setStreamName("h264")
    return pipeline

def start_ffmpeg(out_file, width, height, fps):
    """Spawn FFmpeg to read raw H264 from stdin and wrap into a fragmented MP4."""
    cmd = [
        'ffmpeg',
        '-y',                    # overwrite output
        '-f', 'h264',            # input format
        '-framerate', str(fps),  # input framerate for raw stream
        '-i', 'pipe:0',          # read from stdin
        '-c', 'copy',            # no re-encoding
        '-movflags',             # initialize moov atom up front & fragment on keyframes
          '+frag_keyframe+empty_moov+faststart',
        out_file
    ]
    return subprocess.Popen(cmd, stdin=subprocess.PIPE)

def main():
    parser = argparse.ArgumentParser(description="Record MP4 from OAK-D LR")
    parser.add_argument('--output', '-o', default='recorded.mp4',
                        help='Output MP4 filename')
    parser.add_argument('--width',  type=int, default=1920,
                        help='Capture width')
    parser.add_argument('--height', type=int, default=1080,
                        help='Capture height')
    parser.add_argument('--fps',    type=int, default=30,
                        help='Frames per second')
    args = parser.parse_args()

    # Build DepthAI pipeline
    pipeline = build_pipeline(args.width, args.height, args.fps)

    # Start FFmpeg subprocess with fragmentation flags
    ffmpeg = start_ffmpeg(args.output, args.width, args.height, args.fps)

    # Graceful shutdown on Ctrl+C
    def signal_handler(sig, frame):
        print("\nStopping recording...")
        ffmpeg.stdin.close()
        ffmpeg.wait()
        sys.exit(0)
    signal.signal(signal.SIGINT, signal_handler)

    # Begin streaming
    with dai.Device(pipeline) as device:
        q = device.getOutputQueue(name="h264", maxSize=30, blocking=True)
        print(f"Recording to {args.output} ({args.width}×{args.height}@{args.fps}fps). Press Ctrl+C to stop.")

        while True:
            packet = q.get()             # blocking until next H264 packet
            data   = packet.getData()
            ffmpeg.stdin.write(data)

if __name__ == '__main__':
    main()
