import socket
import turtle
import threading

# Configure socket
HOST = '0.0.0.0'  # Bind to all available interfaces
PORT = 65432

def handle_client(conn, addr):
    print('Connected by', addr)
    while True:
        data = conn.recv(1024)
        if not data:
            break
        control_turtle(data.decode())

def control_turtle(command):
    if command == 'w':
        t.forward(20)
    elif command == 's':
        t.backward(20)
    elif command == 'a':
        t.left(45)
    elif command == 'd':
        t.right(45)

def server_thread():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, PORT))
        s.listen()
        print('Waiting for connection...')
        while True:
            conn, addr = s.accept()
            threading.Thread(target=handle_client, args=(conn, addr)).start()

if __name__ == "__main__":
    # Set up turtle
    screen = turtle.Screen()
    t = turtle.Turtle()
    
    # Start server thread
    threading.Thread(target=server_thread, daemon=True).start()
    
    # Keep turtle window open
    turtle.mainloop()
