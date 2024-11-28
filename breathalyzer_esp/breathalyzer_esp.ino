#include <WiFi.h>
#include <HTTPClient.h>

#define BUZZER 25
#define MQ3_PIN 34 // Analog pin for MQ-3 sensor
#define TMP_PIN 35
const int buttonPin = 13;

const int trigPin = 26; // Ultrasonic sensor trig pin
const int echoPin = 27; // Ultrasonic sensor echo pin

volatile bool buttonPressed = false;
bool isSensorActive = false;
unsigned long startTime = 0; 
const unsigned long activeDuration = 7000; 
const unsigned long sensorInterval = 10000; // 10 seconds interval
unsigned long lastSensorReadTime = 0; // Timestamp of the last sensor reading

float bacSum = 0.0;        // Sum of BAC readings
int bacReadCount = 0;      // Count of BAC readings

// Wi-Fi credentials
const char* ssid = "J";
const char* password = "brokencl9";

// Server URL for posting data
const char* serverName = "http://httpbin.org/post"; // Replace with your server endpoint

// Use hardware UART2 on ESP32
HardwareSerial arduino(2); // UART2 for communication with Arduino Uno

void setup() {
  Serial.begin(115200);    // Serial Monitor for debugging
  arduino.begin(9600, SERIAL_8N1, 16, 17); // Use RX2 (GPIO16), TX2 (GPIO17) for UART communication
  pinMode(BUZZER, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP); 
  pinMode(trigPin, OUTPUT); // Ultrasonic sensor trig pin
  pinMode(echoPin, INPUT);  // Ultrasonic sensor echo pin
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonISR, FALLING); 

  // Connect to Wi-Fi
  connectToWiFi();
}

void loop() {
  // Check if it's time to read the ultrasonic and temperature sensors
  if (millis() - lastSensorReadTime >= sensorInterval) {
    lastSensorReadTime = millis();
    int distance = readUltrasonicSensor();
    float temperatureF = readTemperatureSensor();
    sendDataToServer(distance, temperatureF);
  }

  // If the sensor is active, read the alcohol sensor until the duration ends
  if (isSensorActive) {
    if (millis() - startTime <= activeDuration) {
      buzz();     // Activate the buzzer while reading
      readAlc();  // Read alcohol sensor data and accumulate BAC
    } else {
      noTone(BUZZER);          // Turn off buzzer when done reading
      calculateAndSendAverageBAC(); // Calculate and send average BAC via UART
      standbySensor();         // Deactivate the sensor after the duration
    }
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

int readUltrasonicSensor() {
  // Send a pulse to the ultrasonic sensor
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Measure the time of flight
  long duration = pulseIn(echoPin, HIGH);
  int distance = duration * 0.034 / 2; // Convert to distance in cm

  Serial.print("Ultrasonic Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Send distance to Arduino Uno
  arduino.println("DIST:" + String(distance));
  delay(10);

  return distance;
}

float readTemperatureSensor() {
  int sensorValue = analogRead(TMP_PIN); // Read analog value from TMP36
  float voltage = (sensorValue / 1023.0) * 5.0; // Convert to voltage (10-bit ADC, 5V reference)
  float temperatureC = (voltage - 0.5) * 100.0; // Convert voltage to temperature in Celsius
  float temperatureF = (temperatureC * 9.0 / 5.0) + 32.0; // Convert Celsius to Fahrenheit

  Serial.print("Temperature: ");
  Serial.print(temperatureF);
  Serial.println(" Â°F");

  // Send temperature to Arduino Uno
  arduino.println("TEMP:" + String(temperatureF, 2));
  delay(10);

  return temperatureF;
}

void readAlc() {
  int sensorValue = analogRead(MQ3_PIN); 
  float voltage = (sensorValue / 4095.0) * 5.0; 
  float bac = map(sensorValue, 200, 4000, 0, 500) / 1000.0; 

  bacSum += bac - 0.02;
  bacReadCount++;

  Serial.print("MQ-3 Sensor Value: ");
  Serial.print(sensorValue);
  Serial.print(" | Voltage: ");
  Serial.print(voltage, 2);
  Serial.print("V | Current BAC: ");
  Serial.println(bac - 0.02, 3);
}

void calculateAndSendAverageBAC() {
  if (bacReadCount > 0) {
    float averageBAC = bacSum / bacReadCount;
    Serial.print("Average BAC during active period: ");
    Serial.println(averageBAC, 2);

    if (averageBAC < 0) averageBAC *= -1;

    // Send average BAC to Arduino Uno
    arduino.println("AVG:" + String(averageBAC, 2));
    Serial.println("Sent to Uno: " + String(averageBAC, 2));

    bacSum = 0.0;
    bacReadCount = 0;
  } else {
    Serial.println("No BAC readings recorded.");
    arduino.println("No BAC readings recorded.");
  }
}

void sendDataToServer(int distance, float temperatureF) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    http.begin(serverName); // Specify server URL
    http.addHeader("Content-Type", "application/json");

    // Create JSON payload
    String payload = "{";
    payload += "\"distance\":" + String(distance) + ",";
    payload += "\"temperature\":" + String(temperatureF, 2);
    payload += "}";

    int httpResponseCode = http.POST(payload);

    // Print the response code
    if (httpResponseCode > 0) {
      Serial.print("HTTP Response Code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error sending data. Code: ");
      Serial.println(httpResponseCode);
    }

    http.end(); // Free resources
  } else {
    Serial.println("Wi-Fi not connected. Cannot send data.");
  }
}

void activateSensor() {
  isSensorActive = true;
  startTime = millis(); // Record the current time
  bacSum = 0.0;         // Reset sum
  bacReadCount = 0;     // Reset count
  Serial.println("Sensor Activated");
}

void standbySensor() {
  isSensorActive = false;
  Serial.println("Sensor in Standby");
}

void buttonISR() {
  if (!isSensorActive) {
    activateSensor(); // Activate sensor on button press
  }
}

void buzz() {
  tone(BUZZER, 500); // Generate a 500Hz tone
}
