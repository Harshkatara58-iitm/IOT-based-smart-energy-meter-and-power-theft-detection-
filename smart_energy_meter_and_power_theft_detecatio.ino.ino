#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#define BLYNK_TEMPLATE_ID "TMPL3I4Du6yPU"
#define BLYNK_TEMPLATE_NAME "Smart energy meter and power theft detecation"
#define BLYNK_AUTH_TOKEN "JS7ewJb76dO6bxBaOxlKRfVrUYURLnDR"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <PZEM004Tv30.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// LCD I2C Setup (0x27, 16 columns, 2 rows)
LiquidCrystal_I2C lcd(0x27, 16, 2); 

char ssid[] = "TECNO SPARK Go 1";
char pass[] = "8765432125";

// PZEM Sensors Setup
PZEM004Tv30 pzemSource(Serial2, 16, 17); // Source (Grid) on Serial2 (GPIO 16, 17)
PZEM004Tv30 pzemLoad(Serial1, 32, 33);   // Load on Serial1 (GPIO 32, 33)

// Pin Definitions
const int RED_LED_PIN = 4;
const int GREEN_LED_PIN = 2;
const int BUZZER_PIN = 18;       // Passive buzzer pin
const int RELAY_PIN = 26;        // Relay module pin

const float THEFT_THRESHOLD = 5.0; // 5W difference limit

BlynkTimer timer;

void sendSensorData() {
  // Read Power values
  float power1 = pzemSource.power();
  float power2 = pzemLoad.power();
  
  // Read Voltage & Current for extra datastreams
  float voltage1 = pzemSource.voltage();
  float current1 = pzemSource.current();
  float voltage2 = pzemLoad.voltage();
  float current2 = pzemLoad.current();

  // Check if sensors are responding
  if (isnan(power1) || isnan(power2)) {
    Serial.println("Error reading PZEM sensors!");
    lcd.setCursor(0, 0);
    lcd.print("PZEM Error!     "); 
    lcd.setCursor(0, 1);
    lcd.print("Check Wiring... ");
    return;
  }

  // Send all parameters to Blynk Cloud
  Blynk.virtualWrite(V0, power1);
  Blynk.virtualWrite(V1, power2);
  Blynk.virtualWrite(V4, voltage1);
  Blynk.virtualWrite(V5, current1);
  Blynk.virtualWrite(V6, voltage2);
  Blynk.virtualWrite(V7, current2);

  // Serial Monitor Print
  Serial.print("Source: "); Serial.print(power1); Serial.print("W | Load: "); Serial.print(power2); Serial.println("W");

  // LCD Row 1: Power Values
  lcd.setCursor(0, 0);
  lcd.print("S:");
  lcd.print(power1, 1);
  lcd.print("W L:");
  lcd.print(power2, 1);
  lcd.print("W   "); 

  // Theft Detection Logic
  if ((power1 - power2) > THEFT_THRESHOLD) {
    digitalWrite(GREEN_LED_PIN, LOW);   // Green LED OFF
    digitalWrite(RED_LED_PIN, HIGH);    // Red LED ON
    
    tone(BUZZER_PIN, 3500);             // Passive buzzer start (1000 Hz tone)
    
    digitalWrite(RELAY_PIN, HIGH);      // Relay TRIPPED (Power Cut)

    Blynk.virtualWrite(V2, 1);          // Blynk Alert ON
    Blynk.logEvent("power_theft", "Alert! Power Theft Detected!"); 
    Serial.println("ALERT: Power Theft Detected! Relay Tripped.");
    
    lcd.setCursor(0, 1);
    lcd.print("ALERT: THEFT!!! "); 
  } 
  else {
    digitalWrite(GREEN_LED_PIN, HIGH);  // Green LED ON
    digitalWrite(RED_LED_PIN, LOW);     // Red LED OFF
    
    noTone(BUZZER_PIN);                 // Passive buzzer stop
    
    digitalWrite(RELAY_PIN, LOW);       // Relay NORMAL (Power ON)

    Blynk.virtualWrite(V2, 0);          // Blynk Alert OFF
    Serial.println("Status: Normal");

    lcd.setCursor(0, 1);
    lcd.print("Status: Normal  ");
  }
}

// Blynk App se manual Relay control karne ke liye (V3 Switch)
BLYNK_WRITE(V3) {
  int relayState = param.asInt();
  if(relayState == 1) {
    digitalWrite(RELAY_PIN, LOW);  // ON
  } else {
    digitalWrite(RELAY_PIN, HIGH); // OFF
  }
}

void setup() {
  // Disable brownout detector
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 

  Serial.begin(115200);
  delay(500);

  // Pin Modes configure karein
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);

  // Wi-Fi connect hone se pehle sabhi heavy loads ko safe/OFF rakhein
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
  noTone(BUZZER_PIN);
  digitalWrite(RELAY_PIN, LOW);

  // LCD Boot Screen
  lcd.init();                      
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("INNOVATORS ARENA");
  lcd.setCursor(0, 1);
  lcd.print("System Booting..");
  Serial.println("System Booting..");
  delay(1000);

  // Serial1 for Second PZEM
  Serial1.begin(9600, SERIAL_8N1, 32, 33);

  // Wi-Fi Connection Phase
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi.");
  lcd.setCursor(0, 1);
  lcd.print("Please wait...  ");

  delay(500);
  WiFi.begin(ssid, pass);

  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    timeout++;
    if (timeout > 40) {
      break;
    }
  }

  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected! ");
  lcd.setCursor(0, 1);
  lcd.print("Starting Meter..");
  delay(1500);
  lcd.clear();

  // Timer interval for sensor check (Every 2 seconds)
  timer.setInterval(2000L, sendSensorData);
}

void loop() {
  Blynk.run();
  timer.run();
}