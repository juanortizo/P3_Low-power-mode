#include <UbiConstants.h>
#include <UbidotsEsp32Mqtt.h>
#include <UbiTypes.h>
#include <stdio.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include "DHT.h"
#include <esp_sleep.h>

#define DHTPIN 27
#define DHTTYPE DHT11
#define MI_ABS(x) ((x) < 0 ? -(x) : (x))

TFT_eSPI tft = TFT_eSPI();
DHT dht(DHTPIN, DHTTYPE);

const char *UBIDOTS_TOKEN = "BBUS-XAorU8jJCxplTIudzCGCoYT9sTmhkV";
const char *WIFI_SSID = "FAMOSSA";
const char *WIFI_PASS = "salo1327";
const char *DEVICE_LABEL = "esp32";
const char *VARIABLE_LABEL1 = "sw1";
const char *VARIABLE_LABEL2 = "sw2";
const char *VARIABLE_LABEL3 = "Temp";
const char *VARIABLE_LABEL4 = "Hum";
const int PUBLISH_FREQUENCY = 5000;

unsigned long timer;
Ubidots ubidots(UBIDOTS_TOKEN);
int tamano;
int posicion;
char boton = '0';
char val = '0';
bool sw1State = false;
bool sw2State = false;

void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  tamano = strlen(topic);
  posicion = tamano - 4;
  printf("switch: %c\n", topic[posicion]);
  boton = topic[posicion];
  val = payload[0];
  if (boton == '1') {
    if ((char)payload[0] == '1') {
      sw1State = true;
      tft.fillCircle(35, 100, 10, TFT_GREEN);
    } else {
      sw1State = false;
      tft.fillCircle(35, 100, 10, TFT_DARKGREY);
    }
  }
  if (boton == '2') {
    if ((char)payload[0] == '1') {
      sw2State = true;
      tft.fillCircle(145, 100, 10, TFT_GREEN);
    } else {
      sw2State = false;
      tft.fillCircle(145, 100, 10, TFT_DARKGREY);
    }
  }
}

void setup() {
  esp_sleep_enable_timer_wakeup(10 * 1000000);
  Serial.begin(115200);
  ubidots.connectToWifi(WIFI_SSID, WIFI_PASS);
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();
  float Hum = dht.readHumidity();
  float Temp = dht.readTemperature();
  tft.init();
  tft.fillScreen(TFT_WHITE);
  tft.setRotation(1);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.drawString("Realizado por:", 10, 5, 2);
  tft.setTextColor(TFT_GREEN, TFT_WHITE);
  tft.drawString("Juan Daniel", 50, 35, 4);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.drawString("Temperatura", 25, 70, 2);
  tft.drawString("Humedad", 150, 70, 2);
  tft.fillCircle(35, 100, 10, TFT_DARKGREY);
  tft.fillCircle(145, 100, 10, TFT_DARKGREY);
  ubidots.subscribeLastValue(DEVICE_LABEL, VARIABLE_LABEL1);
  ubidots.subscribeLastValue(DEVICE_LABEL, VARIABLE_LABEL2);
  Serial.println(F("DHTxx test!"));
  dht.begin();
  timer = millis();
}

void loop() {
  if (!ubidots.connected()) {
    ubidots.reconnect();
    ubidots.subscribeLastValue(DEVICE_LABEL, VARIABLE_LABEL1);
    ubidots.subscribeLastValue(DEVICE_LABEL, VARIABLE_LABEL2);
  }
  float Hum = dht.readHumidity();
  float Temp = dht.readTemperature();
  if ((MI_ABS(millis() - timer)) > PUBLISH_FREQUENCY) {
    Serial.print("Temperatura: ");
    Serial.print(Temp);
    Serial.print(" Humedad: ");
    Serial.println(Hum);
    tft.drawString(String(Hum), 160, 100);
    tft.drawString(String(Temp), 50, 100);
    ubidots.add(VARIABLE_LABEL3, Temp);
    ubidots.add(VARIABLE_LABEL4, Hum);
    ubidots.publish(DEVICE_LABEL);
    timer = millis();
  }
  ubidots.loop();

  if (sw1State) {
    tft.fillCircle(35, 100, 10, TFT_GREEN);
  } else {
    tft.fillCircle(35, 100, 10, TFT_DARKGREY);
  }
  if (sw2State) {
    tft.fillCircle(145, 100, 10, TFT_GREEN);
  } else {
    tft.fillCircle(145, 100, 10, TFT_DARKGREY);
  }

  esp_deep_sleep_start();
}
