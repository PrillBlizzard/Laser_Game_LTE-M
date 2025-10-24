
const uint8_t laserPin = 2;      // S du laser (D2)
const unsigned int bitTime = 1000; // µs (1 ms = ~1000 bps)
const unsigned int interFrameMs = 30; // ms entre trames
const uint8_t PREAMBLE = 0xAA;   // 10101010

const uint8_t buttonPIN = 4;      // S du laser (D2)

void sendByte(uint8_t trame) {
  // Envoi bit par bit, LSB first

  //pour test de code :
    Serial.println("## Début : ");
    
  for (uint8_t i = 0; i < 8; ++i) {
    bool bit = (trame >> i) & 1;
    digitalWrite(laserPin, bit ? HIGH : LOW);
    delayMicroseconds(bitTime);

    //pour test de code :
    Serial.print(bit);
  }
  
  //pour test de code :
    Serial.println();
    Serial.print("Verif: ");
    Serial.println(trame);
    Serial.println("## Fin");
  
  digitalWrite(laserPin, LOW); //éteint après transmission de trames
}

void ISR_start_Test(){

   // préambule
    sendByte(PREAMBLE);

   // payload (exemple: 0..9)
   uint8_t frame = 0x00;
  
  // 10 trames
  for (int i =0; i < 10; ++i) {
    frame = frame+1;
    
    sendByte(frame);
    delay(interFrameMs);
  
  }
}


void setup() {

  pinMode(laserPin, OUTPUT);
  digitalWrite(laserPin, LOW);
  //délai de démarrage
  delay(1000);

  //interrupt bouton
  attachInterrupt(digitalPinToInterrupt(buttonPIN), ISR_start_Test, FALLING);

  // pour test de code :
  Serial.begin(230400);
}

void loop() {

  
}
