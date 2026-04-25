#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

// LCD I2C Configuration (Address 0x27, 16 columns, 2 rows)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// KEYPAD Configuration (from your wiring)
const byte ROWS = 4;
const byte COLS = 3;

char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

// Wiring: Rows -> {9, 8, 7, 6}, Columns -> {A0, A1, A2}
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {A0, A1, A2};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

String inputCode = "";

void setup() {
  // Start Serial at 9600 baud for Qt communication
  Serial.begin(9600);

  // Initialize LCD
  lcd.init();
  lcd.backlight();

  showReadyMessage();
}

void loop() {
  // 1. Handle Keypad Input
  char key = keypad.getKey();

  if (key) {
    if (key == '*') { // RESET
      inputCode = "";
      showReadyMessage();
    } 
    else if (key != '#') { // SAISIE (Limité à 4 chiffres)
      if (inputCode.length() < 4) {
        inputCode += key;
        // Affiche le code sur la 2ème ligne
        lcd.setCursor(inputCode.length() - 1, 1);
        lcd.print(key);

        // ENVOI AUTOMATIQUE quand on atteint 4 chiffres
        if (inputCode.length() == 4) {
          delay(200); // Petit délai pour voir le dernier chiffre
          Serial.print(inputCode);
          Serial.print("#"); // Garde le délimiteur pour Qt
          
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Verification...");
          inputCode = ""; 
        }
      }
    }
  }

  // 2. Handle Response from Qt
  if (Serial.available() > 0) {
    char response = Serial.read();
    
    // Ignore les sauts de ligne
    if (response == '\n' || response == '\r') return;

    lcd.clear();
    lcd.setCursor(0, 0);

    if (response >= '1' && response <= '9') {
      lcd.print("ACCES ACCEPTE");
      lcd.setCursor(0, 1);
      lcd.print("QUAI No: ");
      lcd.print(response);
    } 
    else if (response == 'F') {
      lcd.print("PORT COMPLET");
      lcd.setCursor(0, 1);
      lcd.print("Pas de place");
    } 
    else if (response == 'E') {
      lcd.print("CODE INCORRECT");
      lcd.setCursor(0, 1);
      lcd.print("Acces Refuse");
    }
    else if (response == 'R') {
      lcd.print("DEJA AU PORT");
      lcd.setCursor(0, 1);
      lcd.print("Acces Refuse");
    }

    delay(2500); // Délai réduit pour plus de réactivité
    inputCode = "";
    showReadyMessage();
  }
}

void showReadyMessage() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Entrer Code:");
  lcd.setCursor(0, 1);
  lcd.print("____"); 
  lcd.setCursor(0, 1);
}
