#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// OLED Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// DHT Configuration
#define DHTPIN1 2
#define DHTPIN2 3 // Second sensor
#define DHTTYPE DHT11

DHT dht1(DHTPIN1, DHTTYPE);
DHT dht2(DHTPIN2, DHTTYPE);

void setup() {
  Serial.begin(9600);
  dht1.begin();
  dht2.begin();

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(20, 20);
  display.println("PORTFLOW SYSTEM");
  display.display();
  delay(2000);
}

void loop() {
  float temp1 = dht1.readTemperature();
  float temp2 = dht2.readTemperature();

  // Handle missing sensors/errors
  if (isnan(temp1)) temp1 = 0.0; 
  if (isnan(temp2)) temp2 = 0.0;

  // --- SEND TO QT APPLICATION ---
  // Format: S1:<val>;S2:<val>;
  Serial.print("S1:");
  Serial.print(temp1);
  Serial.print(";");
  
  Serial.print("S2:");
  Serial.print(temp2);
  Serial.print(";");
  Serial.println(); // Newline for clean serial monitor viewing

  // --- UPDATE OLED DISPLAY ---
  display.clearDisplay();
  
  // Header
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("MONITORING FRIGOS");
  display.drawLine(0, 10, 128, 10, WHITE);

  // Fridge 1
  display.setCursor(0, 20);
  display.print("F1 (S1): ");
  display.setTextSize(2);
  display.print(temp1);
  display.print("C");

  // Fridge 2
  display.setTextSize(1);
  display.setCursor(0, 45);
  display.print("F2 (S2): ");
  display.setTextSize(2);
  display.print(temp2);
  display.print("C");

  display.display();

  delay(2000); // Wait 2 seconds between updates
}
