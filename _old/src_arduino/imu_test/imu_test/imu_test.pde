import processing.serial.*;
import java.awt.datatransfer.*;
import java.awt.Toolkit;

int sysCal   = 0;
int gyroCal  = 0;
int accelCal = 0;
int magCal   = 0;

float roll  = 0.0F;
float pitch = 0.0F;
float yaw   = 0.0F;
float vect_lin_X  = 0.0F;
float vect_lin_Y  = 0.0F;
float vect_lin_Z  = 0.0F;

float vect_lin_X_last  = 0.0F;
float vect_lin_Y_last  = 0.0F;
float vect_lin_Z_last  = 0.0F;

float vect_acc_X = 0.0F;
float vect_acc_Y = 0.0F;

float last_p1 = 400.0F;
float last_p2 = 400.0F;
// float alt   = 0.0F;

// Serial port state.
Serial       port;
final String serialConfigFile = "serialconfig.txt";
boolean      printSerial = false;

int timeline;


void setup()
{
	size(800,800);
	background(0);
	frameRate(30);
	
	// Serial port setup.
	// Grab list of serial ports and choose one that was persisted earlier or default to the first port.
	int selectedPort = 0;
	String[] availablePorts = Serial.list();
	if (availablePorts == null) {
		println("ERROR: No serial ports available!");
		exit();
	}
	String[] serialConfig = loadStrings(serialConfigFile);
	if (serialConfig != null && serialConfig.length > 0) {
		String savedPort = serialConfig[0];
		// Check if saved port is in available ports.
		for (int i = 0; i < availablePorts.length; ++i) {
			if (availablePorts[i].equals(savedPort)) {
				selectedPort = i;
			} 
		}
		port = new Serial(this, availablePorts[selectedPort], 115200);
		port.bufferUntil('\n');
	} else {
		println(availablePorts);
	}
	// setSerialPort(availablePorts[selectedPort]);
}
 
void draw()
{
	stroke(255);
	float p1, p2;

	// if(sysCal>=0) {
	float yaw_rad = PI*(yaw/180.0F);

	yaw_rad = yaw_rad;

	p1 = vect_acc_X * cos(yaw_rad) - vect_acc_Y * sin(yaw_rad);
	p2 = vect_acc_Y * cos(yaw_rad) + vect_acc_X * sin(yaw_rad);

	p1 = last_p1+(p1/20.F);
	p2 = last_p2+(p2/20.F);

	last_p1 = p1;
	last_p2 = p2;


	if(abs(0-vect_acc_X)>0) {
		vect_acc_X += (0-vect_acc_X)/10;
	} 
	if(abs(0-vect_acc_Y)>0) {
		vect_acc_Y += (0-vect_acc_Y)/10;
	} 
	
	point(p1, p2);
	println("Vx:"+vect_acc_X+" Vy:"+vect_acc_Y+" yaw:"+yaw+" Calibration: Sys=" + sysCal + " Gyro=" + gyroCal + " Accel=" + accelCal + " Mag=" + magCal);
	// }
	drawGraph();	
}

void drawGraph() {
	timeline++;
	if(timeline>800) {
		timeline = 0;
	}
	stroke(126);
	line(0, 50, 800, 50);
	stroke(255,0,0);
	point(timeline,vect_lin_X*5+50);
	stroke(0,255,0);
	point(timeline,vect_lin_Y*5+50);
	stroke(0,0,255);
	point(timeline,vect_lin_Z*5+50);

}

void keyPressed() {
	background(255);
  last_p1 = 400;
  last_p2 = 400;
  background(0);

  timeline = 0;
}

void serialEvent(Serial p) 
{
	String incoming = p.readString();
	if (printSerial) {
		println(incoming);
	}
	
	if ((incoming.length() > 8))
	{
		String[] list = split(incoming, " ");
		if ( (list.length > 0) && (list[0].equals("EULER:")) ) 
		{
			roll  = float(list[3]); // Roll = Z
			pitch = float(list[2]); // Pitch = Y 
			yaw   = float(list[1]); // Yaw/Heading = X
		}
		if ( (list.length > 0) && (list[0].equals("VECT_LIN:")) ) 
		{
			vect_lin_X_last = vect_lin_X;
			vect_lin_Y_last = vect_lin_Y;
			vect_lin_Z_last = vect_lin_Z;

			vect_lin_X = float(list[1]); 
			vect_lin_Y = float(list[2]); 
			vect_lin_Z = float(list[3]); 

			/*
			if(abs(vect_lin_X)<0.1F) vect_lin_X = 0.0F; 
			if(abs(vect_lin_Y)<0.1F) vect_lin_Y = 0.0F; 
			if(abs(vect_lin_Z)<0.1F) vect_lin_Z = 0.0F; 
*/

			vect_acc_X += vect_lin_X;
			vect_acc_Y += vect_lin_Y;
			// vect_lin_X = 0.1F;
			// vect_lin_Y = 0.0F;
		}
		if ( (list.length > 0) && (list[0].equals("CALIBRATION:")) )
		{
			sysCal   = int(list[1]);
			gyroCal  = int(list[2]);
			accelCal = int(list[3]);
			magCal   = int(list[4]);
			// println("Calibration: Sys=" + sysCal + " Gyro=" + gyroCal + " Accel=" + accelCal + " Mag=" + magCal);
		}
	}
}