import socket
import threading
import time
from datetime import datetime

HOST = '0.0.0.0'  # Bind to all available interfaces
PORT = 65432

status = ""
recent_data = []

def handle_client(conn, addr):
    global status
    print('Connected by', addr)
    while True:
        data = conn.recv(1024)
        if not data:
            break
        message = data.decode()
        if message == "no face detected":
            status = "No face detected"
        else:
            x, y = map(int, message.split(','))
            status = determine_status(y)
        recent_data.append((datetime.now(), message, status))
        print(f"Received coordinates: {message}, status={status} ")

def determine_status(distance_mm):
    if distance_mm > 400:
        return "Turning left " + str(distance_mm) +"mm"
    else:
        return "Turning right " + str(distance_mm) +"mm"

def log_data():
    while True:
        time.sleep(5)
        if recent_data:
            with open("turtle_log.txt", "a") as file:
                for data in recent_data:
                    timestamp, coordinates, status = data
                    file.write(f"{timestamp} - coordinates: {coordinates}, status: {status}\n")
            recent_data.clear()

def server_thread():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, PORT))
        s.listen()
        print('Waiting for connection...')
        while True:
            conn, addr = s.accept()
            threading.Thread(target=handle_client, args=(conn, addr)).start()

def display_status():
    global status
    while True:
        print(f"Current status: {status}")
        time.sleep(0.1)

if __name__ == "__main__":
    # Start server thread
    threading.Thread(target=server_thread, daemon=True).start()
    
    # Start logging thread
    threading.Thread(target=log_data, daemon=True).start()
    
    # Start display status thread
    display_status()
