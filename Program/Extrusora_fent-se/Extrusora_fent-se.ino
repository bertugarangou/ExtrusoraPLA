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
MAX6675 tempSensorResistors(8, 9, 10);
MAX6675 tempSensorEnd(11, 12, 13);
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2); //PINS SDA i SCL lcd

/*+++++++++++++Llibreries++++++++++++++*/
/*++Declaració variables i constants+++*/

int const lcdUpdateFrequency = 2500;
int const tempReaderFrequency = 1500;
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
int const filamentUpDetector = 37;
int const filamentDownDetector = 36;
int const INTHeat = 53;
int const INTExtruder = 2;
int const INTExtruderRev = 3;
int const INTCoil = 4;
int const INTCoilRev = 5;
int const RelayResistor = 32;

bool error = false;
bool canExtrude = false;
bool heating = false;
bool canCoil = false;

bool extruding = false;
bool coiling = false;

int currentTempToShow;
float currentTempResistors = 0.0;
float currentTempEnd = 0.0;
float desiredTemp;
unsigned long ultimMillis_LCDMain = 0UL;
unsigned long ultimMillis_extruderStart = 0UL;
unsigned long ultimMillis_extruderStop = 0UL;
unsigned long ultimMillis_coilStart = 0UL;
unsigned long ultimMillis_coilStop = 0UL;
unsigned long ultimMillis_tempReader = 0UL;

int const extruderNEINSpeed = 8;
int const coilNEINSpeed = 20;

/*++Declaració variables i constants+++*/
/*+++++++++++Declaracio funcions+++++++++++*/
void lcdController();
void fansController();
void extruderController();
void coilController();
void filamentDetector();
void heater();
void tempRead();
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
  pinMode(filamentUpDetector, INPUT);
  pinMode(filamentDownDetector, INPUT);
  pinMode(INTExtruder, INPUT);
  pinMode(INTExtruderRev, INPUT);
  pinMode(INTCoil, INPUT);
  pinMode(INTCoilRev, INPUT);
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
  //procedimentError();   //necessàri o només caldria missatge pantalla diferent o altres passos
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
    //Serial.print("Temp: "); Serial.print(currentTemp); Serial.print("/"); Serial.println(desiredTemp);  //enviar per Serial la temperatura (només depuració)
    lcdController();
    filamentDetector();
    fansController();
    heater();
    extruderController();
    coilController();
    tempRead();
  }
} //end

/*+++++++++++Definició funicons++++++++++++*/
void extruderController(){
  if (digitalRead(INTExtruder) == LOW && digitalRead(INTExtruderRev) == HIGH /*&& canExtrude == true*/){//activat
    if(millis() - ultimMillis_extruderStart >= extruderNEINSpeed){
      extruding == true;
      digitalWrite(extruderStep, HIGH);
    if(millis() - ultimMillis_extruderStop >= extruderNEINSpeed){
      digitalWrite(extruderStep, LOW);
      ultimMillis_extruderStop = millis();
      ultimMillis_extruderStart = millis();
      }
    }
  }
  else if(digitalRead(INTExtruder) == LOW && digitalRead(INTExtruderRev) == LOW /*&& canExtrude == true*/){
    if(millis() - ultimMillis_extruderStart >= extruderNEINSpeed){
      extruding == true;
      digitalWrite(extruderDir, HIGH);
      digitalWrite(extruderStep, HIGH);
    if(millis() - ultimMillis_extruderStop >= extruderNEINSpeed){
      digitalWrite(extruderStep, LOW);
      digitalWrite(extruderDir, LOW);
      ultimMillis_extruderStop = millis();
      ultimMillis_extruderStart = millis();
      }
    }
  else{
    extruding = false;
  }
}
}

void coilController(){
  if (digitalRead(INTCoil) == LOW && digitalRead(INTCoilRev) == LOW){ // tots dos activats
    if(millis() - ultimMillis_coilStart >= coilNEINSpeed){
      coiling == true;
      digitalWrite(coilStep, HIGH);
    if(millis() - ultimMillis_coilStop >= coilNEINSpeed){
      digitalWrite(coilStep, LOW);
      ultimMillis_coilStop = millis();
      ultimMillis_coilStart = millis();
      }
    }
  }
  else if(digitalRead(INTCoil) == LOW && digitalRead(INTCoilRev) == HIGH && canCoil == true){ // sense invertir direcció
    if(millis() - ultimMillis_coilStart >= coilNEINSpeed){
      coiling == true;
      digitalWrite(coilDir, HIGH);
      digitalWrite(coilStep, HIGH);
      if(millis() - ultimMillis_coilStop >= coilNEINSpeed){
        digitalWrite(coilStep, LOW);
        digitalWrite(coilDir, LOW);
        ultimMillis_coilStop = millis();
        ultimMillis_coilStart = millis();
      }
    }
  else{
    coiling = false;
  }
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
  if(millis() - ultimMillis_LCDMain >= lcdUpdateFrequency){
    lcd.setCursor(0,0);
    lcd.print(currentTempToShow);
    lcd.print("/");
    lcd.print((int) desiredTemp);
    lcd.print(char(223));
    lcd.print("  ");
      
    if(canExtrude == true){
      lcd.setCursor(10,0);
      lcd.print("ACTIVAT");
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

void filamentDetector(){
  if(filamentDownDetector == LOW){
    if(canCoil == false){
      canCoil == true;
    }
    else if(canCoil == true){
      //do nothing
    }
  }

  else if(filamentUpDetector == LOW){
    if(canCoil == false){
      //do nothing
    }
    else if(canCoil == true){
      canCoil = false;
    }
  }
}

void heater(){
}

void tempRead(){
  if(millis() - ultimMillis_tempReader >= tempReaderFrequency){
    currentTempEnd = tempSensorEnd.readCelsius();
    currentTempResistors = tempSensorResistors.readCelsius();
    
    ultimMillis_tempReader = millis();
  }
}
/*+++++++++++Definició funicons++++++++++++*/
