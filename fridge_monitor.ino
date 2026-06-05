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

bool isDanger1 = false;
bool isDanger2 = false;

void loop() {
  // --- READ COMMANDS FROM QT ---
  while (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd == "D1") isDanger1 = true;
    else if (cmd == "D2") isDanger2 = true;
    else if (cmd == "OK1") isDanger1 = false;
    else if (cmd == "OK2") isDanger2 = false;
    else if (cmd == "OK") { isDanger1 = false; isDanger2 = false; }
  }

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
  Serial.println(); 

  // --- UPDATE OLED DISPLAY ---
  display.clearDisplay();
  
  // 1. TOP ALERT BANNER / HEADER
  if (isDanger1 || isDanger2) {
    display.fillRect(0, 0, 128, 11, WHITE);
    display.setTextColor(BLACK);
    display.setTextSize(1);
    display.setCursor(28, 2);
    display.print("!!! ALERT !!!");
    display.setTextColor(WHITE);
  } else {
    display.setTextSize(1);
    display.setCursor(25, 2);
    display.print("FRIGO MONITOR");
    display.drawLine(0, 11, 128, 11, WHITE);
  }

  // Vertical Divider Line (Starts below header)
  display.drawLine(64, 12, 64, 64, WHITE);

  // --- FRIGO 1 (LEFT SIDE) ---
  display.setTextSize(1);
  display.setCursor(5, 15);
  display.print("FRIGO 1");
  
  display.setCursor(5, 30);
  display.setTextSize(2);
  display.print(temp1, 0); // Round to integer for cleaner look
  display.setTextSize(1);
  // Manual Degree Symbol Fix
  int currentX = display.getCursorX();
  int currentY = display.getCursorY();
  display.drawCircle(currentX + 2, currentY + 2, 1, WHITE); 
  display.setCursor(currentX + 5, currentY);
  display.print("C");
  
  display.setCursor(5, 52);
  if (isDanger1) {
    display.setTextColor(BLACK, WHITE);
    display.print(" DANGER ");
    display.setTextColor(WHITE);
  } else {
    display.print("  SAFE  ");
  }

  // --- FRIGO 2 (RIGHT SIDE) ---
  display.setTextSize(1);
  display.setCursor(69, 15);
  display.print("FRIGO 2");
  
  display.setCursor(69, 30);
  display.setTextSize(2);
  display.print(temp2, 0);
  display.setTextSize(1);
  // Manual Degree Symbol Fix
  currentX = display.getCursorX();
  currentY = display.getCursorY();
  display.drawCircle(currentX + 2, currentY + 2, 1, WHITE); 
  display.setCursor(currentX + 5, currentY);
  display.print("C");
  
  display.setCursor(69, 52);
  if (isDanger2) {
    display.setTextColor(BLACK, WHITE);
    display.print(" DANGER ");
    display.setTextColor(WHITE);
  } else {
    display.print("  SAFE  ");
  }

  display.display();
  delay(1500); 
}
