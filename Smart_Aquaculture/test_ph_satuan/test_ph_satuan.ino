// Sensor pH PH4502C
const int pin_ph1 = 39;

float nilai_analog_ph1;
float kalibrasi1;
float Ph1;
float Ph_Kolam1;

void setup() {
  Serial.begin(9600);
  pinMode(pin_ph1, INPUT);
}

void loop() {
//  nilai_analog_ph1 = analogRead(pin_ph1) / 4;
//  Serial.println(nilai_analog_ph1);

  Ph_Kolam1 = bacapH1();
  Serial.println(Ph_Kolam1);
  delay(1000);
}

float bacapH1(){
  nilai_analog_ph1 = analogRead(pin_ph1) / 4;
  kalibrasi1 = 1023 - nilai_analog_ph1;
  Ph1 = (7 * kalibrasi1) / 313;

  return Ph1;
}
