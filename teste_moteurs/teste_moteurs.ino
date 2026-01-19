#define stepX 7
#define dirX 8
#define enX 9 

#define stepY 4
#define dirY 5
#define enY 6 

void setup() {
  pinMode(stepX, OUTPUT);
  pinMode(dirX, OUTPUT);
  pinMode(enX, OUTPUT);

  pinMode(stepY, OUTPUT);
  pinMode(dirY, OUTPUT);
  pinMode(enY, OUTPUT);

  // Ativa os drivers (LOW ativa o motor)
  digitalWrite(enX, LOW);
  digitalWrite(enY, LOW);
  
  Serial.begin(115200);
  Serial.println("Teste de Motores Iniciado...");
}

void rodarMotor(int stepPin, int dirPin, int voltas, String nome) {
  Serial.println("Girando " + nome);
  
  // Sentido 1
  digitalWrite(dirPin, HIGH);
  for(int i = 0; i < voltas; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(400); // Velocidade mais lenta e segura para teste
    digitalWrite(stepPin, LOW);
    delayMicroseconds(400);
  }
  
  delay(500);
  
  // Sentido 2
  digitalWrite(dirPin, LOW);
  for(int i = 0; i < voltas; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(400);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(400);
  }
}

void loop() {
  rodarMotor(stepX, dirX, 500, "Eixo X");
  delay(1000);
  rodarMotor(stepY, dirY, 2000, "Eixo Y");
  delay(1000);
}
