#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Joystick pins
#define VRx 34
#define VRy 35
#define SW  32

#define BLOCK_SIZE 4
#define FIELD_WIDTH 10
#define FIELD_HEIGHT 16

// Tetrimino shapes (4x4 grids)
byte shapes[7][4][4] = {
  // I
  {
    {0, 0, 0, 0},
    {1, 1, 1, 1},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
  },
  // O
  {
    {0, 0, 0, 0},
    {0, 1, 1, 0},
    {0, 1, 1, 0},
    {0, 0, 0, 0}
  },
  // T
  {
    {0, 0, 0, 0},
    {1, 1, 1, 0},
    {0, 1, 0, 0},
    {0, 0, 0, 0}
  },
  // L
  {
    {0, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 0, 0}
  },
  // J
  {
    {0, 0, 1, 0},
    {0, 0, 1, 0},
    {0, 1, 1, 0},
    {0, 0, 0, 0}
  },
  // S
  {
    {0, 0, 0, 0},
    {0, 1, 1, 0},
    {1, 1, 0, 0},
    {0, 0, 0, 0}
  },
  // Z
  {
    {0, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 0, 0}
  }
};

// Game variables
int field[FIELD_HEIGHT][FIELD_WIDTH] = {0};
int blockX = 3, blockY = 0;
int currentShape = 0;
bool gameStarted = false;
bool gameOver = false;
int score = 0;
unsigned long lastMoveTime = 0;
int speed = 500;

void setup() {
  Serial.begin(115200);

  pinMode(SW, INPUT_PULLUP);
  pinMode(VRx, INPUT);
  pinMode(VRy, INPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.display();

  randomSeed(analogRead(34));  // Randomize shapes
}

void loop() {
  if (!gameStarted) {
    showMenu();
    if (digitalRead(SW) == LOW) {
      delay(200);
      startGame();
    }
  } else {
    if (!gameOver) {
      runGame();
    } else {
      showGameOver();
      if (digitalRead(SW) == LOW) {
        delay(200);
        gameStarted = false;
      }
    }
  }
}

void showMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20, 20);
  display.println("TETRIS");
  display.setCursor(5, 50);
  display.println("Click to Start Game");
  display.display();
}

void startGame() {
  memset(field, 0, sizeof(field));
  blockX = 3;
  blockY = 0;
  score = 0;
  speed = 500;
  gameStarted = true;
  gameOver = false;
  spawnNewBlock();
}

void spawnNewBlock() {
  currentShape = random(0, 7);
  blockX = 3;
  blockY = 0;
  if (checkCollision(blockX, blockY)) {
    gameOver = true;
  }
}

bool checkCollision(int xOffset, int yOffset) {
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      if (shapes[currentShape][y][x]) {
        int fx = xOffset + x;
        int fy = yOffset + y;
        if (fx < 0 || fx >= FIELD_WIDTH || fy >= FIELD_HEIGHT)
          return true;
        if (fy >= 0 && field[fy][fx])
          return true;
      }
    }
  }
  return false;
}

void lockBlock() {
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      if (shapes[currentShape][y][x]) {
        int fx = blockX + x;
        int fy = blockY + y;
        if (fy >= 0 && fx >= 0 && fx < FIELD_WIDTH && fy < FIELD_HEIGHT) {
          field[fy][fx] = 1;
        }
      }
    }
  }
  checkFullLines();
  spawnNewBlock();
}

void checkFullLines() {
  for (int y = FIELD_HEIGHT - 1; y >= 0; y--) {
    bool fullLine = true;
    for (int x = 0; x < FIELD_WIDTH; x++) {
      if (!field[y][x]) {
        fullLine = false;
        break;
      }
    }
    if (fullLine) {
      // Move lines down
      for (int yy = y; yy > 0; yy--) {
        for (int xx = 0; xx < FIELD_WIDTH; xx++) {
          field[yy][xx] = field[yy - 1][xx];
        }
      }
      for (int xx = 0; xx < FIELD_WIDTH; xx++) {
        field[0][xx] = 0;
      }
      score += 10;
      speed = max(100, speed - 20); // Increase speed
      y++;
    }
  }
}

void drawBlock(int x, int y) {
  display.fillRect(x * BLOCK_SIZE, y * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, SSD1306_WHITE);
}

void runGame() {
  // Joystick movement
  int xVal = analogRead(VRx);
  if (xVal < 1000) {  // left
    if (!checkCollision(blockX - 1, blockY)) blockX--;
    delay(100);
  }
  if (xVal > 3000) {  // right
    if (!checkCollision(blockX + 1, blockY)) blockX++;
    delay(100);
  }

  int yVal = analogRead(VRy);
  if (yVal > 3000) {  // down
    if (!checkCollision(blockX, blockY + 1)) blockY++;
    delay(50);
  }

  if (digitalRead(SW) == LOW) {  // rotate
    delay(200);
    rotateBlock();
  }

  // Move down based on timer
  if (millis() - lastMoveTime > speed) {
    if (!checkCollision(blockX, blockY + 1)) {
      blockY++;
    } else {
      lockBlock();
    }
    lastMoveTime = millis();
  }

  // Draw everything
  display.clearDisplay();

  // Draw locked field blocks
  for (int y = 0; y < FIELD_HEIGHT; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      if (field[y][x]) {
        drawBlock(x, y);
      }
    }
  }

  // Draw current falling block
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      if (shapes[currentShape][y][x]) {
        drawBlock(blockX + x, blockY + y);
      }
    }
  }

  // Draw score
  display.setTextSize(1);
  display.setCursor(70, 0);
  display.print("S:");
  display.print(score);

  display.display();
}

void rotateBlock() {
  byte temp[4][4];
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      temp[y][x] = shapes[currentShape][3 - x][y];
    }
  }

  // Check if rotation is possible
  bool canRotate = true;
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      if (temp[y][x]) {
        int fx = blockX + x;
        int fy = blockY + y;
        if (fx < 0 || fx >= FIELD_WIDTH || fy >= FIELD_HEIGHT)
          canRotate = false;
        else if (fy >= 0 && field[fy][fx])
          canRotate = false;
      }
    }
  }

  if (canRotate) {
    for (int y = 0; y < 4; y++) {
      for (int x = 0; x < 4; x++) {
        shapes[currentShape][y][x] = temp[y][x];
      }
    }
  }
}

void showGameOver() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(25, 20);
  display.println("GAME OVER");
  display.setCursor(20, 35);
  display.print("Score: ");
  display.println(score);
  display.setCursor(5, 50);
  display.println("Click to go Menu");
  display.display();
}





/*
 * 
 *      Components
   1. OLED Display (I2C: SDA, SCL)

   2. Joystick Module (X, Y, Button)
   
   3. NodeMCU ESP32

   4. Jumper Wires



        Connections

Component  ESP32 GPIO
OLED SDA   GPIO 21
OLED SCL   GPIO 22
Joystick   VRX  GPIO 34
Joystick   VRY  GPIO 35
Joystick   SW GPIO 32

*/
