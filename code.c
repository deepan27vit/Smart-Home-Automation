/* Full Home Automation System with NodeMCU and Blynk


#include <LiquidCrystal_I2C.h>
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>

// ---------------------------
// WiFi & Blynk Credentials
// ---------------------------
char auth[] = ""; // Enter your Blynk Auth token
char ssid[] = ""; // Enter your WiFi name
char pass[] = ""; // Enter your WiFi password

// ---------------------------
// LCD & Sensor Setup
// ---------------------------
LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(D4, DHT11);
BlynkTimer timer;
bool pirbutton = 0;

// ---------------------------
// Pin Definitions
// ---------------------------
#define Buzzer 10
#define MQ2 A0
#define flame D0
#define PIR D3
#define trig D5
#define echo D6
#define relay1 D7
#define relay2 D8

// Virtual Button for PIR (V0)
BLYNK_WRITE(V0) {
  pirbutton = param.asInt();
}

// ---------------------------
// Setup
// ---------------------------
void setup() {
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();

  pinMode(Buzzer, OUTPUT);
  pinMode(flame, INPUT);
  pinMode(PIR, INPUT);
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);

  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);

  Blynk.begin(auth, ssid, pass);
  dht.begin();

  timer.setInterval(100L, gassensor);
  timer.setInterval(100L, DHT11sensor);
  timer.setInterval(100L, flamesensor);
  timer.setInterval(100L, pirsensor);
  timer.setInterval(100L, ultrasonic);
}

// ---------------------------
// Gas Sensor (MQ2)
// ---------------------------
void gassensor() {
  int value = analogRead(MQ2);
  Serial.println(value);

  value = map(value, 0, 1024, 0, 100);

  if (value <= 35) {
    digitalWrite(Buzzer, LOW);
  } else {
    Blynk.notify("Warning! Gas leak detected");
    digitalWrite(Buzzer, HIGH);
  }

  Blynk.virtualWrite(V1, value);

  lcd.setCursor(9, 0);
  lcd.print("G :");
  lcd.print(value);
  lcd.print("  ");
}

// ---------------------------
// Temperature & Humidity (DHT11)
// ---------------------------
void DHT11sensor() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Blynk.virtualWrite(V2, t);
  Blynk.virtualWrite(V3, h);

  lcd.setCursor(0, 0);
  lcd.print("T :");
  lcd.print(t);

  lcd.setCursor(0, 1);
  lcd.print("H :");
  lcd.print(h);
}

// ---------------------------
// Flame Sensor
// ---------------------------
void flamesensor() {
  bool value = digitalRead(flame);

  if (value == 1) {
    digitalWrite(Buzzer, LOW);
  } else {
    Blynk.notify("Warning! Fire was detected");
    digitalWrite(Buzzer, HIGH);
  }
}

// ---------------------------
// PIR Motion Sensor
// ---------------------------
void pirsensor() {
  bool value = digitalRead(PIR);

  if (pirbutton == 1) {
    if (value == 0) {
      digitalWrite(Buzzer, LOW);
    } else {
      Blynk.notify("Warning! Please check your security system");
      digitalWrite(Buzzer, HIGH);
    }
  }
}

// ---------------------------
// Ultrasonic Sensor
// ---------------------------
void ultrasonic() {
  digitalWrite(trig, LOW);
  delayMicroseconds(4);

  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  long t = pulseIn(echo, HIGH);
  long cm = t / 29 / 2;

  Blynk.virtualWrite(V4, cm);

  lcd.setCursor(9, 1);
  lcd.print("W :");
  lcd.print(cm);
  lcd.print("  ");
}

// ---------------------------
// Main Loop
// ---------------------------
void loop() {
  Blynk.run();
  timer.run();
}
