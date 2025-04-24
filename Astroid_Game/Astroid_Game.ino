#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Joystick pins
#define JOY_X 34
#define JOY_Y 35
#define JOY_BTN 25

int gunX = SCREEN_WIDTH / 2;
int score = 0;
int mode = 0; // 0 to 4 for 5 modes
int asteroidSpeed[] = {1, 2, 3, 4, 5}; // speed per mode

unsigned long lastBulletTime = 0;
unsigned long bulletCooldown = 300;

struct Bullet {
  int x, y;
  bool active;
};

struct Asteroid {
  int x, y, size, speed, health;
  bool active;
};

#define MAX_BULLETS 5
Bullet bullets[MAX_BULLETS];

#define MAX_ASTEROIDS 5
Asteroid asteroids[MAX_ASTEROIDS];

void setup() {
  pinMode(JOY_BTN, INPUT_PULLUP);
  analogReadResolution(10);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(WHITE);
  randomSeed(analogRead(0)); // for randomness
}

void loop() {
  showMenu();          // Select mode
  startGame();         // Start game loop
}

void showMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Select Mode:");
  display.println("Push joystick to select:");
  display.println("LEFT - Easy (0)");
  display.println("MID - Medium (1-2)");
  display.println("RIGHT - Hard (3-4)");
  display.display();

  while (true) {
    int x = analogRead(JOY_X);
    if (x < 300) { mode = 0; break; }        // Left
    if (x > 900) { mode = 4; break; }        // Right
    if (digitalRead(JOY_BTN) == LOW) {
      mode++;
      if (mode > 4) mode = 1;
      delay(300); // debounce delay
      display.setCursor(0, 50);
      display.setTextColor(WHITE);
      display.print("Mode set: ");
      display.print(mode);
      display.display();
      delay(1000);
      break;
    }
    delay(100);
  }
  resetGame();
}

void startGame() {
  while (true) {
    handleInput();
    updateBullets();
    updateAsteroids();
    checkCollisions();
    spawnAsteroids();
    drawEverything();
    delay(50);  // increased delay for slower asteroid movement
  }
}

void resetGame() {
  gunX = SCREEN_WIDTH / 2;
  score = 0;

  for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;
  for (int i = 0; i < MAX_ASTEROIDS; i++) asteroids[i].active = false;
}

void handleInput() {
  int x = analogRead(JOY_X);
  if (x < 500 && gunX > 0) gunX -= 2;
  if (x > 900 && gunX < SCREEN_WIDTH - 5) gunX += 2;

  if (digitalRead(JOY_BTN) == LOW && millis() - lastBulletTime > bulletCooldown) {
    fireBullet();
    lastBulletTime = millis();
  }
}

void fireBullet() {
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (!bullets[i].active) {
      bullets[i] = { gunX + 2, SCREEN_HEIGHT - 8, true };
      break;
    }
  }
}

void updateBullets() {
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (bullets[i].active) {
      bullets[i].y -= 4;
      if (bullets[i].y < 0) bullets[i].active = false;
    }
  }
}

void spawnAsteroids() {
  for (int i = 0; i < MAX_ASTEROIDS; i++) {
    if (!asteroids[i].active && random(100) < 3) {
      int health = random(1, 6); // 1 to 5
      int size = 3 + health * 2;
      asteroids[i] = {
        random(0, SCREEN_WIDTH - size),
        0,
        size,
        asteroidSpeed[mode],
        health,
        true
      };
    }
  }
}

void updateAsteroids() {
  for (int i = 0; i < MAX_ASTEROIDS; i++) {
    if (asteroids[i].active) {
      asteroids[i].y += asteroids[i].speed;
      if (asteroids[i].y > SCREEN_HEIGHT) {
        asteroids[i].active = false;
        gameOver();
        return;
      }
    }
  }
}

void checkCollisions() {
  for (int i = 0; i < MAX_ASTEROIDS; i++) {
    if (!asteroids[i].active) continue;
    for (int j = 0; j < MAX_BULLETS; j++) {
      if (!bullets[j].active) continue;

      if (bullets[j].x >= asteroids[i].x && bullets[j].x <= asteroids[i].x + asteroids[i].size &&
          bullets[j].y >= asteroids[i].y && bullets[j].y <= asteroids[i].y + asteroids[i].size) {
        bullets[j].active = false;
        asteroids[i].health--;
        if (asteroids[i].health <= 0) {
          asteroids[i].active = false;
          score++;
        }
      }
    }

    if (asteroids[i].y + asteroids[i].size >= SCREEN_HEIGHT - 6 &&
        asteroids[i].x + asteroids[i].size >= gunX &&
        asteroids[i].x <= gunX + 4) {
      gameOver();
      return;
    }
  }
}

void drawEverything() {
  display.clearDisplay();

  display.fillRect(gunX, SCREEN_HEIGHT - 6, 4, 6, WHITE);

  for (int i = 0; i < MAX_BULLETS; i++) {
    if (bullets[i].active) {
      display.drawPixel(bullets[i].x, bullets[i].y, WHITE);
    }
  }

  for (int i = 0; i < MAX_ASTEROIDS; i++) {
    if (asteroids[i].active) {
      display.fillRect(asteroids[i].x, asteroids[i].y, asteroids[i].size, asteroids[i].size, WHITE);
      display.setCursor(asteroids[i].x, asteroids[i].y);
      display.setTextSize(1);
      display.setTextColor(BLACK, WHITE);
      display.print(asteroids[i].health);
    }
  }

  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print("Score: ");
  display.print(score);

  display.display();
}

void gameOver() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(10, 20);
  display.println("GAME OVER");
  display.setTextSize(1);
  display.setCursor(10, 50);
  display.print("Score: ");
  display.print(score);
  display.display();
  delay(3000);
  showMenu();
}


/*

ESP32 Pin Connections:

OLED (I2C Display):

VCC → 3.3V

GND → GND

SDA → GPIO 21

SCL → GPIO 22

Joystick Module:

VCC → 3.3V

GND → GND

VRx → GPIO 34

VRy → GPIO 35

SW → GPIO 25


Fix the "Failed to connect to ESP32" issue

1.Connect ESP32 to your PC via USB

2.Hold down the BOOT button (sometimes called FLASH)

3.While holding BOOT, click "Upload" in Arduino IDE

4.Keep holding BOOT until you see Connecting... change to "Uploading...", then release it


*/
