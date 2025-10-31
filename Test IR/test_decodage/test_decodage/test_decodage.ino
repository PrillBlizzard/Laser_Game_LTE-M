/*
  A sketch to capture the codes transmitted by our IR Remote Control
*/

#include <IRremote.hpp> // Include the IRremote.hpp library

const byte IR_RECEIVE_PIN = 2; // Receive IR Data Out on Pin 2

void setup() {
  // Enable the Serial Monitor at 9600 baud
  Serial.begin(9600);
  // Activate the IR Receiver
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
}

void loop() {
  // Check to see if an IR code is received
  if (IrReceiver.decode()) {
    // Print out the IR code data to the Serial Monitor
    IrReceiver.printIRResultShort(&Serial);
    // Print out a blank line to the Serial Monitor
    Serial.println();
    // Tell the IR Receiver to listen for the next code
    IrReceiver.resume();
  }
}
