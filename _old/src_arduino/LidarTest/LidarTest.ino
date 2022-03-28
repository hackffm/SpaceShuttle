/* 

http://pulsedlight3d.com

This sketch demonstrates getting distance with the LIDAR-Lite Sensor

It utilizes the 'Arduino I2C Master Library' from DSS Circuits:
http://www.dsscircuits.com/index.php/articles/66-arduino-i2c-master-library 

You can find more information about installing libraries here:
http://arduino.cc/en/Guide/Libraries


*/
// #include <Servo.h> 
#include <AccelStepper.h>
#include <I2C.h>
#define    LIDARLite_ADDRESS   0x62          // Default I2C Address of LIDAR-Lite.
#define    RegisterMeasure     0x00          // Register to write to initiate ranging.
#define    MeasureValue        0x04          // Value to initiate ranging.

#define    RegisterHighLowB    0x8f          // Register to get both High and Low bytes in 1 call.

#define    RegisterAcquisitionCount     0x02          // Register to write to initiate ranging.
#define    AcquisitionCount        128          // Value to initiate ranging.

// Servo azimuthServo;

// int azimuthServoAngel = 90;
// int azimuthServoDir = 1;

AccelStepper stepper1(AccelStepper::DRIVER, 2, 3);

struct lidarDataPoint {
	int angel;
	int dist;
};

lidarDataPoint lidarRadialDataPoints[121];

void setup(){

	//stepper1.setMaxSpeed(2000.0);
	// stepper1.setAcceleration(200.0);
	// stepper1.moveTo(10000);
	// stepper1.setSpeed(200.0);
	Serial.begin(115200); //Opens serial connection at 9600bps.     

	// struct lidarDataPoint lidarRadialDataPoints[120];

	// azimuthServo.attach(9);

	// azimuthServo.write(azimuthServoAngel);

	I2c.begin(); // Opens & joins the irc bus as master
	delay(100); // Waits to make sure everything is powered up before sending or receiving data  
	I2c.timeOut(200); // Sets a timeout to ensure no locking up of sketch if I2C communication fails



	pinMode(13,OUTPUT);

	// Write 0x04 to register 0x00
	uint8_t nackack = 100; // Setup variable to hold ACK/NACK resopnses     

	while (nackack != 0){ // While NACK keep going (i.e. continue polling until sucess message (ACK) is received )
		nackack = I2c.write(LIDARLite_ADDRESS,RegisterMeasure, 0); // Write 0x04 to 0x00
		delay(1); // Wait 1 ms to prevent overpolling
	}
	

	/*
	nackack = 100; // Setup variable to hold ACK/NACK resopnses     
	while (nackack != 0){ // While NACK keep going (i.e. continue polling until sucess message (ACK) is received )
		nackack = I2c.write(LIDARLite_ADDRESS,RegisterAcquisitionCount, AcquisitionCount); // Write 0x04 to 0x00
		delay(1); // Wait 1 ms to prevent overpolling
	}
	*/
	pinMode(2, OUTPUT);
	pinMode(3, OUTPUT);

	digitalWrite(2, HIGH);
	digitalWrite(3, HIGH);
}



void loop(){

	digitalWrite(2, HIGH);
	//digitalWrite(3, HIGH);
	delay(10);
	digitalWrite(2, LOW);
	//digitalWrite(3, LOW);

	//stepper1.runSpeed();
	// stepper1.runSpeed();
	/*
	// uint8_t nackack = 100; // Setup variable to hold ACK/NACK resopnses     
	// Write 0x04 to register 0x00
	uint8_t nackack = 100; // Setup variable to hold ACK/NACK resopnses     
	digitalWrite(13,HIGH);
	// while (nackack != 0){ // While NACK keep going (i.e. continue polling until sucess message (ACK) is received )
		nackack = I2c.write(LIDARLite_ADDRESS,RegisterMeasure, MeasureValue); // Write 0x04 to 0x00
		delay(10); // Wait 1 ms to prevent overpolling
	// }
	digitalWrite(13,LOW);
	
	byte distanceArray[2]; // array to store distance bytes from read function
	
	// Read 2byte distance from register 0x8f
	nackack = 100; // Setup variable to hold ACK/NACK resopnses     
	while (nackack != 0){ // While NACK keep going (i.e. continue polling until sucess message (ACK) is received )
		nackack = I2c.read(LIDARLite_ADDRESS,RegisterHighLowB, 2, distanceArray); // Read 2 Bytes from LIDAR-Lite Address and store in array
		delay(1); // Wait 1 ms to prevent overpolling
	}
	int distance = (distanceArray[0] << 8) + distanceArray[1];  // Shift high byte [0] 8 to the left and add low byte [1] to create 16-bit int
*/
	
}


void serialOutDist() {
	Serial.print('s');
	for(int idx=0; idx<=120; idx++) {
		Serial.print(lidarRadialDataPoints[idx].angel,DEC);
		Serial.print('n');
		Serial.print(lidarRadialDataPoints[idx].dist,DEC);
		idx<120 ? Serial.print('n') : false;
	}
	Serial.print('e');
}
