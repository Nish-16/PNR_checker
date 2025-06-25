#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "HostelB";
const char* password = "hostelnet";
const char* serverUrl = "http://172.16.129.220:3000/verify";

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Keypad config
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {18, 25, 26, 14};
byte colPins[COLS] = {27, 13, 23, 33};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

String pnr = "";
String deviceId = String((uint32_t)ESP.getEfuseMac(), HEX);

// LED Pins (LED 1, 2, and 3)
const int redPins[3] = {4, 12, 32};     // LED 1 red, LED 2 red, LED 3 red
const int greenPins[3] = {5, 15, 19};   // LED 1 green, LED 2 green, LED 3 green

bool ledStatus[3] = {true, true, true}; // false = green, true = red

void setup() {
  Serial.begin(115200);

  // LED pinMode setup
  for (int i = 0; i < 3; i++) {
    pinMode(redPins[i], OUTPUT);
    pinMode(greenPins[i], OUTPUT);
    digitalWrite(redPins[i], LOW);
    digitalWrite(greenPins[i], HIGH);
    ledStatus[i] = false;
  }

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Connecting...");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected!");
  lcd.clear();
  lcd.print("Connected!");
  delay(1000);
  lcd.clear();
}

void loop() {
  char key = keypad.getKey();
  if (key) {
    if (key == '#') {
      if (pnr.length() == 6) {
        sendPNR(pnr);
      } else {
        lcd.clear();
        lcd.print("Invalid PNR!");
        delay(1500);
        lcd.clear();
      }
      pnr = "";
    } else if (key == '*') {
      pnr = "";
      lcd.clear();
      lcd.print("Cleared!");
      delay(1000);
      lcd.clear();
    } else if (key == 'C') {
      // Reset all to green
      for (int i = 0; i < 3; i++) {
        setLED(i, false);
      }
      lcd.clear();
      lcd.print("LEDs reset");
      delay(1000);
      lcd.clear();
    } else {
      pnr += key;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("PNR: ");
      lcd.setCursor(0, 1);
      lcd.print(pnr);
    }
  }
}

void sendPNR(String pnr) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    String body = "{\"pnr\":\"" + pnr + "\", \"deviceId\":\"" + deviceId + "\"}";
    int httpResponseCode = http.POST(body);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Server: " + response);
      lcd.clear();
      lcd.print(response);

      if (response == "PNR Exsists") {
        int index = getLEDIndex(pnr);
        if (index != -1) {
          setLED(index, true);  // Turn red (green = false)
        }
      }
    } else {
      lcd.clear();
      lcd.print("HTTP Error");
    }

    http.end();
  } else {
    lcd.clear();
    lcd.print("WiFi Lost!");
  }
}

// Convert PNR to LED index
int getLEDIndex(String pnr) {
  if (pnr == "345678") return 0; // LED 1
  if (pnr == "456789") return 1; // LED 2
  if (pnr == "567890") return 2; // LED 3
  return -1;
}

// Set individual LED color
void setLED(int index, bool green) {
  digitalWrite(greenPins[index], green ? LOW : HIGH); // LOW = ON
  digitalWrite(redPins[index], green ? HIGH : LOW);   // LOW = ON
  ledStatus[index] = green;
}
