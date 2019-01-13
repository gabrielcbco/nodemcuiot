#include <ESP8266WiFi.h>
#include <PubSubClient.h>
 
const char* ssid = "Cysneiros2";
const char* password =  "52579297";
const char* mqttServer = "m15.cloudmqtt.com";
const int mqttPort = 18749;
const char* mqttUser = "fihdgkdf";
const char* mqttPassword = "WsGFam3RDpCs";
float mytemp;

#define LEDPIN 0 // LED conectado à saida 0 (d3)
#define ALARM 23 //Valor para acionar o alarme

WiFiClient espClient;
PubSubClient client(espClient);
 
void callback(char* topic, byte* payload, unsigned int length) {
 
  Serial.print("Messagem chegou no tópico: ");
  Serial.println(topic);
 
  Serial.print("Messagem:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);

  }

  mytemp = atof((const char*)payload); //converte string para float
  Serial.print("Temperatura em float: ");
  Serial.println(mytemp); 
  Serial.println();
  Serial.println("-----------------------");
}
 
void setup() {
 
  Serial.begin(115200);

  pinMode(LEDPIN, OUTPUT);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Conectando ao WiFi..");
  }
  Serial.println("Conectado à rede WiFi !!!!");
 
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
 
  while (!client.connected()) {
    Serial.println("Conectando ao MQTT...");
 
    if (client.connect("ESP32Client", mqttUser, mqttPassword )) {
 
      Serial.println("Conectado !!!!");  
 
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
 
    }
  }
 
  client.subscribe("ct80936/temperatura");
 
}
 
void loop() {
  client.loop();
  if (mytemp > ALARM) {
    digitalWrite(LEDPIN, HIGH);
  }
  else {
    digitalWrite(LEDPIN, LOW);
  }
}
