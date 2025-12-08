// Attention à placer correctement les moteurs en position initiale avant démarrage (moteurs X et Y : (0,0), moteur Z : 180)

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

// Moteur de l'appareil à succion
#define succion 11


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

  // Moteur 5 (Appareil à succion)

  pinMode(succion, OUTPUT);

  // Capteurs
  pinMode(interruptCap1, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptCap1), interruptY, FALLING);

  pinMode(interruptCap2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptCap2), interruptX, FALLING);

  Serial.println("Calibration");
  delay(5000);
  Serial.println("Calibration ON");
  calibration();
  
}

void calibration(){
  state = LOW; // State = LOW (Calibration)

  digitalWrite(enY, LOW);
  digitalWrite(dirY, HIGH);
  while(digitalRead(interruptCap1)){
    Serial.println(digitalRead(interruptCap1));
    digitalWrite(stepY, HIGH);
    delayMicroseconds(700);
    digitalWrite(stepY, LOW);
    delayMicroseconds(700);
  }
  digitalWrite(enY, HIGH);

  digitalWrite(enX, LOW);
  digitalWrite(dirX, HIGH);
  while(digitalRead(interruptCap2)){
    digitalWrite(stepX, HIGH);
    delayMicroseconds(1000);
    digitalWrite(stepX, LOW);
    delayMicroseconds(1000);
  }
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
        digitalWrite(dirX, HIGH);  // LOW antihoraire / HIGH horaire
      } else {
        digitalWrite(dirX, LOW);
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
        digitalWrite(dirY, HIGH);
      } else {
        digitalWrite(dirY, LOW);
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
  }
}

void interruptX(){
  if(state == HIGH){
    digitalWrite(stepY, LOW);
    digitalWrite(stepX, LOW);
    delay(1000);
    Serial.println("Motor travado em X");
    delay(1000);
  }
}

void interruptY(){
  if(state == HIGH){
    digitalWrite(stepY, LOW);
    digitalWrite(stepX, LOW);
    delay(1000);
    Serial.println("Motor travado em Y");
    delay(1000);
  }
}

void loop() {

  int x, y, prendre; // (x, y) coordonnées du compartiment, prendre = 1 si l'appareil à succion doit être activé

  if (Serial.available() > 0) { // informe si quelque chose s'est produit sur la liaison série
      
    String commande = Serial.readStringUntil('\n'); // Lire la commande jusqu'au saut de ligne
    // commande de la forme '((x, y), prendre)' avec :
    // - (x, y) : les coordonnées où se rendre (zone photo, zone bac, zone compartiments, zone poubelle)
    // - prendre : un entier (0, 1 ou 2) pour savoir si un composant doit être déposé (0), pris (1) ou si rien ne doit être fait (2)
  
      
    if (sscanf(commande.c_str(), "((%d, %d), %d)", &x, &y, &prendre) == 3) { // x et y en mm
      
      // 2. commande des moteurs
      mouvement(x, AXE_X);
      mouvement(y, AXE_Y);
      delay(1000);

      // Prise/dépose du composant
      moteurZ.write(180); // position basse

      // Activation/désactivation de l'appareil à succion ou attente
      switch(prendre) {
        case 1:
          digitalWrite(succion, HIGH);
          break;
        case 0:
          digitalWrite(succion, LOW);
          break;
        case 2:
          break;
      }
      
      delay(2000); // Temps pour permettre à l'appareil à succion de bien adhérer au composant

      // Replacement en position initiale
      moteurZ.write(100); //position haute


      // 3. fin (message d'ack de l'action)
      Serial.println("ok"); // la raspberry est bloquée en attente de ce message

      }
    }
  
}
