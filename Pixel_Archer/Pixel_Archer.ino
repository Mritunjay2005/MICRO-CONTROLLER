#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET     -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define JOY_X 34
#define JOY_Y 35
#define JOY_BTN 32

int aimX = 64;
int score = 0;
bool arrowFired = false;
int arrowY = 0;

// Target movement
int targetX = 64;
int targetDir = 1; // 1 for right, -1 for left
int targetSpeed = 1;
unsigned long lastSpeedChange = 0;

void setup() {
  Serial.begin(115200);
  pinMode(JOY_BTN, INPUT_PULLUP);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 failed"));
    for (;;);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Archery Game");
  display.display();
  delay(1000);

  randomSeed(analogRead(0));
}

void loop() {
  display.clearDisplay();

  int xVal = analogRead(JOY_X);
  int btnVal = digitalRead(JOY_BTN);

  // Joystick control
  if (!arrowFired) {
    if (xVal < 1500) aimX -= 2;
    if (xVal > 2500) aimX += 2;
    aimX = constrain(aimX, 0, 127);
  }

  // Target movement
  targetX += targetDir * targetSpeed;
  if (targetX < 10 || targetX > 118) {
    targetDir *= -1;
  }

  // Change speed every 3 seconds
  if (millis() - lastSpeedChange > 3000) {
    targetSpeed = random(1, 4); // Speed between 1 and 3
    lastSpeedChange = millis();
  }

  // Draw moving target
  display.drawCircle(targetX, 5, 5, SSD1306_WHITE);
  display.drawCircle(targetX, 5, 3, SSD1306_WHITE);

  // Draw aim
  display.drawLine(aimX, 63, aimX, 50, SSD1306_WHITE);

  // Fire arrow
  if (btnVal == LOW && !arrowFired) {
    arrowFired = true;
    arrowY = 63;
  }

  // Move arrow upward
  if (arrowFired) {
    display.drawPixel(aimX, arrowY, SSD1306_WHITE);
    arrowY -= 5;
    if (arrowY <= 5) {
      arrowFired = false;
      // Hit detection
      if (abs(aimX - targetX) < 5) {
        score++;
      }
    }
  }

  // Display score
  display.setCursor(0, 55);
  display.print("Score: ");
  display.print(score);

  display.display();
  delay(50);
}


/*
 HARDWARE CONNECTIONS
ESP32 to OLED (SSD1306 - I2C):
OLED Pin  ESP32 Pin
VCC 3.3V
GND GND
SCL GPIO 22
SDA GPIO 21

Joystick Module:
Joystick Pin  ESP32 Pin
GND GND
+5V/VCC 3.3V or 5V
VRx GPIO 34
VRy GPIO 35
SW (Button) GPIO 32
*/
