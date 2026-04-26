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
const int buzzerPin = 11;

void showReadyMessage();

void boatArrival() {
  tone(buzzerPin, 500);
  delay(150);
  tone(buzzerPin, 800);
  delay(150);
  tone(buzzerPin, 1100);
  delay(200);
  noTone(buzzerPin);
}

void boatDeparture() {
  tone(buzzerPin, 1100);
  delay(150);
  tone(buzzerPin, 800);
  delay(150);
  tone(buzzerPin, 500);
  delay(200);
  noTone(buzzerPin);
}

void dockingSuccess() {
  tone(buzzerPin, 700);
  delay(120);
  tone(buzzerPin, 1000);
  delay(180);
  noTone(buzzerPin);
}

void errorAlert() {
  for (int i = 0; i < 3; i++) {
    tone(buzzerPin, 2000);
    delay(100);
    noTone(buzzerPin);
    delay(80);
  }
}

void playSoundCommand(char command) {
  if (command == 'A') {
    boatArrival();
  } else if (command == 'L') {
    boatDeparture();
  } else if (command == 'S' || command == 'B') {
    dockingSuccess();
  } else if (command == 'E') {
    errorAlert();
  }
}

void showResponseMessage(char response) {
  lcd.clear();
  lcd.setCursor(0, 0);

  if (response >= '1' && response <= '9') {
    lcd.print("ACCES ACCEPTE");
    lcd.setCursor(0, 1);
    lcd.print("QUAI No: ");
    lcd.print(response);
  } else if (response == 'F') {
    lcd.print("PORT COMPLET");
    lcd.setCursor(0, 1);
    lcd.print("Pas de place");
  } else if (response == 'C') {
    lcd.print("CODE INCORRECT");
    lcd.setCursor(0, 1);
    lcd.print("Acces Refuse");
  } else if (response == 'R') {
    lcd.print("DEJA AU PORT");
    lcd.setCursor(0, 1);
    lcd.print("Acces Refuse");
  } else {
    return;
  }

  delay(2500);
  inputCode = "";
  showReadyMessage();
}

void handleSerialCommand(char command) {
  if (command == '\n' || command == '\r') {
    return;
  }

  if (command == 'A' || command == 'L' || command == 'S' || command == 'B' || command == 'E') {
    playSoundCommand(command);
    return;
  }

  if (command == 'F' || command == 'C' || command == 'R') {
    showResponseMessage(command);
    return;
  }

  if (command >= '1' && command <= '9') {
    showResponseMessage(command);
  }
}

void setup() {
  // Start Serial at 9600 baud for Qt communication
  Serial.begin(9600);

  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

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
    } else if (key != '#') {
      if (inputCode.length() < 4) {
        inputCode += key;
        lcd.setCursor(inputCode.length() - 1, 1);
        lcd.print(key);

        if (inputCode.length() == 4) {
          delay(200);
          Serial.print(inputCode);
          Serial.print("#");

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
    handleSerialCommand(Serial.read());
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
