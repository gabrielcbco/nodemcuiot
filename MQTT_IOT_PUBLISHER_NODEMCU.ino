#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

#define INTERVALO_ENVIO       60000

#define DHTPIN 2      // o sensor dht11 foi conectado ao pino 2( D4 do node MCU)
#define DHTTYPE DHT22
 
DHT dht(DHTPIN, DHTTYPE);

#define DEBUG

//informações da rede WIFI
const char* ssid = "Cysneiros2";                 //SSID da rede WIFI
const char* password =  "52579297";    //senha da rede wifi
 
//informações do broker MQTT - Verifique as informações geradas pelo CloudMQTT
const char* mqttServer = "m15.cloudmqtt.com";   //server
const char* mqttUser = "fihdgkdf";              //user
const char* mqttPassword = "WsGFam3RDpCs";      //password
const int mqttPort = 18749;                     //port
const char* mqttTopicSub ="ct80936";            //tópico que sera assinado

int ultimoEnvioMQTT = 0;
 
WiFiClient espClient;
PubSubClient client(espClient);
 
void setup() {
 
  Serial.begin(115200);
  pinMode(DHTPIN, INPUT);
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    #ifdef DEBUG
    Serial.println("Conectando ao WiFi..");
    #endif
  }
  #ifdef DEBUG
  Serial.println("Conectado na rede WiFi");
  #endif
 
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
 
  while (!client.connected()) {
    #ifdef DEBUG
    Serial.println("Conectando ao Broker MQTT...");
    #endif
 
    if (client.connect("ESP8266Client", mqttUser, mqttPassword )) {
      #ifdef DEBUG
      Serial.println("Conectado");  
      #endif
 
    } else {
      #ifdef DEBUG 
      Serial.print("falha estado  ");
      Serial.print(client.state());
      #endif
      delay(2000);
 
    }
  }

  //subscreve no tópico
  client.subscribe(mqttTopicSub);

  dht.begin();
 
}
 
void callback(char* topic, byte* payload, unsigned int length) {

  //armazena msg recebida em uma sring
  payload[length] = '\0';
  String strMSG = String((char*)payload);

  #ifdef DEBUG
  Serial.print("Mensagem chegou do tópico: ");
  Serial.println(topic);
  Serial.print("Mensagem:");
  Serial.print(strMSG);
  Serial.println();
  Serial.println("-----------------------");
  #endif

 }

//função pra reconectar ao servido MQTT
void reconect() {
  //Enquanto estiver desconectado
  while (!client.connected()) {
    #ifdef DEBUG
    Serial.print("Tentando conectar ao servidor MQTT");
    #endif
     
    bool conectado = strlen(mqttUser) > 0 ?
                     client.connect("ESP8266Client", mqttUser, mqttPassword) :
                     client.connect("ESP8266Client");

    if(conectado) {
      #ifdef DEBUG
      Serial.println("Conectado!");
      #endif
      //subscreve no tópico
      client.subscribe(mqttTopicSub, 1); //nivel de qualidade: QoS 1
    } else {
      #ifdef DEBUG
      Serial.println("Falha durante a conexão.Code: ");
      Serial.println( String(client.state()).c_str());
      Serial.println("Tentando novamente em 10 s");
      #endif
      //Aguarda 10 segundos 
      delay(10000);
    }
  }
}
 
void loop() {
  if (!client.connected()) {
    reconect();
  }

  //envia a cada X segundos
  if ((millis() - ultimoEnvioMQTT) > INTERVALO_ENVIO)
  {
      enviaDHT();
      ultimoEnvioMQTT = millis();
  }
  
  client.loop();
}

//função para leitura do DHT11
void enviaDHT(){

  char MsgUmidadeMQTT[10];
  char MsgTemperaturaMQTT[10];
  
  float umidade = dht.readHumidity();
  float temperatura = dht.readTemperature();
  

  if (isnan(temperatura) || isnan(umidade)) 
  {
    #ifdef DEBUG
    Serial.println("Falha na leitura do dht11...");
    #endif
  } 
  else 
  {
    #ifdef DEBUG
    Serial.print("Umidade: ");
    Serial.print(umidade);
    Serial.print(" \n"); //quebra de linha
    Serial.print("Temperatura: ");
    Serial.print(temperatura);
    Serial.println(" °C");
    #endif

    sprintf(MsgUmidadeMQTT,"%f",umidade-10);
    client.publish("ct80936/umidade", MsgUmidadeMQTT);
    sprintf(MsgTemperaturaMQTT,"%f",temperatura-7);
    client.publish("ct80936/temperatura", MsgTemperaturaMQTT);
  }
}
