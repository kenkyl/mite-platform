import socket

UDP_IP="10.10.90.249"
UDP_PORT=5000

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

print ("Waiting for messages at ", UDP_IP, ":", UDP_PORT, "...")

while True:
    data, addr = sock.recvfrom(1024)
    print("received message from host: ", addr, ": ", data) 
