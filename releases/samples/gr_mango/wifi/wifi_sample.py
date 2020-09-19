#>>> esp = network.ESP8266()
#AT ver=1.4.0.0(May  5 2017 16:10:59)
#SDK ver=2.1.0(116b762)

import network
from pyb import Pin
esp = network.ESP8266(reset=Pin.cpu.P40)
esp.connect("xxxxxx", "xxxxxx")
esp.ifconfig()

import wsocket as socket
s = socket.socket()
addr = socket.getaddrinfo('www.google.com', 80)[0][-1]
s.connect(addr)
s.send(b"GET / HTTP/1.0\r\n\r\n")
data=s.recv(8192)
s.close()
print(data)

import wsocket as socket
import ussl
s = socket.socket()
addr = socket.getaddrinfo("www.yahoo.com", 443)[0][-1]
s.connect(addr)
ss = ussl.wrap_socket(s)
ss.write(b"GET / HTTP/1.0\r\n\r\n")
data=ss.read(4096)
s.close()
print(data)
