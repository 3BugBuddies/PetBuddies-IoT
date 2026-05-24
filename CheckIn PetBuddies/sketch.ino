#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN   5
#define RST_PIN  22

const char* SSID      = "Wokwi-GUEST";
const char* PASSWORD  = "";
const char* MQTT_HOST = "mqtt3.thingspeak.com";
const int   MQTT_PORT = 1883;
const char* THINGSPEAK_CHANNEL_ID    = "SEU_CHANNEL_ID_CHECKIN";

const char* MQTT_CLIENT_ID = "SEU_CLIENT_ID_MQTT";
const char* MQTT_USER      = "SEU_USERNAME_MQTT";
const char* MQTT_PASS      = "SUA_PASSWORD_MQTT";
String topicPub;
MFRC522      rfid(SS_PIN, RST_PIN);
WiFiClient   wifiClient;
PubSubClient mqtt(wifiClient);

void reconnectMQTT();

void setup() {
  Serial.begin(115200);
  SPI.begin();
  rfid.PCD_Init();
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi: " + WiFi.localIP().toString());
  topicPub = "channels/";
  topicPub += THINGSPEAK_CHANNEL_ID;
  topicPub += "/publish";
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  reconnectMQTT();
}
void reconnectMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Conectando ao MQTT ThingSpeak...");
    if (mqtt.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS)) {
      Serial.println("OK");
    } else {
      Serial.print("Falhou (Codigo de erro: ");
      Serial.print(mqtt.state());
      Serial.println(") - tentando em 5s");
      delay(5000);
    }
  }
}
void loop() {
  if (!mqtt.connected()) reconnectMQTT();
  mqtt.loop();

  if (!rfid.PICC_IsNewCardPresent() ||
      !rfid.PICC_ReadCardSerial()) return;

  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();

  String payload = "field1=" + uid + "&field2=recepcao";
  bool publicado = mqtt.publish(topicPub.c_str(), payload.c_str());
  if (publicado) {
    Serial.println("Publicado no ThingSpeak: " + payload);
  } else {
    Serial.println("Falha ao publicar no ThingSpeak");
  }
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(1500);
}