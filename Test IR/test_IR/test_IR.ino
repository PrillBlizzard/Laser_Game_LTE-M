/*
 * IRremote: IRrecvDemo - demonstrates receiving IR codes with IRrecv
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 */

#include <IRremote.h>

int RECV_PIN = 11;

IRrecv irrecv(RECV_PIN);

decode_results results;

void setup()
{
  Serial.begin(2000000);
  irrecv.enableIRIn(); // Start the receiver
  Serial.println("Starting Listening");
}

void loop() {
  if (irrecv.decode(&results)) {
    
    Serial.println(results.decode_type);
    Serial.println(results.value, HEX);
    Serial.println("--------------------------");
    irrecv.resume(); // receive the next value
  }
  delay(100);
}
