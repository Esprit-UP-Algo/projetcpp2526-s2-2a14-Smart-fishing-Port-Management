#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int vibrationPin = 2;
const int buzzerPin = 11;

int selectedQuaiId = -1;
int selectedQuaiLabel = -1;
int lastVibrationState = LOW;
bool maintenanceMode = false;

void showWaitingMessage()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waiting quai");
}

void alarmBeep()
{
  for (int i = 0; i < 3; i++) {
    digitalWrite(buzzerPin, HIGH);
    delay(300);
    digitalWrite(buzzerPin, LOW);
    delay(300);
  }
}

void setup()
{
  Serial.begin(9600);

  pinMode(vibrationPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  lcd.init();
  lcd.backlight();

  showWaitingMessage();
}

void loop()
{
  if (Serial.available()) {
    String msg = Serial.readStringUntil('\n');
    msg.trim();

    if (msg.startsWith("Q")) {
      int separatorIndex = msg.indexOf(':');
      if (separatorIndex > 1) {
        selectedQuaiId = msg.substring(1, separatorIndex).toInt();
        selectedQuaiLabel = msg.substring(separatorIndex + 1).toInt();
      } else {
        selectedQuaiId = msg.substring(1).toInt();
        selectedQuaiLabel = selectedQuaiId;
      }
      maintenanceMode = false;

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Selected Quai:");
      lcd.setCursor(0, 1);
      lcd.print(selectedQuaiLabel);

      Serial.print("QUAI_SELECTED:");
      Serial.println(selectedQuaiLabel);
    }
  }

  const int vibrationState = digitalRead(vibrationPin);

  if (vibrationState == HIGH && lastVibrationState == LOW && selectedQuaiId != -1 && !maintenanceMode) {
    maintenanceMode = true;

    Serial.print("MAINTENANCE_QUAI:");
    Serial.println(selectedQuaiId);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("QUAI ");
    lcd.print(selectedQuaiLabel);
    lcd.setCursor(0, 1);
    lcd.print("MAINTENANCE");

    alarmBeep();

    selectedQuaiId = -1;
    selectedQuaiLabel = -1;
    showWaitingMessage();
  }

  lastVibrationState = vibrationState;
}
