#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Joystick pins
#define VRx 34
#define VRy 35
#define SW 32

#define SNAKE_SIZE 4
int snakeX[100], snakeY[100];
int snakeLength = 5;

int foodX, foodY;
int dirX = 1, dirY = 0; // 1 unit right initially

bool gameRunning = false;
bool gameOverScreen = false;

int score = 0;

unsigned long buttonPressStartTime = 0;
bool buttonHeld = false;

void setup() {
  Serial.begin(115200);

  pinMode(VRx, INPUT);
  pinMode(VRy, INPUT);
  pinMode(SW, INPUT_PULLUP);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for (;;);
  }

  display.clearDisplay();
  display.display();

  showStartMenu();
}

void initGame() {
  snakeLength = 5;
  dirX = 1;
  dirY = 0;
  score = 0;

  for (int i = 0; i < snakeLength; i++) {
    snakeX[i] = 20 - i * SNAKE_SIZE;
    snakeY[i] = 20;
  }
  spawnFood();
}

void spawnFood() {
  foodX = random(0, SCREEN_WIDTH / SNAKE_SIZE) * SNAKE_SIZE;
  foodY = random(0, SCREEN_HEIGHT / SNAKE_SIZE) * SNAKE_SIZE;
}

void readJoystick() {
  int xValue = analogRead(VRx);
  int yValue = analogRead(VRy);

  // thresholds to ignore noise
  if (xValue < 1000 && dirX != 1) { dirX = -1; dirY = 0; }
  else if (xValue > 3000 && dirX != -1) { dirX = 1; dirY = 0; }
  else if (yValue < 1000 && dirY != 1) { dirX = 0; dirY = -1; }
  else if (yValue > 3000 && dirY != -1) { dirX = 0; dirY = 1; }
}

void moveSnake() {
  for (int i = snakeLength - 1; i > 0; i--) {
    snakeX[i] = snakeX[i - 1];
    snakeY[i] = snakeY[i - 1];
  }

  snakeX[0] += dirX * SNAKE_SIZE;
  snakeY[0] += dirY * SNAKE_SIZE;
}

void checkCollision() {
  if (snakeX[0] < 0 || snakeX[0] >= SCREEN_WIDTH || snakeY[0] < 0 || snakeY[0] >= SCREEN_HEIGHT) {
    showGameOverScreen();
  }
  for (int i = 1; i < snakeLength; i++) {
    if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) {
      showGameOverScreen();
    }
  }
}

void checkFood() {
  if (snakeX[0] == foodX && snakeY[0] == foodY) {
    if (snakeLength < 100) {
      snakeLength++;
      score++;
    }
    spawnFood();
  }
}

void showGameOverScreen() {
  gameRunning = false;
  gameOverScreen = true;

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(30, 10);
  display.print("GAME END");

  display.setCursor(20, 30);
  display.print("Score: ");
  display.print(score);

  display.setCursor(5, 50);
  display.print("[Press to Restart]");

  display.display();
}

void drawSnake() {
  for (int i = 0; i < snakeLength; i++) {
    display.fillRect(snakeX[i], snakeY[i], SNAKE_SIZE, SNAKE_SIZE, SSD1306_WHITE);
  }
}

void drawFood() {
  display.fillRect(foodX, foodY, SNAKE_SIZE, SNAKE_SIZE, SSD1306_WHITE);
}

void showStartMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(25, 5);
  display.print("SNAKE GAME");

  display.setCursor(0, 20);
  display.print("Press joystick");

  display.setCursor(0, 30);
  display.print("to START");

  if (score > 0) {
    display.setCursor(0, 45);
    display.print("Last Score: ");
    display.print(score);
  }

  display.display();
}

void checkLongPress() {
  if (digitalRead(SW) == LOW) {
    if (!buttonHeld) {
      buttonPressStartTime = millis();
      buttonHeld = true;
    } else {
      if (millis() - buttonPressStartTime > 3000) {
        gameRunning = false;
        gameOverScreen = false;
        showStartMenu();
      }
    }
  } else {
    buttonHeld = false;
  }
}

void loop() {
  if (!gameRunning && !gameOverScreen) {
    if (digitalRead(SW) == LOW) {
      delay(300); // debounce
      initGame();
      gameRunning = true;
    }
  } 
  else if (gameOverScreen) {
    if (digitalRead(SW) == LOW) {
      delay(300); // debounce
      gameOverScreen = false;
      showStartMenu();
    }
  }
  else {
    readJoystick();
    moveSnake();
    checkCollision();
    checkFood();
    checkLongPress();

    display.clearDisplay();
    drawSnake();
    drawFood();

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("S:");
    display.print(score);

    display.display();

    delay(100);
  }
}





/** 
 *  Joystick Module Pin  ESP32 Pin
         VRx               GPIO 34
         VRy               GPIO 35
         SW                (Button) GPIO 32
         VCC               3.3V
         GND               GND

OLED Module Pin   ESP32 Pin
SDA               GPIO 21
SCL               GPIO 22
VCC               3.3V
GND               GND


Install these libraries via Arduino Library Manager:

 1. SSD1306 BY Adafruit 

 2. GFX BY Adafruit

 *//
