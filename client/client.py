import socket
import struct
import time

def send_data(client, data):
    cmd = 1
    data_len = len(data)
    data = struct.pack(f'8sII{data_len}s', b'work', cmd, data_len, data.encode('utf-8'))
    client.send(data)  # send data in tcp
    print("send data len:")
    print(len(data))
    info = client.recv(1024).decode()
    print(info)

if __name__ == '__main__':
    data=''
    for i in range(10):
        data=data+'aaaaaaaaaaaaaaaaaaaa高万禄aaaaaaaaaaaaaaaabhdsbjchbfvjbdfjvbfjdhvbjhfbh'
    index = 0 

    #host = '61.171.51.135'
    host = '172.29.94.203'
    port = 20023
    start_time = time.time()
    m_list = [];
    
    client = socket.socket()  # tcp socket
    client.connect((host, port))  # init connection
    m_list.append(client)
    print('connected')
    while True:
        send_data(client, data)
        print(index)
        index+=1
    client.close()
    end_time=time.time()
    print('time count: ')
    print((end_time-start_time))
