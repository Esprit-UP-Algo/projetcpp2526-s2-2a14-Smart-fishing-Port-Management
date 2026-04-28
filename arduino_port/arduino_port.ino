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

// ================= SW-420 =================
const int vibrationPin = 2;
int lastState = LOW;
bool collisionActive = false;

void setup() {
  Serial.begin(9600);

  pinMode(vibrationPin, INPUT);

  lcd.init();
  lcd.backlight();

  showReadyMessage();
}

void loop() {

  // ================= SW-420 COLLISION LOGIC =================
  int vibrationState = digitalRead(vibrationPin);

  if (vibrationState == HIGH && lastState == LOW) {

    collisionActive = true;

    Serial.println("VIBRATION");

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("COLLISION !");
    lcd.setCursor(0,1);
    lcd.print("CHECK SYSTEM");

    delay(1500);

    showReadyMessage();
    inputCode = "";
  }

  lastState = vibrationState;

  if (collisionActive) {
    // still allow keypad but ignore input if needed
    collisionActive = false;
  }

  // ================= KEYPAD INPUT =================
  char key = keypad.getKey();

  if (key) {

    if (key == '*') {
      inputCode = "";
      showReadyMessage();
    }

    else if (key != '#') {

      if (inputCode.length() < 4) {
        inputCode += key;

        lcd.setCursor(inputCode.length() - 1, 1);
        lcd.print(key);

        if (inputCode.length() == 4) {
          delay(200);

          Serial.print(inputCode);
          Serial.print("#");

          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Verification...");

          inputCode = "";
        }
      }
    }
  }

  // ================= QT RESPONSE =================
  if (Serial.available() > 0) {
    char response = Serial.read();

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

    delay(2500);
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
}
