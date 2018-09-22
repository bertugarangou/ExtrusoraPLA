/*
  Extrusora de PLA controlada per Arduino Mega 2560.

  Projecte:
  - https://github.com/bertugarangou/ExtrusoraPLA
  Circuit:
  - Extrusora feta per Albert Garangou,
    com a Treball de Recerca a 2n de Batxillerat,
    curs 2018/2019, tutor: Jordi Fanals Oriol,
    codi amb Arduino IDE 1.8.5,
    Copyright (c) 2018 Albert Garangou Culebras (albertgarangou@gmail.com)
    Aquest codi està llicenciat sota la SIL Open Font License, Version 1.1.

*/
/*+++++++++++++Llibreries++++++++++++++*/
#include <max6675.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2); //PINS SDA i SCL
/*+++++++++++++Llibreries++++++++++++++*/
/*++Declaració variables i constants+++*/
int const INTFanFil = 53;
int const INTFanTube = 52;
int const RelayFanFil = 2;
int const RelayFanTube = 3;
int const STOPBtn = 13;

bool error = false;
#define HIGH 0x0 //0 = HIGH     HIGH per quan 1 no activa
#define LOW 0x1 // 1 = LOW      1 per normal
/*++Declaració variables i constants+++*/
/*+++++++++++Declaracio funcions+++++++++++*/

/*+++++++++++Declaracio funcions+++++++++++*/
/*+++++++++++Declaracio funcions+++++++++++*/

void setup(){
  Serial.begin(9600); //inicia la depuració
  lcd.init();
  lcd.backlight();
  lcd.clear();
  pinMode(INTFanFil, INPUT);
  pinMode(INTFanTube, INPUT);
  pinMode(STOPBtn, INPUT);
  pinMode(RelayFanFil, OUTPUT);
  pinMode(RelayFanTube, OUTPUT);
    lcd.print("HI!!!");
}

void loop(){
 if(digitalRead(STOPBtn) == 0 || error == true){
  Serial.println("*****************************************");
  Serial.print("*   STOP");
  Serial.println(", entrant en mode emergència!   *");
  Serial.println("*                                       *");
  Serial.println("* NO DESCONECTAR FINS QUE ESTIGUI FRED! *");
  Serial.println("*****************************************");
  lcd.clear();
  lcd.print("   ALERTA !!!");
  lcd.setCursor(0,1);
  //procedimentError();   //necessàri o només caldria missatge pantalla diferent?
  digitalWrite(RelayFanFil, HIGH); //HIGH!?!?!?
  digitalWrite(RelayFanTube, HIGH);  //HIGH!?!?!
  while(true){//bucle infinit
  lcd.noBacklight();
  delay(500);
  lcd.backlight();
  delay(2000);
  }
 }
  else{

    if(digitalRead(INTFanFil) == HIGH){
      digitalWrite(RelayFanFil, 1);
    }
    else{
      digitalWrite(RelayFanFil, 0);
    }
    
    if(digitalRead(INTFanTube) == HIGH){
      digitalWrite(RelayFanTube, 1);
    }
    else{
      digitalWrite(RelayFanTube, 0);
    }
 }
}
/*+++++++++++Definició funicons++++++++++++*/
/*+++++++++++Definició funicons++++++++++++*/


