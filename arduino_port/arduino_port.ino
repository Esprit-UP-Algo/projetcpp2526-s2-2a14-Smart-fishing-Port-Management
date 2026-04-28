#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C* lcd = nullptr;

const int vibrationPin = 2;
const int buzzerPin = 11;
const int greenLedPin = 10;  // LED that lights on collision

int selectedQuaiId = -1;     // Real DB key: IDQUAI
int selectedQuaiLabel = -1;  // Friendly label: 1,2,3... for LCD
int lastVibrationState = LOW;
bool maintenanceMode = false;

static uint8_t detectLcdAddress()
{
  // Common backpack addresses are 0x27 and 0x3F. We scan the bus and pick one.
  for (uint8_t addr = 1; addr < 127; ++addr) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      if (addr == 0x27 || addr == 0x3F) {
        return addr;
      }
    }
  }
  return 0;
}

static void showWaitingMessage()
{
  if (!lcd) return;
  lcd->clear();
  lcd->setCursor(0, 0);
    lcd->print("Waiting quai");
}

static void alarmBeep()
{
  // Single short beep on collision
  digitalWrite(buzzerPin, HIGH);
  delay(120);
  digitalWrite(buzzerPin, LOW);
}

void setup()
{
  Serial.begin(9600);
  Wire.begin();

  pinMode(vibrationPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);
  digitalWrite(greenLedPin, LOW);

  delay(100);
  const uint8_t lcdAddr = detectLcdAddress();
  if (lcdAddr == 0) {
    Serial.println("LCD_I2C_NOT_FOUND");
  } else {
    Serial.print("LCD_I2C_ADDR:0x");
    Serial.println(lcdAddr, HEX);
    lcd = new LiquidCrystal_I2C(lcdAddr, 16, 2);
    // Some LiquidCrystal_I2C variants prefer begin(), others init(). We can safely call init().
    lcd->init();
    lcd->backlight();
    delay(50);
    showWaitingMessage();
  }

  if (!lcd) {
    // Still allow the rest of the system to run; just give a small beep to signal LCD failure.
    digitalWrite(buzzerPin, HIGH);
    delay(120);
    digitalWrite(buzzerPin, LOW);
  }
}

void loop()
{
  String msg = "";
  bool hasNewQuaiSelection = false;

  if (Serial.available()) {
    msg = Serial.readStringUntil('\n');
    msg.trim();

    // Expected: "Q<IDQUAI>:<label>" (example: Q12:1)
    if (msg.startsWith("Q")) {
      int separatorIndex = msg.indexOf(':');
      if (separatorIndex > 1) {
        selectedQuaiId = msg.substring(1, separatorIndex).toInt();
        selectedQuaiLabel = msg.substring(separatorIndex + 1).toInt();
      } else {
        selectedQuaiId = msg.substring(1).toInt();
        selectedQuaiLabel = selectedQuaiId;
      }

      hasNewQuaiSelection = true;
      maintenanceMode = false;
      // Turn OFF LED when new quai is selected (maintenance resolved)
      digitalWrite(greenLedPin, LOW);

      if (lcd) {
        lcd->clear();
        lcd->setCursor(0, 0);
        lcd->print("Selected Quai:");
        lcd->setCursor(0, 1);
        lcd->print(selectedQuaiLabel);
      }

      Serial.print("QUAI_SELECTED:");
      Serial.println(selectedQuaiLabel);
    }
  }

  const int vibrationState = digitalRead(vibrationPin);

  if (vibrationState == HIGH && lastVibrationState == LOW && selectedQuaiId != -1 && !maintenanceMode) {
    maintenanceMode = true;

    Serial.print("MAINTENANCE_QUAI:");
    Serial.println(selectedQuaiId);

    if (lcd) {
      lcd->clear();
      lcd->setCursor(0, 0);
      lcd->print("QUAI ");
      lcd->print(selectedQuaiLabel);
      lcd->setCursor(0, 1);
      lcd->print("MAINTENANCE");
    }

    // Turn ON green LED when collision detected
    digitalWrite(greenLedPin, HIGH);

    alarmBeep();

    selectedQuaiId = -1;
    selectedQuaiLabel = -1;
    showWaitingMessage();
  }

  lastVibrationState = vibrationState;
}
