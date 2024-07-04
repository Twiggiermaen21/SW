#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>

/* Constructor */
U8G2_SSD1327_VISIONOX_128X96_1_4W_SW_SPI oled(U8G2_R0, /* clock=*/13, /* data=*/11, /* cs=*/10, /* dc=*/9, /* reset=*/8);
#define SCREEN_WIDTH 128 //Szerokość ekranu
#define SCREEN_HEIGHT 96 //Wysokość ekranu
#define SNAKE_PIECE_SIZE 5 //Ilosc części węża
#define MAX_SANKE_LENGTH 165 //Najwieksza długość węża
#define MAP_SIZE_X 24 //Szerokosc mapy
#define MAP_SIZE_Y 15 //Wysokosc mapy
#define STARTING_SNAKE_SIZE 8 //Początkowa długość węża

// Game states
#define gameStart 0
#define gameEnd 1
#define gamePlaying 2
int8_t fruit[2];
int8_t snake[MAX_SANKE_LENGTH][2];
uint8_t snake_length;
int snake_delay;
int score;
int dir;
int newDir;
volatile int gameStatus = gameStart;
const byte buttonPins[] = { 5, 4, 3, 6 };  // Lewo, Góra, Prawo, Dół

//przypisanie pinów
void setup(void) {
  oled.begin();
  pinMode(3, INPUT_PULLUP); // Prawo
  pinMode(4, INPUT_PULLUP); // Góra
  pinMode(5, INPUT_PULLUP); // Lewo
  pinMode(6, INPUT_PULLUP); // Dół
  pinMode(2, OUTPUT); // Buzzer
  generateFruit();
  resetSnake();
}

void loop(void) {
  //Pętla rysująca na ekranie obiekty
  oled.firstPage();
  do {
    draw();
  } while (oled.nextPage());
//Odczyt stanu gry oraz przebieg gry
  if (gameStatus == gamePlaying) {
    readDirection(); 
    dir = newDir;
    checkFruit(); 
    delay(snake_delay);
    if (moveSnake()) {
      tone(2, 500, 100);
      gameStatus = gameEnd;
    }
  }
}

// Zmiana kierunku poruszania sie węża
bool moveSnake() {
  int8_t x = snake[0][0];
  int8_t y = snake[0][1];
  switch (dir) {
    case 4:
      x -= 1;
      break;
    case 2:
      y -= 1;
      break;
    case 6:
      x += 1;
      break;
    case 8:
      y += 1;
      break;
  }

  //znajdowanie kolizji
  if (collisionCheck(x, y))
    return true;
  for (int i = snake_length - 1; i > 0; i--) {
    snake[i][0] = snake[i - 1][0];
    snake[i][1] = snake[i - 1][1];
  }
  snake[0][0] = x;
  snake[0][1] = y;
  return false;
}
bool collisionCheck(int8_t x, int8_t y) {
  for (int i = 1; i < snake_length; i++) {
    if (x == snake[i][0] && y == snake[i][1]) return true;
  }
  if (x < 0 || y < 0 || x >= MAP_SIZE_X || y >= MAP_SIZE_Y) return true;
  return false;
}


//funkcja rysująca na ekranie poszczególne elementy
void draw(void) {
  oled.setFont(u8g2_font_ncenB14_tr);
  oled.setFontRefHeightExtendedText();
  oled.setFontPosTop();
  if (gameStatus == gamePlaying) {
    drawMap();
    //Wyświetlanie menu startu
  } else if (gameStatus == gameStart) {
    oled.setFont(u8g2_font_unifont_t_animals);
    oled.drawGlyph(25, 60, 0x002D);
    oled.drawGlyph(40, 60, 0x002D);
    oled.drawGlyph(55, 60, 0x002D);
    oled.drawGlyph(70, 60, 0x002D);
    oled.drawGlyph(85, 60, 0x002D);
    oled.setFont(u8g2_font_ncenB14_tr);
    oled.drawStr(25, 20, "SNAKE!!");
    oled.drawStr(0, 70, "Press to start");
    if (buttonPress()) {
      gameStatus = gamePlaying;
    }
    //Wyświetlanie menu końca i restartu
  } else {
    oled.drawStr(20, 20, "GameOver");
    oled.drawStr(25, 50, "Press to ");
    oled.drawStr(30, 70, "restart");
    resetSnake();
    if (buttonPress()) {
      gameStatus = gameStart;
    }
  }
}
//Resetowanie węża do stanu początkowego
void resetSnake() {
  dir = 6;
  newDir = 6;
  snake_delay = 200;
  score = 0;
  snake_length = STARTING_SNAKE_SIZE;
  for (int i = 0; i < snake_length; i++) {
    snake[i][0] = MAP_SIZE_X / 2 - i;
    snake[i][1] = MAP_SIZE_Y / 2;
  }
}
//Znajdowanie jabłka na swojej drodze, dodawanie punktu oraz wydawanie dzwięku zdobycia punktu
void checkFruit() {
  if (fruit[0] == snake[0][0] && fruit[1] == snake[0][1]) {
    if (snake_length + 1 <= MAX_SANKE_LENGTH)
      snake_length++;
    score++;
    tone(2, 1500, 100);
    if (snake_delay > 15) {
      snake_delay = snake_delay - 10;
    }
    generateFruit();
  }
}
//Generowanie jabłka w losowej cześci mapy
void generateFruit() {
  bool b = false;
  do {
    b = false;
    fruit[0] = random(3, MAP_SIZE_X - 3);
    fruit[1] = random(3, MAP_SIZE_Y - 3);
    for (int i = 0; i < snake_length; i++) {
      if (fruit[0] == snake[i][0] && fruit[1] == snake[i][1]) {
        b = true;
        continue;
      }
    }
  } while (b);
}
//Wykrywanie czy przycisk został nacisnięty
bool buttonPress() {
  for (byte i = 0; i < 4; i++) {
    byte buttonPin = buttonPins[i];
    if (digitalRead(buttonPin) == LOW) {
      return true;
    }
  }
  return false;
}
//Odczyt, który przycisk został nacisnięty oraz przypisanie nowego kierunku poruszania się
void readDirection() {
  if (digitalRead(4) == LOW && dir != 2) {
    newDir = 8;
  }
  if (digitalRead(6) == LOW && dir != 8) {
    newDir = 2;
  }
  if (digitalRead(5) == LOW && dir != 6) {
    newDir = 4;
  }
  if (digitalRead(3) == LOW && dir != 4) {
    newDir = 6;
  }
}

void drawMap() {
  int offsetMapX = SCREEN_WIDTH - SNAKE_PIECE_SIZE * MAP_SIZE_X - 2;
  int offsetMapY = 2;
  //rysowanie jabłka
  oled.drawDisc(fruit[0] * SNAKE_PIECE_SIZE + offsetMapX, fruit[1] * SNAKE_PIECE_SIZE + offsetMapY, SNAKE_PIECE_SIZE - 2);
  //rysowanie ramki mapy
  oled.drawFrame(offsetMapX - 2, 0, SNAKE_PIECE_SIZE * MAP_SIZE_X + 4, SNAKE_PIECE_SIZE * MAP_SIZE_Y + 4);


  //rysowanie węża
  for (int i = 0; i < snake_length; i++) {
    if (i == 0) {
      //głowa gdy porusza się w pionie
      if (dir == 8 || dir == 2) {
        oled.drawFilledEllipse(snake[i][0] * SNAKE_PIECE_SIZE + offsetMapX, snake[i][1] * SNAKE_PIECE_SIZE + offsetMapY, SNAKE_PIECE_SIZE - 2, SNAKE_PIECE_SIZE);
        oled.setDrawColor(0);
        oled.drawBox(snake[i][0] * SNAKE_PIECE_SIZE + offsetMapX - 2, snake[i][1] * SNAKE_PIECE_SIZE - 1 + offsetMapY, 1, 1);
        oled.drawBox(snake[i][0] * SNAKE_PIECE_SIZE + offsetMapX + 2, snake[i][1] * SNAKE_PIECE_SIZE - 1 + offsetMapY, 1, 1);
        oled.setDrawColor(1);
      }
      //głowa gdy porusza się w poziomie
      if (dir == 4 || dir == 6) {
        oled.drawFilledEllipse(snake[i][0] * SNAKE_PIECE_SIZE + offsetMapX, snake[i][1] * SNAKE_PIECE_SIZE + offsetMapY, SNAKE_PIECE_SIZE, SNAKE_PIECE_SIZE - 2);
        oled.setDrawColor(0);
        oled.drawBox(snake[i][0] * SNAKE_PIECE_SIZE + offsetMapX - 1, snake[i][1] * SNAKE_PIECE_SIZE - 2 + offsetMapY, 1, 1);
        oled.drawBox(snake[i][0] * SNAKE_PIECE_SIZE + offsetMapX - 1, snake[i][1] * SNAKE_PIECE_SIZE + 2 + offsetMapY, 1, 1);
        oled.setDrawColor(1);
      }
    } else
      //reszta ciała
      oled.drawDisc(snake[i][0] * SNAKE_PIECE_SIZE + offsetMapX, snake[i][1] * SNAKE_PIECE_SIZE + offsetMapY, SNAKE_PIECE_SIZE - 2);
  }
  //rysowanie licznika punktu
  oled.setFont(u8g2_font_6x10_tf);
  oled.drawStr(10, 85, "score:");
  oled.setCursor(50, 85);
  oled.print(score);
}
