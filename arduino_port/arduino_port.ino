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
String serialBuffer = "";
const int buzzerPin = 11;
const int vibrationPin = 2;
const unsigned long SERIAL_COMMAND_TIMEOUT_MS = 30;
const unsigned long COLLISION_COOLDOWN_MS = 1500;

int selectedQuai = -1;
int lastVibrationState = LOW;
unsigned long serialBufferStartedAt = 0;
unsigned long lastCollisionAt = 0;

void showReadyMessage();
void processSerialMessage(String message);
void handleImmediateCommand(char command);
void handleVibrationSensor();

bool isImmediateSoundCommand(char command) {
  return command == 'A' || command == 'L' || command == 'S' || command == 'B' || command == 'E';
}

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
  } else if (response == 'C' || response == 'E') {
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

void showAccessGranted(int quaiNumber) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ACCES ACCEPTE");
  lcd.setCursor(0, 1);
  lcd.print("QUAI No: ");
  lcd.print(quaiNumber);
  delay(2500);
  inputCode = "";
  showReadyMessage();
}

void showSelectedQuai(int quaiNumber) {
  selectedQuai = quaiNumber;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Quai selected:");
  lcd.setCursor(0, 1);
  lcd.print(quaiNumber);
  delay(1200);
  showReadyMessage();
}

int parseLeadingNumber(const String& text, int startIndex) {
  String digits = "";

  for (int i = startIndex; i < text.length(); i++) {
    char c = text[i];
    if (c >= '0' && c <= '9') {
      digits += c;
    } else {
      break;
    }
  }

  return digits.length() > 0 ? digits.toInt() : -1;
}

void handleImmediateCommand(char command) {
  if (isImmediateSoundCommand(command)) {
    playSoundCommand(command);
  }
}

void processSerialMessage(String message) {
  message.trim();

  if (message.length() == 0) {
    return;
  }

  if (message.startsWith("Q")) {
    int quaiNumber = parseLeadingNumber(message, 1);
    if (quaiNumber != -1) {
      showSelectedQuai(quaiNumber);
    }
    return;
  }

  if (message.startsWith("A:")) {
    int quaiNumber = parseLeadingNumber(message, 2);
    if (quaiNumber != -1) {
      showAccessGranted(quaiNumber);
    }
    return;
  }

  if (message == "F" || message == "C" || message == "E" || message == "R") {
    showResponseMessage(message[0]);
    return;
  }

  if (message.length() == 1 && message[0] >= '1' && message[0] <= '9') {
    showResponseMessage(message[0]);
    return;
  }

  if (message.length() == 1 && isImmediateSoundCommand(message[0])) {
    playSoundCommand(message[0]);
    return;
  }

  if (message.startsWith("CODE:")) {
    message = message.substring(5);
  }

  if (message.endsWith("#")) {
    message.remove(message.length() - 1);
  }

  if (message.length() > 0) {
    Serial.print(message);
    Serial.print("#");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Verification...");
    inputCode = "";
  }
}

void handleSerialInput() {
  while (Serial.available() > 0) {
    char incoming = Serial.read();

    if (incoming == '\r') {
      continue;
    }

    if (serialBuffer.length() == 0) {
      serialBufferStartedAt = millis();
    }

    if (incoming == '\n') {
      processSerialMessage(serialBuffer);
      serialBuffer = "";
      serialBufferStartedAt = 0;
      continue;
    }

    serialBuffer += incoming;
  }

  if (serialBuffer.length() == 1 &&
      (millis() - serialBufferStartedAt) >= SERIAL_COMMAND_TIMEOUT_MS &&
      isImmediateSoundCommand(serialBuffer[0])) {
    handleImmediateCommand(serialBuffer[0]);
    serialBuffer = "";
    serialBufferStartedAt = 0;
  }
}

void handleVibrationSensor() {
  const int vibrationState = digitalRead(vibrationPin);
  const unsigned long now = millis();

  if (vibrationState == HIGH &&
      lastVibrationState == LOW &&
      selectedQuai != -1 &&
      (now - lastCollisionAt) >= COLLISION_COOLDOWN_MS) {
    lastCollisionAt = now;
    inputCode = "";

    Serial.print("MAINTENANCE_QUAI:");
    Serial.println(selectedQuai);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("IMPACT DETECTE");
    lcd.setCursor(0, 1);
    lcd.print("QUAI ");
    lcd.print(selectedQuai);

    errorAlert();
    delay(800);
    showReadyMessage();
  }

  lastVibrationState = vibrationState;
}

void handleKeypadInput() {
  char key = keypad.getKey();

  if (!key) {
    return;
  }

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
  } else if (inputCode.length() > 0) {
    Serial.print(inputCode);
    Serial.print("#");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Verification...");
    inputCode = "";
  }
}

void loop() {
  // 1. Handle Keypad Input
  handleKeypadInput();

  // 2. Handle Response from Qt
  handleSerialInput();

  // 3. Detect collisions on the berth sensor
  handleVibrationSensor();
}

void showReadyMessage() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Entrer Code:");
  lcd.setCursor(0, 1);
  lcd.print("____");
  lcd.setCursor(0, 1);
}
void setup() {
  // Start Serial at 9600 baud for Qt communication
  Serial.begin(9600);

  pinMode(buzzerPin, OUTPUT);
  pinMode(vibrationPin, INPUT);
  digitalWrite(buzzerPin, LOW);

  // Initialize LCD
  lcd.init();
  lcd.backlight();

  showReadyMessage();
}
