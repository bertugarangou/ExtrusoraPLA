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
int const RelayFanFil = 52;
int const RelayFanTube = 50;
int const STOPBtn = 30;
int const brunzidor = 51;

bool error = false;
bool canExtrude = false;
int currentTemp = 120; //FAKEEEEE
int desiredTemp = 190; //FAKEEE
int const stepperSpeed = 1.5; //0.5-100
int const timeBetweenSteps = 1.5; //0-100
/*++Declaració variables i constants+++*/
/*+++++++++++Declaracio funcions+++++++++++*/
void lcdController();
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
  pinMode(brunzidor, OUTPUT);
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
  digitalWrite(RelayFanFil, HIGH); //HIGH!?!?!?
  digitalWrite(RelayFanTube, HIGH);  //HIGH!?!?!
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
    
    if(digitalRead(INTFanFil) == HIGH){ //quan s'activa l'interruptor adequat
      digitalWrite(RelayFanFil, 0); //activar relé ventilador
      //Serial.println("Ventilador: Filament **ON**");
    }
    else{ //sinó
      digitalWrite(RelayFanFil, 1); //desactiva'l
      //Serial.println("Ventilador: Filament **OFF**");
    }
    
    if(digitalRead(INTFanTube) == HIGH){  //si s'activa l'interruptor adequat
      digitalWrite(RelayFanTube, 0);  //activa el relé del ventilador
      //Serial.println("Ventilador: Extrusora **ON**");
    }
    else{ //sinó
      digitalWrite(RelayFanTube, 1);  //desactiva'l
      //Serial.println("Ventilador: Extrusora **OFF**");
    }
 }
 lcdController();
} //end

/*+++++++++++Definició funicons++++++++++++*/
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
      else{
        lcd.setCursor(10,0);
        lcd.print("ESPERA");
      }
      
      lcd.print(millis());
      ultimMillis_LCDMain = millis();
    }
  }
/*+++++++++++Definició funicons++++++++++++*/
