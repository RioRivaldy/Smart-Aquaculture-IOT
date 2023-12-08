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

LiquidCrystal_I2C lcd(0x27, 20, 4);

char auth[] = "4GQPj-DQPFGC2JUo5N5R2Je2Gu9vbk8T";  //Enter your Blynk Auth token
char ssid[] = "Kos Biru";  //Enter your WIFI SSID
char pass[] = "Bagyo123";  //Enter your WIFI Password

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

int nilai_analog_ph1;
float ph_step1;
float Ph_Kolam1;
double TeganganPh1;

//kalibrasi sensor Ph
float ph4_1 = 4.10;
float ph4_2 = 3.46;
float ph7_1 = 3.44;
float ph7_2 = 2.80;

void setup() {
  // put your setup code here, to run once:
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

  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();

  if (! rtc.begin()) {
    Serial.println("RTC Tidak Ditemukan");
    Serial.flush();
    abort();
  }

  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  // Aktuator
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
  // put your main code here, to run repeatedly.
  Blynk.run();//Run the Blynk library
  
  bacartc();

  if (jam == 19 && menit == 54 && detik == 30) {
    servoputar(10);
  }

  if (jam == 19 && menit == 55 && detik == 30) {
    servoputar(10);
  }

  suhukolam1 = bacasuhu1();
  Blynk.virtualWrite(V0, suhukolam1);
  
  keruhkolam1 = bacakeruh1();
  Blynk.virtualWrite(V1, keruhkolam1);
  
  Ph_Kolam1 = bacapH1();
  Blynk.virtualWrite(V2, Ph_Kolam1);

  lcd.setCursor(0, 0);
  lcd.print(String() + hari + ", " + tanggal + "-" + bulan + "-" + tahun);
  lcd.setCursor(12, 1);
  lcd.print (suhukolam1);
  lcd.setCursor(12, 2);
  lcd.print(keruhkolam1);
  lcd.setCursor(12, 3);
  lcd.print(Ph_Kolam1);
}

float bacasuhu1() {
  sensorSuhu1.requestTemperatures();
  float suhu1 = sensorSuhu1.getTempCByIndex(0);
  return suhu1;

  Blynk.virtualWrite(V0, suhukolam1);
}

int bacakeruh1() {
  nilai_analog_keruh1 = analogRead(pin_keruh1) / 4;
  kolam1 = map(nilai_analog_keruh1, 0, 1023, 100, 1);
  return kolam1;
}

float bacapH1() {
  nilai_analog_ph1 = analogRead(pin_ph1);
  TeganganPh1 = 5 / 1023.0 * nilai_analog_ph1;

  ph_step1 = (ph4_1 - ph7_1) / 2.4;
  float Ph1 = 7 + ((ph7_1 - TeganganPh1) / ph_step1);
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

void servoputar(int jumlah) {
  for (int i = 1; i <= jumlah; i++) {
    mekanik1.write(180);
    delay(100);
    mekanik1.write(0);
    delay(100);
  }
}
