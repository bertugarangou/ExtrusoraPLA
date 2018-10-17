void setup() {
  // put your setup code here, to run once:
pinMode(49, INPUT);
pinMode(32, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
if(digitalRead(49) == LOW){
  digitalWrite(32, HIGH);
}
else{
  digitalWrite(32, LOW);
}
}
