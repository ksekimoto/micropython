import network
esp = network.WLAN()
esp.connect("xxxxxx","xxxxxx")
# esp.ifconfig(('192.168.11.21', '192.168.11.1', '255.255.255.0', '8.8.8.8'))
esp.ifconfig()

import wsocket as socket
# import socket
s = socket.socket()
addr = socket.getaddrinfo('www.google.com', 80)[0][-1]
s.connect(addr)
s.send(b"GET / HTTP/1.0\r\n\r\n")
data=s.recv(8192)
s.close()
print(data)