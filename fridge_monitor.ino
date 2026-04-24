#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// OLED Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// DHT Configuration (Single Sensor)
#define DHTPIN 2
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  dht.begin();

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(15, 20);
  display.println("PORTFLOW SYSTEM");
  display.setCursor(15, 35);
  display.println("SINGLE SENSOR MODE");
  display.display();
  delay(2000);
}

void loop() {
  float temp = dht.readTemperature();

  // Handle missing sensor/errors
  bool sensorError = false;
  if (isnan(temp)) {
    temp = 0.0;
    sensorError = true;
  }

  // --- SEND TO QT APPLICATION ---
  // Format: S1:<val>;
  Serial.print("S1:");
  Serial.print(temp);
  Serial.print(";");
  Serial.println(); 

  // --- UPDATE OLED DISPLAY ---
  display.clearDisplay();
  
  // Header
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("SURVEILLANCE FRIGO 1");
  display.drawLine(0, 10, 128, 10, WHITE);

  if (sensorError) {
    display.setCursor(0, 30);
    display.setTextSize(1);
    display.println("ERREUR CAPTEUR !");
    display.setCursor(0, 45);
    display.println("Verifiez Pin 2");
  } else {
    // Current Temperature
    display.setCursor(10, 25);
    display.setTextSize(1);
    display.println("TEMPERATEUR LIVE:");
    
    display.setCursor(25, 40);
    display.setTextSize(3); // Big font for single sensor
    display.print((int)temp);
    display.setTextSize(1);
    display.print(" .");
    display.print((int)((temp - (int)temp) * 10));
    display.setTextSize(2);
    display.print("C");
  }

  display.display();
  delay(2000); 
}
