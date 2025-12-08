#include <Servo.h>

// Définition des broches

// Moteur 3 (stepperX)
#define stepX 7
#define dirX 8
#define enX 9 // Actif à l'état bas

// Moteurs 1&2 (stepperY)
#define stepY 4
#define dirY 5
#define enY 6 // Actif à l'état bas

// Moteur Z : pin 10

// Moteur de l'appareil à lumiere
#define lumiere 11


#define AXE_X 1
#define AXE_Y 2
#define AXE_Z 3
#define PAS_PAR_MM_X 50
#define PAS_PAR_MM_Y 200

// "Flag" de function
// LOW = Étalonage
// HIGH = Operation
volatile byte state;

Servo moteurZ; // Fils : blanc = signal, noir = gnd, rouge = 5V

// Capteur de fin de course
#define interruptCap1 2
#define interruptCap2 3

void setup() {
  
  pinMode(stepX, OUTPUT);
  pinMode(dirX, OUTPUT);
  pinMode(enX, OUTPUT);

  pinMode(stepY, OUTPUT);
  pinMode(dirY, OUTPUT);
  pinMode(enY, OUTPUT);

  // Moteur 4 (Z)
  
  moteurZ.attach(10); // signal du servo sur le pin 10

  // Communication avec la Raspberry

  Serial.begin(9600);  // Initialisation de la communication série USB (avec la Raspberry)

  // Moteur 5 (Appareil à lumiere)

  pinMode(lumiere, OUTPUT);

  // Capteurs
  pinMode(interruptCap1, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptCap1), interruptY, FALLING);

  pinMode(interruptCap2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptCap2), interruptX, FALLING);
  
}

void calibration(){
  state = LOW; // State = LOW (Calibration)

  digitalWrite(enY, LOW);
  digitalWrite(dirY, HIGH);
  while(digitalRead(interruptCap1)){
    digitalWrite(stepY, HIGH);
    delayMicroseconds(400);
    digitalWrite(stepY, LOW);
    delayMicroseconds(400);
  }
  mouvement(1, AXE_Y);
  digitalWrite(enY, HIGH);

  digitalWrite(enX, LOW);
  digitalWrite(dirX, HIGH);
  while(digitalRead(interruptCap2)){
    digitalWrite(stepX, HIGH);
    delayMicroseconds(1000);
    digitalWrite(stepX, LOW);
    delayMicroseconds(1000);
  }
  mouvement(1, AXE_X);
  digitalWrite(enX, HIGH);
  
  delay(1000);
  state = HIGH; // State = HIGH (Mouvement)
}

void mouvement(signed int distance, unsigned int axe) { // distance en mm

  unsigned long pas = 0;
  switch (axe) {
    case AXE_X:
      digitalWrite(enX, LOW);    // LOW embrayé / HIGH débrayé
      if (distance > 0) {
        digitalWrite(dirX, LOW);  // LOW antihoraire / HIGH horaire
      } else {
        digitalWrite(dirX, HIGH);
        distance = -distance;
      }
      while (pas <= PAS_PAR_MM_X * distance) {
        digitalWrite(stepX, HIGH);
        delayMicroseconds(70);
        digitalWrite(stepX, LOW);
        delayMicroseconds(70);
        pas++;
      }
      digitalWrite(enX, HIGH);    // LOW embrayé / HIGH débrayé
      break;

    case AXE_Y:
      digitalWrite(enY, LOW);
      if (distance > 0) {
        digitalWrite(dirY, LOW);
      } else {
        digitalWrite(dirY, HIGH);
        distance = -distance;
      }
      while (pas <= PAS_PAR_MM_Y * distance) {
        digitalWrite(stepY, HIGH);
        delayMicroseconds(70);
        digitalWrite(stepY, LOW);
        delayMicroseconds(70);
        pas++;
      }
      digitalWrite(enY, HIGH);
      break;

      case AXE_Z:
        moteurZ.write(distance);
        delay(15);
      break;
  }
}

void interruptX(){
  if(state == HIGH){
    digitalWrite(stepY, LOW);
    digitalWrite(stepX, LOW);
    delay(1000);
    Serial.println("Motor travado em X");
    mouvement(1, AXE_X);
    delay(1000);
  }
}

void interruptY(){
  if(state == HIGH){
    digitalWrite(stepY, LOW);
    digitalWrite(stepX, LOW);
    delay(1000);
    Serial.println("Motor travado em Y");
    mouvement(1, AXE_Y);
    delay(1000);
  }
}

void loop() {
  state = HIGH;
  
  int d, axe; // (x, y, z) coordonnées du compartiment, act = 1 si l'appareil à lumiere doit être activé
  float t0;
  float t;
  
  Serial.println("Teste de velocidade");
  delay(1000);

  if (Serial.available() > 0) { // informe si quelque chose s'est produit sur la liaison série
      
    String commande = Serial.readStringUntil('\n'); // Lire la commande jusqu'au saut de ligne
 
    if (sscanf(commande.c_str(), "%d, %d", &x, &axe) == 2) { // x, y en mm et z en degrees
      
      // 2. commande des moteurs
      t0 = millis();
      mouvement(d, axe);
      t = millis() - t0;
      delay(1000);
     
      Serial.println("Teste de velocidade finalizado");
      Serial.println("Tempo = %d", t);
      delay(2000);

      }
    }
  
}
