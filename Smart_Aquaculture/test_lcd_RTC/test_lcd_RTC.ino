#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "RTClib.h"

LiquidCrystal_I2C lcd(0x27, 20, 4);

// RTC DS3231
RTC_DS3231 rtc;

char dataHari[7][12] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu"};
String hari;
int tanggal, bulan, tahun, jam, menit, detik;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();

  if (! rtc.begin()) {
    Serial.println("RTC Tidak Ditemukan");
    Serial.flush();
    abort();
  }

  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

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

  bacartc();
  
  lcd.setCursor(0, 0);
  lcd.print(String() + hari + ", " + tanggal + "-" + bulan + "-" + tahun);
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
