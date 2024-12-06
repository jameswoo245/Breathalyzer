#include <WiFi.h>
#include <ArduinoJson.h> // Include the ArduinoJson library

// Pins
#define BUZZER 25
#define MQ3_PIN 34 // Analog pin for MQ-3 sensor
#define TMP_PIN 35
#define freqSwitch 15
#define PHOTO_PIN 32


const int buttonPin = 13;
const int trigPin = 23; // Ultrasonic sensor trig pin
const int echoPin = 22; // Ultrasonic sensor echo pin

const int redLed = 2;    // Red LED pin
const int greenLed = 4;  // Green LED pin

volatile bool isPeriodicMode = true; // Initially enabled

// Wi-Fi credentials
const char* ssid = "Master";
const char* password = "password123";

// Master ESP32 IP and port
const char* masterIP = "192.168.4.1"; // Update with the master ESP32's IP
const int masterPort = 80;

// WiFi Client
WiFiClient client;

// Sensor variables
volatile bool buttonPressed = false;
bool isSensorActive = false;
unsigned long startTime = 0;
const unsigned long activeDuration = 7000;
unsigned long sensorInterval = 10000; // 10 seconds interval
unsigned long lastSensorReadTime = 0;

// Global variables for distance and temperature
int globalDistance = -1; // -1 indicates no valid reading yet
float globalTemperature = -1.0; // -1 indicates no valid reading yet
int globalphotocellValue = -1;
const int maxAllowedDistance = 200; // Maximum distance allowed for on-demand mode (in cm)
const float maxAllowedTemperature = 100.0; // Maximum temperature allowed for on-demand mode (in Â°F)

float bacSum = 0.0;
int bacReadCount = 0;

// Functions declarations
void connectToWiFi();
void connectToMaster();
void sendDataToMaster(float bac, int distance, float temperatureF, int globalphotocellValue);
void activateSensor();
void standbySensor();
void buzz();
void readAlc();
String calculateAndSendAverageBAC();
int readUltrasonicSensor();
float readTemperatureSensor();
void buttonISR();
String mode;

void setup() {
  Serial.begin(115200);
  pinMode(freqSwitch, INPUT);

  pinMode(BUZZER, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(PHOTO_PIN, INPUT);

  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonISR, FALLING);

  connectToWiFi();
}

void loop() {
  // Update the global distance and temperature variables
  globalDistance = readUltrasonicSensor();
  globalTemperature = readTemperatureSensor();
  globalphotocellValue = readPhotocellSensor();


  // Check for commands from the master
  if (client.available()) {
    String receivedData = client.readStringUntil('\n');
    Serial.println("Received from Master: " + receivedData);

    // Parse the received JSON data
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, receivedData);

    if (!error) {
      if (doc.containsKey("PeriodicMode")) {
        const char* periodicMode = doc["PeriodicMode"];
        isPeriodicMode = strcmp(periodicMode, "Enabled") == 0;
      }
    }
  }

  int switchState = digitalRead(freqSwitch);
  if (switchState == HIGH) {
    sensorInterval = 10000; // 10-second interval
  } else {
    sensorInterval = 2000;  // 2-second interval
  }

  // If the button is pressed (sensor active), read and send data immediately
  if (isSensorActive) {
    if (millis() - startTime >= activeDuration) {
      // Stop on-demand processing after active duration
      isSensorActive = false;
      noTone(BUZZER); // Stop the buzzer
      Serial.println("On Demand Mode Ended. Returning to Periodical Mode.");
    } else {
      // Perform immediate sensor readings and send data
      mode = "On Demand";
      buzz();
      readAlc();
      String bac = calculateAndSendAverageBAC();
      sendDataToMaster(bac, globalDistance, globalTemperature,globalphotocellValue);
      return; // Skip periodic processing while in on-demand mode
    }
  }

  if (isPeriodicMode && millis() - lastSensorReadTime >= sensorInterval) {
    lastSensorReadTime = millis();
    mode = "Periodical";
    readAlc();
    // Perform periodic sensor readings and send data
    String bac = calculateAndSendAverageBAC();
    sendDataToMaster(bac, globalDistance, globalTemperature, globalphotocellValue);
  }
}

void connectToWiFi() {
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nConnected to Wi-Fi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void connectToMaster() {
  if (!client.connected()) {
    Serial.println("Connecting to Master...");
    for (int attempt = 0; attempt < 3; attempt++) { // Retry up to 3 times
      if (client.connect(masterIP, masterPort)) {
        Serial.println("Connected to Master!");
        return;
      } else {
        Serial.println("Failed to connect to Master. Retrying...");
        delay(500); // Short delay before retrying
      }
    }
    Serial.println("Failed to connect to Master after 3 attempts.");
  }
}

void sendDataToMaster(String bac, int distance, float temperatureF, int photocellValue) {
  connectToMaster();
  if (bac == "HI") {
    digitalWrite(redLed, HIGH);   // Turn on red LED
    digitalWrite(greenLed, LOW);  // Turn off green LED
  } else {
    digitalWrite(greenLed, HIGH); // Turn on green LED
    digitalWrite(redLed, LOW);    // Turn off red LED
  }

 if (client.connected()) {
    // Create and send JSON data
    String payload = "{";
    payload += "\"Mode\":\"" + mode + "\",";
    payload += "\"Alcohol Level\":\"" + bac + "\",";
    payload += "\"distance\":" + String(distance) + ",";
    payload += "\"temperature\":" + String(temperatureF, 2) + ",";
    payload += "\"photocell\":" + String(photocellValue) + ",";
    payload += "\"time\":" + String(sensorInterval/1000);
    payload += "}";

    client.println(payload);
    Serial.println("Data sent to Master: " + payload);

  } else {
    Serial.println("Cannot send data, Master not connected.");
  }
}

void activateSensor() {
  if (globalDistance > 0 && globalDistance <= maxAllowedDistance && globalTemperature >= 0 && globalTemperature <= maxAllowedTemperature) {
    isSensorActive = true;
    startTime = millis();
    bacSum = 0.0;
    bacReadCount = 0;
    Serial.println("Sensor Activated for On-Demand Reading.");
  } else {
    Serial.println("On-Demand Activation Failed: Distance or Temperature Out of Range.");
  }
}

void standbySensor() {
  isSensorActive = false;
  Serial.println("Sensor in Standby");
}

void buzz() {
  tone(BUZZER, 500); // Generate a 500Hz tone
}

void readAlc() {
  int sensorValue = analogRead(MQ3_PIN);
  bacSum += sensorValue;
  bacReadCount++;

  Serial.print("MQ-3 Sensor Value: ");
  Serial.print(sensorValue);
  Serial.println();
}

String calculateAndSendAverageBAC() {
  float averageBAC = bacSum / bacReadCount;
  bacSum = 0.0;
  bacReadCount = 0;

  if (averageBAC > 800)
    return "HI";
  else
    return "LO";
}

int readUltrasonicSensor() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000); // Timeout after 30 ms

  if (duration == 0) {
    return -1;
  }
  int distance = (duration * 0.0343) / 2;
  if (distance < 2 || distance > 400) {
    return -1; // Invalid readings
  }
  return distance;
}

float readTemperatureSensor() {
  int sensorValue = analogRead(TMP_PIN);
  float voltage = (sensorValue / 4095.0) * 3.3;
  float temperatureC = (voltage - 0.5) * 100.0;
  float temperatureF = (temperatureC * 9.0 / 5.0) + 32.0;

  // Ensure only positive values are returned
  return (temperatureF >= 0) ? temperatureF : 0;
}

int readPhotocellSensor()
{
  return analogRead(PHOTO_PIN);
}

void buttonISR() {
  activateSensor();
}
