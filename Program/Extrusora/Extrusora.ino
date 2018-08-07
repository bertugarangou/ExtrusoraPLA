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
#include <Stepper.h>
/*+++++++++++++Llibreries++++++++++++++*/

/*++Declaració variables i constants+++*/
Stepper extruder(200, 2, 3, 4, 5); //declara un motor pas-a-pas "extrusor" de "200" passos per rotació
Stepper coil(200, 6, 7, 8, 9);  //declara un motor pas-a-pas "coil" de "200" passos per rotació
int const hotTempOKLED = 10; //LED temperatura llesta per extrusió
int const coldTempOKLED = 11; //LED temperatura llesta per maipulació


int const coldTempOKDegrees = 50; //temperatura llesta per manipulació
int const hotTempOKDegrees = 200; // temperatura llesta per extrusió
int const extremeTemp = 230; //temperatura màxima a la que pot arribar abans de activar refrigeració d'emergència.
int currentTemp = 0; //temperatura actual actualitzada cada certs segons. Comença amb 0 per comprovar que s'actualitza, ja que mai arribarà als 0ºC
/*++Declaració variables i constants+++*/

/*+++++++++Configuració components+++++++++*/
void setup() { //Declaració de components a la placa
  extruder.setSpeed(100); //motor pas-a-pas a velocitat inicial indicada PPRPM
  coil.setSpeed(100); //motor pas-a-pas de la bobina a velocitat inicial indicada PPRPM
  
  pinMode(coldTempOKLED, OUTPUT); //LED temperatura llesta per maipulació
  pinMode(hotTempOKLED, OUTPUT);  //LED temperatura llesta per extrusió
}
/*+++++++++Configuració components+++++++++*/

/*+++++++++++Declaracio funcions+++++++++++*/
void coldTempOK();
void callError();
/*+++++++++++Declaracio funcions+++++++++++*/

/*++++++++++++++++Processos++++++++++++++++*/
void loop() {

}
/*++++++++++++++++Processos++++++++++++++++*/

/*+++++++++++Definició funicons++++++++++++*/
void TempAction(){
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

void callError(int errorCode){
  switch (errorCode){
    case(0):
    //print on screen "ERROR 0"
      break;
    case(1):
      //print on screen "Temp. massa alta"
      break;
  }
}

/*+++++++++++Definició funicons++++++++++++*/
