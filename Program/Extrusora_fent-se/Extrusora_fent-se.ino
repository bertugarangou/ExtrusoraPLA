/*
  Extrusora de PLA controlada per Arduino Mega 2560.

  Projecte:
  - https://github.com/bertugarangou/ExtrusoraPLA
  - Extrusora feta per Albert Garangou,
    com a Treball de Recerca a 2n de Batxillerat,
    curs 2018/2019, tutor: Jordi Fanals Oriol,
    codi amb Arduino IDE.
  - Tots els drets reservats 2019 Albert Garangou Culebras (albertgarangou@gmail.com).
    Aquest codi està llicenciat amb base a la Creative "Commons Attribution-NonCommercial-NoDerivatives 4.0 International License".
    Altres regles i mesures pròpies sobreescriuen i modifiquen aquesta llicència són presents, consulta el web origen per a més informació.

*/
/*+++++++++++++Llibreries++++++++++++++*/
#include <max6675.h>
MAX6675 tempSensorResistors(11, 12, 13);
MAX6675 tempSensorEnd(8, 9, 10);

#include <Wire.h>

#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2); //PINS SDA i SCL lcd

/*+++++++++++++Llibreries++++++++++++++*/
/*++Declaració variables i constants+++*/
int const lcdUpdateFrequency = 250;  //1000-1500-x
int const tempReaderFrequency = 1000; //1000-1500-x
int const heaterFrequency = 0; //0 (instantari)-500-1000-x

int const INTFanFil = 6;
int const INTFanTube = 7;
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
int const filamentDetector = 36;
int const INTHeat = 53;
int const INTExtruder = 2;
int const INTExtruderRev = 3;
int const INTCoil = 4;
int const INTCoilRev = 5;
int const INTHeater = 49;
int const relayResistors = 32;

bool error = false;
bool canExtrude = false;
bool canCoil = false;

bool extruding = false;
bool coilingFwd = false;
bool coilingRev = false;

bool heatingPause = false;
bool heating = false;

int tempToShow;
float currentTempResistors = 0.0;
float currentTempEnd = 0.0;

float desiredTemp = 200.0;
float desiredTempEnd = 165;
float desiredTempResistors = 200;

float tempResistors1 = 0.0;
float tempResistors2 = 0.0;
float tempResistors3 = 0.0;
float tempEnd1 = 0.0;
float tempEnd2 = 0.0;
float tempEnd3 = 0.0;
float finalTempEnd = 0.0;
float finalTempResistors = 0.0;

int const slowTempRange = 5;  

unsigned long ultimMillis_LCDMain = 0UL;
unsigned long ultimMillis_extruderStart = 0UL;
unsigned long ultimMillis_extruderStop = 0UL;
unsigned long ultimMillis_coilStart = 0UL;
unsigned long ultimMillis_coilStop = 0UL;
unsigned long ultimMillis_tempReader = 0UL;
unsigned long ultimMillis_heaterMain = 0UL;

int const extruderNEINSpeed = 8;
int const coilNEINSpeed = 20;

byte downArrow[8] = {
  B00100,
  B00100,
  B00100,
  B00100,
  B00100,
  B10101,
  B01110,
  B00100
};

byte upArrow[8] = {
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B00100,
  B00100,
  B00100
};

byte cross[8] = {
  B00000,
  B10001,
  B01010,
  B00100,
  B01010,
  B10001,
  B00000,
  B00000
};

byte check[8] = {
  B00000,
  B00000,
  B00001,
  B00010,
  B10100,
  B01000,
  B00000,
  B00000
};

byte rev[8] = {
  B00010,
  B00110,
  B01110,
  B11110,
  B01110,
  B00110,
  B00010,
  B00000
};

byte pause[8] = {
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
void lcdController();
void fansController();
void extruderController();
void coilController();
void filamentDetectorFunction();
void heater();
void tempRead();
void errorProcedure();
void quickTempRead();
/*+++++++++++Declaracio funcions+++++++++++*/

void setup(){
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
  lcd.print("!NO DESCONECTAR!");
  lcd.setCursor(9,1);
  lcd.print("ALERTA!");
  errorProcedure();
  while(true){//bucle infinit
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
  else{ //funcionament estandart del programa (aka no hi ha cap error)
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
void extruderController(){
  if (digitalRead(INTExtruder) == LOW && digitalRead(INTExtruderRev) == HIGH && canExtrude == true){//activat
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
  else if(digitalRead(INTExtruder) == LOW && digitalRead(INTExtruderRev) == LOW && canExtrude == true){
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
      coilingFwd = false;
      coilingRev = true;
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
      coilingFwd = true;
      coilingRev = false;
      digitalWrite(coilDir, HIGH);
      digitalWrite(coilStep, HIGH);
      if(millis() - ultimMillis_coilStop >= coilNEINSpeed){
        digitalWrite(coilStep, LOW);
        digitalWrite(coilDir, LOW);
        ultimMillis_coilStop = millis();
        ultimMillis_coilStart = millis();
      }
    }
  }
  else{
    coilingFwd = false;
    coilingRev = false;
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
    lcd.print((int) currentTempResistors);
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

    if(extruding == true && canCoil == true){ //estat extrusor
      lcd.setCursor(0,1);
      lcd.print("E:");
      lcd.write(4);
    }
    else if(extruding == true && canCoil == false){
      lcd.setCursor(0,1);
      lcd.print("E:");
      lcd.write(4);

    }
    else if(extruding == false){
      lcd.setCursor(0,1);
      lcd.print("E:");
      lcd.write(3);
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

    if(heating == true && heatingPause == false){  //estat resistències
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
  if(filamentDetector == LOW){
    coilingRev = true;
    canCoil = true;
  }
  else {
    coilingFwd = false;
  }

}

void heater(){
  if(digitalRead(INTHeater) == LOW){
    desiredTemp = 190;
    if(millis() - ultimMillis_heaterMain >= heaterFrequency){

      if((desiredTempEnd - currentTempEnd) > 0 && (desiredTempResistors - currentTempResistors) > 0){  //estan els dos per sota
        digitalWrite(relayResistors, HIGH);
        heating = true;
      }
      else if((desiredTempEnd - currentTempEnd) > 15){ //per sota el final amb molta distància
        digitalWrite(relayResistors, HIGH);
        heating = true;
      }
      else{
        digitalWrite(relayResistors, LOW);
        heating = false;
      }
    }
  }
  else{
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
  digitalWrite(extruderStep, LOW);
  digitalWrite(coilStep, LOW);
  digitalWrite(relayResistors, LOW);
  //desiredTempAutoSetterVariableIDon'tKnowTheName = 0;
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
    Serial.println(currentTempEnd);
    Serial.println(currentTempResistors);
    Serial.println("-------------");

    tempToShow = currentTempEnd * 0.75 + currentTempResistors * 0.25;
    
    ultimMillis_tempReader = millis();
  }

}
/*+++++++++++Definició funicons++++++++++++*/
