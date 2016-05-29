/**
 * oscP5sendreceive by andreas schlegel
 * example shows how to send and receive osc messages.
 * oscP5 website at http://www.sojamo.de/oscP5
 */
 
import processing.serial.*;
  

// The serial port:
Serial myPort;       

void setup() {
  size(400,400);
  frameRate(25);

  
  // List all the available serial ports:
  println(Serial.list());

  // Open the port you are using at the rate you want:
  myPort = new Serial(this, Serial.list()[7], 115200);  
  // myPort = new Serial(this, Serial.list()[0], 57600);

  println("p1");
  delay(3000);
  myPort.write("rm");
  myPort.write(0x0D);
  println("p2");
}

void readData() {
  // Send a capital A out the serial port:
  myPort.write("rp");
  myPort.write(0x0D);
}

int lf = 10;

void draw() {
  background(0);

  
  serialParser();
}


void mousePressed() {  
  readData();
}


byte charCount = 0;
int[] dists = new int[121];

char[] buf = new char[10];

int distArrayCounter = 0;

void serialParser() {


  
  if(myPort.available()>0) {
    int c = myPort.read();

    // println(c);

    // if(charCount>0) charCount--;

    distArrayCounter++;
    
    if(c=='s') {
      distArrayCounter = 0;
    }
    
    if(c=='e') {
      
    }
    
  }
}