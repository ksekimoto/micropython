import network
esp = network.WLAN()
# esp = network.ESP8266()
esp.connect("xxxxxx","xxxxxx")
esp.ifconfig()

import wsocket as socket
s = socket.socket()
addr = socket.getaddrinfo('www.google.com', 80)[0][-1]
s.connect(addr)
s.send(b"GET / HTTP/1.0\r\n\r\n")
data=s.recv(8192)
s.close()
print(data)