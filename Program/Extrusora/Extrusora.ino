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
#include <LiquidCrystal.h>  //importa la llibreria per la LCD

#define TASKER_MAX_TASKS 20 //estableix, de la següent llibreria, que 20 tasques a la vegada poden funcionar a la vegada
#include <Tasker.h> //importa la llibreria pel multitasking
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
int const switchFanControllers = 14; //interruptor habilitar refrigeració controladores TB6600
int const switchFanTube = 15; //interruptor habilitar refrigeració del tub
int const switchResistor = 16;  //interruptor habilitar escalfador
int const switchExtrude = 17; //interruptor motor extrusora
int const switchWind = 18;  //interruptor motor bobina
int const switchExtruderInvert = 19; //interruptor motor extrusora invertir direcció
int const switchWinderInvert = 20; //interruptor motor bobina invertir direcció

int const fanRelayFilament = 21;  //relé habilitar ventiladors filament
int const fanRelayArduino = 22; //relé habilitar ventilador Arduino
int const fanRelayControllers = 23; //relé habilitar ventilador controladores TB6600
int const fanRelayWinder = 24;  //relé habilitar ventilador bobina
int const fanRelayTube = 25; //relé resistència per refredar el tub

int const resistorSSRelay = 26; //relé d'estat sòlid de la resistència del tub

int const stopButton = 27;  //botó aturada d'emergència

int const NTC = A0; //NTC temperatura

int const RS = 52, E = 53, d4 = 51, d5 = 49, d6 = 47, d7 = 45;  //pins pantalla lcd

//variables temperatures
int const coldTempMin = 1;   //temperatura freda mínima per manipular (podria omitir-se, però quan és 0 també pot ser degut a un error de lectura)
int const coldTempMax = 50;  //temperatura freda màxima per manipular
int const coldTemp = 50; //temperatura freda predeterminada
int const hotTempMin = 170;  //temperatura calenta mínima per extrudir
int const hotTempMax = 190;  //temperatura calenta màxima per extrudir
int const hotTemp = 180; //temperatura calenta per extrudir

int const extremeTemp = 200; //temperatura massa calenta

int tempK = 0.0;  //default "0.0"     //temperatura actual en Kelvins
int tempC = 0.0;  //default "0.0"     //temperatura actual en Centígrafs

int const ntcCorrection = -1; //correció del valor raw de la NTC
float rntc = 0.0; //default "0.0"     //pel calcul de raw a tº de la NTC

int rpm = analogRead(A1); //valor llegit del potenciometre per calcular la velocitat dels motors
int stepperSpeed; //temps entre activar un pas i desactivar-lo = velocitat motors
int const timeBetweenSteps = 1.5; //default "1.5"     //temps entre dos passos

unsigned long currentTimeReadTemp = 0; //default "0"     //temps actual obtingut de "millis()" actualitzad per la funció readTemp()
unsigned long currentTimeHeater = 0;  //default "0"     //temps actual obringut de "millis()" actualitzat per la funció heaterSwitch()

int tempVariation;  //diferència entre  tempC i hotTemp

int errorCode = 0; //default "0"      //codi d'error que llança una funció quan algo no funciona degudament

//booleans per saber si es pot extrudir
bool check_extrudeByRefrigeration = false;  //comprovació per extrudir segons refrigeració
bool check_extrudeByTemp = false; //comprovació per extrudir segons temperatura
bool check_extrudeBySwitch = false; //comprovació per extrudir segons interruptor per extrudir
//booleans per saber si es pot extrudir del ventiladors (fills de check_extrudeByRefrigeration)
bool check_arduinoFan = false;  //comprova si el ventilador de l'Arduino està operatiu
bool check_controllersFans = false; //comprova si el ventilador dels controladors està operatiu
bool check_tubeFan = false; //comprova si el ventilador del tub està operatiu
bool check_filamentFans = false;  //comprova si els ventiladors del filament estan operatius
bool check_coilFan = false; //comprova si el ventilador de la bobina està operatiu

//booleans processos per lcd
bool extruding = false;
bool winding = false;
//Pre-configuració de llibreries
Tasker tasker;  //estableix que no hi ha prioritats de tasques (Tasker tasker(TRUE) si n'hi ha)
LiquidCrystal lcd(RS, E, d4, d5, d6, d7); //declaració pantalla lcd
/*++Declaració variables i constants+++*/

/*+++++++++++Declaracio funcions+++++++++++*/
void ledAction(); //funció per dur a terme diverses accions depenent de la temperatura actual
void fansSwitches(); //funció habilitar/deshabilitar refrigeració
void readTemp();//funció per llegir la temp. actual i mostrar-la a la pantalla
void ExtruderSwitches(); //funció per fer passos al motor del extrusor a través dels interruptors
void WinderSwitches();  //funció per fer passos al motor de la bobina a través dels interruptors
void heaterSwitch();  //funció per escalfar la el tub a través dels interruptors
void tempCheck(); //funció per comprovar que la temp. sigui correcta per poder extrudir
void fansCheck(); //funció per comprovar quins ventiladors estan operatius per poder extrudir
void lcdProcessess(); //funció per actualitzar la pantalla amb els processos actuals
/*+++++++++++Declaracio funcions+++++++++++*/

/*+++++++++Configuració components+++++++++*/
void setup() { //Declaració de components a la placa
  //OUTPUT: surt senyal de la placa (leds, motors, etc.)
  //INPUT: entra senyal a la placa( botons, NTC, sensors...)
  Serial.begin(9600); //inicia la depuració

  //components motor
  pinMode(extruderDir, OUTPUT);
  pinMode(extruderStep, OUTPUT);
  pinMode(extruderDisable, OUTPUT);
  pinMode(coilDir, OUTPUT);
  pinMode(coilStep, OUTPUT);
  pinMode(coilDisable, OUTPUT);

  //LEDs
  pinMode(hotTempOKLED, OUTPUT);
  pinMode(coldTempOKLED, OUTPUT);

  //interruptors ventiladors
  pinMode(switchFansFilament, INPUT);
  pinMode(switchFanArduino, INPUT);
  pinMode(switchFanCoil, INPUT);
  pinMode(switchFanControllers, INPUT);
  pinMode(switchFanTube, INPUT);

  //interruptor resistència
  pinMode(switchResistor, INPUT);

  //interruptors motors
  pinMode(switchExtrude, INPUT);
  pinMode(switchWind, INPUT);
  pinMode(switchExtruderInvert, INPUT);
  pinMode(switchWinderInvert, INPUT);

  //relés ventiladors
  pinMode(fanRelayFilament, OUTPUT);
  pinMode(fanRelayArduino, OUTPUT);
  pinMode(fanRelayControllers, OUTPUT);
  pinMode(fanRelayWinder, OUTPUT);
  pinMode(fanRelayTube, OUTPUT);

  //NTC
  pinMode(NTC, INPUT);

  //botó d'emergència
  pinMode(stopButton, INPUT);

  //relé d'Estat Sòlid de la resistènica
  pinMode(resistorSSRelay, OUTPUT);

  //procediments inicials de la pantalla
  lcd.clear(); //buidar i esatblir 0,0 el cursor

  //tasques periòdiques (es repeteixen cada X milisegons)
  tasker.setInterval(readTemp, 2000);
  tasker.setInterval(ledAction, 2500);
}
/*+++++++++Configuració components+++++++++*/

/*++++++++++++++++Processos++++++++++++++++*/
void loop() { //funció dins "main" que es repeteix en bucle
  if(digitalRead(stopButton) == LOW){ //si el botó no està pres
    while(errorCode == 0){  //mentre errorCode sigui 0
      tasker.setTimeout(ExtruderSwitches, 1); //registra la tasca
      tasker.setTimeout(WinderSwitches, 1); //registra la tasca
      tasker.setTimeout(heaterSwitch, 1); //registra la tasca
      tasker.setTimeout(fansSwitches, 1); //registra la tasca
      tasker.setTimeout(lcdProcessess, 1);  //registra la tasca
      tasker.loop();  //executa les tasques registrades
    }

    while(errorCode != 0){  //mentre errorCode no sigui 0
      Serial.println("ERROR TROBAT: "); //envia per depuració el missatge
      Serial.println(errorCode);  //enviaper depuració la variable
      tasker.cancel(readTemp);  //cancela la tasca periòdica programada
      tasker.cancel(ledAction); //cancela la tasca perióida programada

      switch(errorCode){  //depen de què conté errorCode
        case 1: //si és 1: temperatura massa elevada
          do { //fes
            lcd.clear();  //neteja la lcd
            lcd.print("TEMP ELEVADA !");  //imprimeix a la pantalla
            lcd.setCursor(1,0); //mou el cursos a la segona línia
            lcd.print("espera/reinicia"); //imprimeix a la pantalla

            check_extrudeByTemp = false;  //estableix el boolean false
            check_arduinoFan = false; //estableix el boolean fals
            check_controllersFans = false;  //estableix el boolean fals
            check_tubeFan = false;  //estableix el boolean fals
            check_filamentFans = false; //estableix el boolean fals
            check_coilFan = false;  //estableix el boolean fals
            check_extrudeByRefrigeration = false; //estableix el boolean fals

            digitalWrite(extruderDisable, HIGH);  //deshabilita el motor
            digitalWrite(coilDisable, HIGH);  //deshabilita el motor

            digitalWrite(fanRelayFilament, HIGH); //activa el relé del ventilador
            digitalWrite(fanRelayArduino, HIGH);  //activa el relé del ventilador
            digitalWrite(fanRelayControllers, HIGH);  //activa el relé del ventilador
            digitalWrite(fanRelayWinder, HIGH); //activa el relé del ventilador
            digitalWrite(fanRelayTube, HIGH); //activa el relé del ventilador

            readTemp(); //executa la funció
          } while(tempC > extremeTemp); //fes tot l'anteriar mentres tempC sigui menor a extremeTemp
          digitalWrite(extruderDisable, LOW); //activa el motor
          digitalWrite(coilDisable, LOW); //activa el motor
          errorCode = 0;  //estableix errorCode 0
          break;  //surt del switch

        case 2: //si és 2: NTC llegeix 0
          do{ //fes
            lcd.clear();  //neteja la lcd
            lcd.print("NTC NO RESPON !"); //imprimeix a la lcd
            lcd.setCursor(1,0); //mou el cursor a la segona línia
            lcd.print("espera/reinicia"); //imprimeix a la pantalla

            check_extrudeByTemp = false;  //estableix el boolean false
            check_arduinoFan = false;  //estableix el boolean false
            check_controllersFans = false;  //estableix el boolean false
            check_tubeFan = false;  //estableix el boolean false
            check_filamentFans = false;  //estableix el boolean false
            check_coilFan = false;  //estableix el boolean false
            check_extrudeByRefrigeration = false;  //estableix el boolean false

            digitalWrite(extruderDisable, HIGH);  //deshabilita el motor
            digitalWrite(coilDisable, HIGH);  //deshabilita el motor

            digitalWrite(fanRelayFilament, HIGH); //activa el relé del ventilador
            digitalWrite(fanRelayArduino, HIGH); //activa el relé del ventilador
            digitalWrite(fanRelayControllers, HIGH); //activa el relé del ventilador
            digitalWrite(fanRelayWinder, HIGH); //activa el relé del ventilador
            digitalWrite(fanRelayTube, HIGH); //activa el relé del ventilador

            Serial.print("ERROR: ");  //envia per depuració el missatge
            Serial.println(errorCode);  //envia per depuració el missatge
            Serial.println("ESPERA'T A QUE ES RECUPERI EL CONTROL O REINICIA L'ARDUINO"); //envia per depuració el missatge
          } while(tempC == 0); //fes tot l'anteriar mentres tempC sigui menor a extremeTemp
        digitalWrite(extruderDisable, LOW); //activa el motor
        digitalWrite(coilDisable, LOW); //activa el motor
        errorCode = 0;  //estableix errorCode 0
        break;  //surt del switch
      };
      tasker.setInterval(ledAction, 2000);  //repren la tasca periòdica
      tasker.setInterval(readTemp, 2000); //repren la tasca periòdica
    }
  }

  else { //sinó (si sí està pres el botó)
    lcd.clear();  //neteja la lcd
    lcd.blink();  //fes que la pantalla sigui intermitent
    lcd.print("ATURADA MANUAL !");  //imprimeix a la pantalla
    lcd.setCursor(1,0); //mou el cursor a la segona línia
    lcd.print("refreda i atura"); //imprimeix a la pantalla

    Serial.println("ATURADA MANUAL !"); //envia per depuració el missatge
    Serial.println("ATURANT TOTS ELS PROCESSOS"); //envia per depuració el missatge
    Serial.println("ESPERA'T A QUE S'HAGUI REFREDAT, DESPRÉS JA POTS REINICIAR L'ARDUINO"); //envia per depuració el missatge

    check_extrudeByTemp = false;  //estableix el boolean false
    check_arduinoFan = false;  //estableix el boolean false
    check_controllersFans = false;  //estableix el boolean false
    check_tubeFan = false;  //estableix el boolean false
    check_filamentFans = false;  //estableix el boolean false
    check_coilFan = false;  //estableix el boolean false
    check_extrudeByRefrigeration = false;  //estableix el boolean false

    digitalWrite(extruderDisable, HIGH);  //deshabilita el motor
    digitalWrite(coilDisable, HIGH);  //deshabilita el motor

    digitalWrite(fanRelayFilament, HIGH); //activa el relé del ventilador
    digitalWrite(fanRelayArduino, HIGH); //activa el relé del ventilador
    digitalWrite(fanRelayControllers, HIGH); //activa el relé del ventilador
    digitalWrite(fanRelayWinder, HIGH); //activa el relé del ventilador
    digitalWrite(fanRelayTube, HIGH); //activa el relé del ventilador
    while(true){//mentre "sempre"
      //bucle infinit
    }
  }
}
/*++++++++++++++++Processos++++++++++++++++*/

/*+++++++++++Definició funicons++++++++++++*/
void lcdProcessess(){ //funció per actualitzar la pantalla amb els processos actuals
  lcd.setCursor(0,0); //mou el cursor de la lcd a l'inici
  lcd.print("Temp:"); //imprimeix a la lcd el missatge
  lcd.print(tempC); //imprimeix a la lcd la variable
  lcd.print("ºC ");  //imprimeix a la lcd el missatge

  lcd.print("Vel:");  //imprimeix a la lcd el missatge
  lcd.print(rpm); //imprimeix a la lcd la variable
  lcd.print(" u ");  //imprimeix a la lcd el missatge

  Serial.print("temp.: ");  //envia per depuració el missatge
  Serial.print(tempC);  //envia per depuració la variable
  Serial.println("ºC"); //envia per depuració el missatge
  
  Serial.print("Vel.:");  //envia per depuració el missatge
  Serial.print(rpm);  //envia per depuració la variable
  Serial.print(" u --> ");  //envia per depuració el missatge
  Serial.print(stepperSpeed); //envia per depuració la variable
  Serial.print(" u=s");  //envia per depuració el missatge
 
}

void tempCheck(){ //defineix la funció per comprovar que la temp. sigui correcta per poder extrudir
  if(tempC < hotTempMax && tempC > hotTempMin) {  //si hotTempMax > tempC > hotTempMin
    check_extrudeByTemp = true; //estableix el boolean true
  }
  else {  //sinó
    check_extrudeByTemp = false;  //estableix el boolean false
  }
}

void fansCheck(){ //defineix la funció per comprovar que els ventiladors estiguin activats per poder extrudir
  if(check_arduinoFan == true && check_controllersFans == true && check_tubeFan == true && check_filamentFans == true && check_coilFan == true){  //si tots els relés dels ventiladors estàn ectivats
    check_extrudeByRefrigeration = true;  //estableix el boolean true
  }
  else{ //sinó
    check_extrudeByRefrigeration = false; //estableix el boolean false
  }
}

void ExtruderSwitches(){  //funció per fer passos a través dels interruptors
  fansCheck();//executa la funció
  tempCheck();  //executa la funció
  stepperSpeed = map(rpm, 0, 1023, 1.5, 200); //valor a canviar, in main, in max, out min, out max
  if(check_extrudeByRefrigeration == true && check_extrudeByTemp == true) { //si els dos booleans són true
    if(switchExtrude == HIGH && switchExtruderInvert == LOW){ //si està el boolean de l'interruptor del motor activat, però no el d'invertir la direcció
      Serial.println("Motor: Extruder: ON");  //envia per depuració el missatge
      digitalWrite(extruderStep, HIGH); //activa per fer un pas
      delay(stepperSpeed);  //espera
      digitalWrite(extruderStep, LOW);  //desactiva per poder torar a fer un altre pas
      delay(timeBetweenSteps);  //espera
      extruding = true;

    }
    else if(switchExtrude == HIGH && switchExtruderInvert == HIGH){ //sinó si el boolean del motor està activat i també el d'invertir
      Serial.println("Motor: Extruder: REVERSE"); //envia per depuració el missatge
      digitalWrite(extruderDir, HIGH);  //activa per canviar la direcció
      digitalWrite(extruderStep, HIGH); //activa per fer un pas
      delay(stepperSpeed);  //espera
      digitalWrite(extruderStep, LOW);  //desactiva per poder fer un altre pas
      digitalWrite(extruderDir, LOW); //desactiva per tornar a la direcció normal
      delay(timeBetweenSteps);  //espera
      extruding = true;

    }
    else {  //sinó
      Serial.println("Motor: Extruder: OFF"); //envia per depuració el missatge
      digitalWrite(extruderDisable, HIGH);  //activa el desactivacior del motor
      extruding = false;
    }
  }
}

void WinderSwitches(){  //funció per fer passos a través dels interruptors
  if(switchWind == HIGH && switchWinderInvert == LOW){  //si està el boolean de l'interruptor del motor activat, però no el d'invertir la direcció
    Serial.println("Motor: Winder: ON");  //envia per depuració el missatge
    digitalWrite(coilStep, HIGH); //activa per fer un pas
    delay(stepperSpeed);  //espera
    digitalWrite(coilStep, LOW);  //desactiva per poder fer un altre pas
    delay(timeBetweenSteps);  //espera
    winding = true;
  }
  else if(switchWind == HIGH && switchWinderInvert == HIGH){  //sinó si el boolean del motor està activat i també el d'invertir
    Serial.println("Motor: Winder: REVERSE"); //envia per depuració el missatge
    digitalWrite(coilDir, HIGH);  //activa per fer canviar la direcció
    digitalWrite(coilStep, HIGH); //activa per fer un pas
    delay(stepperSpeed);  //espera
    digitalWrite(coilStep, LOW);  //desactiva per poder fer un atre pas
    digitalWrite(coilDir, LOW); //desactiva per tornar a la direcció normal
    delay(timeBetweenSteps);  //espera
    winding = true;
  }
  else {  //sinó
    Serial.println("Motor: Winder: OFF"); //envia per depuració el missatge
    digitalWrite(coilDisable, HIGH);  //activa per desactivar el motor
    winding = false;
  }
}

void heaterSwitch(){  //funció per activar la resistència segons PI
  tempCheck();  //executa la funció
  fansCheck();  //executa la funció
  if(check_extrudeByTemp == true && check_extrudeByRefrigeration == true){  //si els dos booleans són true
    if(switchResistor == HIGH){ //si l'interruptor està activat

      tempVariation = hotTemp - tempC;  //resa tempC a hotTemp
      while(tempVariation > 5){ //de 50 a 199 //mentre la resta sigui major a 5
        digitalWrite(resistorSSRelay, HIGH);  //activa el relé de la resistència
        lcd.setCursor(1,0);
        lcd.print("HOTING");
      }

      while(tempVariation <= 5 && tempVariation > 0){ //de 200 a 204  //mentre la resta sigui menor a 5 i major a 0 (positiu)
        currentTimeHeater = millis(); //estableix la variable el temps actual
        do{ //fes
          digitalWrite(resistorSSRelay, HIGH);  //activa el relé de la resistència
          lcd.setCursor(1,0);
          lcd.print("HOTING");
        } while(millis() < currentTimeHeater + tempVariation);  //repeteix mentre no hagin passat el mateixos segons que graus restants (màxim 5)
        digitalWrite(resistorSSRelay, LOW); //desactiva el relé de la resistència
        lcd.setCursor(1,0);
        lcd.print("COLING");
      }

      digitalWrite(resistorSSRelay, LOW); //205 o més //desactiva el relé de la resistència
      lcd.setCursor(1,0);
      lcd.print("COLING");
  }
}

  else{ //sinó
    digitalWrite(resistorSSRelay, LOW); //desactiva la resistència
  }
}

void readTemp(){  //funció per llegir i calcular la temperatura a partir del valor raw (cru) de la NTC
  rntc = 10000.0 / ((5/((5.0 / 1023)*( analogRead(0)))) + ntcCorrection ); // valor raw (cru) de la ntc = resistència extra / ((5V / (( 5V / valor màxim ntc) * valor NTC)) + correcció)
  tempK = 3950.0/(log(rntc/100000.0)+(3950/298.0)); // Kelvins = valor Beta NTC / (log( valor raw NTC / resistència NTC) + (valor Beta NTC / Kelvins estat normal NTC))
  tempC = tempK - 272.15; //Centígrafs = Kelvins - 272.15

}

void ledAction(){ //funció per encendre els LEDs de temperatura
  if(tempC <= coldTempMax && tempC >= coldTempMin) {  //si coldTempMax >= tempC >= coldTempMin
    digitalWrite(coldTempOKLED, HIGH);  //activa el led
    digitalWrite(hotTempOKLED, LOW);  //desactiva el led
  }
  else if(tempC >= hotTempMin && tempC <= hotTempMax){  //sinó si hoTempMin <= tempC <= hotTempMax
    digitalWrite(coldTempOKLED, LOW); //desactiva el led
    digitalWrite(hotTempOKLED, HIGH); //activa el led

  }
  else if(tempC > extremeTemp){ //sinó si la temp. actual és major a la temp. extrema (d'emergència)
    digitalWrite(hotTempOKLED, LOW);  //desactiva el led
    digitalWrite(coldTempOKLED, LOW); //desactiva el led
    errorCode = 1;  //estableix la variable 1
  }
  else if(tempC == 0 || tempC == 1023){  //sinó si la temperatura actual és 0
    digitalWrite(hotTempOKLED, LOW);  //desactiva el led
    digitalWrite(coldTempOKLED, LOW); //desactiva el led
    errorCode = 2;  //estableix la variable a 2
  }
  else{ //sinó
    digitalWrite(hotTempOKLED, LOW);  //desactiva el led
    digitalWrite(coldTempOKLED, LOW); //desactiva el led
  }
}

void fansSwitches(){  //funció habilitar/deshabilitar refrigeració
  if(switchFanArduino == HIGH){ //si l'interruptor està activat
    digitalWrite(fanRelayArduino, HIGH);  //activa el relé
    check_arduinoFan = true;  //estableix el boolean true
    Serial.println("Fan: Arduino: ON"); //envia per depuració el missatge
  }
  else {  //sinó
    digitalWrite(fanRelayArduino, LOW); //desactiva el relé
      check_arduinoFan = false; //estableix el boolean false
      Serial.println("Fan: Arduino: OFF");  //envia per depuració elmissatge
  }

  if(switchFanControllers == HIGH){ //si l'interruptor està activat
    digitalWrite(fanRelayControllers, HIGH);  //activa el relé
    check_controllersFans = true; //estableix el boolean true

      Serial.println("Fan: Controllers: ON"); //envia per depuració el missatge
  }
  else {  //sinó
    digitalWrite(fanRelayControllers,LOW); //desactiva el relé
    check_controllersFans = false; //estableix el boolean false

      Serial.println("Fan: Controllers: OFF"); //envia per depuració el missatge
  }

  if(switchFansFilament == HIGH){ //si l'interruptor està activat
    digitalWrite(fanRelayFilament, HIGH);
    check_filamentFans = true; //estableix el boolean true

      Serial.println("Fan: Filament: ON"); //envia per depuració el missatge
  }
  else {  //sinó
    digitalWrite(fanRelayFilament, LOW); //desactiva el relé
    check_filamentFans = false; //estableix el boolean false

      Serial.println("Fan: Filament: OFF"); //envia per depuració el missatge
  }

  if(switchFanCoil == HIGH){ //si l'interruptor està activat
    digitalWrite(fanRelayWinder, HIGH);
    check_coilFan = true; //estableix el boolean true

    Serial.println("Fan: Coil: ON"); //envia per depuració el missatge
  }
  else{ //sinó
    digitalWrite(fanRelayWinder, LOW); //desactiva el relé
    check_coilFan = false; //estableix el boolean false

    Serial.println("Fan: Coil: OFF"); //envia per depuració el missatge
  }

  if(switchFanTube == HIGH){ //si l'interruptor està activat
    digitalWrite(fanRelayTube, HIGH);
    check_tubeFan = true; //estableix el boolean true

    Serial.println("Fan: Tube: ON"); //envia per depuració el missatge
  }
  else{ //sinó
    digitalWrite(fanRelayTube, LOW); //desactiva el relé
    check_tubeFan = false; //estableix el boolean false

    Serial.println("Fan: Tube: OFF"); //envia per depuració el missatge
  }
}
/*+++++++++++Definició funicons++++++++++++*/
