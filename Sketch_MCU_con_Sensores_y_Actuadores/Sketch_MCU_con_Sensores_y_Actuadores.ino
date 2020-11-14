#include <ESP8266WiFi.h>
#include <PubSubClient.h>

int ventilador = 14;
int estado = 0;
char rx[13];
String instruccionRecibida = "";
int sizeRx;
double temp;
char temperatura[5];
const int analogInPin = A0;

const char* ssid = "iPhone de Luis";
const char* password =  "luisluis";

const char* mqttServer = "tailor.cloudmqtt.com";
const int mqttPort =  17344;
const char* mqttUser = "xorapbza";
const char* mqttPassword = "VOTE84FaFvST";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  delay(10);
  pinMode(ventilador, OUTPUT);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED){
    delay(200);
    Serial.print('.');
  }
  Serial.println("Connected to the WiFi network");

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect("ESP8266Client", mqttUser, mqttPassword )) {
      Serial.println("connected");  
    }else{
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
  client.subscribe("/Casa/MiCasa");
}

void callback(char* topic, byte* payload, unsigned int length) {
 
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
 
  Serial.print("Message: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    rx[i] = (char)payload[i];
  }
  sizeRx = sizeof(rx) / sizeof(char);
  instruccionRecibida = convertToString(rx, sizeRx);
  Serial.println();
  Serial.println("-----------------------");
  Serial.println("Recibi::::::::::::::::::::: ");
  Serial.print(instruccionRecibida);
 
}

void loop() {
  client.loop();
  
  if(estado == 1){
    digitalWrite(ventilador, HIGH);  
  }else{
    digitalWrite(ventilador, LOW);
  }
  
  temp = analogRead(analogInPin);  
  delay(500);
  temp = (5.0 * (temp-32) * 100.0)/1024.0 - 2;
  //temp = (5.0 * temp * 100.0) / 1024.0;
  dtostrf(temp, 4, 2, temperatura);

  if(instruccionRecibida == "Temperatura"){
    client.publish("/Casa/Temperatura", temperatura);
    delay(500);
    instruccionRecibida = "";
    Serial.println("RECIBI TEMPERATURA");
  }

  if(instruccionRecibida == "VentiladorON"){
    estado = 1;
    instruccionRecibida = "";
    Serial.println("RECIBI VENTILADOR ON");
  }
  
  if(instruccionRecibida == "VentiladorOFF"){
    estado = 0; 
    instruccionRecibida = "";
    for(int i=0; i < 13; i++){
      rx[i] = NULL;  
    }
    Serial.println("RECIBI VENTILADOR OFF");
  }
}

String convertToString(char* a, int size) 
{ 
    int i; 
    String s = ""; 
    for (i = 0; i < size; i++) { 
        s = s + a[i]; 
    } 
    return s; 
} 
