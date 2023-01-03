import network
net=network.LAN()
net.active(True)
net.ifconfig("dhcp")
# net.ifconfig(('192.168.11.58', '255.255.255.0', '192.168.11.1', '192.168.11.1'))
net.ifconfig()

import socket as socket
s = socket.socket()
addr = socket.getaddrinfo('www.google.com', 80)[0][-1]
s.connect(addr)
s.send(b"GET / HTTP/1.0\r\n\r\n")
data=s.recv(8192)
s.close()
print(data)