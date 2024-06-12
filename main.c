#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128  
#define SCREEN_HEIGHT 64  

#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* colors[] = { "W", "R", "B", "G" }; // White, Red, Blue, Green
const char* randomCode[4];
const int numColors = 4;
const int numCodes = 4;  

const int buttonPins[] = { 0, 4, 8, 12 };
const int redPins[] = { 1, 5, 9, 13 };
const int greenPins[] = { 2, 6, 10, A0 };
const int bluePins[] = { 3, 7, 11, A1 };
const int submitPin = A3;

struct LEDState {
  int red;
  int green;
  int blue;
};


LEDState validationDesLeds[4];


int buttonStates[4] = { 0, 0, 0, 0 };
int lastButtonStates[4] = { 0, 0, 0, 0 };
int ledStates[4] = { 0, 0, 0, 0 };  // Stores the current color state of each LED

void setup() {
  Serial.println("Computer's chosen colors:");
  for (int i = 0; i < numCodes; i++) {
    int randomIndex = random(numColors);
    randomCode[i] = colors[randomIndex];
    Serial.print(randomCode[i]);
    Serial.print(" ");
  }
  Serial.println();

  // OLED display initialization
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  display.clearDisplay();
  display.setTextSize(1);               
  display.setTextColor(SSD1306_WHITE);  
  display.setCursor(0, 0);              
  display.display();                    
  delay(2000);                          
  randomSeed(analogRead(0));            

  for (int i = 0; i < 4; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
    pinMode(redPins[i], OUTPUT);
    pinMode(greenPins[i], OUTPUT);
    pinMode(bluePins[i], OUTPUT);
  }
  pinMode(submitPin, INPUT_PULLUP);

  // Turn on LEDs in white at startup
  for (int i = 0; i < 4; i++) {
    SetColor(i, 1, 1, 1);
    validationDesLeds[i] = { 1, 1, 1 };  // Initialisation avec couleur blanche
  }
}

void SetColor(int ledIndex, int r, int g, int b) {
  digitalWrite(redPins[ledIndex], r == 1 ? HIGH : LOW);
  digitalWrite(greenPins[ledIndex], g == 1 ? HIGH : LOW);
  digitalWrite(bluePins[ledIndex], b == 1 ? HIGH : LOW);

  validationDesLeds[ledIndex].red = r;
  validationDesLeds[ledIndex].green = g;
  validationDesLeds[ledIndex].blue = b;
}

void BlinkCurrentColors() {
  LEDState currentColors[4];
  for (int i = 0; i < 4; i++) {
    currentColors[i] = validationDesLeds[i];
  }

  for (int i = 0; i < 5; i++) {  // Clignotement 5 fois
    for (int j = 0; j < 4; j++) {
      SetColor(j, 0, 0, 0);  //toutes les leds sont éteintes
    }
    delay(500); 
    for (int j = 0; j < 4; j++) {
      SetColor(j, currentColors[j].red, currentColors[j].green, currentColors[j].blue);  // Restore original color
    }
    delay(500); 
  }
}

bool isValidColor(const char* color) {
  for (int i = 0; i < numColors; i++) {
    if (strcmp(color, colors[i]) == 0) {
      return true;
    }
  }
  return false;
}

void loop() {
  display.clearDisplay();
  display.setCursor(0, 20);  


  bool playerWon = false;
  while (!playerWon) {

    for (int i = 0; i < 4; i++) {
      buttonStates[i] = digitalRead(buttonPins[i]);
      if (buttonStates[i] != lastButtonStates[i]) {
        if (buttonStates[i] == LOW) {
          ledStates[i] = (ledStates[i] + 1) % 4;  
          switch (ledStates[i]) {
            case 0: SetColor(i, 1, 1, 1); break; // White
            case 1: SetColor(i, 1, 0, 0); break;  // Red
            case 2: SetColor(i, 0, 0, 1); break;  // Blue
            case 3: SetColor(i, 0, 1, 0); break;  // Green
          }
        }
        delay(50); 
      }
      lastButtonStates[i] = buttonStates[i];
    }

    if (digitalRead(submitPin) == LOW) {
      delay(50); 

      int validOrNot[4] = {0, 0, 0, 0};

      bool allCorrect = true;
      for (int i = 0; i < numCodes; i++) {
        if (strcmp(randomCode[i], colors[ledStates[i]]) == 0) {
          validOrNot[i] = 1;  // Correct color and position (R)
        } else {
          bool found = false;
          for (int j = 0; j < numCodes; j++) {
            if (i != j && strcmp(randomCode[j], colors[ledStates[i]]) == 0) {
              found = true;
              break;
            }
          }
          validOrNot[i] = found ? 2 : 0;  // 2 pour bonne couleur mais mauvaise position (W), 0 pour incorrect
          allCorrect = false;
        }
      }


      display.clearDisplay();
      display.setCursor(0, 0);
      for (int i = 0; i < numCodes; i++) {
        if (validOrNot[i] == 1) {
          display.print("R ");  // Bonne couleur et position
        } else if (validOrNot[i] == 2) {
          display.print("W ");  // Bonne couleur mais mauvaise position
        } else {
          display.print("_ ");  // Couleur incorrecte et position incorrecte
        }
      }
      display.display();  // Mettre à jour l'affichage

      // Les LED ne changent pas après la validation, elles restent dans l'état sélectionné

      // Vérifier si le joueur a trouvé toutes les positions et couleurs correctes
      if (allCorrect) {
        playerWon = true;  // Le joueur a gagné
        BlinkCurrentColors(); 
      }
    }
  }
}