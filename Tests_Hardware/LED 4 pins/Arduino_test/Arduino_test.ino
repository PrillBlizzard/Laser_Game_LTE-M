
  
  int redPin = 3 ;
  int greenPin = 5;
  int bluePin = 6;

void setup() {

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  analogWrite(redPin, 0);
  analogWrite(greenPin, 175);
  analogWrite(bluePin, 175);  

  delay(3000);
  
}

void loop() {

  setRGB(255, 0, 0); // Red Color
  delay(1000);
  setRGB(0, 255, 0); // Green Color
  delay(1000);
  setRGB(0, 0, 255); // Blue Color
  delay(1000);
  setRGB(0, 255, 255); // Cyan Color
  delay(1000);
  setRGB(255, 255, 0); // Yellow Color
  delay(1000);
 
}

void setRGB(int red, int green, int blue)
{
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);  
}
