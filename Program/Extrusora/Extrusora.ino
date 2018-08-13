

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

#define TASKER_MAX_TASKS 20
#include <Tasker.h>
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

int const switchFansFilament = 10; //interruptor habilitar ventiladors filament
int const switchFanArduino = 11; //interruptor habilitar ventilador placa arduino
int const switchFanCoil = 12; //interruptor habilitar ventilador bobina
int const switchFanControllers = 13; //interruptor habilitar refrigeració controladores TB6600
int const switchFanTube = 14; //interruptor habilitar refrigeració del tub
int const switchResistor = 15;  //interruptor habilitar escalfador
int const switchExtrude = 16; //interruptor motor extrusora
int const switchWind = 17;  //interruptor motor bobina 
int const switchExtruderInvert = 18; //interruptor motor extrusora invertir direcció
int const switchWinderInvert = 19; //interruptor motor bobina invertir direcció

int const fanRelayFilament = 20;  //relé habilitar ventiladors filament
int const fanRelayArduino = 21; //relé habilitar ventilador Arduino
int const fanRelayControllers = 22; //relé habilitar ventilador controladores TB6600
int const fanRelayWinder = 23;  //relé habilitar ventilador bobina
int const fanRelayTube = 24; //relé resistència per refredar el tub

int const resistorSSRelay = 25; //relé d'estat sòlid de la resistència del tub

int const NTC = A0; //NTC temperatura

int const RS = 52, E = 53, d4 = 51, d5 = 49, d6 = 47, d7 = 45;  //pins pantalla lcd

//variables temperatures
int const coldTempMin = 1;   //temperatura freda mínima per manipular (podria omitir-se, però quan és 0 també pot ser degut a un error de lectura)
int const coldTempMax = 50;  //temperatura freda màxima per manipular
int const coldTemp = 50; //temperatura freda predeterminada
int const hotTempMin = 198;  //temperatura calenta mínima per extrudir
int const hotTempMax = 220;  //temperatura calenta màxima per extrudir
int const hotTemp = 205; //temperatura calenta per extrudir

int const extremeTemp = 225; //temperatura massa calenta


int tempK = 0.0;  //default "0.0"     //temperatura actual en Kelvins
int tempC = 0.0;  //default "0.0"     //temperatura actual en Centígrafs

float VOut = 0.0; //default "0.0"     //pel calcul de raw a tº de la NTC
float rntc = 0.0; //default "0.0"     //pel calcul de raw a tº de la NTC

int const timeToStopStep = 1.5; //default "1.5"     //temps entre activar pas i desactivar-lo
int const timeBetweenSteps = 1.5; //default "1.5"     //temps entre dos passos

unsigned long currentTimeReadTemp = 0; //default "0"     //temps actual obtingut de "millis()"
unsigned long currentTimeHeater = 0;

int tempVariation;

int errorCode = 0; //default "0"      //codi d'error que llança una funció quan algo no funciona degudament

//booleans per saber si es pot extrudir
bool check_extrudeByRefrigeration = false;  //comprovació per extrudir segons refrigeració
bool check_extrudeByTemp = false; //comprovació per extrudir segons temperatura
bool check_extrudeBySwitch = false; //comprovació per extrudir segons interruptor per extrudir
//booleans per saber si es pot extrudir del ventiladors (fills de check_extrudeByRefrigeration)
bool check_arduinoFan = false;
bool check_controllersFans = false;
bool check_tubeFan = false;
bool check_filamentFans = false;
bool check_coilFan = false;

//Pre-configuració de llibreries
Tasker tasker;
LiquidCrystal lcd(RS, E, d4, d5, d6, d7); //declaració pantalla lcd
/*++Declaració variables i constants+++*/

/*+++++++++++Declaracio funcions+++++++++++*/
void ledAction(); //funció per dur a terme diverses accions depenent de la temperatura actual
void fansSwitches(); //funció habilitar/deshabilitar refrigeració
void readTemp();//funció per llegir la temp. actual i mostrar-la a la pantalla
void ExtruderSwitches(); //funció per fer passos al motor del extrusor a través dels interruptors (pot substituir doStep() en un futur)
void WinderSwitches();  //funció per fer passos al motor de la bobina a través dels interruptors (pot substituir doStep() en un futur)
void heaterSwitch();
void tempCheck();
void fansCheck();
/*+++++++++++Declaracio funcions+++++++++++*/

/*+++++++++Configuració components+++++++++*/
void setup() { //Declaració de components a la placa
  Serial.begin(9600); //inicia la depuració
  
pinMode(extruderDir, OUTPUT);
pinMode(extruderStep, OUTPUT);
pinMode(extruderDisable, OUTPUT);
pinMode(coilDir, OUTPUT);
pinMode(coilStep, OUTPUT);
pinMode(coilDisable, OUTPUT);

pinMode(hotTempOKLED, OUTPUT);
pinMode(coldTempOKLED, OUTPUT);

pinMode(switchFansFilament, INPUT);
pinMode(switchFanArduino, INPUT);
pinMode(switchFanCoil, INPUT);
pinMode(switchFanControllers, INPUT);
pinMode(switchFanTube, INPUT);

pinMode(switchResistor, INPUT);

pinMode(switchExtrude, INPUT);
pinMode(switchWind, INPUT);
pinMode(switchExtruderInvert, INPUT);
pinMode(switchWinderInvert, INPUT);

pinMode(fanRelayFilament, OUTPUT);
pinMode(fanRelayArduino, OUTPUT);
pinMode(fanRelayControllers, OUTPUT);
pinMode(fanRelayWinder, OUTPUT);
pinMode(fanRelayTube, OUTPUT);

pinMode(NTC, INPUT);

pinMode(resistorSSRelay, OUTPUT);

//procediments inicials pantalla
lcd.clear(); //buidar i esatblir 0,0 el cursor

//tasques independents
tasker.setInterval(readTemp, 2000);
tasker.setInterval(ledAction, 2500);
}
/*+++++++++Configuració components+++++++++*/

/*++++++++++++++++Processos++++++++++++++++*/
void loop() { //funció dins "main" que es repeteix en bucle
  while(errorCode == 0){
  tasker.setTimeout(ExtruderSwitches, 1);
  tasker.setTimeout(WinderSwitches, 1);
  tasker.setTimeout(WinderSwitches, 1);
  tasker.setTimeout(heaterSwitch, 1);
  tasker.loop();
  }
  //posar aquí sota els procediments per quan hi hagi un error (errorCode !=0)
}
/*++++++++++++++++Processos++++++++++++++++*/

/*+++++++++++Definició funicons++++++++++++*/

void tempCheck(){
  if(tempC < hotTempMax && tempC > hotTempMin) {
    check_extrudeByTemp = true;
  }
  else {
    check_extrudeByTemp = false;
  }
}

void fansCheck(){
  if(check_arduinoFan == true && check_controllersFans == true && check_tubeFan == true && check_filamentFans == true && check_coilFan == true){
    check_extrudeByRefrigeration = true;
  }
  else{
    check_extrudeByRefrigeration = false;
  }
}

void ExtruderSwitches(){  //funció per fer passos a través dels interruptors
  fansCheck();
  tempCheck();
  if(check_extrudeByRefrigeration == true && check_extrudeByTemp == true) {
    if(switchExtrude == HIGH && switchExtruderInvert == LOW){
      Serial.println("Motor: Extruder: ON");
      digitalWrite(extruderStep, HIGH);
      delay(timeToStopStep);
      digitalWrite(extruderStep, LOW);
      delay(timeBetweenSteps);

    }
    else if(switchExtrude == HIGH && switchExtruderInvert == HIGH){
      Serial.println("Motor: Extruder: REVERSE");
      digitalWrite(extruderDir, HIGH);
      digitalWrite(extruderStep, HIGH);
      delay(timeToStopStep);
      digitalWrite(extruderStep, LOW);
      digitalWrite(extruderDir, LOW);
      delay(timeBetweenSteps);

    }
    else {
      Serial.println("Motor: Extruder: OFF");
      digitalWrite(extruderDisable, HIGH);
    }
  }
}

void WinderSwitches(){  //funció per fer passos a través dels interruptors
  if(switchWind == HIGH && switchWinderInvert == LOW){
    Serial.println("Motor: WInder: ON");
    digitalWrite(coilStep, HIGH);
    delay(timeToStopStep);
    digitalWrite(coilStep, LOW);
    delay(timeBetweenSteps);
  }
  else if(switchWind == HIGH && switchWinderInvert == HIGH){
    Serial.println("Motor: Winder: REVERSE");
    digitalWrite(coilDir, HIGH);
    digitalWrite(coilStep, HIGH);
    delay(timeToStopStep);
    digitalWrite(coilStep, LOW);
    digitalWrite(coilDir, LOW);
    delay(timeBetweenSteps);
  }
  else {
    Serial.println("Motor: Winder: OFF");
    digitalWrite(coilDisable, HIGH);
  }
}


void heaterSwitch(){
  tempCheck();
  fansCheck();
  if(check_extrudeByTemp == true && check_extrudeByRefrigeration == true){
    if(switchResistor == HIGH){
  
      tempVariation = hotTemp - tempC;
      while(tempVariation > 5){ //de 50 a 199
        digitalWrite(resistorSSRelay, HIGH);
      }
  
      while(tempVariation <= 5 && tempVariation > 0){ //de 200 a 204
        currentTimeHeater = millis();
        do{
          digitalWrite(resistorSSRelay, HIGH);
        } while(millis() < currentTimeHeater + tempVariation);
        digitalWrite(resistorSSRelay, LOW);
      }
  
      digitalWrite(resistorSSRelay, LOW); //205 o més
  }
}
  
  else{
    digitalWrite(resistorSSRelay, LOW);
  }
}

void readTemp(){
  //if(millis() >= currentTimeReadTemp + 2000) {
  VOut=(5.0 / 1023)*( analogRead(0) );
  rntc = 10000.0 / ((5/((5.0 / 1023)*( analogRead(0))))-1);
  tempK = 3950.0/(log(rntc/100000.0)+(3950/298.0)); 
  tempC = tempK - 272.15;

  lcd.setCursor(0,0);
  lcd.print(tempC);
  lcd.print("ºC");
  
  Serial.print("temp.: ");
  Serial.print(tempC);
  Serial.println("ºC");

  //currentTimeReadTemp = millis();
  //}
}

void ledAction(){ //funció per encendre els LEDs de temperatura
  if(tempC <= coldTempMax && tempC >= coldTempMin) {
    digitalWrite(coldTempOKLED, HIGH);
    digitalWrite(hotTempOKLED, LOW);
  }
  else if(tempC >= hotTempMin && tempC <= hotTempMax){
    digitalWrite(hotTempOKLED, HIGH);
    digitalWrite(coldTempOKLED, LOW);
  }
  else if(tempC > extremeTemp){
    digitalWrite(hotTempOKLED, LOW);
    digitalWrite(coldTempOKLED, LOW);
    errorCode = 1;
  }
  else if(tempC == 0){
    digitalWrite(hotTempOKLED, LOW);
    digitalWrite(coldTempOKLED, LOW);
    errorCode = 2;
  }
  else{
    digitalWrite(hotTempOKLED, LOW);
    digitalWrite(coldTempOKLED, LOW);

  }
}

void fansSwitches(){
  if(switchFanArduino == HIGH){
    digitalWrite(fanRelayArduino, HIGH);
    check_arduinoFan = true;
    Serial.println("Fan: Arduino: ON");
  }
  else {
    digitalWrite(fanRelayArduino, LOW);
      check_arduinoFan = false;
      Serial.println("Fan: Arduino: OFF");
  }
  
  if(switchFanControllers == HIGH){
    digitalWrite(fanRelayControllers, HIGH);
    check_controllersFans = true;
    
      Serial.println("Fan: Controllers: ON");
  }
  else {
    digitalWrite(fanRelayControllers,LOW);
    check_controllersFans = false;
    
      Serial.println("Fan: Controllers: OFF");
  }

  if(switchFansFilament == HIGH){
    digitalWrite(fanRelayFilament, HIGH);
    check_filamentFans = true;
    
      Serial.println("Fan: Filament: ON");
  }
  else {
    digitalWrite(fanRelayFilament, LOW);
    check_filamentFans = false;
    
      Serial.println("Fan: Filament: OFF");
  }

  if(switchFanCoil == HIGH){
    digitalWrite(fanRelayWinder, HIGH);
    check_coilFan = true;
    
    Serial.println("Fan: Coil: ON");
  }
  else{
    digitalWrite(fanRelayWinder, LOW);
    check_coilFan = false;
    
    Serial.println("Fan: Coil: OFF");
  }

  if(switchFanTube == HIGH){
    digitalWrite(fanRelayTube, HIGH);
    check_tubeFan = true;
    
    Serial.println("Fan: Tube: ON");
  }
  else{
    digitalWrite(fanRelayTube, LOW);
    check_tubeFan = false;
    
    Serial.println("Fan: Tube: OFF");
  }
}
/*+++++++++++Definició funicons++++++++++++*/
