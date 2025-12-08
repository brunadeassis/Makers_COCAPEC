// --- CÓDIGO DE CALIBRAÇÃO DE PASSOS/MM ---
// Autor: Assistente IA
// Hardware: Arduino Uno + CNC Shield (Pinos baseados no seu código anterior)

#define BAUD 115200

// Configuração dos Pinos (Baseado no seu código)
#define X_STEP 7
#define X_DIR  8
#define X_EN   9

#define Y_STEP 4
#define Y_DIR  5
#define Y_EN   6

// --- VALORES ATUAIS PARA TESTE (Seus valores originais) ---
float stepsPerMM_X = 50.0*100/99.23;  
float stepsPerMM_Y = 200.0*100/100.33;

void setup() {
  Serial.begin(BAUD);
  
  pinMode(X_STEP, OUTPUT); pinMode(X_DIR, OUTPUT); pinMode(X_EN, OUTPUT);
  pinMode(Y_STEP, OUTPUT); pinMode(Y_DIR, OUTPUT); pinMode(Y_EN, OUTPUT);
  
  // Ativa os motores (LOW geralmente ativa na CNC Shield)
  digitalWrite(X_EN, LOW);
  digitalWrite(Y_EN, LOW);

  Serial.println(F("--- MODO CALIBRACAO ---"));
  Serial.println(F("Digite o comando: eixo + distancia (mm)"));
  Serial.println(F("Exemplos:"));
  Serial.println(F("X100  -> Move X 100mm"));
  Serial.println(F("Y50   -> Move Y 50mm"));
  Serial.println(F("X-100 -> Move X 100mm (volta)"));
  Serial.print(F("Valores Atuais -> X: ")); Serial.print(stepsPerMM_X);
  Serial.print(F(" | Y: ")); Serial.println(stepsPerMM_Y);
}

void loop() {
  if (Serial.available()) {
    char eixo = Serial.read();
    float distancia = Serial.parseFloat(); // Lê o número após a letra
    
    // Limpa buffer restante
    while(Serial.available()) Serial.read(); 

    if (eixo == 'X' || eixo == 'x') {
      moveMotor(X_STEP, X_DIR, distancia, stepsPerMM_X);
    } 
    else if (eixo == 'Y' || eixo == 'y') {
      moveMotor(Y_STEP, Y_DIR, distancia, stepsPerMM_Y);
    }
  }
}

void moveMotor(int pinStep, int pinDir, float mm, float stepsPerMM) {
  unsigned long stepsToTake = abs(mm) * stepsPerMM;
  
  Serial.print("Movendo "); Serial.print(mm); Serial.print("mm... ");
  Serial.print("Passos totais: "); Serial.println(stepsToTake);

  // Define direção
  if (mm > 0) digitalWrite(pinDir, LOW); // Verifique se LOW é o sentido que você quer
  else digitalWrite(pinDir, HIGH);

  // Executa os passos
  for (unsigned long i = 0; i < stepsToTake; i++) {
    digitalWrite(pinStep, HIGH);
    delayMicroseconds(500); // Velocidade moderada para não perder passos
    digitalWrite(pinStep, LOW);
    delayMicroseconds(500);
  }
  Serial.println("Concluido.");
}

/*
 * Mesure 1: 100X -> 99.24
 * Mesure 2: 100X -> 99.18
 * Mesure 3: 100X -> 99.27
 * Moyenne = 99.23 -> facteur de correction = 100/99.23
 * 
 * Mesure 1: 100Y -> 100.37
 * Mesure 2: 100Y -> 100.23
 * Mesure 3: 100Y -> 100.40
 * Moyenne = 100.33 -> facteur de correction = 100/100.33 
 * */
 */
