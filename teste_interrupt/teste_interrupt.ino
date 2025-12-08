const byte ledPin = LED_BUILTIN;
    const byte interruptPin = 2;
    volatile byte state = LOW;

    void setup() {
      pinMode(ledPin, OUTPUT);
      pinMode(interruptPin, INPUT_PULLUP);
      attachInterrupt(digitalPinToInterrupt(interruptPin), blink, RISING);
      Serial.begin(9600);
    }

    void loop() {
      digitalWrite(ledPin, state);
      Serial.println(digitalRead(interruptPin));
      delay(100);
    }

    void blink() {
      state = !state;
    }
