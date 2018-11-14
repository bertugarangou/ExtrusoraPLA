#include <max6675.h>


int ktcSO = 8;
int ktcCS = 9;
int ktcCLK = 10;
MAX6675 ktc(ktcCLK, ktcCS, ktcSO);
Serial.begin(9600);

void setup() {
pinMode(2, OUTPUT);
pinMode(3, INPUT);
}

void loop() {
  while(true){
    
    if(digitalRead(3) == HIGH){
       if(ktc.readCelsius() < 170){
        Serial.println(ktc.readCelsius())
         digitalWrite(2, HIGH);
      }
      else {
        digitalWrite(2, LOW);
      }
    }
    else {
      digitalWrite(2, LOW);
    }
  }
}
