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

  -Aquesta versió del programa, conté totes les línies necessàries comentades i amb depuració a velocitat 9600.
  -Funciona idènticament al original, però serveix per entendre més a fons el codi i coneixer errors i els diferents càlculs del programa.
  -S'actualitza al mateix moment que l'original, també està sota la mateixa llicència i normes.
  -Permet veure com "pensa el programa" i què fa en cada moment.
  -Es recomana que només s'utilitzi en cas de necessitat, ja que consumeix més recursos de la placa.

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
//Frequüències d'actualització de funcions
int const lcdUpdateFrequency = 250;  //250
int const tempReaderFrequency = 1000; //1000
int const heaterFrequency = 0; //0
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
bool error = false;

bool extrudingFwd = false;
bool extrudingRev = false;
bool coilingFwd = false;
bool coilingRev = false;

bool heating = false;
bool heatingPause = false;

//booleans de comprovació
bool canExtrude = false;
bool canCoil = false;

bool canCoilByFilamentDetector = false;

//valors de temperatures
int tempToShow;
float currentTempResistors = 0.0;
float currentTempEnd = 0.0;

float desiredTemp;
float desiredTempEnd;
float desiredTempResistors;

int const slowTempRange = 5;

int tempResistorsRest;
int tempEndRest;
float tempRest = 0.0;
//valors de temperatures per càlculs ràpids
float tempResistors1 = 0.0;
float tempResistors2 = 0.0;
float tempResistors3 = 0.0;
float tempEnd1 = 0.0;
float tempEnd2 = 0.0;
float tempEnd3 = 0.0;

float finalTempEnd = 0.0;
float finalTempResistors = 0.0;

//ultim timestamp pròpi d'execució de funcions
unsigned long ultimMillis_LCDMain = 0UL;
unsigned long ultimMillis_extruderStart = 0UL;
unsigned long ultimMillis_extruderStop = 0UL;
unsigned long ultimMillis_coilStart = 0UL;
unsigned long ultimMillis_coilStop = 0UL;
unsigned long ultimMillis_tempReader = 0UL;
unsigned long ultimMillis_heaterMain = 0UL;

//velocitats dels motors
int const extruderNEINSpeed = 6;  //6
int const coilNEINSpeed = 20; //20

//matrius de caràcters personalitzats lcd
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
      if(millis() - ultimMillis_extruderStart >= extruderNEINSpeed){
        extrudingFwd = true;
        extrudingRev = false;
        digitalWrite(extruderStep, HIGH);
      if(millis() - ultimMillis_extruderStop >= extruderNEINSpeed){
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
      if(millis() - ultimMillis_extruderStart >= extruderNEINSpeed){
        extrudingRev = true;
        extrudingFwd = false;
        digitalWrite(extruderDir, HIGH);
        digitalWrite(extruderStep, HIGH);
      if(millis() - ultimMillis_extruderStop >= extruderNEINSpeed){
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
  else if(digitalRead(INTCoil) == LOW && digitalRead(INTCoilRev) == HIGH && canCoilByFilamentDetector == true){ // sense invertir direcció
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
