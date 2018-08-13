/*
  Extrusora de PLA controlada per Arduino Mega 2560.

  Projecte:
  - https://github.com/bertugarangou/ExtrusoraPLA
  Circuit:
  - Extrusora feta per Albert Garangou,
    com a Treball de Recerca a 2n de Batxillerat,
    curs 2018/2019, tutor Jordi Fanals Oriol,
    codi amb Arduino IDE 1.8.5,
    Copyright (c) 2018 Albert Garangou Culebras (albertgarangou@gmail.com)
    This Font Software is licensed under the SIL Open Font License, Version 1.1.

*/
/*+++++++++++++Llibreries++++++++++++++*/
#include <LiquidCrystal.h>
/*+++++++++++++Llibreries++++++++++++++*/

/*++Declaració variables i constants+++*/
//Pins components
int const extruderDir = 2;  //motor extruder step
int const extruderStep = 3;  //motor extruder canviar dir
int const extruderDisable = 4; //motor extruder enable/holding o diable
int const coilDir = 5; //motor coil step
int const coilStep = 6;  //motor coil canviar dir
int const coilDisable = 7; //motor coil enable/holding o disable

int const hotTempOKLED = 8; //LED temperatura llesta per extrusió
int const coldTempOKLED = 9; //LED temperatura llesta per maipulació
int const switchFan = 10; //interruptor habilitar ventiladors
int const switchHeater = 11;  //interruptor habilitar escalfador
int const switchExtrude = 12; //interruptor motor extrusor
int const switchWind = 14;  //interruptor motor bobina
int const switchInvertDir = 15; //interruptor invertir direcció

int const fanRelay = 16;  //relé abilitar ventiladors

int const NTC = A0;

int const RS = 52, E = 53, d4 = 51, d5 = 49, d6 = 47, d7 = 45;
LiquidCrystal lcd(RS, E, d4, d5, d6, d7);

//variables temperatures
int const coldTempOKDegrees = 50; //temperatura freda
int const hotTempOKDegrees = 200; // temperatura calenta
int const extremeTemp = 230; //temperatura massa calenta
int tempK = 0.0;  //default "0.0"
int tempC = 0.0;  //default "0.0"

float VOut = 0.0; //default "0.0"
float rntc = 0.0; //default "0.0"

int const timeToStopStep = 1.5; //default "1.5"
int const timeBetweenSteps = 1.5; //default "1.5"

unsigned long current_time = 0; //default "0"

int errorCode = 0; //default "0"

//booleans per saber si es pot extrudir
bool check_extrudeByRefrigeration = false;
bool check_extrudeByTemp = false;
bool check_extrudeBySwitch = false;

//boolean per saber si ha saltat un error i parar tots els processos
bool fail = false;

/*++Declaració variables i constants+++*/

/*+++++++++++Declaracio funcions+++++++++++*/
void tempAction(); //funció per dur a terme diverses accions depenent de la temperatura
void callError(); //funció per escollir un missatge d'error i certes accions al respecte quan es cridi amb un codi d'error
void toggleRefrigeration();
void doStep();
void readTemp();
/*+++++++++++Declaracio funcions+++++++++++*/

/*+++++++++Configuració components+++++++++*/
void setup() { //Declaració de components a la placa
  Serial.begin(9600);

  pinMode(coldTempOKLED, OUTPUT); //LED temperatura llesta per maipulació
  pinMode(hotTempOKLED, OUTPUT);  //LED temperatura llesta per extrusió
  
  pinMode(extruderDir, OUTPUT);
  pinMode(extruderStep, OUTPUT);
  pinMode(extruderDisable, OUTPUT);
  pinMode(coilDir, OUTPUT);
  pinMode(coilStep, OUTPUT);
  pinMode(coilDisable, OUTPUT);

  pinMode(NTC, INPUT);

//procediments inicials pantalla
lcd.clear();
}
/*+++++++++Configuració components+++++++++*/

/*++++++++++++++++Processos++++++++++++++++*/
void loop() {
  while(fail =! true) {
}
}
/*++++++++++++++++Processos++++++++++++++++*/

/*+++++++++++Definició funicons++++++++++++*/
void doStep(int motor, int dir, int steps){
  switch (motor) {  //Estructura en switch per escollir direcció i motor
                    //motor 0 = extrusor
                    //motor 1 = bobina
                    //direcció 0 = forward
                    //direcció 1 = reverse
                    //steps = nombre de passos
    case 0:
      switch (dir) {
        case 0:
          while(steps != 0){
            --steps;
            
            Serial.print("Passos restants: ");
            Serial.println(steps);
            
            digitalWrite(extruderStep, HIGH);
            delay(timeToStopStep);
            digitalWrite(extruderStep, LOW);
            delay(timeBetweenSteps);

            
          }
          break;

        case 1:
          while(steps != 0) {

            Serial.print("Passos restants: ");
            Serial.println(steps);
            
            digitalWrite(extruderDir, HIGH);
            digitalWrite(extruderStep, HIGH);
            delay(timeToStopStep);
            digitalWrite(extruderStep, LOW);
            digitalWrite(extruderDir, LOW);
            --steps;
            delay(timeBetweenSteps);
          }
        break;
      }
      break;

    case 1:
      switch (dir) {
        case 0:

            Serial.print("Passos restants: ");
            Serial.println(steps);
            
            digitalWrite(coilStep, HIGH);
            delay(timeToStopStep);
            digitalWrite(extruderStep, LOW);
            --steps;
            delay(timeBetweenSteps);
          break;

        case 1:

            Serial.print("Passos restants: ");
            Serial.println(steps);
            
            digitalWrite(coilDir, HIGH);
            digitalWrite(coilStep, HIGH);
            delay(timeToStopStep);
            digitalWrite(coilStep, LOW);
            digitalWrite(coilDir, LOW);
            --steps;
            delay(timeBetweenSteps);
          break;
      }
      break;
    }
}

void readTemp(){
  if(millis() >= current_time + 2000) {
  VOut=(5.0 / 1023)*( analogRead(0) );
  rntc = 10000.0 / ((5/((5.0 / 1023)*( analogRead(0) )))-1);
  tempK = 3950.0/(log(rntc/100000.0)+(3950/298.0)); 
  tempC = tempK - 272.15;
  
  Serial.print("temp.: ");
  Serial.print(tempC);
  Serial.println("ºC");

  current_time = millis();
  
  }
}

void TempAction(){ //funció per dur a terme diverses accions depenent de la temperatura
  if(tempC < coldTempOKDegrees) {
    digitalWrite(coldTempOKLED, HIGH);
  }
  else if(tempC >= hotTempOKDegrees){
    digitalWrite(hotTempOKLED, HIGH);
  }
  else if(tempC > extremeTemp){
    callError(1);
  }
  else if(tempC == 0){
    callError(2);
  }
  else{
    digitalWrite(hotTempOKLED, LOW);
    digitalWrite(coldTempOKLED, LOW);
  }
}

void callError(int errorCode){//funció per escollir un missatge d'error i certes accions al respecte quan es cridi amb un codi d'error
  fail = true; //estableix que hi ha un error
  switch (errorCode){ //switch per escollir què fer depen de cada errorCode
    case(0):  //cas 0: sense missatge
    lcd.clear();
    lcd.setCursor(0,5);
    lcd.blink();
    lcd.print("ERROR ??");
    
      break;
      lcd.noBlink();
      
    case(1):  //cas 1: temp. massa alta
    lcd.clear();
    lcd.setCursor(0,5);
    lcd.blink();
    lcd.print("Carai! 1");
    lcd.setCursor(1,0);
    lcd.print("");
      break;
      lcd.noBlink();
      
    case(2):  //cas 2: temperatura és igual a "res", ergo no s'està llegint el sensor NTC, no respon la funció o hem arribat a la temp. de 0 absolut (0K, -273.15°C)...
    lcd.clear();
    lcd.setCursor(0,1);
    lcd.blink();
    lcd.print("Vatua l’olla! 2");
    lcd.setCursor(1,0);
    lcd.print("");
    
      break;
      lcd.noBlink();
      
  }
  
    Serial.print("Yeep! T'has estimbat! Codi: ");
    Serial.println(errorCode);
}

void toggleRefrigeration(){
  if(digitalRead(switchFan) == HIGH){
    digitalWrite(fanRelay, HIGH);
    Serial.println("Fans: ON");
    check_extrudeByRefrigeration = true;
    Serial.println("check_extruderByRefrigeration: TRUE");
  }
  else {
    digitalWrite(fanRelay, LOW);
    Serial.println("Fans: OFF");
    check_extrudeByRefrigeration = false;
    Serial.println("check_extruderByRefrigeration: FALSE");
  }
}

/*+++++++++++Definició funicons++++++++++++*/
