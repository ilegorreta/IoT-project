import paho.mqtt.client as mqtt
import time

# Define event callbacks 
def on_connect(client, userdata, flags, rc):     
    print("rc: " + str(rc))     
    if rc == 0:
        client.subscribe("/esp/test", 0)    

def on_message(client, obj, msg):     
    print(msg.topic + " " + str(msg.qos) + " " + str(msg.payload))  
    if msg.topic == "/esp/test":
        print(str(msg.payload))

def on_publish(client, obj, mid):     
    #print("mid: " + str(mid))   
    print(".")

def on_subscribe(client, obj, mid, granted_qos):     
    print("Subscribed: " + str(mid) + " " + str(granted_qos))   

def on_log(client, obj, level, string):     
    print(string)

client = mqtt.Client() 
# Assign event callbacks 
client.on_message = on_message 
client.on_connect = on_connect 
client.on_publish = on_publish 
client.on_subscribe = on_subscribe

#Credentials
hostname = "tailor.cloudmqtt.com"
port = 17344
username = "xorapbza"
password = "VOTE84FaFvST"

#Start connection 
client.connect(hostname, port, 60) 
client.username_pw_set(username, password) 
client.loop_start()

run = True

while run:
    client.publish("/Casa/Sala", "Prender Led")
    time.sleep(15)