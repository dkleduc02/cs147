#include <Arduino.h>
#include <adafruit_AHTX0.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <inttypes.h>
#include <stdio.h>

#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"


char ssid[50] = "Tim_Iphone";
char pass[50] = "weighanchor";

const int kNetworkTimeout = 30 * 1000;
const int kNetworkDelay = 1000;

// put function declarations here:

#define timeSeconds 7
// Set GPIOs for LED and PIR Motion Sensor
const int led = 2;
const int motionSensor = 15;

Adafruit_AHTX0 aht;

unsigned long now = millis();
unsigned long lastTrigger = 0;
boolean startTimer = false;
boolean motion = false;
boolean lightsOn = false;

const double desiredTemp = 25.50;
const double tempThreshold = 1.25;

// Checks if motion was detected, sets LED HIGH and starts a timer
void IRAM_ATTR detectsMovement() {
  digitalWrite(led, HIGH);
  startTimer = true;
  lastTrigger = millis();
}

void nvs_access() {
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);

  nvs_handle_t my_handle;
  err = nvs_open("storange", NVS_READWRITE, &my_handle);

  if (err != ESP_OK) {
    Serial.printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
  }
  else {
    Serial.printf("Done\n");

    size_t ssid_len;
    size_t pass_len;

    err = nvs_get_str(my_handle, "ssid", ssid, &ssid_len);
    err |= nvs_get_str(my_handle, "pass", pass, & pass_len);

    switch(err) {
      case ESP_OK:
        Serial.printf("Done\n");
        break;
      case ESP_ERR_NVS_NOT_FOUND:
        Serial.printf("The value is not initialized yet!\n");
        break;
      default:
        Serial.printf("Error (%s) reading\n", esp_err_to_name(err));
    }
  }

  nvs_close(my_handle);
}

void setup() {
  // Serial port for debugging purposes
  Serial.begin(9600);
  
  if(!aht.begin()) {

    Serial.println(F("Could not find AHT? Check wiring!"));
    while (1) delay (10);
  }
  Serial.print(F("AHT10 or AHT20 found"));

  delay(1000);
  // Initialize NVS
  nvs_access();
  delay(1000);

  Serial.print(F("Connecting to "));
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  // PIR Motion Sensor mode INPUT_PULLUP
  pinMode(motionSensor, INPUT_PULLUP);
  // Set motionSensor pin as interrupt, assign interrupt function and set RISING mode
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  // Set LED to LOW
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);
}

void loop() {
  // Current time
  sensors_event_t humidity, temp;

  if(aht.getEvent(&humidity, &temp)) {
    Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" degrees C");
    //Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% rH");
    delay(2000);
  }



  now = millis();
  if((digitalRead(led) == HIGH) && (motion == false)) {
    Serial.println("MOTION DETECTED!!!");
    motion = true;
    lightsOn = true;
  }
  // Turn off the LED after the number of seconds defined in the timeSeconds variable
  if(startTimer && (now - lastTrigger > (timeSeconds*1000))) {
    Serial.println("Motion stopped...");
    digitalWrite(led, LOW);
    startTimer = false;
    motion = false;
    lightsOn = false;
  }

  delay(500);

  if(temp.temperature > (desiredTemp + tempThreshold)) {

    Serial.println("Temp is too high! Turning AC on");    

  }
  else if(temp.temperature < (desiredTemp - tempThreshold)) {

    Serial.println("Temp is too low! Turning heat up");

  }
  else {
    Serial.println("Temp at desired value");
  }

  String url = "/?var1=" + String(temp.temperature) + "&var2=" + String(lightsOn ? "ON" : "OFF") + "\n";
  
  int err = 0;

  WiFiClient c;
  HttpClient http(c);

  err = http.get("3.16.138.175", 5000, url.c_str(), NULL);
  if(err == 0) {

    err = http.responseStatusCode();

    if(err >= 0) {

      Serial.print(("Got status code: "));
      Serial.println(err);

      err = http.skipResponseHeaders();

      if(err >= 0) {

        int bodyLen = http.contentLength();

        unsigned long timeoutStart = millis();
        char c;

        while ((http.connected() || http.available()) && ((millis() - timeoutStart) < kNetworkTimeout)) {
          if(http.available()) {

            c = http.read();
            bodyLen--;
            timeoutStart = millis();

          }
          else {

            delay(kNetworkDelay);

          }
        }

      }
      else {
        Serial.println(err);
      }
    }
    else {
      Serial.println(err);
    }

  }
  else {
    Serial.println(err);
  }
  http.stop();

  delay(2000);

}

// put function definitions here: