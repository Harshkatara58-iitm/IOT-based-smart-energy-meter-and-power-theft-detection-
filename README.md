# Smart Energy Meter and Power Theft Detection System ⚡

An IoT-based smart energy monitoring and automated power theft detection system designed to eliminate technical and commercial losses.

## Features
* Real-time dual-point power auditing (Source vs Load)
* Instant anomaly & power theft detection
* Automated relay power cutoff
* Blynk Cloud integration, live gauges, and instant event alerts

## Tech Stack
* **Microcontroller:** ESP32
* **Sensors:** PZEM-004T Modules
* **Cloud Platform:** Blynk IoT

## 🔌 Hardware Connections & Pinout (ESP32)

| Component | Component Pin | ESP32 GPIO Pin | Description |
| :--- | :--- | :--- | :--- |
| **I2C LCD Display** | SDA | GPIO 21 | Data line for LCD |
| | SCL | GPIO 22 | Clock line for LCD |
| **Relay Module** | IN | GPIO 5 | Automated power cutoff signal |
| **Source PZEM-004T** | RX / TX | GPIO 16 / 17 | Serial2 communication (Grid side) |
| **Load PZEM-004T** | RX / TX | GPIO 32 / 33 | Serial1 communication (Home side) |
| **Indicators & Alert** | Red LED | GPIO 4 | Theft alert indicator |
| | Green LED | GPIO 2 | Normal state indicator |
| | Buzzer | GPIO 18 | Audible alarm trigger |

## ⚡ AC Mains Wiring (Relay Safety)
* **Live (Phase) Wire:** Main AC feed ko Relay ke **COM (Common)** terminal me connect karein.
* **Appliance/Load Wire:** Relay ke **NO (Normally Open)** terminal se wire nikal kar load (bulb/appliance) me dein.
* **Neutral Wire:** Direct load tak jaye bina relay ke connect hoga.
