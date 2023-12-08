#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "RTClib.h"
#include <ESP32Servo.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#define BLYNK_PRINT Serial

/*
   ANALOG
   Turbidity          D36 / VP
   pH                 D39 / VN
*/

/* DIGITAL
   Temperature        D5
   Buzzer             D33
   Servo              D2
*/

/*
   SDA                D21
   SCL                D22
   GND                GND
*/

/*
  Relay1              D34
  Relay1              D35
*/

LiquidCrystal_I2C lcd(0x27, 20, 4);

char auth[] = "4GQPj-DQPFGC2JUo5N5R2Je2Gu9vbk8T";  //Enter your Blynk Auth token
char ssid[] = "modal";  //Enter your WIFI SSID
char pass[] = "anggi123";  //Enter your WIFI Password

// RTC DS3231
RTC_DS3231 rtc;

char dataHari[7][12] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu"};
String hari;
int tanggal, bulan, tahun, jam, menit, detik;

// Servo
Servo mekanik1;

// Sensor Suhu DS18B20
#define ONE_WIRE_BUS1 5

// Inisialisasi objek OneWire dan DallasTemperature
OneWire oneWire1(ONE_WIRE_BUS1);
DallasTemperature sensorSuhu1(&oneWire1);

float suhukolam1;

// Sensor Turbidity SEN0189
const int pin_keruh1 = 36;

int nilai_analog_keruh1;
int kolam1;
int keruhkolam1;

// Sensor pH PH4502C
const int pin_ph1 = 39;

float nilai_analog_ph1;
float kalibrasi1;
float Ph1;
float Ph_Kolam1;

// Relay
const int relay1 = 12;
const int relay2 = 13;
int relayON = LOW;
int relayOFF = HIGH;

const byte buzzerPin = 33;

void setup() {
  Serial.begin(9600);

  WiFi.begin(ssid, pass); // Ganti dengan nama dan kata sandi WiFi Anda
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);

  // Sensor
  sensorSuhu1.begin();
  pinMode(pin_keruh1, INPUT);
  pinMode(pin_ph1, INPUT);
  pinMode(buzzerPin, OUTPUT);
  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();

  if (! rtc.begin()) {
    Serial.println("RTC Tidak Ditemukan");
    Serial.flush();
    abort();
  }

  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  // Aktuator : Servo
  mekanik1.attach(2); // Hubungkan servo ke pin D2 pada ESP32
  mekanik1.write(0);

  // Relay
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  digitalWrite(relay1, relayOFF);
  digitalWrite(relay2, relayOFF);

  lcd.setCursor(5, 0);
  lcd.print("WELCOME!!!");
  lcd.setCursor(2, 1);
  lcd.print("SMART AQUACULTURE");
  lcd.setCursor(0, 2);
  lcd.print("CONTROL & MONITORING");
  lcd.setCursor(2, 3);
  lcd.print("FOR WATER QUALITY");
  delay(3000);
  lcd.clear();

  lcd.setCursor(0, 1);
  lcd.print("Suhu      :  ");
  lcd.setCursor (17, 1);
  lcd.print ((char)223);
  lcd.print ("C");
  lcd.setCursor(0, 2);
  lcd.print("Kekeruhan : ");
  lcd.setCursor(0, 3);
  lcd.print("pH        :   ");
}

void loop() {
  Blynk.run();//Run the Blynk library

  bacartc();

  // Suhu
  suhukolam1 = bacasuhu1();
  Blynk.virtualWrite(V0, suhukolam1);
  Serial.println(suhukolam1);
  // Kekeruhan
  keruhkolam1 = bacakeruh1();
  Blynk.virtualWrite(V1, keruhkolam1);
  Serial.println(keruhkolam1);
  // pH
  Ph_Kolam1 = bacapH1();
  Blynk.virtualWrite(V2, Ph_Kolam1);
  Serial.println(Ph_Kolam1);

  bacaLcd();

  // Servo nyala pakan ikan
  if ((jam == 9 && menit == 00 && detik == 30) || (jam == 15 && menit == 00 && detik == 30)) {
    servoputar(10);
  }

  // Widget LED Servo
  if ((jam == 9 && menit == 00 && detik == 30) || (jam == 15 && menit == 00 && detik == 30)) {
    WidgetLED LED(V8);
    LED.on();
    delay(5000);
  } else {
    WidgetLED LED(V8);
    LED.off();
  }

  // Suhu
  if (suhukolam1 > 34.0) {
    digitalWrite(relay1, relayON);
    digitalWrite(relay2, relayON);
    Blynk.logEvent("tingkat_suhu", "Pemberitahuan! Suhu aquarium saat ini telah melebihi batas normal / tinggi (Suhu > 34 °C)"); //Enter your Event Name
    bacaLcd();
    WidgetLED LED(V5);
    LED.on();
    digitalWrite(buzzerPin, HIGH);
    delay(5000);
    digitalWrite(relay1, relayOFF);
    digitalWrite(relay2, relayOFF);
    digitalWrite(buzzerPin, LOW);
  } else {
    bacaLcd();
    WidgetLED LED(V5);
    LED.off();
    digitalWrite(buzzerPin, LOW);
  }

  // Kekeruhan
  if (keruhkolam1 > 20) {
    digitalWrite(relay1, relayON);
    digitalWrite(relay2, relayON);
    Blynk.logEvent("tingkat_keruh", "Pemberitahuan! Tingkat kekeruhan air dalam aquarium telah melebihi batas keruh (Tingkat Keruh > 50 NTU)."); //Enter your Event Name
    bacaLcd();
    WidgetLED LED(V6);
    LED.on();
    digitalWrite(buzzerPin, HIGH);
    delay(5000);
    digitalWrite(relay1, relayOFF);
    digitalWrite(relay2, relayOFF);
    digitalWrite(buzzerPin, LOW);
  } else {
    bacaLcd();
    WidgetLED LED(V6);
    LED.off();
    digitalWrite(buzzerPin, LOW);
  }

  // pH
  if (Ph_Kolam1 < 6.5) {
    digitalWrite(relay1, relayON);
    digitalWrite(relay2, relayON);
    Blynk.logEvent("tingkat_ph", "pH di dalam aquarium telah berada di bawah kondisi netral / terlalu asam (pH < 7)"); //Enter your Event Name
    bacaLcd();
    WidgetLED LED(V7);
    LED.on();
    digitalWrite(buzzerPin, HIGH);
    delay(5000);
    digitalWrite(relay1, relayOFF);
    digitalWrite(relay2, relayOFF);
    digitalWrite(buzzerPin, LOW);
  } else {
    bacaLcd();
    WidgetLED LED(V7);
    LED.off();
    digitalWrite(buzzerPin, LOW);
  }
}

float bacasuhu1() {
  sensorSuhu1.requestTemperatures();
  float suhu1 = sensorSuhu1.getTempCByIndex(0);
  return suhu1;
}

int bacakeruh1() {
  nilai_analog_keruh1 = analogRead(pin_keruh1) / 4;
  kolam1 = map(nilai_analog_keruh1, 0, 1023, 100, 1);
  return kolam1;
}

float bacapH1() {
  nilai_analog_ph1 = analogRead(pin_ph1) / 4;
  kalibrasi1 = 1023 - nilai_analog_ph1;
  Ph1 = (7 * kalibrasi1) / 313;
  Ph1 = Ph1 - 0.5;

  return Ph1;
}

void bacartc() {
  DateTime now = rtc.now();
  hari    = dataHari[now.dayOfTheWeek()];
  tanggal = now.day(), DEC;
  bulan   = now.month(), DEC;
  tahun   = now.year(), DEC;
  jam     = now.hour(), DEC;
  menit   = now.minute(), DEC;
  detik   = now.second(), DEC;

  Serial.println(String() + hari + ", " + tanggal + "-" + bulan + "-" + tahun);
  Serial.println(String() + jam + ":" + menit + ":" + detik);
  delay(1000);
}

void bacaLcd() {
  lcd.setCursor(0, 0);
  lcd.print(String() + hari + ", " + tanggal + "-" + bulan + "-" + tahun);
  lcd.setCursor(12, 1);
  lcd.print (suhukolam1);
  lcd.setCursor(12, 2);
  lcd.print(keruhkolam1);
  lcd.setCursor(12, 3);
  lcd.print(Ph_Kolam1);
}

void servoputar(int jumlah) {
  for (int i = 1; i <= jumlah; i++) {
    mekanik1.write(180);
    delay(100);
    mekanik1.write(0);
    delay(100);
  }
}
