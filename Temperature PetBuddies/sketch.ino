#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define SS_PIN        5
#define RST_PIN       22
#define ONE_WIRE_BUS  4

const char* SSID      = "Wokwi-GUEST";
const char* PASSWORD  = "";

const char* MQTT_HOST = "mqtt3.thingspeak.com";
const int   MQTT_PORT = 1883;

const char* THINGSPEAK_CHANNEL_ID = "SEU_CHANNEL_ID_TEMPERATURA";

// Preencha com as credenciais do menu Devices > MQTT do ThingSpeak.
// Nao e a mesma coisa que a Write API Key do canal.
const char* MQTT_CLIENT_ID = "SEU_CLIENT_ID_MQTT";
const char* MQTT_USER      = "SEU_USERNAME_MQTT";
const char* MQTT_PASS      = "SUA_SENHA_MQTT";

String topicPub;

MFRC522           rfid(SS_PIN, RST_PIN);
OneWire           oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);
WiFiClient        wifiClient;
PubSubClient      mqtt(wifiClient);

void reconnectMQTT();

void setup() {
  Serial.begin(115200);
  SPI.begin();
  rfid.PCD_Init();
  ds18b20.begin();

  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Conectado! IP: " + WiFi.localIP().toString());

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

  ds18b20.requestTemperatures();
  float tempC = ds18b20.getTempCByIndex(0);

  if (tempC == DEVICE_DISCONNECTED_C) {
    Serial.println("Erro: Sensor DS18B20 nao encontrado no circuito!");
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    return;
  }

  // Field1 = UID, Field2 = temperatura em Celsius.
  // O horario fica no created_at do ThingSpeak.
  String payload = "field1=" + uid + "&field2=" + String(tempC, 2);

  bool publicado = mqtt.publish(topicPub.c_str(), payload.c_str());
  if (publicado) {
    Serial.printf("[Sucesso] Publicado no ThingSpeak: UID=%s | Temp=%.2f C\n", uid.c_str(), tempC);
  } else {
    Serial.println("[Erro] Falha ao publicar no ThingSpeak.");
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(2000);
}
