int const coilStep = 24;
int const coilDir = 27;
int const coilEn = 29;

int const extruderStep = 25;
int const extruderDir = 26;
int const extruderEn = 28;

void setup() {
pinMode(coilEn, OUTPUT);
pinMode(coilDir, OUTPUT);
pinMode(coilStep, OUTPUT);
pinMode(extruderStep, OUTPUT);
pinMode(extruderDir, OUTPUT);
pinMode(extruderEn, OUTPUT);
}

void loop() {
digitalWrite(coilStep, HIGH);
digitalWrite(extruderStep, HIGH);
    delay(20);
      digitalWrite(coilStep, LOW);
      digitalWrite(extruderStep, LOW);
      delay(20);
}
