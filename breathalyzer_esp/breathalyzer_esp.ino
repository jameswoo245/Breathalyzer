#define BUZZER 25
#define MQ3_PIN 34 // Analog pin for MQ-3 sensor
#define sober 120
#define drunk 400

const float K = 0.00048;

#include <SoftwareSerial.h>
SoftwareSerial arduino(10, 11);

void setup() {
  pinMode(BUZZER, OUTPUT);
  Serial.begin(115200);
  arduino.begin(9600);
}

void loop() {
  int sensorValue = analogRead(MQ3_PIN); // Read analog value from MQ-3
  float voltage = (sensorValue / 4095.0) * 5; // Convert to voltage (12-bit ADC for ESP32)

  // Approximate ppm calculation (replace A and B with your calibration constants)
  float ppm = 200; 
  // Estimate BAC
  float bac = ppm * K;

  Serial.print("MQ-3 Sensor Value: ");
  Serial.print(sensorValue);
  Serial.print(" | Voltage: ");
  Serial.print(voltage);
  Serial.print(" | BAC: ");
  Serial.println(bac);

  if (voltage > 1.0) { // Example threshold for activating buzzer
    tone(BUZZER, 1000); // Turn on the buzzer
  } 
  else {
    noTone(BUZZER);
  }
  delay(1000); // Wait for 1 second

}
