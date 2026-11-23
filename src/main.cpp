#include <Arduino.h>
#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include <ezTime.h>
#include <PubSubClient.h>
#include "internet.h"


const char *mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;
const char *mqtt_client_id = "senai134_esp_giovanna";
const char *mqtt_topic_sub = "shrek123/pub";
const char *mqtt_topic_pub = "burro123/pub";

float tempCAnterior = -999;
float tempFAnterior = -999;
float umiAnterior = -999;
int lumiAnterior = -999;

LiquidCrystal_I2C lcd (0x27, 20, 4);


void conectaMqtt();
void retornoMqtt(char *topic, byte *payload, unsigned int length);

WiFiClient espClient;
PubSubClient client(espClient);

Timezone sp;

void setup() {

  Serial.begin(115200);
  Serial.setTimeout(5000);

  conexaoWifi();
  sp.setLocation("America/Sao_Paulo");

  waitForSync();

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Conectando...");

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(retornoMqtt);

  lcd.clear();
  lcd.print("Aguardando dados...");



}

void loop() {

  events();
  reconexaoWifi();
  client.loop();

  if (!client.connected())
  {
    conectaMqtt();
  }
  
}

void conectaMqtt(){
  while (!client.connected())
  {
    Serial.println("Conectando ao MQTT ");

    if (client.connect(mqtt_client_id))
    {
      Serial.println("Conectado");
      client.subscribe(mqtt_topic_sub);
    }
    else{
      Serial.println("Falha ao se conectar ");
      Serial.println(client.state());
      Serial.println("Tentando novamente em 5s");
      delay(5000);

    }
    
  }
  
}

void retornoMqtt(char *topic, byte *payload, unsigned int length){
  Serial.print("Mensagem recebida em ");
  Serial.print(topic);
  Serial.print(": ");

  String msgRecebida = "";
  for (int i = 0; i < length; i++)
  {
    msgRecebida += (char)payload[i];
  }

  Serial.println(msgRecebida);

  JsonDocument doc;
  DeserializationError erro = deserializeJson(doc, msgRecebida);

  if (erro)
  {
    Serial.print("Erro ao decodificar o JSON: ");
    Serial.println(erro.c_str());
    return;
  }

  String dispositivo = doc["disp"] | "?";
  float tempC = doc["tempC"].as<float>();
  float tempF = doc["tempF"].as<float>();
  float umidade = doc["umi"].as<float>();
  int luminosidade = doc["lumi"].as<int>();

  
  if (tempCAnterior != tempC || tempFAnterior != tempF|| umiAnterior != umidade || lumiAnterior != luminosidade){
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("Temp: " + String(tempC, 1) + "C " + String(tempF, 1) + "F");

    lcd.setCursor(0, 1);
    lcd.print("Umidade: " + String(umidade, 1) + "%");

    lcd.setCursor(0, 2);
    lcd.print("Luminosidade: " + String(luminosidade));

    lcd.setCursor(0, 3);
    lcd.print("Disp: " + dispositivo);

    tempCAnterior = tempC;
    tempFAnterior = tempF;
    umiAnterior = umidade;
    lumiAnterior = luminosidade;

  }
    
  
  
}

