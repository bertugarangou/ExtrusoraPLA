/*
  Extrusora de PLA controlada per Arduino Mega 2560.
  Projecte:
  - https://github.com/bertugarangou/ExtrusoraPLA
  - Extrusora feta per Albert Garangou,
    com a Treball de Recerca a 2n de Batxillerat,
    curs 2018/2019, tutor: Jordi Fanals Oriol,
    codi amb Atom i Arduino-upload package.
  - Tots els drets reservats Albert Garangou Culebras (albertgarangou@gmail.com).
    Aquest codi és conegut com a "Proprietary software".
    Consulta el web origen per a més informació.
*/

void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
pinMode(49, INPUT);
pinMode(32, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
 if(digitalRead(49) == LOW){
  Serial.println("49 LOW");
    digitalWrite(32, LOW);
 }
 else if(digitalRead(49) == HIGH){
  Serial.print("49 HIGH");
  digitalWrite(32, HIGH);
 }
}
