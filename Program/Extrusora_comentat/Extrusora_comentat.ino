/*
  Extrusora de PLA controlada per Arduino Mega 2560.

  Projecte:
  - https://github.com/bertugarangou/ExtrusoraPLA
  - Extrusora feta per Albert Garangou,
    com a Treball de Recerca a 2n de Batxillerat,
    curs 2018/2019, tutor: Jordi Fanals Oriol,
    codi amb Atom i Arduino-upload package.
  - Tots els drets reservats 2018 Albert Garangou Culebras (albertgarangou@gmail.com).
    Aquest codi és conegut com a "Proprietary software".
    Consulta el web origen per a més informació.

*/
/*

  -Aquesta versió del programa, conté totes les línies necessàries comentades.
  -Funciona idènticament al original, però serveix per entendre més a fons el codi.
  -S'actualitza al mateix moment que l'original, també està sota la mateixa llicència i normes.

*/
/*+++++++++++++Llibreries++++++++++++++*/
#include <max6675.h>  //importa llibreria pel modul de temperatura
MAX6675 tempSensorResistors(11, 12, 13);  //declara sensor temp1
MAX6675 tempSensorEnd(8, 9, 10);  //declara sensor temp2

#include <Wire.h> //importa la llibreria per sensors I2C
#include <LiquidCrystal_I2C.h>  //importa llibreria per la pantalla I2C
LiquidCrystal_I2C lcd(0x27, 16, 2); //PINS SDA i SCL lcd

/*+++++++++++++Llibreries++++++++++++++*/
/*++Declaració variables i constants+++*/
//Frequüències d'actualització de funcions
int const lcdUpdateFrequency = 250;  //250  //frequüència pantalla lcd
int const tempReaderFrequency = 1000; //1000  //frequüència lectura temperatura
int const heaterFrequency = 0; //0  //frequüència accions escalfador
//entrades i sortides
int const INTExtruder = 2;
int const INTExtruderRev = 3;
int const INTCoil = 4;
int const INTCoilRev = 5;
int const INTFanFil = 6;
int const INTFanTube = 7;
int const coilStep = 24;
int const extruderStep = 25;
int const extruderDir = 26;
int const coilDir = 27;
int const extruderEn = 28;
int const coilEn = 29;
int const STOPBtn = 30;
int const relayResistors = 32;
int const filamentDetector = 36;
int const INTHeater = 49;
int const relayFanTube = 50;
int const brunzidor = 51;
int const relayFanFil = 52;
int const INTHeat = 53;

//booleans d'estat
bool error = false; //boolean per avisar quan tingui lloc un error

bool extrudingFwd = false;  //extrudint
bool extrudingRev = false;  //extrudint del revés
bool coilingFwd = false;  //bobinant
bool coilingRev = false;  //bobinant del revés

bool heating = false; //estableix que s'està extrudint
bool heatingPause = false;  //estableix que l'extrussió està en pausa

//booleans de comprovació
bool canExtrude = false;  //comprova si es pot extrudir
bool canCoil = false; //comprova si es pot bobinar

bool canCoilByFilamentDetector = false; //comprova si el detector IR permet bobinar

//valors de temperatures
int tempToShow; //temp per mostrar a la lcd
float currentTempResistors = 0.0; //temp actual de les resistències
float currentTempEnd = 0.0; //temp actual final tub

float desiredTemp;  //temp desitjat total
float desiredTempEnd; //temp desithat final tub
float desiredTempResistors; //temp desitjat resistències

int const slowTempRange = 5;  //marge de frenada (coat) quan s'aproxima a la temp desitjada

int tempResistorsRest;  //calcul |temp actual resistències - temp volguda resistències|
int tempEndRest;  //calcul | temp acual tub - temp volguda final tub
float tempRest = 0.0; //calcul |temp acual - temp volguda|
//valors de temperatures per càlculs ràpids
float tempResistors1 = 0.0; //lectures temp 1
float tempResistors2 = 0.0; //2
float tempResistors3 = 0.0; //3
float tempEnd1 = 0.0; //1
float tempEnd2 = 0.0; //2
float tempEnd3 = 0.0; //3

float finalTempEnd = 0.0; //mitjana de les 3 lectures de temp del final del tub
float finalTempResistors = 0.0; //mitjana de les 3 lectures de temp de les resistències

//ultim timestamp pròpi d'execució de funcions
unsigned long ultimMillis_LCDMain = 0UL;  //últim cop (temps en milisegons) que s'ha executat la funció
unsigned long ultimMillis_extruderStart = 0UL;
unsigned long ultimMillis_extruderStop = 0UL;
unsigned long ultimMillis_coilStart = 0UL;
unsigned long ultimMillis_coilStop = 0UL;
unsigned long ultimMillis_tempReader = 0UL;
unsigned long ultimMillis_heaterMain = 0UL;

//velocitats dels motors
int const extruderEmulatedSpeed = 6;  //6 // temps entre l'activació de la bobina del motor i la desactivació
int const coilEmulatedSpeed = 20; //20

//matrius de caràcters personalitzats lcd
byte downArrow[8] = { //caràcter lcd fletxa senyalant cap a baix
  B00100,
  B00100,
  B00100,
  B00100,
  B00100,
  B10101,
  B01110,
  B00100
};

byte upArrow[8] = { //caràcter lcd fletxa senyalant cap a dalt
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B00100,
  B00100,
  B00100
};

byte cross[8] = { //caràcter lcd creu "X"
  B00000,
  B10001,
  B01010,
  B00100,
  B01010,
  B10001,
  B00000,
  B00000
};

byte check[8] = { //caràcter lcd "tick"
  B00000,
  B00000,
  B00001,
  B00010,
  B10100,
  B01000,
  B00000,
  B00000
};

byte rev[8] = { //caràcter lcd signe revés "<<"
  B00010,
  B00110,
  B01110,
  B11110,
  B01110,
  B00110,
  B00010,
  B00000
};

byte pause[8] = { //caràcter lcd pausa "||"
  B00000,
  B01010,
  B01010,
  B01010,
  B01010,
  B01010,
  B00000,
  B00000
};
/*++Declaració variables i constants+++*/
/*+++++++++++Declaracio funcions+++++++++++*/
void lcdController();  //funció per mostrar informació a la pantalla
void fansController();  //funció per activar i desactivar els ventiladors
void extruderController();  //funció per controlar l'extrusor
void coilController();  //funció per controlar la bobina
void filamentDetectorFunction();  //funció per coneixer i enviar l'estat del sensor IR de penajment
void heater();  //funció per escalfar
void tempRead();  //funció per llegir la temp i calcular la mitjana no proporcional de les dues lectures
void errorProcedure();  //funció amb els procediemnts per quan hi ha una emergència
void quickTempRead(); //funció per llegir 3 vegades la temp i fer-ne la mitjana (només utilitzada quan hi ha un error)
/*+++++++++++Declaracio funcions+++++++++++*/

void setup() {
  Serial.begin(9600); //inicia la depuració

  lcd.init();
  lcd.backlight();
  lcd.createChar(1, downArrow);
  lcd.createChar(2, upArrow);
  lcd.createChar(3, cross);
  lcd.createChar(4, check);
  lcd.createChar(5, rev);
  lcd.createChar(6, pause);
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
  pinMode(filamentDetector, INPUT);
  pinMode(INTExtruder, INPUT);
  pinMode(INTExtruderRev, INPUT);
  pinMode(INTCoil, INPUT);
  pinMode(INTCoilRev, INPUT);
  pinMode(INTHeater, INPUT);
  pinMode(relayResistors, OUTPUT);
  digitalWrite(relayResistors, LOW);
} //end

void loop() {
 if(digitalRead(STOPBtn) == 0 || error == true){
  digitalWrite(brunzidor, HIGH);
  Serial.println("*****************************************");
  Serial.print("*   STOP");
  Serial.println(", entrant en mode emergència!   *");
  Serial.println("*                                       *");
  Serial.println("* NO DESCONECTAR FINS QUE ESTIGUI FRED! *");
  Serial.println("*****************************************");
  lcd.clear();
  lcd.print("!NO DESCONECTAR!");
  lcd.setCursor(9,1);
  lcd.print("ALERTA!");
  errorProcedure();
  while(true) {//bucle infinit
    digitalWrite(brunzidor, LOW);
    lcd.noBacklight();
    quickTempRead();
    digitalWrite(brunzidor, HIGH);
    lcd.setCursor(0,1);
    lcd.print((int) finalTempResistors);
    lcd.print(char(223));
    lcd.print("/");
    lcd.print((int) finalTempEnd);
    lcd.print(char(223));
    lcd.backlight();
    delay(2000);
  }
}
  else { //funcionament estandart del programa (no hi ha cap error)
    lcdController();
    filamentDetectorFunction();
    fansController();
    tempRead();
    heater();
    extruderController();
    coilController();
  }
} //end

/*+++++++++++Definició funicons++++++++++++*/
void extruderController() {
  if (digitalRead(INTExtruder) == LOW && digitalRead(INTExtruderRev) == HIGH){//activat
    if(tempToShow > 169 && tempRest > -10){
      canCoil = true;
      if(millis() - ultimMillis_extruderStart >= extruderEmulatedSpeed){
        extrudingFwd = true;
        extrudingRev = false;
        digitalWrite(extruderStep, HIGH);
      if(millis() - ultimMillis_extruderStop >= extruderEmulatedSpeed){
        digitalWrite(extruderStep, LOW);
        ultimMillis_extruderStop = millis();
        ultimMillis_extruderStart = millis();
        }
      }
    }
  }
  else if(digitalRead(INTExtruder) == LOW && digitalRead(INTExtruderRev) == LOW){//invertit
    if(tempToShow > 169 && tempRest > -10){
      canCoil = false;
      if(millis() - ultimMillis_extruderStart >= extruderEmulatedSpeed){
        extrudingRev = true;
        extrudingFwd = false;
        digitalWrite(extruderDir, HIGH);
        digitalWrite(extruderStep, HIGH);
      if(millis() - ultimMillis_extruderStop >= extruderEmulatedSpeed){
        digitalWrite(extruderStep, LOW);
        digitalWrite(extruderDir, LOW);
        ultimMillis_extruderStop = millis();
        ultimMillis_extruderStart = millis();
        }
      }
    }
  }
  else{ //desactivat
    extrudingFwd = false;
    extrudingRev = false;
    canCoil = false;
  }
}

void coilController(){
  if (digitalRead(INTCoil) == LOW && digitalRead(INTCoilRev) == LOW){ // tots dos activats
    if(millis() - ultimMillis_coilStart >= coilEmulatedSpeed){
      coilingFwd = false;
      coilingRev = true;
      digitalWrite(coilStep, HIGH);
    if(millis() - ultimMillis_coilStop >= coilEmulatedSpeed){
      digitalWrite(coilStep, LOW);
      ultimMillis_coilStop = millis();
      ultimMillis_coilStart = millis();
      }
    }
  }
  else if(digitalRead(INTCoil) == LOW && digitalRead(INTCoilRev) == HIGH && canCoilByFilamentDetector == true){ // sense invertir direcció
    if(millis() - ultimMillis_coilStart >= coilEmulatedSpeed){
      coilingFwd = true;
      coilingRev = false;
      digitalWrite(coilDir, HIGH);
      digitalWrite(coilStep, HIGH);
      if(millis() - ultimMillis_coilStop >= coilEmulatedSpeed){
        digitalWrite(coilStep, LOW);
        digitalWrite(coilDir, LOW);
        ultimMillis_coilStop = millis();
        ultimMillis_coilStart = millis();
      }
    }
  }
  else{
    digitalWrite(coilStep, HIGH);
    coilingFwd = false;
    coilingRev = false;
  }
}

void fansController(){
  if(digitalRead(INTFanFil) == LOW){ //quan s'activa l'interruptor adequat
    digitalWrite(relayFanFil, LOW); //activar relé ventilador
  }
  else{ //sinó
    digitalWrite(relayFanFil, HIGH); //desactiva'l
  }
  if(digitalRead(INTFanTube) == LOW){  //si s'activa l'interruptor adequat
    digitalWrite(relayFanTube, LOW);  //activa el relé del ventilador
  }
  else{ //sinó
    digitalWrite(relayFanTube, HIGH);  //desactiva'l
  }
}

void lcdController(){
  if(millis() - ultimMillis_LCDMain >= lcdUpdateFrequency){
    lcd.setCursor(0,0);
    lcd.print((int) tempToShow);
    lcd.print("/");
    lcd.print((int) desiredTemp);
    lcd.print(char(223));
    lcd.print("  ");

    if(canExtrude == true){ //estat general
      lcd.setCursor(10,0);
      lcd.print("ACTIVAT");
    }
    else if(canExtrude == false && heating == true){
      lcd.setCursor(10,0);
      lcd.print("ESPERA");
    }
    else{
      lcd.setCursor(10, 0);
      lcd.print(" PAUSA");
    }

    if(extrudingFwd == true){ //estat extrusor
      lcd.setCursor(0,1);
      lcd.print("E:");
      lcd.write(4);
      lcd.print(" ");
    }
    else if(extrudingRev == true){
      lcd.setCursor(0,1);
      lcd.print("E:");
      lcd.write(5);
      lcd.write(5);
    }
    else{
      lcd.setCursor(0,1);
      lcd.print("E:");
      lcd.write(3);
      lcd.print(" ");
    }

    if(coilingFwd == true){  //estat bobina
      lcd.setCursor(11,1);
      lcd.print("B:");
      lcd.write(4);
      lcd.print(" ");
    }
    else if(coilingRev == true){
      lcd.setCursor(11,1);
      lcd.print("B:");
      lcd.write(5);
      lcd.write(5);
    }
     else{
      lcd.setCursor(11,1);
      lcd.print("B:");
      lcd.write(3);
      lcd.print(" ");
    }

    if(heating == true && heatingPause == false){  //estat heating
      lcd.setCursor(6,1);
      lcd.print("H:");
      lcd.write(4);
    }
    else if(heating == true && heatingPause == true){
      lcd.setCursor(6,1);
      lcd.print("H:");
      lcd.write(6);
    }
    else{
      lcd.setCursor(6,1);
      lcd.print("H:");
      lcd.write(3);
    }


      if(coilingFwd == true){
      lcd.setCursor(15,1);
      lcd.write(2);
      }
      else if(coilingRev == true){
      lcd.setCursor(15,1);
      lcd.write(1);
      }
    else{
      lcd.setCursor(15,1);
      lcd.print(" ");
      }

    ultimMillis_LCDMain = millis();
  }
}

void filamentDetectorFunction(){
  if(extrudingFwd == true){
    if(digitalRead(filamentDetector) == LOW){
      canCoilByFilamentDetector = true;
    }
    else{
      canCoilByFilamentDetector = false;
    }
  }
}

void heater(){
  if(digitalRead(INTHeater) == LOW){
    desiredTemp = 175;
    desiredTempResistors = 190;
    desiredTempEnd = 165;

    //if(millis() - ultimMillis_heaterMain >= heaterFrequency){
    tempRest = desiredTemp - tempToShow;
      if(tempToShow > 0 && tempRest > 1){  //escalfant de lluny
        digitalWrite(relayResistors, HIGH);
        heating = true;
        heatingPause = false;
      }
      else if(tempRest <= -2){  //massa alta
        digitalWrite(relayResistors, LOW);
        heating = true;
        heatingPause = true;
        digitalWrite(relayFanTube, LOW);
      }
      else if(tempToShow >0 && tempRest <= 1 && tempRest > -3){ // pausant temperatura està ok
        digitalWrite(relayResistors, LOW);
        heatingPause = true;
        heating = true;
      }
      else{ //no escalfa
        digitalWrite(relayResistors, LOW);
        heating = false;
        heatingPause = false;
      }
    //}
  }
  else if (digitalRead(INTHeater) == HIGH){
    desiredTempEnd = 0;
    desiredTempResistors = 0;
    desiredTemp = 0;
    digitalWrite(relayResistors, LOW);
    heating = false;
  }
}

void errorProcedure(){
  digitalWrite(relayFanFil, LOW);
  digitalWrite(relayFanTube, LOW);
  digitalWrite(extruderStep, HIGH);
  digitalWrite(coilStep, LOW);
  digitalWrite(relayResistors, LOW);
}

void quickTempRead(){
  tempEnd1 = tempSensorEnd.readCelsius();
  tempResistors1 = tempSensorResistors.readCelsius();
  delay(500);
  tempEnd2 = tempSensorEnd.readCelsius();
  tempResistors2 = tempSensorResistors.readCelsius();
  delay(500);
  tempEnd3 = tempSensorEnd.readCelsius();
  tempResistors3 = tempSensorResistors.readCelsius();
  finalTempEnd = (tempEnd1 + tempEnd2 + tempEnd3) / 3;
  finalTempResistors = (tempResistors1 + tempResistors2 + tempResistors3) / 3;
}

void tempRead(){
  if(millis() - ultimMillis_tempReader >= tempReaderFrequency){
    currentTempEnd = tempSensorEnd.readCelsius();
    currentTempResistors = tempSensorResistors.readCelsius();

    tempToShow = (currentTempEnd * 70 + currentTempResistors * 30) / 100;

    ultimMillis_tempReader = millis();
  }
}
/*+++++++++++Definició funicons++++++++++++*/
//end
