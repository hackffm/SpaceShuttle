/* 

http://pulsedlight3d.com

This sketch demonstrates getting distance with the LIDAR-Lite Sensor

It utilizes the 'Arduino I2C Master Library' from DSS Circuits:
http://www.dsscircuits.com/index.php/articles/66-arduino-i2c-master-library 

You can find more information about installing libraries here:
http://arduino.cc/en/Guide/Libraries


*/
// #include <Servo.h> 
#include <I2C.h>
#include <util/atomic.h>

#define    LIDARLite_ADDRESS   0x62          // Default I2C Address of LIDAR-Lite.
#define    RegisterMeasure     0x00          // Register to write to initiate ranging.
#define    MeasureValue        0x04          // Value to initiate ranging.

#define    RegisterHighLowB    0x8f          // Register to get both High and Low bytes in 1 call.

#define    RegisterAcquisitionCount     0x02          // Register to write to initiate ranging.
#define    AcquisitionCount        0x10          // Value to initiate ranging.

#define PIN_LED		13
#define PIN_STEP 	9 // toggled by interrupt OCR1A
#define PIN_DIR 	2
#define PIN_ENCODER	3

// Servo azimuthServo;

// int azimuthServoAngel = 90;
// int azimuthServoDir = 1;

uint16_t stepPerRev;

uint8_t state;

struct lidarDataPoint {
	int angel;
	int dist;
};

lidarDataPoint lidarRadialDataPoints[121];

volatile uint16_t targetStepOCR1A = 4;

volatile uint16_t currentPos;

void setup(){

	// init pin's
	pinMode(PIN_LED,OUTPUT);
	digitalWrite(PIN_LED, LOW);

	pinMode(PIN_STEP, OUTPUT);
	pinMode(PIN_DIR, OUTPUT);

	digitalWrite(PIN_STEP, LOW);
	digitalWrite(PIN_DIR, HIGH);

	// init serial
	Serial.begin(115200); //Opens serial connection at 9600bps.     

	// struct lidarDataPoint lidarRadialDataPoints[120];

	// azimuthServo.attach(9);

	// azimuthServo.write(azimuthServoAngel);

	
	I2c.begin(); // Opens & joins the irc bus as master
	delay(100); // Waits to make sure everything is powered up before sending or receiving data  
	I2c.timeOut(200); // Sets a timeout to ensure no locking up of sketch if I2C communication fails
	


	// Write 0x04 to register 0x00
	
	uint8_t nackack = 100; // Setup variable to hold ACK/NACK resopnses     

	while (nackack != 0){ // While NACK keep going (i.e. continue polling until sucess message (ACK) is received )
		nackack = I2c.write(LIDARLite_ADDRESS,RegisterMeasure, 0); // Write 0x04 to 0x00
		delay(1); // Wait 1 ms to prevent overpolling
	}
	
	

	
	nackack = 100; // Setup variable to hold ACK/NACK resopnses     
	while (nackack != 0){ // While NACK keep going (i.e. continue polling until sucess message (ACK) is received )
		nackack = I2c.write(LIDARLite_ADDRESS,RegisterAcquisitionCount, AcquisitionCount); // Write 0x04 to 0x00
		delay(1); // Wait 1 ms to prevent overpolling
	}
	

	// init interrupt stepper driver control 
	noInterrupts();           // disable all interrupts
	TCCR1A = 0;
	TCCR1B = 0;
	TCCR1C = 0;
	TCNT1  = 0;

	OCR1A = -1; // 62500;      // compare match register 16MHz/256/2Hz
	TCCR1A |= (1 << COM1A0);	
	TCCR1B |= (1 << WGM12);   // CTC mode
	TCCR1B |= (1 << CS12);    // 256 prescaler 
	TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
	interrupts();             // enable all interrupts
}

volatile bool lastEncoderPinState;

ISR(TIMER1_COMPA_vect)
{
	uint16_t deltaTargetOCR1A;
	noInterrupts();
	deltaTargetOCR1A = OCR1A-targetStepOCR1A;
	OCR1A -= deltaTargetOCR1A/2;
	if(deltaTargetOCR1A<10) {
		OCR1A = targetStepOCR1A;
		/// TIMSK1 = 0;
		// digitalWrite(PIN_LED, HIGH);
	}
	currentPos++;
	bool currentEncoderState = digitalRead(PIN_ENCODER);
	if(lastEncoderPinState!=currentEncoderState) {
		lastEncoderPinState = currentEncoderState;
		currentEncoderState==true ? currentPos = 0 : false;
	}
	interrupts();	
}


void loop(){
	scan();
}

void scan() {
	uint16_t currentPosTmp;
	// uint8_t nackack = 100; // Setup variable to hold ACK/NACK resopnses     
	// Write 0x04 to register 0x00
	uint8_t nackack = 100; // Setup variable to hold ACK/NACK resopnses     
	digitalWrite(13,HIGH);
	while (nackack != 0){ // While NACK keep going (i.e. continue polling until sucess message (ACK) is received )
		nackack = I2c.write(LIDARLite_ADDRESS,RegisterMeasure, MeasureValue); // Write 0x04 to 0x00
		delay(10); // Wait 1 ms to prevent overpolling
	}
	
	byte distanceArray[2]; // array to store distance bytes from read function
	
	// Read 2byte distance from register 0x8f
	nackack = 100; // Setup variable to hold ACK/NACK resopnses     
	while (nackack != 0){ // While NACK keep going (i.e. continue polling until sucess message (ACK) is received )
		nackack = I2c.read(LIDARLite_ADDRESS,RegisterHighLowB, 2, distanceArray); // Read 2 Bytes from LIDAR-Lite Address and store in array
		delay(1); // Wait 1 ms to prevent overpolling
	}

	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		currentPosTmp = currentPos;
	}

	int distance = (distanceArray[0] << 8) + distanceArray[1];  // Shift high byte [0] 8 to the left and add low byte [1] to create 16-bit int
	digitalWrite(13,LOW);

	Serial.print('s');
	Serial.print(distance,DEC);
	Serial.print(',');
	Serial.print(currentPosTmp,DEC);
	Serial.println('e');
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
