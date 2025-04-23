#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Joystick pins
#define JOY_X 36
#define JOY_Y 39
#define JOY_SW 25

int selectedIndex = 0;
int totalNetworks = 0;

void setup() {
  Serial.begin(115200);
  pinMode(JOY_SW, INPUT_PULLUP);

  // OLED
  Wire.begin(21, 22); // SDA = 21, SCL = 22
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("WiFi Scanner ESP32");
  display.display();
  delay(1000);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
}

void loop() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Scanning...");

  totalNetworks = WiFi.scanNetworks();
  if (totalNetworks == 0) {
    display.println("No networks found");
  } else {
    display.println("Networks:");
    int y = 20;
    int displayCount = min(4, totalNetworks);

    for (int i = 0; i < displayCount; i++) {
      if (i == selectedIndex) {
        display.setTextColor(BLACK, WHITE);
      } else {
        display.setTextColor(WHITE);
      }

      String ssid = WiFi.SSID(i);
      int rssi = WiFi.RSSI(i);
      display.setCursor(0, y);
      display.printf("%s [%ddBm]", ssid.substring(0, 10).c_str(), rssi);
      y += 10;
    }
  }

  display.display();
  handleJoystick();
  delay(1500);
}

void handleJoystick() {
  int xVal = analogRead(JOY_X);

  if (xVal < 1000) {
    selectedIndex--;
    if (selectedIndex < 0) selectedIndex = 0;
  } else if (xVal > 3000) {
    selectedIndex++;
    if (selectedIndex > totalNetworks - 1) selectedIndex = totalNetworks - 1;
  }

  if (digitalRead(JOY_SW) == LOW) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextColor(WHITE);
    display.println("Selected:");
    display.println(WiFi.SSID(selectedIndex));
    display.print("RSSI: ");
    display.println(WiFi.RSSI(selectedIndex));
    display.display();
    delay(2000);
  }
}



   /*
    * omponent          Pin on ESP32
      OLED SDA          GPIO21
      OLED SCL          GPIO22
      Joystick VRx      GPIO36 (VP)
      Joystick VRy      GPIO39 (VN)
      Joystick          SW GPIO25
      VCC (OLED & Joy)  3.3V
      GND               GND


       Libraries
       
       1. ESP8266WiFi.h

       2. Wire.h

       3. Adafruit_GFX.h

       4. Adafruit_SSD1306.h
   
   */
