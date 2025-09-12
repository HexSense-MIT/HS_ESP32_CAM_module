import socket

host = "0.0.0.0"
port = 1234
s = socket.socket()
s.bind((host, port))
s.listen(1)
print(f"Listening on {host}:{port}...")

conn, addr = s.accept()
print(f"Connection from {addr}")
while True:
    data = conn.recv(4096)
    if not data:
        break
    print("Received:", data.decode().strip())
