import machine
#from network import WLAN
from network import LAN
import network
import time
from mqtt.simple import MQTTClient
import ussl

net=network.LAN()
net.active(True)
net.ifconfig("dhcp")
net.ifconfig()

last_message = 0
message_interval = 5
counter = 0

DISCONNECTED = 0
CONNECTING = 1
CONNECTED = 2
MQTT_HOST = "xxxxxx.iot.ap-northeast-1.amazonaws.com"
TOPIC = "/demo/msg"
MQTT_CLIENT_ID = "gr_mango"
MQTT_PORT = 8883
state = DISCONNECTED

connection = None

def pub_msg(msg):
    global connection
    connection.publish(topic=TOPIC, msg=msg, qos=0)
    print('Sending: ' + msg)

def run():
    global state
    global connection
    global client
    
    KEY_PATH = "/xxxxxx-private.txt"
    CERT_PATH = "/xxxxxx-certificate.txt"
    with open(KEY_PATH, 'r') as f:
        key1 = f.read()
    #print(key1)
    #print(" ")
    with open(CERT_PATH, 'r') as f:
        cert1 = f.read()
    #print(cert1)
    #print(" ")
    
    client = MQTTClient(client_id=MQTT_CLIENT_ID, server=MQTT_HOST, port=MQTT_PORT, keepalive=10000, ssl=True, ssl_params={"key":key1, "cert":cert1, "server_side":False})
    client.set_callback(sub_cb)
    client.connect()
    print('MQTT LIVE!')
    client.subscribe(TOPIC)
    print('Connected to %s MQTT broker, subscribed to %s topic' % (MQTT_HOST , TOPIC))  
    
def sub_cb(topic, msg):
    print((topic, msg))
    if topic == b'notification' and msg == b'received':
        print('ESP received hello message')

while state == CONNECTED:
    msg = '{"device_id":"some_id", "data":"some_data"}'
    pub_msg(msg)
    time.sleep(2.0)

while True:
    if 1==1:
        #wlan.connect('Andreia Oi Miguel 2.4G', 'XXXXXXXXX')
        #while not wlan.isconnected():
        #      machine.idle()
        print('Connecting...')
        run()
        while True:
            try:
                client.check_msg()
                if (time.time() - last_message) > message_interval:
                    msg = b'Hello #%d' % counter
                    client.publish(TOPIC, msg)
                    last_message = time.time()
                    counter += 1
            except OSError as e:
                run()
        break