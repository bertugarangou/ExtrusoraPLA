/*

  Extrusora de PLA controlada per Arduino Mega 2560.

  Circuit:
  - Extrusora feta per Albert Garangou,
    com a Treball de Recerca a 2n de Batxillerat,
    curs 2018/2019, tutor Jordi Fanals Oriol,
    codi amb Arduino IDE 1.8.5,

    Copyright (c) 2018 Albert Garangou Culebras (albertgarangou@gmail.com)
    This Font Software is licensed under the SIL Open Font License, Version 1.1.

*/

/*++++++++++Declaracio components++++++++++*/
void setup() { //Declaració de components a la placa
  pinMode(extrIN1, OUTPUT); //4 seguents per les entrades del segon motor, el del extrusor
  pinMode(extrIN2, OUTPUT);
  pinMode(extrIN3, OUTPUT);
  pinMode(extrIN4, OUTPUT);
}
/*++++++++++Declaracio components++++++++++*/
/*++Declaració variables i constants+++*/
int const extrIN1 = 2; //Pins motor passos EXTRUSOR
int const extrIN2 = 3;
int const extrIN3 = 4;
int const extrIN4 = 5;
int const stepDelay = 1; //temps entre actualitzacions de les bobines dels motors perque no perdi passos
int pasGuide [4][4] = {  //iniciem la matriu i li diguem que és de 4x4 valors, cada valor 1 significa una bobina encesa, que atrau l'eix cap a ella. D'aquesta manera es crea el mobiment de l'eix del motor
  {1, 1, 0, 0}, //En forma d'escala, es van activant i desactivant les bobines per crear els camps magnètics
  {0, 1, 1, 0}, //Cada índex és un pas
  {0, 0, 1, 1},
  {1, 0, 0, 1}
};
/*++Declaració variables i constants+++*/

/*+++++++++++Declaracio funcions+++++++++++*/
/*+++++++++++Declaracio funcions+++++++++++*/

/*++++++++++++++++Processos++++++++++++++++*/
void loop() {
}
/*++++++++++++++++Processos++++++++++++++++*/

/*+++++++++++Declaracio funicons+++++++++++*/
void Extruder() {
    for (int countExtruder = 0; countExtruder < 4; countExtruder++) { //Es fa un compta fins a 4 per saber quin índex de la matriu escollir, cada índex és un pas
      digitalWrite(extrIN1, pasGuide [countExtruder][0]); //Un "digitalWrite" (escriptura en pins digitals) per cada pin del motor
      delay(stepDelay);
      digitalWrite(extrIN2, pasGuide [countExtruder][1]); //Pel motor de l'extrusor
      delay(stepDelay);
      digitalWrite(extrIN3, pasGuide [countExtruder][2]);
      delay(stepDelay);
      digitalWrite(extrIN4, pasGuide [countExtruder][3]);
      delay(stepDelay); //Es repeteix i s'asigna un índex diferent cada X segons, per separar cada pas
  }
}
/*+++++++++++Declaracio funicons+++++++++++*/
