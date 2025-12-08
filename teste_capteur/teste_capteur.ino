// Capteur de fin de course
#define interruptCap1 2
#define interruptCap2 3

void setup() {
  Serial.begin(9600);
  pinMode(interruptCap2, INPUT_PULLUP);
  pinMode(interruptCap1, INPUT_PULLUP);
}

void loop() {
  Serial.print("CapteurX = ");
  Serial.println(digitalRead(interruptCap2));

  Serial.print("CapteurY = ");
  Serial.println(digitalRead(interruptCap1));
  delay(100);
}
