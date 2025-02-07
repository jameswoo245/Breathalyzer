# 🍻 Breathalyzer System  

A portable **Blood Alcohol Concentration (BAC) Breathalyzer** using an **ESP32 microcontroller** and **MQ-3 alcohol sensor** to measure BAC levels in real-time. This project integrates multiple sensors for enhanced functionality and provides user-friendly feedback through an **OLED display**, LEDs, and a buzzer.  

## 🚀 Features  
- **Real-time BAC detection** using the **MQ-3 alcohol sensor**.  
- **Multi-sensor integration** including an ultrasonic distance sensor, temperature sensor, and photocell for dynamic OLED brightness adjustment.  
- **Wi-Fi communication** for JSON-based data transmission between master and slave ESP32 microcontrollers.  
- **User-friendly indicators** including an **OLED display, LED status lights, and a buzzer system**.  
- **Portable and battery-powered design** for easy use on the go.  

## 🛠️ Hardware Components  
- **ESP-WROOM-32** (ESP32 microcontroller)  
- **MQ-3 Alcohol Sensor**  
- **Ultrasonic Sensor** (for distance measurement)  
- **Temperature Sensor**  
- **Photocell** (for automatic OLED brightness adjustment)  
- **OLED Display** (for visual BAC level output)  
- **LED Indicators & Buzzer** (for feedback)  

## 🎯 Installation & Setup  
### 1️⃣ Clone the Repository  
```bash
git clone https://github.com/jameswoo245/Breathalyzer.git
cd Breathalyzer
```  

### 2️⃣ Install Dependencies  
Ensure you have the **Arduino IDE** installed along with the required libraries:  
- **ESP32 Board Package** (Install via Arduino Board Manager)  
- **Adafruit SSD1306** (For OLED display)  
- **MQUnifiedSensor** (For MQ-3 Alcohol Sensor)  
- **WiFi Library** (For ESP32 communication)  

### 3️⃣ Upload the Code  
1. Open the `.ino` file in **Arduino IDE**.  
2. Select the **ESP32 Board** under **Tools → Board Manager**.  
3. Connect the ESP32 via USB and **upload the sketch**.  

## 📷 Example Output  
### 📌 OLED Display  
```
BAC Level: 0.05%  
Status: Safe to drive 🚗  
```
### 📌 LED & Buzzer Feedback  
- **Green LED** (Low BAC, safe)  
- **Yellow LED** (Warning)  
- **Red LED + Buzzer** (High BAC, not safe)

![image](https://github.com/user-attachments/assets/9fcfe6f6-1df8-4294-b038-9fc7b98b20f9)
![image](https://github.com/user-attachments/assets/3c715541-a55b-4160-8e30-5a24559df0e9)
