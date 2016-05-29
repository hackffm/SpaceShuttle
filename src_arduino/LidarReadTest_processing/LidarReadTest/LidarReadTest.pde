/**
 * Simple Read
 * 
 * Read data from the serial port and change the color of a rectangle
 * when a switch connected to a Wiring or Arduino board is pressed and released.
 * This example works with the Wiring / Arduino program that follows below.
 */


import processing.serial.*;

Serial myPort;  // Create object from Serial class
int val;      // Data received from the serial port

void setup() 
{
  size(500, 500);
  // I know that the first port in the serial list on my mac
  // is always my  FTDI adaptor, so I open Serial.list()[0].
  // On Windows machines, this generally opens COM1.
  // Open whatever port is the one you're using.
  // print(Serial.list());
  String portName = Serial.list()[3];
  println(Serial.list()[3]);
  myPort = new Serial(this, portName, 115200);
  background(255);             // Set background to white
}

String result = null;
int maxRevPos = 0;

void draw()
{
  stroke(204);
  fill(255,0.2);                 // set fill to light gray
  rect(0, 0, 500, 500);
  
  if ( myPort.available() > 0) {  // If data is available,
    char lf = 'e';
    result = myPort.readStringUntil(lf);

    
    if(result!=null) {
      String[] parts = split(result, ',');
      String[] partA = split(parts[0],'s');
      String[] partB = split(parts[1],'e');
      
      int dist = 0;
      int revPos = 0;
      try {
        dist = Integer.parseInt(partA[1]);
      } finally {
      }
      try {
        revPos = Integer.parseInt(partB[0]);
      } finally {
      }
      
      if(revPos>maxRevPos) {
        maxRevPos = revPos;
      }
      // println(maxRevPos);
      
      float revPosRadian = revPos * ((PI*2)/maxRevPos);
      
      float distFloat = float(dist) * 0.7;
      
      float x = sin(revPosRadian)*distFloat;
      float y = cos(revPosRadian)*distFloat;
      
      fill(0);                 // set fill to light gray
      rect(x+250, y+250, 5, 5);
    }
  }
  
  
  stroke(204);
  fill(204);                 // set fill to light gray
  rect(250, 250, 10, 10);
  
}



/*

// Wiring / Arduino Code
// Code for sensing a switch status and writing the value to the serial port.

int switchPin = 4;                       // Switch connected to pin 4

void setup() {
  pinMode(switchPin, INPUT);             // Set pin 0 as an input
  Serial.begin(9600);                    // Start serial communication at 9600 bps
}

void loop() {
  if (digitalRead(switchPin) == HIGH) {  // If switch is ON,
    Serial.write(1);               // send 1 to Processing
  } else {                               // If the switch is not ON,
    Serial.write(0);               // send 0 to Processing
  }
  delay(100);                            // Wait 100 milliseconds
}

*/