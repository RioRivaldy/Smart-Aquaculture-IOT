// Heart Rate Max30102
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "MAX30105.h"
#include "heartRate.h"

// DS18B20
#include <OneWire.h>
#include <DallasTemperature.h>

MAX30105 particleSensor;
LiquidCrystal_I2C lcd(0x27, 20, 4);  // Alamat I2C LCD 20x4 bisa berbeda, sesuaikan jika diperlukan

const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred

float beatsPerMinute;
int beatAvg;
  
// Inisialisasi pin data untuk sensor DS18B20
const int oneWireBusPin = 4;  // Sesuaikan dengan pin yang digunakan pada Arduino Anda

// Inisialisasi objek OneWire dan DallasTemperature
OneWire oneWire(oneWireBusPin);
DallasTemperature sensors(&oneWire);

unsigned long lastTimeTemp = 0;
unsigned long timerDelayTemp = 1000;

void setup()
{
  Serial.begin(115200);
  sensors.begin(); // DS18B20
  Serial.println("Initializing...");

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");

  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("  Initializing  ");
  for (int a = 5; a <= 10; a++) {
    lcd.setCursor(a, 1);
    lcd.print(".");
    delay(500);
  }
  lcd.clear();

  lcd.setCursor(0, 2);
  lcd.print("Suhu   : ");
  lcd.setCursor(0, 3);
  lcd.print("BPM    : ");
//  lcd.setCursor(0, 3);
//  lcd.print("Avg BPM: ");
}

void loop()
{
  long irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true)
  {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = (60 / (delta / 1000.0) + 30);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }

    // Update LCD if there's a beat detected
//    lcd.setCursor(9, 2);
//    lcd.print(beatsPerMinute);

    lcd.setCursor(9, 3);
    lcd.print(beatAvg);
  }

  Serial.print("IR=");
  Serial.print(irValue);
  Serial.print(", BPM=");
  Serial.print(beatsPerMinute);
  Serial.print(", Avg BPM=");
  Serial.print(beatAvg);

  if (irValue < 60000) {
    Serial.print(" No finger?");
    //    lcd.setCursor(0, 0);
    //    lcd.print("No finger       ");
  } else {
    //    lcd.setCursor(0, 0);
    //    lcd.print("Figer detected!");
  }

  if ((millis() - lastTimeTemp) > timerDelayTemp) {
    sensors.requestTemperatures();  // Minta sensor untuk membaca suhu

    // Baca suhu dalam Celsius dan Fahrenheit
    float celsius = sensors.getTempCByIndex(0);
    float fahrenheit = sensors.toFahrenheit(celsius);

    Serial.print("Suhu Celsius: ");
    Serial.print(celsius);
    Serial.print("°C | Suhu Fahrenheit: ");
    Serial.print(fahrenheit);
    Serial.println("°F");

    lcd.setCursor(5, 0);
    lcd.print("LANSICARE");
    lcd.setCursor(9, 2);
    lcd.print(celsius);

    lastTimeTemp = millis();
  }

  Serial.println();
}
