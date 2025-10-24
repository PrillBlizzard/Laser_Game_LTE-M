
const uint8_t rxPin = 7;         // OUT du module photosensible (D7)
const unsigned int bitTime = 1000; // µs : doit être identique à l'emetteur
const uint8_t PREAMBLE = 0xAA;
const unsigned long PREAMBLE_TIMEOUT_MS = 5000; // attente max

bool readBitSample() {
  return digitalRead(rxPin) == HIGH;
}

// attend récepteur stable (digital) et cherche préambule
bool waitForPreamble() {
  unsigned long startMs = millis();
  // On collecte 8 bits consécutifs, en recherchant 0xAA
  while (millis() - startMs < PREAMBLE_TIMEOUT_MS) {
    // attendre une transition à l'état HIGH (début possible du préambule)
    if (digitalRead(rxPin) == HIGH) {
      // on a rencontré HIGH : on attend un demi-bit pour sample mid-bit
      delayMicroseconds(bitTime / 2);
      // lire 8 bits
      uint8_t val = 0;
      for (uint8_t i = 0; i < 8; ++i) {
        bool b = readBitSample();
        if (b) val |= (1 << i); // LSB first
        delayMicroseconds(bitTime);
      }
      if (val == PREAMBLE) return true;
      // sinon continuer la recherche
    }
  }
  return false; // timeout
}

uint8_t readPayloadByte() {
  // supposons qu'on est aligné après avoir détecté le préambule.
  // attendre une demi-bit pour arriver au milieu du premier bit du payload
  delayMicroseconds(bitTime / 2);
  uint8_t val = 0;
  for (uint8_t i = 0; i < 8; ++i) {
    bool b = readBitSample();
    if (b) val |= (1 << i);
    delayMicroseconds(bitTime);
  }
  return val;
}

void setup() {
  
  pinMode(rxPin, INPUT);
  Serial.begin(230400);
  Serial.println("Recepteur démarré...");
}

void loop() {
  
 if (!waitForPreamble()) {
    Serial.println("Preamble non trouve (timeout). Recommence.");
    delay(200);
    return;
  }

 Serial.println("Preamble detecte. Lecture payloads...");

  //10 trames
  for (int i =0; i < 10; ++i) {

     uint8_t payload = readPayloadByte();
     Serial.print(payload, BIN);
     Serial.print(" (0x");
     Serial.print(payload, HEX);
     Serial.print(") (dec ");
     Serial.print(payload);
     Serial.print(" )");
  }
}
