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

void setup() {  //inici de la funció d'Arduino que s'executarà un sol cop
  Serial.begin(9600); //inicia la depuració a la frequüència 9600

  lcd.init(); //activa la lcd
  lcd.backlight();  //activa la llum de la pantalla
  lcd.createChar(1, downArrow); //crea els següents caràcters
  lcd.createChar(2, upArrow);
  lcd.createChar(3, cross);
  lcd.createChar(4, check);
  lcd.createChar(5, rev);
  lcd.createChar(6, pause);
  lcd.clear();  //neteja la lcd dels caràcters errònis o antics

  pinMode(INTFanFil, INPUT);  //estableix cada component conectat com a entrada o sortida
  pinMode(INTFanTube, INPUT); //input=entrada i output=sortida
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

void loop() { //inici de la funció de l'Arduino que es repetirà indefinidament
 if(digitalRead(STOPBtn) == 0 || error == true){  //si el el botó no està pres o no hi ha cap error
  digitalWrite(brunzidor, HIGH);  //brunzeix
  Serial.println("*****************************************");  //envia per depuració el conjunt de missatges
  Serial.print("*   STOP");
  Serial.println(", entrant en mode emergència!   *");
  Serial.println("*                                       *");
  Serial.println("* NO DESCONECTAR FINS QUE ESTIGUI FRED! *");
  Serial.println("*****************************************");
  lcd.clear();  //neteja la cld
  lcd.print("!NO DESCONECTAR!");  //escriu a la lcd
  lcd.setCursor(9,1); //canvia el cursor de lloc
  lcd.print("ALERTA!"); //escriu a la lcd
  errorProcedure(); //crida la funció
  while(true) { //bucle infinit
    digitalWrite(brunzidor, LOW); //desactiva el brunzidor
    lcd.noBacklight();  //apaga la llum de la lcd
    quickTempRead();  //crida la funció
    digitalWrite(brunzidor, HIGH);  //brunzeix
    lcd.setCursor(0,1); //canvia el cursor de lloc
    lcd.print((int) finalTempResistors);  //mostra la part entera de la variable
    lcd.print(char(223)); //escriu a la lcd el símbol de graus (º)
    lcd.print("/"); //escriu a la lcd
    lcd.print((int) finalTempEnd);  //mostra la part entera de la variable
    lcd.print(char(223)); //escriu el símbol  de graus (º)
    lcd.backlight();  //activa la llum de la lcd
    delay(2000);  //espera't 2 segons
  }
}
  else { //funcionament estandart del programa (no hi ha cap error)
    lcdController();  //funció per mostrar informació a la pantalla
    filamentDetectorFunction(); //funció per coneixer i enviar l'estat del sensor IR de penajment
    fansController(); //funció per activar i desactivar els ventiladors
    tempRead(); //funció per llegir la temp i calcular la mitjana no proporcional de les dues lectures
    heater(); //funció per escalfar
    extruderController(); //funció per controlar l'extrusor
    coilController(); //funció per controlar la bobina
  }
} //end

/*+++++++++++Definició funicons++++++++++++*/
void extruderController() { //funció per controlar l'extrusor
  if (digitalRead(INTExtruder) == LOW && digitalRead(INTExtruderRev) == HIGH){  //si l'interrupotr està activat
    if(tempToShow > 169 && tempRest > -10){ //si la temperatura està prou calenta
      canCoil = true; //permet bobinar
      if(millis() - ultimMillis_extruderStart >= extruderEmulatedSpeed){  //comprova si ha passat el suficient temps com per executar
        extrudingFwd = true;  //estableix que extrudeix
        extrudingRev = false; //estableix que no extrudeix al revés
        digitalWrite(extruderStep, HIGH); //fes que el motor faci un pas
      if(millis() - ultimMillis_extruderStop >= extruderEmulatedSpeed){ //si ha passar el suficient temps
        digitalWrite(extruderStep, LOW);  //desactiva la sortida per fer el pas quan ja l'hagi fet
        ultimMillis_extruderStop = millis();  //estableix quan ha fet per últim cop la funció
        ultimMillis_extruderStart = millis(); //estableix quan ha fet per últim cop la funció
        }
      }
    }
  }
  else if(digitalRead(INTExtruder) == LOW && digitalRead(INTExtruderRev) == LOW){ //si l'interrupotr està activat i també el d'invertir
    if(tempToShow > 169 && tempRest > -10){ //si la temperatura està prou calenta
      canCoil = false;  //no permetis bobinar
      if(millis() - ultimMillis_extruderStart >= extruderEmulatedSpeed){  //comprova si ha passat el suficient temps com per executar
        extrudingRev = true;  //estableix que extrudeix al revés
        extrudingFwd = false; //estableix que no extrudeix a la direcció normal
        digitalWrite(extruderDir, HIGH);  //activa l'invertidor de direcció
        digitalWrite(extruderStep, HIGH); //fes que el motor faci un pas
      if(millis() - ultimMillis_extruderStop >= extruderEmulatedSpeed){ //si ha passar el suficient temps
        digitalWrite(extruderStep, LOW); //desactiva la sortida per fer el pas quan ja l'hagi fet
        digitalWrite(extruderDir, LOW); //retorna a la direcció normal
        ultimMillis_extruderStop = millis();  //estableix quan ha fet per últim cop la funció
        ultimMillis_extruderStart = millis(); //estableix quan ha fet per últim cop la funció
        }
      }
    }
  }
  else{ //si tots 2 interruptors estan apagats
    extrudingFwd = false; //estableix que no extrudeix
    extrudingRev = false; //estableix que no extrudeix al revés
    canCoil = false;  //no peretis bobinar
  }
}

void coilController(){  //funció per controlar la bobina
  if (digitalRead(INTCoil) == LOW && digitalRead(INTCoilRev) == LOW){ //si l'interrupotr està activat i també el d'invertir
    if(millis() - ultimMillis_coilStart >= coilEmulatedSpeed){  //si ha passar el suficient temps
      coilingFwd = false; //estableix que no bobina en la direcció normal
      coilingRev = true;  //estableix que bobina a l'inversa
      digitalWrite(coilStep, HIGH); //fes que el motor faci un pas
    if(millis() - ultimMillis_coilStop >= coilEmulatedSpeed){ //si ha passar el suficient temps
      digitalWrite(coilStep, LOW);  //desactiva la sortida per fer el pas quan ja l'hagi fet
      ultimMillis_coilStop = millis();  //estableix quan ha fet per últim cop la funció
      ultimMillis_coilStart = millis(); //estableix quan ha fet per últim cop la funció
      }
    }
  }
  else if(digitalRead(INTCoil) == LOW && digitalRead(INTCoilRev) == HIGH && canCoilByFilamentDetector == true){ //si l'interruptor de bobinar està activat i el fil penja
    if(millis() - ultimMillis_coilStart >= coilEmulatedSpeed){
      coilingFwd = true;  //estableix que bobina
      coilingRev = false; //estableix que no bobina al'inversa
      digitalWrite(coilDir, HIGH);  //activa l'invertidor de direcció
      digitalWrite(coilStep, HIGH); //fes que el motor faci un pas
      if(millis() - ultimMillis_coilStop >= coilEmulatedSpeed){
        digitalWrite(coilStep, LOW);  //desactiva la sortida per fer el pas quan ja l'hagi fet
        digitalWrite(coilDir, LOW); //retorna a la direcció normal
        ultimMillis_coilStop = millis();  //estableix quan ha fet per últim cop la funció
        ultimMillis_coilStart = millis(); //estableix quan ha fet per últim cop la funció
      }
    }
  }
  else{ //si els 2 interruptors estan desactivats
    //provar de treure la linia seguent perque no tingui espasmes
    digitalWrite(coilStep, HIGH); //bloqueja el motor (no funcional)
    coilingFwd = false; //estableix que no bobina del revés
    coilingRev = false; //estableix que no bobina en direcció normal
  }
}

void fansController(){  //funció per activar i desactivar els ventiladors
  if(digitalRead(INTFanFil) == LOW){ //quan s'activa l'interruptor adequat
    digitalWrite(relayFanFil, LOW); //activar relé del ventilador
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

void lcdController(){ //funció per activar i desactivar els ventiladors
  if(millis() - ultimMillis_LCDMain >= lcdUpdateFrequency){ //si ha passat prou temps des de la última vegada que s'ha executat
    lcd.setCursor(0,0); //mou el cursor
    lcd.print((int) tempToShow);  //mostra la part entera de la variable
    lcd.print("/"); //escriu a la pantalla
    lcd.print((int) desiredTemp); //mostra la aprt entera de la variable
    lcd.print(char(223)); //escriu a la pantalla el símbol de graus (º)
    lcd.print("  ");  //escriu a la pantalla 2 espais en blanc

    //estat general
    if(canExtrude == true){  //si es permet extrudir
      lcd.setCursor(10,0);  //mou el cursor
      lcd.print("ACTIVAT"); //escriu a la pantalla
    }
    else if(canExtrude == false && heating == true){  //sinó, si es permet extrudir i s'està escalfant
      lcd.setCursor(10,0);  //mou el cursor
      lcd.print("ESPERA");  //escriu a la pantalla
    }
    else{ //sinó
      lcd.setCursor(10, 0); //mou el cursor
      lcd.print(" PAUSA");  //escriu a la pantalla
    }

    //estat extrusor
    if(extrudingFwd == true){ //si s'està extrudint en direcció normal
      lcd.setCursor(0,1); //mou el cursor
      lcd.print("E:");  //escriu a la pantalla
      lcd.write(4); //mostra el caràcter creat pròpi
      lcd.print(" "); //borra l'espai següent
    }
    else if(extrudingRev == true){  //sinó si s'està extrudint al revés
      lcd.setCursor(0,1); //mou el cursor
      lcd.print("E:");  //escriu a la pantalla
      lcd.write(5); //mostra el caràcter creat pròpi 2 vegades per fer el símbol "<<"
      lcd.write(5);
    }
    else{ //sinó
      lcd.setCursor(0,1); //mou el cursor
      lcd.print("E:");  //escriu a la pantalla
      lcd.write(3); //escriu el caràcter creat pròpi
      lcd.print(" "); //borra el següent espai
    }

    //estat bobina
    if(coilingFwd == true){   //si s'està bobinant al revés
      lcd.setCursor(11,1);  //mou el cursor
      lcd.print("B:");  //escriu a la pantalla
      lcd.write(4); //mostra el caràcter pròpi
      lcd.print(" "); //borra el següent espai
    }
    else if(coilingRev == true){  //sinó si es bobina del revés
      lcd.setCursor(11,1);  //mou el cursor
      lcd.print("B:");  //escriu a la pantalla
      lcd.write(5); //mostra el caràcter creat pròpi 2 vegades per fer el símbol "<<"
      lcd.write(5);
    }
     else{  //sinó
      lcd.setCursor(11,1);  //mou el cursor
      lcd.print("B:");  //escriu a la pantalla
      lcd.write(3); //mostra el caràcter creat pròpi
      lcd.print(" "); //borra el següent espai
    }

    //estat heating
    if(heating == true && heatingPause == false){  // si s'està escalfant i no està en pausa
      lcd.setCursor(6,1); //mou el cursor
      lcd.print("H:");  //escriu a la pantalla
      lcd.write(4); //mostra el caràcter creat pròpi
    }
    else if(heating == true && heatingPause == true){ //sinó si s'està escalfant i està en pausa
      lcd.setCursor(6,1); //mou el cursor
      lcd.print("H:");  //escriu a la pantalla
      lcd.write(6); //mostra el caràcter creat pròpi
    }
    else{ //sinó
      lcd.setCursor(6,1); //mou el cursor
      lcd.print("H:");  //escriu a la pantalla
      lcd.write(3); //mostra el caràcter creat pròpi
    }
      //direcció fil  (fletxes)
    if(coilingFwd == true){ //si s'extrudeix en direcció normal
      lcd.setCursor(15,1);  //mou el cursor
      lcd.write(2); //mostra el caràcter creat pròpi "fletxa amunt"
    }
    else if (coilingRev == true){
      lcd.setCursor(15,1);  //mou el cursor
      lcd.write(1); //mostra el caràcter creat pròpi "fletxa avall"
    }
    else{ //sinó
      lcd.setCursor(15,1);  //mou el cursor
      lcd.print(" "); //borra el següent espai (cap fletxa)
    }

    ultimMillis_LCDMain = millis(); //estableix quan ha fet per últim cop la funció
  }
}

void filamentDetectorFunction(){  //funció per coneixer i enviar l'estat del sensor IR de penajment
  if(extrudingFwd == true){ //si s'està extrudint en direcció normal
    if(digitalRead(filamentDetector) == LOW){ //si es detecta el fil  (està penjant)
      canCoilByFilamentDetector = true; //estableix que s'ha de bobinar
    }
    else{ //sinó
      canCoilByFilamentDetector = false;  //estableix que no s'ha de bobinar
    }
  }
}

void heater(){  //funció per escalfar
  if(digitalRead(INTHeater) == LOW){  //si l'interruptor d'escalfament està activat
    desiredTemp = 175;  //estableix la temp total desitjada a 175ºC
    desiredTempResistors = 190; //estableix la temp de les resistències desitjada a 190ºC
    desiredTempEnd = 165; //estableix la temp del final del tub desitjada a 190ºC

    //if(millis() - ultimMillis_heaterMain >= heaterFrequency){ //si ha passat el suficient temps des de l'últim cop que s'ha executat
    tempRest = desiredTemp - tempToShow;  //fes la diferència de la temp desitjada total i la temp per mostrar (variació de temperatures actual i desitjada)
      if(tempToShow > 0 && tempRest > 1){  //si la diferència anterior és major a 1 i la temp per motrar és major a 0
        digitalWrite(relayResistors, HIGH); //activa el relé de les resistències
        heating = true; //estableix que està escalfant
        heatingPause = false; //estableix que en aquest precís moment està escalfant
      }
      else if(tempRest <= -2){  //si la diferènica anterior és igual o menor a -2 (la temp és massa elevada)
        digitalWrite(relayResistors, LOW);  //desactiva les resistències
        heating = true; //estableix que estàs escalfant
        heatingPause = true;  //estableix que en aquest instant no escalfa
        digitalWrite(relayFanTube, LOW);  //activa el ventilador del tub per accelerar la pèrdua de calor
      }
      else if(tempToShow >0 && tempRest <= 1 && tempRest > -3){ //si la temperatura és adequada, dins l'interval "-3<temp<=1"
        digitalWrite(relayResistors, LOW);  //desactiva les resistències
        heatingPause = true;  //estableix que en aquest instant no escalfa
        heating = true; //estableix que estàs escalfant
      }
      else{ //sinó
        digitalWrite(relayResistors, LOW);  //desactiva les resistènices
        heating = false;  //estableix que no escalfes
        heatingPause = false; //estableix que no està en pausa l'escalfament, sinó desactivat
      }
    //}
  }
  else if (digitalRead(INTHeater) == HIGH){ //sinó si l'interruptor d'escalfar està desactivat
    desiredTempEnd = 0; //estableix la temp del final del tub desitjada 0
    desiredTempResistors = 0; //estableix la temp de les resistències desitjada 0
    desiredTemp = 0;  //estableix la temperatura total desitjada 0
    digitalWrite(relayResistors, LOW);  //desactiva les resistènices
    heating = false;  //estableix que no està escalfant
  }
}

void errorProcedure(){  //funció amb els procediemnts per quan hi ha una emergència
  digitalWrite(relayFanFil, LOW); //activa el ventilador
  digitalWrite(relayFanTube, LOW);  //activa el ventilador
  //la següent línia pot prevenir la corrent residual al motor
  digitalWrite(extruderStep, HIGH); //desbloqueja/bloqueja el motor
  //la següent línia pot prevenir la corrent residual al motor
  digitalWrite(coilStep, LOW);  // desbloqueja/bloqueja el motor
  digitalWrite(relayResistors, LOW);  //desactiva les resistències
}

void quickTempRead(){ //funció per llegir 3 vegades la temp i fer-ne la mitjana (només utilitzada quan hi ha un error)
  tempEnd1 = tempSensorEnd.readCelsius(); //llegeix amb centígrafs la temp del final del tub i guarda-la
  tempResistors1 = tempSensorResistors.readCelsius(); //llegeix amb centígrafs la temp de les resistènices i guarda-la
  delay(500);
  tempEnd2 = tempSensorEnd.readCelsius(); //llegeix amb centígrafs la temp del final del tub i guarda-la
  tempResistors2 = tempSensorResistors.readCelsius(); //llegeix amb centígrafs la temp de les resistènices i guarda-la
  delay(500);
  tempEnd3 = tempSensorEnd.readCelsius(); //llegeix amb centígrafs la temp del final del tub i guarda-la
  tempResistors3 = tempSensorResistors.readCelsius(); //llegeix amb centígrafs la temp de les resistènices i guarda-la
  finalTempEnd = (tempEnd1 + tempEnd2 + tempEnd3) / 3; //fes la mitjana dels tres valors i guarda-la
  finalTempResistors = (tempResistors1 + tempResistors2 + tempResistors3) / 3;  //fes la mitjana dels tres valors i guarda-la
}

void tempRead(){  //funció per llegir la temp i calcular la mitjana no proporcional de les dues lectures
  if(millis() - ultimMillis_tempReader >= tempReaderFrequency){ //si ha passat el temps necessàri des d ela última execució
    currentTempEnd = tempSensorEnd.readCelsius(); //llegeix la temp en centígrafs del final del tub i guarda-la
    currentTempResistors = tempSensorResistors.readCelsius(); //llegiex la temp en centígrafs de les resistències i gruarda-la

    tempToShow = (currentTempEnd * 70 + currentTempResistors * 30) / 100; //calcula, segons la proporció trobada de la importància de les temperatures, la temperatura total de tot l'extrusor
    //un 70% de la temp total és del final de l'extrusor i un 30% de les resistències
    ultimMillis_tempReader = millis();  //estableix quan ha fet per últim cop la funció
  }
}
/*+++++++++++Definició funicons++++++++++++*/
//end
