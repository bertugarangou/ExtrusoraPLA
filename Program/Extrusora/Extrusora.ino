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
/*+++++++++++++Llibreries++++++++++++++*/

/*++Declaració variables i constants+++*/
//Pins components
int const extruderDir = 2;  //motor extruder step
int const extruderStep = 3;  //motor extruder canviar dir
int const extruderEnable = 4; //motor extruder enable/holding o diable
int const coilDir = 5; //motor coil step
int const coilStep = 6;  //motor coil canviar dir
int const coilEnable = 7; //motor coil enable/holding o disable

int const hotTempOKLED = 8; //LED temperatura llesta per extrusió
int const coldTempOKLED = 9; //LED temperatura llesta per maipulació
int const switchFan = 10; //interruptor habilitar ventiladors
int const switchHeater = 11;  //interruptor habilitar escalfador
int const switchExtrude = 12; //interruptor motor extrusor
int const switchWind = 13;  //interruptor motor bobina
int const switchInvertDir = 14; //interruptor invertir direcció

int const fanRelay = 15;  //relé ventiladors

//variables temperatures
int const coldTempOKDegrees = 50; //temperatura freda
int const hotTempOKDegrees = 200; // temperatura calenta
int const extremeTemp = 230; //temperatura massa calenta
int currentTemp = 0; //temperatura actual actualitzada cada certs segons. Comença amb 0 per comprovar que s'actualitza, ja que mai arribarà als 0ºC

int const timeToStopStep = 2;
int const timeBetweenSteps = 2;
//booleans per saber si es pot extrudir
bool check_extrudeByRefrigeration = false;
bool check_extrudeByTemp = false;
bool check_extrudeBySwitch = false;

/*++Declaració variables i constants+++*/

/*+++++++++Configuració components+++++++++*/
void setup() { //Declaració de components a la placa
  pinMode(coldTempOKLED, OUTPUT); //LED temperatura llesta per maipulació
  pinMode(hotTempOKLED, OUTPUT);  //LED temperatura llesta per extrusió

}
/*+++++++++Configuració components+++++++++*/

/*+++++++++++Declaracio funcions+++++++++++*/
void tempAction(); //funció per dur a terme diverses accions depenent de la temperatura
void callError(); //funció per escollir un missatge d'error i certes accions al respecte quan es cridi amb un codi d'error
void toggleRefrigeration();
void doStep();
/*+++++++++++Declaracio funcions+++++++++++*/

/*++++++++++++++++Processos++++++++++++++++*/
void loop() {

}
/*++++++++++++++++Processos++++++++++++++++*/

/*+++++++++++Definició funicons++++++++++++*/
void doStep(int motor, int dir){
  switch (motor) {  //Estructura en switch per escollir direcció i motor
                    //motor 0 = extrusor
                    //motor 1 = bobina
                    //direcció 0 = forward
                    //direcció 1 = reverse
    case 0:
      switch (dir) {
        case 0:
          digitalWrite(extruderStep, HIGH);
          delay(timeToStopStep);
          digitalWrite(extruderStep, LOW);
          delay(timeBetweenSteps);
          break;
          
        case 1:
          digitalWrite(extruderDir, HIGH);
          digitalWrite(extruderStep, HIGH);
          delay(timeToStopStep);
          digitalWrite(extruderStep, LOW);
          digitalWrite(extruderDir, LOW);
          delay(timeBetweenSteps);
        break;
      }
      break;

    case 1:
      switch (dir) {
        case 0:
          digitalWrite(coilStep, HIGH);
          delay(timeToStopStep);
          digitalWrite(extruderStep, LOW);
          delay(timeBetweenSteps);
          break;
          
        case 1:
          digitalWrite(coilDir, HIGH);
          digitalWrite(coilStep, HIGH);
          delay(timeToStopStep);
          digitalWrite(coilStep, LOW);
          digitalWrite(coilDir, LOW);
          delay(timeBetweenSteps);
          break;
      }
      break;
    }
}

void TempAction(){ //funció per dur a terme diverses accions depenent de la temperatura
  if(currentTemp < coldTempOKDegrees) {
    digitalWrite(coldTempOKLED, HIGH);
  }
  else if(currentTemp >= hotTempOKDegrees){
    digitalWrite(hotTempOKLED, HIGH);
  }
  else if(currentTemp > extremeTemp){
    callError(1);
  }
  else if(currentTemp == 0){
    callError(2);
  }
  else{
    digitalWrite(hotTempOKLED, LOW);
    digitalWrite(coldTempOKLED, LOW);
  }
}

void callError(int errorCode){//funció per escollir un missatge d'error i certes accions al respecte quan es cridi amb un codi d'error
  switch (errorCode){
    case(0):
    //print on screen "ERROR 0"
      break;
    case(1):
      //print on screen "Temp. massa alta"
      break;
    case(2):
    //print on screen "Temp no disponible"
      break;
  }
}

void toggleRefrigeration(){
  if(digitalRead(switchFan) == HIGH){
    digitalWrite(fanRelay, HIGH);
    check_extrudeByRefrigeration = true;
  }
  else {
    digitalWrite(fanRelay, LOW);
  }
}

/*+++++++++++Definició funicons++++++++++++*/
