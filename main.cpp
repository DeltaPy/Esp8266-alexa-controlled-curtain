#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <fauxmoESP.h>

const char* WIFI_SSID = "YOUR_SSID";
const char* WIFI_PASS = "YOUR_PASSWORD";
String wifi_ssid = "";
String device_ip = "";

const int pinMotorOpen = D1;
const int pinMotorClose = D2;
const int pinLimitSwitchOpened = D7;
const int pinLimitSwitchClosed = D8;
const int pinOpenButton = D5;
const int pinCloseButton = D6;

fauxmoESP fauxmo;

bool ext_state = false;
bool opening = false;
bool closing = false;
bool alexa = false;

#define DEVICE_NAME "curtain"

void wifiSetup() {
    WiFi.mode(WIFI_STA);

    // Connect
    Serial.printf("[WIFI] Connecting to %s ", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(100);
    }
    Serial.println();

    // Connected!
    Serial.printf("[WIFI] STATION Mode, SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
    wifi_ssid = WiFi.SSID().c_str();
    device_ip = WiFi.localIP().toString().c_str();
}

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing...");

  pinMode(pinMotorOpen, OUTPUT);
  pinMode(pinMotorClose, OUTPUT);
  pinMode(pinLimitSwitchOpen, INPUT_PULLUP);
  pinMode(pinLimitSwitchClosed, INPUT_PULLUP);
  pinMode(pinOpenButton, INPUT_PULLUP);
  pinMode(pinCloseButton, INPUT_PULLUP);

  wifiSetup();
  fauxmo.createServer(true);
  fauxmo.setPort(80);
  fauxmo.enable(true);
  fauxmo.addDevice(DEVICE_NAME);

  fauxmo.onSetState([](unsigned char device_id, const char* device_name, bool state, unsigned char value) {
    Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);
    ext_state = state;
    if (strcmp(device_name, DEVICE_NAME) == 0) {
      if (state == true && (digitalRead(pinLimitSwitchOpen) == LOW)) { //apri
        Serial.println("Opening with Alexa...");
        alexa = true;
        opening = true;
        closing = false;
      } 

      if (state == false && (digitalRead(pinLimitSwitchClosed) == LOW)) { //chiudi
        Serial.println("Closing with Alexa...");
        alexa = true;
        opening = false;
        closing = true;
      }
    }
  });

  Serial.println("Setup completed.");
}

void loop() {
  fauxmo.handle();
  
   

  if (digitalRead(pinOpenButton) == HIGH && (digitalRead(pinLimitSwitchOpen) == LOW) && alexa == false) {
    Serial.println("Opening...");
    opening = true;
    closing = false;
  }
  if (digitalRead(pinCloseButton) == HIGH && (digitalRead(pinLimitSwitchClosed) == LOW) && alexa == false) {
    Serial.println("Closing...");
    closing = true;
    opening = false;
  }

  if (digitalRead(pinOpenButton) == HIGH && digitalRead(pinCloseButton) == HIGH) {
      opening = false;
      closing = false;
      delay(200);
  }
  
  // Opening
  if (opening && digitalRead(pinLimitSwitchOpen) == LOW) {
    digitalWrite(pinMotorOpen, HIGH);
  } else {
      //Uncomment if you want to report back the state to Alexa. Buggy!
      // if(alexa) {
      //   fauxmo.setState(DEVICE_NAME, true, 255);
      // }
      alexa = false;
      opening = false;
      digitalWrite(pinMotorOpen, LOW);
    }
  

  // Closing
  if (closing && digitalRead(pinLimitSwitchClosed) == LOW) {
    digitalWrite(pinMotorClose, HIGH);
  } else {
      //Uncomment if you want to report back the state to Alexa. Buggy!
      // if(alexa) {
      //   fauxmo.setState(DEVICE_NAME, false, 255);
      // }
      alexa = false;
      closing = false;
      digitalWrite(pinMotorClose, LOW);
   }
}