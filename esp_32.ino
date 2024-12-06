#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h> // Include the ArduinoJson library

// WiFi Credentials
const char* ssid = "Master";
const char* password = "password123";


// Server Port
WiFiServer server(80);

// OLED Display Settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

volatile int tC = 0;
volatile int pC = 0;
static unsigned long previousTime = 0; // Stores the last time pC was incremented
unsigned long currentTime = millis(); // Current time

void setup() {
  Serial.begin(115200);

  // Start WiFi Access Point
  if (WiFi.softAP(ssid, password, 1, 0, 3)) {
    Serial.println("Access Point Started Successfully");
    Serial.print("SoftAP IP Address: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("Failed to Start Access Point");
  }
  Serial.println("Master WiFi Access Point Started");
  Serial.print("Master IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Start the server
  server.begin();

  // Initialize the OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("WARMING UP");
  display.display();
  delay(30000); // 30 sec warm up period
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("Slave Connected");
    while (client.connected()) {
      if (client.available()) {
        // Read until a newline to ensure full JSON payload is received
        String receivedData = client.readStringUntil('\n');
        Serial.println("Received from Slave: " + receivedData);

        // Parse the JSON data
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, receivedData);

        if (error) {
          Serial.println("JSON Parsing Failed: " + String(error.c_str()));  // Print error details
          display.clearDisplay();
          display.setCursor(0, 10);
          display.setTextSize(1);
          display.println("Error:");
          display.println("Invalid JSON");
          display.display();
          continue;
        }

        tC++;

        // Extract data from the JSON document
        int interval = doc["time"]; // FOR 2 SEC or 10 SEC PERIOD
        const char* mode = doc["Mode"]; // FOR PERIODICAL OR ON-DEMAND
        const char* bac = doc["Alcohol Level"];
        int distance = doc["distance"];
        float temperature = doc["temperature"];
        int photocellValue = doc["photocell"];

        // Adjust brightness based on photocell reading
        int brightness = (photocellValue > 2000) ? 255 : 50; // Bright or dim based on threshold
        // Adjust OLED brightness (contrast)
        display.ssd1306_command(SSD1306_SETCONTRAST);
        display.ssd1306_command((photocellValue > 900) ? 255 : 50); // Bright or dim based on threshold

        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.print("Interval: ");
        display.println(String(interval) + " sec");
        display.setCursor(0, 10);
        display.print("Report Mode: ");
        display.println(String(mode)); // REPORT MODE
        display.setCursor(0, 20);
        display.print("Alcohol Level: ");
        display.println(String(bac));
        display.setCursor(0, 30);
        display.print("Distance: ");
        display.println(String(distance) + " cm");
        display.setCursor(0, 40);
        display.print("Temp: ");
        display.println(String(temperature, 1) + " F");
        display.setCursor(0, 50);
        display.print("T->C:");
        display.print(tC);
        display.print(" C->P");

        // Check if the interval is 2 or 10 seconds and update pC accordingly
        currentTime = millis(); // Update the current time
        if (interval == 2) {
          if (currentTime - previousTime >= 2000) { // 2000 milliseconds = 2 seconds
            pC++; // Increment pC
            previousTime = currentTime; // Update the last increment time
          }
        } else {
          if (currentTime - previousTime >= 10000) { // 10000 milliseconds = 10 seconds
            pC++; // Increment pC
            previousTime = currentTime; // Update the last increment time
          }
        }

        display.println(pC);
        display.display();

        client.println("ACK: " + receivedData);
      }
    }
    client.stop();
    Serial.println("Slave Disconnected");
  }
}
