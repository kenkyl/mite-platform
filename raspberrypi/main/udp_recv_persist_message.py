import sys
sys.path.append('../..')

from modules import messages

import socket
import redis
#import modules.messages

UDP_IP = '10.10.90.249'
UDP_PORT = 5000
REDIS_HOST = 'localhost'
REDIS_PORT = 6379
REDIS_DB = 0

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

strictRedis = redis.StrictRedis(host=REDIS_HOST, port=REDIS_PORT, db=REDIS_DB)

print ('Waiting for messages at ', UDP_IP, ':', UDP_PORT, '...')

while True:
    data, addr = sock.recvfrom(1024)
    print('received message from host: ', addr, ': ', data)
    sm = messages.SensorMessage(data)
    if (sm.valid == True):
        print('id: ', sm.id, 'val: ', sm.value)
        strictRedis.set(sm.id, sm.value)
    else:
        print('Invalid message received :', data) 
    
    
