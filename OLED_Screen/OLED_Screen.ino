#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define RX_PIN 10
#define TX_PIN 11

SoftwareSerial mySerial(RX_PIN, TX_PIN);  // RX, TX

String BAC_Level = "";

void setup() {
  Serial.begin(115200);
  mySerial.begin(9600);  // Match the ESP32 baud rate

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("BAC: ");
  display.display();
}

void loop() {
  if (mySerial.available()) {
    // Read the entire line until newline character
    String receivedData = mySerial.readStringUntil('\n');

    // Print the full received line to the Serial Monitor for debugging
    Serial.print("Received full line: '");
    Serial.print(receivedData);
    Serial.println("'");

    // Update OLED with the received data
    display.clearDisplay();
    display.setCursor(0, 10);
    display.setTextSize(1);
    display.println("BAC: ");
    display.setCursor(0, 30);
    display.println(receivedData);
    display.display();
  }
}
