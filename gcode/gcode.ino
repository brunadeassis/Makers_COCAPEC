#define LOG "14/01/2026"

#include <Servo.h>
#include <gcode.h>

void mouvement(long distance, unsigned int axe); // Mouvement lineaire avec la vitesse F[n] -> G1 X[n] Y[n] Z[n] F[n]
void gotoLocation(); // Mouvement vite -> G0 X[n] Y[n] Z[n] 
void homing(); // Identifier le position pour l'etalonnage
void absolute();
void relative();
void drying();
void help();
void stopMotors();
void etallonageZ();

// Commandes GCODE
#define NUM 9

/*
G28 - HOMING
G00 = G0 - Mouvement linéaire rapide -> G00 X## Y## Z## F####
G01 = G1 - Mouvement linéaire lent -> G00 X## Y## Z## F####
G90 - Mouvement absolut
G91 - Mouvement relative
M18 - disable motors
M100 - help message
*/

commandscallback commands[NUM] = {{"G28", homing}, {"G90", absolute},{"G91", relative},{"G00", gotoLocation},{"G0", gotoLocation},{"G01", drying},{"G1", drying},{"M18", stopMotors},{"M100", help}};
gcode Commands(NUM, commands);
 
// Moteur 3 (stepperX)
#define stepX A0 // Antigo 7
#define dirX A1 // Antigo 8
#define enX A2 // Antigo 9 // Actif à l'état bas

// Moteurs 1&2 (stepperY)
#define stepY 4
#define dirY 5
#define enY 6 // Actif à l'état bas

// Moteur Z : pin 10
#define Z_PLUS 13
#define Z_MOINS 12
Servo moteurZ; // Fils : blanc = signal, noir = gnd, rouge = 5V
int offsetZ = 0;

// Sortie de l'appareil à lumiere
#define lumiere 11

#define AXE_X 1
#define AXE_Y 2
#define AXE_Z 3
#define PAS_PAR_MM_X 100  // Correia GT2 + Polia 20 dentes !!! 
#define PAS_PAR_MM_Y 400 // Fuso T8 (Lead 8mm) !!! TROQUEI DE 200 PARA 400
 
// "Flag" de function
// LOW = Étalonage
// HIGH = Operation
volatile byte state = HIGH;
volatile byte mode = HIGH; // HIGH = Absolute, LOW = Relative

// Capteur de fin de course
#define interruptCapY 3
#define interruptCapX 2

// Structure de communication GCODE
#define BAUD (115200) // How fast is the Arduino talking?
#define MAX_BUF (64) // What is the longest message Arduino can store?

// Anciens positions
int X = 0;
int Y = 0;
int Z = 90;

void setup() {
  // Communication avec la Raspberry
  Serial.begin(BAUD);  // Initialisation de la communication série USB (avec la Raspberry)
  
  pinMode(stepX, OUTPUT);
  pinMode(dirX, OUTPUT);
  pinMode(enX, OUTPUT);

  pinMode(stepY, OUTPUT);
  pinMode(dirY, OUTPUT);
  pinMode(enY, OUTPUT);

  // Moteur 4 (Z)
  moteurZ.attach(10); // signal du servo sur le pin 10
  moteurZ.write(Z);
  pinMode(Z_PLUS, INPUT);
  pinMode(Z_MOINS, INPUT);

  // Appareil à lumiere
  pinMode(lumiere, OUTPUT);

  // Capteurs
  pinMode(interruptCapY, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptCapY), interruptY, FALLING);

  pinMode(interruptCapX, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptCapX), interruptX, FALLING);

  Serial.println("Éloignez-vous du système - Temps d’attente de 15 secondes");
  delay(2000);
  homing();
//  help();
}

void etallonageZ(){
  if(digitalRead(Z_PLUS) == 0){
    if(Z < 180) Z++;
    moteurZ.write(Z);
    delay(30);
  }

  if(digitalRead(Z_MOINS) == 0){
    if(Z > 0) Z--;
    moteurZ.write(Z);
    delay(30);
  }
}

void homing(){
  Serial.println("Homing ON");
  state = HIGH;
  delay(2000);

  digitalWrite(enY, LOW);
  digitalWrite(dirY, LOW);
  while(digitalRead(interruptCapY)){
    digitalWrite(stepY, HIGH);
    delayMicroseconds(500);
    digitalWrite(stepY, LOW);
    delayMicroseconds(500);
  }
  mouvement(2, AXE_Y); // Sortir de le capteur
  digitalWrite(enY, HIGH);
  
  digitalWrite(enX, LOW);
  digitalWrite(dirX, LOW);
  while(digitalRead(interruptCapX)){
    digitalWrite(stepX, HIGH);
    delayMicroseconds(600);
    digitalWrite(stepX, LOW);
    delayMicroseconds(600);
  }
  mouvement(2, AXE_X); // Sortir de le capteur
  digitalWrite(enX, HIGH);
  X = 0; Y = 0;

  Serial.println("Étalonnage de l'axe Z - Quelque commande pour finir");
  delay(100);
  while(Serial.available() > 0){ Serial.read(); delay(5); }

  state = HIGH;
  while(Serial.available() == 0){
    etallonageZ();
    delay(20);
    if (state == LOW) {
       Serial.println("Aviso: Sensor acionado durante espera. Reiniciando state.");
       state = HIGH; // Impede que saia sozinho se esbarrar no sensor
    }
  }

   while(Serial.available() > 0){
     Serial.read(); 
  }

  Serial.println("Homing OFF - Aguardando comandos GCODE");
  state = HIGH;
  X = 0; Y = 0;
}

void gotoLocation(){
    //Serial.println(">> G00 Reçu");
    int newX = X, newY= Y, newZ = Z;
    digitalWrite(lumiere,LOW);

    if(Commands.availableValue('X')){ newX = Commands.GetValue('X'); }
    if(Commands.availableValue('Y')){ newY = Commands.GetValue('Y'); }
    if(Commands.availableValue('Z')){ newZ = Commands.GetValue('Z'); }  
    //if(Commands.availableValue('F')){ vitesse = Commands.GetValue('F')}

    if(mode){
      mouvement(newX - X, AXE_X);
      mouvement(newY - Y, AXE_Y);
      mouvement(newZ, AXE_Z);
      X = newX;
      Y = newY;
      Z = newZ;
    } else {
      mouvement(newX, AXE_X);
      mouvement(newY, AXE_Y);

      int targetZ = Z + newZ;
      if(targetZ > 180) targetZ = 180;
      if(targetZ < 0) targetZ = 0;
      mouvement(targetZ, AXE_Z);
      
      X = X + newX; Y = Y + newY; Z = targetZ;
    }
    
    delay(1000); // Temps pour permettre à l'appareil à lumiere de bien adhérer au composant
  
    Commands.comment("X:" + String(X) + "; Y:" + String(Y) + "; Z:" +String(Z)); // DEBUG SERIAL
    //Commands.comment("X:" + String(X) + "; Y:" + String(Y)); // DEBUG SERIAL
}

void drying(){
    Serial.println(">> G01 Reçu");
    int newX = X, newY= Y, newZ = Z;
    //int newX = X, newY = Y;
    digitalWrite(lumiere,HIGH);

    if(Commands.availableValue('X')){ newX = Commands.GetValue('X'); }
    if(Commands.availableValue('Y')){ newY = Commands.GetValue('Y'); }
    if(Commands.availableValue('Z')){ newZ = Commands.GetValue('Z'); }  
    //if(Commands.availableValue('F')){ vitesse = Commands.GetValue('F')}

    if(mode){
      mouvement(newX - X, AXE_X);
      mouvement(newY - Y, AXE_Y);
      mouvement(newZ, AXE_Z);
      X = newX;
      Y = newY;
      Z = newZ;
    } else {
      mouvement(newX, AXE_X);
      mouvement(newY, AXE_Y);

      int targetZ = Z + newZ;
      if(targetZ > 180) targetZ = 180;
      if(targetZ < 0) targetZ = 0;
      mouvement(targetZ, AXE_Z);
      
      X = X + newX; Y = Y + newY; Z = targetZ;
    }
    
    delay(1000); // Temps pour permettre à l'appareil à lumiere de bien adhérer au composant
  
    Commands.comment("X:" + String(X) + "; Y:" + String(Y) + "; Z:" +String(Z)); // DEBUG SERIAL
    //Commands.comment("X:" + String(X) + "; Y:" + String(Y)); // DEBUG SERIAL
}

void mouvement(long distance, unsigned int axe) { // distance en mm
  if(distance == 0) return;
  
  unsigned long pas = 0;
  if(state == LOW && abs(distance)>10) return;
  
  switch (axe) {
    case AXE_X:
      digitalWrite(enX, LOW);    // LOW embrayé / HIGH débrayé
      if (distance > 0) {
        digitalWrite(dirX, HIGH);  // LOW antihoraire / HIGH horaire
      } else {
        digitalWrite(dirX, LOW);
        distance = -distance;
      }
      while (pas <= (unsigned long) PAS_PAR_MM_X * distance) {
        if (state == LOW && distance > 5) break;
        
        digitalWrite(stepX, HIGH);
        delayMicroseconds(400);
        digitalWrite(stepX, LOW);
        delayMicroseconds(400);
        pas++;
      }
      digitalWrite(enX, HIGH);    // LOW embrayé / HIGH débrayé
      break;

    case AXE_Y:
      digitalWrite(enY, LOW);
      if (distance > 0) {
        digitalWrite(dirY, HIGH);
      } else {
        digitalWrite(dirY, HIGH);
        distance = -distance;
      }
      while (pas <= (unsigned long) PAS_PAR_MM_Y * distance) {
        if (state == LOW && distance > 5) break;
      
        digitalWrite(stepY, HIGH);
        delayMicroseconds(400);
        digitalWrite(stepY, LOW);
        delayMicroseconds(400);
        pas++;
      }
      digitalWrite(enY, HIGH);
      break;

      case AXE_Z:
        if(distance <0) distance = 0;
        if(distance > 180) distance = 180;
        moteurZ.write(distance);
        delay(300);
      break;
  }
}

void absolute(){  mode = HIGH;  Serial.println("Mode: ABS");}

void relative(){  mode = LOW;  Serial.println("Mode: REL");}

void help() {
  Serial.print(F("CNC : Robot pour lumière pulsée - LOG : "));
  Serial.println(LOG);
  Serial.println(F("Commands:"));
  Serial.println(F("G28 - Homing"));
  Serial.println(F("G00 [X(steps)] [Y(steps)] [Z(steps)] [F(feedrate)]; - linear move"));
  Serial.println(F("G01 [X(steps)] [Y(steps)] [Z(steps)] [F(feedrate)]; - linear move"));
  Serial.println(F("G90; - absolute mode"));
  Serial.println(F("G91; - relative mode"));
  Serial.println(F("M18; - disable motors"));
  Serial.println(F("M100; - help message"));
  //Serial.println(F("G04 P[seconds]; - delay"));
  //Serial.println(F("G92 [X(steps)] [Y(steps)]; - change logical position"));
  //Serial.println(F("M114; - report position and feedrate"));
}

void stopMotors(){
  digitalWrite(enX, HIGH);
  digitalWrite(enY, HIGH);
}

void interruptX(){  if(digitalRead(interruptCapX) == LOW) state = LOW; }

void interruptY(){  if(digitalRead(interruptCapY) == LOW) state = LOW; }

void loop() {
  if(state == HIGH){
    // DEBUG SERIAL
    Commands.available();
  }
  else {
    Serial.println("ALERTA : Motor travado");
    if(!digitalRead(interruptCapY)){
      Serial.println("Destravando Y...");
      mouvement(2, AXE_Y);
    }
    
    if(!digitalRead(interruptCapX)){
      Serial.println("Destravando X...");
      mouvement(2, AXE_X);
    }

    if(digitalRead(interruptCapX) && digitalRead(interruptCapY)){
        Serial.println("Sensores liberados. Retomando...");
        delay(2000);
        state = HIGH;
    } else {
        delay(1000);
    }
  }
}
