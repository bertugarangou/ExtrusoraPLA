/*
  Extrusora de PLA controlada per Arduino Mega 2560.

  Projecte:
  - https://github.com/bertugarangou/ExtrusoraPLA
  Circuit:
  - Extrusora feta per Albert Garangou,
    com a Treball de Recerca a 2n de Batxillerat,
    curs 2018/2019, tutor: Jordi Fanals Oriol,
    codi amb Arduino IDE
    Copyright (c) 2018 Albert Garangou Culebras (albertgarangou@gmail.com)
    Aquest codi està llicenciat sota la SIL Open Font License, Version 1.1.

*/
/*+++++++++++++Llibreries++++++++++++++*/
#include <max6675.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2); //PINS SDA i SCL lcd
/*+++++++++++++Llibreries++++++++++++++*/
/*++Declaració variables i constants+++*/
unsigned long ultimMillis_LCDMain = 0UL;
int const INTFanFil = 7;
int const INTFanTube = 6;
int const relayFanFil = 52;
int const relayFanTube = 50;
int const STOPBtn = 30;
int const brunzidor = 51;
int const extruderStep = 25;
int const extruderDir = 26;
int const extruderEn = 28;
int const coilStep = 24;
int const coilDir = 27;
int const coilEn = 29;

bool error = false;
bool canExtrude = false;
bool heating = false;

bool extruding = false;
bool coiling = false;

int currentTemp;
int desiredTemp;

int const extruderSpeed = 50; //1-1.5-2-5-10-50-100
int const coilSpeed = 50; //1-1.5-2-5-10-50-100
int const timeBtwExtruderSteps = 50; //1-1.5-2-5-10-50-100
int const timeBtwCoilSteps = 50; //1-1.5-2-5-10-50-100
/*++Declaració variables i constants+++*/
/*+++++++++++Declaracio funcions+++++++++++*/
void lcdController();
void fansController();
void extruderController();
void coilController();
/*+++++++++++Declaracio funcions+++++++++++*/


void setup(){
  Serial.begin(9600); //inicia la depuració
  lcd.init();
  lcd.backlight();
  lcd.clear();
  pinMode(INTFanFil, INPUT);
  pinMode(INTFanTube, INPUT);
  pinMode(STOPBtn, INPUT);
  pinMode(relayFanFil, OUTPUT);
  pinMode(relayFanTube, OUTPUT);
  pinMode(brunzidor, OUTPUT);
  pinMode(extruderStep, OUTPUT);
  pinMode(extruderDir, OUTPUT);
  pinMode(extruderEn, OUTPUT);
  pinMode(coilStep, OUTPUT);
  pinMode(coilDir, OUTPUT);
  pinMode(coilEn, OUTPUT);
  digitalWrite(coilEn, HIGH);
  digitalWrite(extruderEn, HIGH);
}

void loop(){
 if(digitalRead(STOPBtn) == 0 || error == true){
  digitalWrite(brunzidor, HIGH);
  Serial.println("*****************************************");
  Serial.print("*   STOP");
  Serial.println(", entrant en mode emergència!   *");
  Serial.println("*                                       *");
  Serial.println("* NO DESCONECTAR FINS QUE ESTIGUI FRED! *");
  Serial.println("*****************************************");
  lcd.clear();
  lcd.print(" !!! ALERTA !!!");
  lcd.setCursor(0,1);
  lcd.print(" NO DESCONECTAR");
  //procedimentError();   //necessàri o només caldria missatge pantalla diferent?
  digitalWrite(relayFanFil, LOW);
  digitalWrite(relayFanTube, LOW);
  while(true){//bucle infinit
  lcd.noBacklight();
  digitalWrite(brunzidor, LOW);
  delay(500);
  digitalWrite(brunzidor, HIGH);
  lcd.backlight();
  delay(2000);
  }
 }
  else{ //funcionament estandart del programa (aka no hi ha cap error)
    //Serial.print("Temp: "); Serial.print(currentTemp); Serial.print("/"); Serial.println(desiredTemp);  //enviar per Serial la temperatura
    lcdController();
    fansController();
    extruderController();
    coilController();
  }
} //end

/*+++++++++++Definició funicons++++++++++++*/
void extruderController(){
  if (digitalRead(2) == LOW && digitalRead(3) == HIGH /*&& canExtrude == true*/){//activat
    extruding == true;
    digitalWrite(extruderEn, HIGH);
    digitalWrite(extruderStep, HIGH);
    delay(extruderSpeed);
    digitalWrite(extruderStep, LOW);
    delay(timeBtwExtruderSteps);
  }
  else if(digitalRead(2) == LOW && digitalRead(3) == LOW /*&& canExtrude == true*/){
    extruding == true;
    digitalWrite(extruderEn, HIGH);
    digitalWrite(extruderDir, HIGH);
    digitalWrite(extruderStep, HIGH);
    delay(extruderSpeed);
    digitalWrite(extruderStep, LOW);
    digitalWrite(extruderDir, LOW);
    delay(timeBtwExtruderSteps);
  }
  else{
    extruding = false;
    digitalWrite(extruderEn, LOW);
  }
}

void coilController(){
  if (digitalRead(4) == LOW && digitalRead(5) == HIGH /*&& canExtrude == true*/){//activat
    coiling == true;
    digitalWrite(coilEn, HIGH);
    digitalWrite(coilStep, HIGH);
    delay(coilSpeed);
    digitalWrite(coilStep, LOW);
    delay(timeBtwCoilSteps);
  }
  else if(digitalRead(2) == LOW && digitalRead(3) == LOW /*&& canExtrude == true*/){
    extruding == true;
    digitalWrite(coilEn, HIGH);
    digitalWrite(coilDir, HIGH);
    digitalWrite(coilStep, HIGH);
    delay(coilSpeed);
    digitalWrite(coilStep, LOW);
    digitalWrite(coilDir, LOW);
    delay(timeBtwCoilSteps);
  }
  else{
    coiling = false;
    digitalWrite(extruderEn, LOW);
  }

}

void fansController(){
      if(digitalRead(INTFanFil) == LOW){ //quan s'activa l'interruptor adequat
      digitalWrite(relayFanFil, LOW); //activar relé ventilador
      //Serial.println("Ventilador: Filament **ON**");
    }
    else{ //sinó
      digitalWrite(relayFanFil, HIGH); //desactiva'l
      //Serial.println("Ventilador: Filament **OFF**");
    }
    
    if(digitalRead(INTFanTube) == LOW){  //si s'activa l'interruptor adequat
      digitalWrite(relayFanTube, LOW);  //activa el relé del ventilador
      //Serial.println("Ventilador: Extrusora **ON**");
    }
    else{ //sinó
      digitalWrite(relayFanTube, HIGH);  //desactiva'l
      //Serial.println("Ventilador: Extrusora **OFF**");
    }
 }

void lcdController(){
    if(millis() - ultimMillis_LCDMain >= 2000UL){
      lcd.setCursor(0,0);
      lcd.print(currentTemp);
      lcd.print("/");
      lcd.print(desiredTemp);
      lcd.print(char(223));
      lcd.print("  ");
      
      if(canExtrude == true){
        lcd.setCursor(10,0);
        lcd.print(" LLEST");
      }
      else if (canExtrude == false && heating == true){
        lcd.setCursor(10,0);
        lcd.print("ESPERA");
      }
      else{
        lcd.setCursor(10, 0);
        lcd.print(" PAUSA");
      }
      
      lcd.print(millis());
      ultimMillis_LCDMain = millis();
    }
  }
/*+++++++++++Definició funicons++++++++++++*/
