#include <math.h>
int analogInput = 13;
int digitalInput = 4;
int onboardLed = 2;
  
void setup () {
  pinMode(analogInput, INPUT);
  pinMode(digitalInput, INPUT);
  pinMode(onboardLed, OUTPUT);
  Serial.begin(9600); // Serial output with 9600 bps
  Serial.println("KY-038 Noise detection");
}
void loop () {

  float maxAnalogValue = 0;
  //float totalAnalogValue = 0;
  //int analogMeasureCount = 0;
  int thresholdReached = 0;

  long time = millis();
  long time2 = millis();

  while(time2 - time < 1000){
    //float analogLecture = analogRead(analogInput) * (5.0 / 1023.0);
    int analogLecture = analogRead(analogInput);
    int digitalLecture = digitalRead(digitalInput);
    if (analogLecture > maxAnalogValue){
      maxAnalogValue = analogLecture;
    }
    if (digitalLecture == 0){
      thresholdReached = 1;
    }
    time2 = millis();
  }
  /*while(time2 - time < 3000){
    float analogLecture = analogRead(analogInput) * (5.0 / 1023.0);
    int digitalLecture = digitalRead(digitalInput);
    
    totalAnalogValue += analogLecture;
    analogMeasureCount++;

    if (thresholdReached == 0 && digitalLecture == 1){
      thresholdReached = 1;
    }
    time2 = millis();
  }*/

  //float analogValue = totalAnalogValue / analogMeasureCount;

  //float decibels = 20 * log10(maxAnalogValue);

  Serial.print("Analog voltage value: ");
  Serial.print(maxAnalogValue, 4);
  Serial.print(", \t Threshold value: ");
  
  if (thresholdReached == 1) {
      Serial.println("reached");
      digitalWrite(onboardLed, HIGH);
  }
  else {
      Serial.println("not yet reached");
      digitalWrite(onboardLed, LOW);
  }
  /*Serial.print(", Sample Count: ");
  Serial.println(analogMeasureCount);*/
}