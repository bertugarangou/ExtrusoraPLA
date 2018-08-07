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

/*++Declaració variables i constants+++*/
int const extrIN1 = 2; //pins motor passos EXTRUSOR
int const extrIN2 = 3;
int const extrIN3 = 4;
int const extrIN4 = 5;
int const coilIN1 = 6;  //pins motor BOBINA
int const coilIN2 = 7;
int const coilIN3 = 8;
int const coilIN4 = 9;

int a = 0;
int b = 0;

int const stepDelay = 1; //canviar a 1 o 2. Temps entre actualitzacions de les bobines dels motors perque no perdi passos

int stepGuide [4][4] = {  //iniciem la matriu i li diguem que és de 4x4 valors, cada valor 1 significa una bobina encesa, que atrau l'eix cap a ella. D'aquesta manera es crea el mobiment de l'eix del motor
  {1, 1, 0, 0}, //En forma d'escala, es van activant i desactivant les bobines per crear els camps magnètics
  {0, 1, 1, 0}, //Cada índex és un pas
  {0, 0, 1, 1},
  {1, 0, 0, 1}
  };


/*++Declaració variables i constants+++*/

/*++++++++++Declaracio components++++++++++*/
void setup() { //Declaració de components a la placa
  pinMode(extrIN1, OUTPUT); //4 pins pel motor de l'extrusora
  pinMode(extrIN2, OUTPUT);
  pinMode(extrIN3, OUTPUT);
  pinMode(extrIN4, OUTPUT);
  pinMode(coilIN1, OUTPUT); //4 pins pel motor de la bobina
  pinMode(coilIN2, OUTPUT);
  pinMode(coilIN3, OUTPUT);
  pinMode(coilIN4, OUTPUT);

}
/*++++++++++Declaracio components++++++++++*/

/*+++++++++++Declaracio funcions+++++++++++*/
void doStep();
/*+++++++++++Declaracio funcions+++++++++++*/

/*++++++++++++++++Processos++++++++++++++++*/
void loop() {
  while(a < 10){
  doStep(0,0);  //fer un pas del motor 0 (extrusor) a direcció 0 (forward)
  doStep(1,0);  //fer un pas del motor 0 (bobina) a direcció 0 (forward)
  a++;
  }
  a = 0;
  while(b < 10){
  doStep(0,1);  //fer un pas del motor 0 (extrusor) a direcció 1 (forward)
  doStep(1,1);  //fer un pas del motor 0 (bobina) a direcció 1 (forward)
  b++;
  }
  b = 0;
}
/*++++++++++++++++Processos++++++++++++++++*/

/*+++++++++++Declaracio funicons+++++++++++*/
void doStep(int motor, int dir){
  switch (motor) {  //Estructura en switch per escollir direcció i motor
                    //motor 0 = extrusor
                    //motor 1 = bobina
                    //direcció 0 = forward
                    //direcció 1 = reverse

    case 0:
      switch (dir) {
        case 0:
          for (int countExtruder = 0; countExtruder < 4; countExtruder++) { //Es fa un compta fins a 4 per saber quin índex de la matriu escollir, cada índex és un pas
            digitalWrite(extrIN1, stepGuide [countExtruder][0]); //Un "digitalWrite" (escriptura en pins digitals) per cada pin del motor
            delay(stepDelay);
            digitalWrite(extrIN2, stepGuide [countExtruder][1]);
            delay(stepDelay);
            digitalWrite(extrIN3, stepGuide [countExtruder][2]);
            delay(stepDelay);
            digitalWrite(extrIN4, stepGuide [countExtruder][3]);
            delay(stepDelay);
          }
          break;
        case 1:
          for (int countExtruder = 0; countExtruder < 4; countExtruder--) { //Es fa un compta fins a 4 per saber quin índex de la matriu escollir, cada índex és un pas
            digitalWrite(extrIN1, stepGuide [countExtruder][0]); //Un "digitalWrite" (escriptura en pins digitals) per cada pin del motor
            delay(stepDelay);
            digitalWrite(extrIN2, stepGuide [countExtruder][1]);
            delay(stepDelay);
            digitalWrite(extrIN3, stepGuide [countExtruder][2]);
            delay(stepDelay);
            digitalWrite(extrIN4, stepGuide [countExtruder][3]);
            delay(stepDelay);
          }
        break;
      }
      break;

    case 1:
      switch (dir) {
        case 0:
          for (int countCoil = 0; countCoil < 4; countCoil++) { //Es fa un compta fins a 4 per saber quin índex de la matriu escollir, cada índex és un pas
            digitalWrite(coilIN1, stepGuide [countCoil][0]); //Un "digitalWrite" (escriptura en pins digitals) per cada pin del motor
            delay(stepDelay);
            digitalWrite(coilIN2, stepGuide [countCoil][1]);
            delay(stepDelay);
            digitalWrite(coilIN3, stepGuide [countCoil][2]);
            delay(stepDelay);
            digitalWrite(coilIN4, stepGuide [countCoil][3]);
            delay(stepDelay);
          }
          break;
        case 1:
          for (int countCoil = 0; countCoil < 4; countCoil--) { //Es fa un compta fins a 4 per saber quin índex de la matriu escollir, cada índex és un pas
            digitalWrite(coilIN1, stepGuide [countCoil][0]); //Un "digitalWrite" (escriptura en pins digitals) per cada pin del motor
            delay(stepDelay);
            digitalWrite(coilIN2, stepGuide [countCoil][1]);
            delay(stepDelay);
            digitalWrite(coilIN3, stepGuide [countCoil][2]);
            delay(stepDelay);
            digitalWrite(coilIN4, stepGuide [countCoil][3]);
            delay(stepDelay);
          }
          break;
      }
      break;
    }
}

/*+++++++++++Declaracio funicons+++++++++++*/
