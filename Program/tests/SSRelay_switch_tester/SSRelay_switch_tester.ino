void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
pinMode(49, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
 if(digitalRead(49) == LOW){
  Serial.println("53 LOW");
 }
 else if(digitalRead(49) == HIGH){
  Serial.print("53 HIGH");
 }
}
