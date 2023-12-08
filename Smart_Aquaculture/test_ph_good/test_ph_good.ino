#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "RTClib.h"
#include <ESP32Servo.h>
#include <OneWire.h>
#include <DallasTemperature.h>


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

const byte buzzerPin = 33;

void setup() {
  Serial.begin(9600);

  // Sensor
  sensorSuhu1.begin();
  pinMode(pin_keruh1, INPUT);
  pinMode(pin_ph1, INPUT);

  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();

  // Aktuator : Servo
  mekanik1.attach(2); // Hubungkan servo ke pin D2 pada ESP32
  mekanik1.write(0);

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
  bacartc();

  // Suhu
  suhukolam1 = bacasuhu1();
//  Blynk.virtualWrite(V0, suhukolam1);
  Serial.println(suhukolam1);
  // Kekeruhan
  keruhkolam1 = bacakeruh1();
//  Blynk.virtualWrite(V1, keruhkolam1);
  Serial.println(keruhkolam1);
  // pH
  Ph_Kolam1 = bacapH1();
//  Blynk.virtualWrite(V2, Ph_Kolam1);
  Serial.println(Ph_Kolam1);
  Serial.println(nilai_analog_ph1);

  bacaLcd();

  // Servo nyala pakan ikan
  if ((jam == 9 && menit == 00 && detik == 30) || (jam == 15 && menit == 00 && detik == 30)) {
    servoputar(10);
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
  Ph1 = (7 * kalibrasi1) / 312;

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
