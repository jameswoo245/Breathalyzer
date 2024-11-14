#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Set up SoftwareSerial to communicate with the ESP32
SoftwareSerial mySerial(10, 11); // RX = pin 10, TX = pin 11


String BAC_Level = "0.00";

void setup() {
  Serial.begin(9600); // Previously 115200

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();
}

void loop() {
  if (mySerial.available()) {
    BAC_Level = mySerial.readStringUntil('\n'); // Read the BAC level from ESP32
    Serial.print("Received BAC Level from ESP32: ");
    Serial.println(BAC_Level);

    // Display the BAC Level on OLED
    display.clearDisplay();
    display.setTextSize(3);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.println("BAC:");
    display.setCursor(0, 40);
    display.println(BAC_Level);
    display.display();
  }

  delay(1000); // update every second
}

