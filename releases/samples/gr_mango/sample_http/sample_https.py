import network
net=network.LAN()
net.active(True)
net.ifconfig("dhcp")
# net.ifconfig(('192.168.11.58', '255.255.255.0', '192.168.11.1', '192.168.11.1'))
net.ifconfig()

import socket as socket
import ussl
s = socket.socket()
addr = socket.getaddrinfo("www.google.com", 443)[0][-1]
s.connect(addr)
ss = ussl.wrap_socket(s)
ss.write(b"GET / HTTP/1.0\r\n\r\n")
data=ss.read(4096)
s.close()
print(data)