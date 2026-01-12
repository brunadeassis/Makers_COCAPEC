#define LOG "05/01/2026"

#include <Servo.h>
#include <gcode.h>

void mouvement(signed int distance, unsigned int axe); // Mouvement lineaire avec la vitesse F[n] -> G1 X[n] Y[n] Z[n] F[n]
void gotoLocation(); // Mouvement vite -> G0 X[n] Y[n] Z[n] 
void homing(); // Identifier le position pour l'etalonnage
void absolute();
void relative();
void drying();
void help();
void stopMotors();
void etallonageZ();

// Commandes GCODE
#define NUM 7

/*
G28 - HOMING
G00 - Mouvement linéaire rapide -> G00 X## Y## Z## F####
G01 - Mouvement linéaire lent -> G00 X## Y## Z## F####
G90 - Mouvement absolut
G91 - Mouvement relative
M18 - disable motors
M100 - help message
*/

commandscallback commands[NUM] = {{"G28", homing}, {"G90", absolute},{"G91", relative},{"G00", gotoLocation},{"G01", drying},{"M18", stopMotors},{"M100", help}};
gcode Commands(NUM, commands);
 
// Moteur 3 (stepperX)
#define stepX 7
#define dirX 8
#define enX 9 // Actif à l'état bas

// Moteurs 1&2 (stepperY)
#define stepY 4
#define dirY 5
#define enY 6 // Actif à l'état bas

// Moteur Z : pin 10
#define Z_PLUS 13
#define Z_MOINS 12

// Sortie de l'appareil à lumiere
#define lumiere 11

#define AXE_X 1
#define AXE_Y 2
#define AXE_Z 3
#define PAS_PAR_MM_X 80  // Correia GT2 + Polia 20 dentes !!! TROQUEI DE 50 PARA 80
#define PAS_PAR_MM_Y 400 // Fuso T8 (Lead 8mm) !!! TROQUEI DE 200 PARA 400
 
// "Flag" de function
// LOW = Étalonage
// HIGH = Operation
volatile byte state = HIGH;
volatile byte mode = HIGH; // HIGH = Absolute, LOW = Relative

Servo moteurZ; // Fils : blanc = signal, noir = gnd, rouge = 5V

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
  digitalWrite(dirY, HIGH);
  while(digitalRead(interruptCapY)){
    digitalWrite(stepY, HIGH);
    delayMicroseconds(1000);
    digitalWrite(stepY, LOW);
    delayMicroseconds(1000);
  }
  mouvement(5, AXE_Y); // Sortir de le capteur
  digitalWrite(enY, HIGH);
  
  digitalWrite(enX, LOW);
  digitalWrite(dirX, HIGH);
  while(digitalRead(interruptCapX)){
    digitalWrite(stepX, HIGH);
    delayMicroseconds(1000);
    digitalWrite(stepX, LOW);
    delayMicroseconds(1000);
  }
  mouvement(5, AXE_X); // Sortir de le capteur
  digitalWrite(enX, HIGH);
  X = 0; Y = 0;

  delay(1000); 
  digitalWrite("Étalonnage de l'axe Z - Quelque commande pour finir");
  
  while(Serial.available() == 0){
    etallonageZ();
    delay(20);
    if (state == LOW) break;
  }


  Serial.println("Homing OFF");
  state = HIGH;
  X = 0; Y = 0; Z = 0;
}

void gotoLocation(){
    //int newX = X, newY= Y, newZ = Z, vitesse;
    int newX = X, newY= Y;
    digitalWrite(lumiere,LOW);

    if(Commands.availableValue('X')){ newX = Commands.GetValue('X'); }
    if(Commands.availableValue('Y')){ newY = Commands.GetValue('Y'); }
    //if(Commands.availableValue('Z')){ newZ = Commands.GetValue('Z'); }  
    //if(Commands.availableValue('F')){ vitesse = Commands.GetValue('F')}

    if(mode){
      mouvement(newX - X, AXE_X);
      mouvement(newY - Y, AXE_Y);
      //mouvement(newZ - Z, AXE_Z);

      X = newX;
      Y = newY;
      //Z = newZ;
    } else {
      mouvement(newX, AXE_X);
      mouvement(newY, AXE_Y);
      //mouvement(newZ, AXE_Z);

      X = X + newX;
      Y = Y + newY;
      //Z = Z + newZ;
    }
    
    
    delay(1000); // Temps pour permettre à l'appareil à lumiere de bien adhérer au composant
  
    //Commands.comment("X:" + String(X) + "; Y:" + String(Y) + "; Z:" +String(Z)); // DEBUG SERIAL
    Commands.comment("X:" + String(X) + "; Y:" + String(Y)); // DEBUG SERIAL
}

void drying(){
    //int newX = X, newY= Y, newZ = Z, vitesse;
    int newX = X, newY = Y;
    digitalWrite(lumiere,HIGH);

    if(Commands.availableValue('X')){ newX = Commands.GetValue('X'); }
    if(Commands.availableValue('Y')){ newY = Commands.GetValue('Y'); }
    //if(Commands.availableValue('Z')){ newZ = Commands.GetValue('Z'); }  
    //if(Commands.availableValue('F')){ vitesse = Commands.GetValue('F')}

    if(mode){
      mouvement(newX - X, AXE_X);
      mouvement(newY - Y, AXE_Y);
      //mouvement(newZ - Z, AXE_Z);

      X = newX;
      Y = newY;
      //Z = newZ;
    } else {
      mouvement(newX, AXE_X);
      mouvement(newY, AXE_Y);
      //mouvement(newZ, AXE_Z);

      X = X + newX;
      Y = Y + newY;
      //Z = Z + newZ;
    }
    
    
    delay(1000); // Temps pour permettre à l'appareil à lumiere de bien adhérer au composant
  
    //Commands.comment("X:" + String(X) + "; Y:" + String(Y) + "; Z:" +String(Z)); // DEBUG SERIAL
    Commands.comment("X:" + String(X) + "; Y:" + String(Y)); // DEBUG SERIAL
}

void mouvement(signed int distance, unsigned int axe) { // distance en mm
  if(distance == 0) return;
  
  unsigned long pas = 0;
  if(state == LOW && abs(distance)>10) return;
  
  switch (axe) {
    case AXE_X:
      digitalWrite(enX, LOW);    // LOW embrayé / HIGH débrayé
      if (distance > 0) {
        digitalWrite(dirX, LOW);  // LOW antihoraire / HIGH horaire
      } else {
        digitalWrite(dirX, HIGH);
        distance = -distance;
      }
      while (pas <= (unsigned long) PAS_PAR_MM_X * distance) {
        if (state == LOW && distance > 5) break;
        
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
      while (pas <= (unsigned long) PAS_PAR_MM_Y * distance) {
        if (state == LOW && distance > 5) break;
      
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

void absolute(){  mode = HIGH;  }

void relative(){  mode = LOW;  }

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

void interruptX(){  state = LOW;  }

void interruptY(){  state = LOW;  }

void loop() {
  if(state == HIGH){
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
