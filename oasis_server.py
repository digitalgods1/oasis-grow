# TCP Chat server which listens for incoming connections from chat clients
# uses port 8000
#import shell modules
import os
import os.path
import sys

#set proper path for modules
sys.path.append('/home/pi/grow-ctrl')
sys.path.append('/usr/lib/python37.zip')
sys.path.append('/usr/lib/python3.7')
sys.path.append('/usr/lib/python3.7/lib-dynload')
sys.path.append('/home/pi/.local/lib/python3.7/site-packages')
sys.path.append('/usr/local/lib/python3.7/dist-packages')
sys.path.append('/usr/lib/python3/dist-packages')


import socket, select
from _thread import *
import json
import pickle5 as pickle
import serial

#open serial port
ser_server = serial.Serial('/dev/ttyUSB0', 9600, timeout=1)
ser_server.flush()

#keep list of all sockets
CONNECTION_LIST = []
RECV_BUFFER = 4096 #fairly arbitrary buffer size, specifies maximum data to be recieved at once
PORT = 8000

server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind(("0.0.0.0", PORT))
server_socket.listen(10)
CONNECTION_LIST.append(server_socket)

#update wpa_supplicant.conf
def modWiFiConfig(SSID, password):
    config_lines = [
    'ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev',
    'update_config=1',
    'country=US',
    '\n',
    'network={',
    '\tssid="{}"'.format(SSID),
    '\tpsk="{}"'.format(password),
    '\tkey_mgmt=WPA-PSK',
    '}'
    ]

    config = '\n'.join(config_lines)
    #print(config)

    with open("/etc/wpa_supplicant/wpa_supplicant.conf", "r+") as w:
        w.seek(0)
        w.write(config)
        w.truncate()
    w.close()
    print("WiFi configs added")

#update access_config.json
def modAccessConfig(wak, local_id, id_token):
    access_config = {}
    access_config["wak"] = str(wak)
    access_config["local_id"] = str(local_id)
    access_config["id_token"] = str(id_token)

    with open("/home/pi/access_config.json", "r+") as a:
        json.dump(access_config, a)
    a.close()
    print("Access configs added")

modWiFiConfig(" "," ")
modAccessConfig(" "," "," ")

#update LED state
with open('/home/pi/device_state.json','r+') as d:
    device_state = json.load(d)
d.close()


##https://stackoverflow.com/questions/166506/finding-local-ip-addresses-using-pythons-stdlib
print("Oasis server started on Port: " + str(PORT)+' on IP: '+socket.gethostbyname(socket.gethostname()))

while True:
    read_sockets, write_sockets, error_sockets = select.select(CONNECTION_LIST, [], [])

    #update LED serial
    ser_server.write(bytes(str(device_state["LEDstatus"])+"\n", 'utf-8'))

    for sock in read_sockets:
        #new connection
        if sock == server_socket:
            sockfd, addr = server_socket.accept()
            CONNECTION_LIST.append(sockfd)
            print("Client (%s, %s) connected" % addr)

        #incoming message from client
        else:
            try:
                data = sock.recv(RECV_BUFFER)
                data = pickle.loads(data)
                print(data)
                print(type(data))
                #print('received data from [%s:%s]: ' % addr + data)
                ##THIS IS WHERE YOU NEED TO VERIFY THAT THE INFORMATION IS RIGHT
                modWiFiConfig(str(data['wifi_name']), str(data['wifi_pass']))
                print("Wifi Added")
                modAccessConfig(str(data['wak']), str(data['local_id']), str(data['id_token']))
                print('Access Added')
                sock.send('connected'.encode())
                sock.close()
                CONNECTION_LIST.remove(sock)
                os.system("sudo cp /etc/dhcpcd_WiFi.conf /etc/dhcpcd.conf")
                os.system("sudo cp /etc/dnsmasq_WiFi.conf /etc/dnsmasq.conf")
                os.system("sudo systemctl disable hostapd")
                #set AccessPoint state to "0" before rebooting
                with open('/home/pi/device_state.json', 'r+') as d:
                    device_state = json.load(d)
                    device_state['AccessPoint'] = "0" # <--- add `id` value.
                    d.seek(0) # <--- should reset file position to the beginning.
                    json.dump(device_state, d)
                    d.truncate() # remove remaining part
                d.close()
                #exit
                os.system("sudo systemctl reboot")

            #disconnect
            except:
                sock.close()
                CONNECTION_LIST.remove(sock)

server_socket.close()

