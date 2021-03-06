#include <Servo.h>                       // Servo motor Library
#include <DallasTemperature.h>           // Temp sensor Library
#include <OneWire.h>                     // Helps with temp sensor (one wire)

const int STEPPER_STEPS = 9;
const int STEPPER_DIRECTION = 8;
const int STEPPER_MS1 = 10;
const int STEPPER_MS2 = 11;
const int STEPPER_SLEEP = 6;
const int SPEED_OF_STEPPER = 5;

const int SERVO_MOTOR = 7;
const int SERVO_CLOSED_VAL = 120;
const int SERVO_OPEN_VAL = 70;

const int RELAY_FAN = 5;
const int RELAY_HEATER = 4;

const int TEMP_SENSOR = 13;
const int OUTSIDE_TEMP_SENSOR = 3;
const int LOW_BOUNDS_TEMP = 67;
const int HIGH_BOUNDS_TEMP = 80;

Servo servoMotor;

// Setting up sensor
OneWire oneWirePin(TEMP_SENSOR);            
DallasTemperature sensors(&oneWirePin);

//global for tracking whether the vents or open or not.
boolean ventsOpen;

float inside = 0;
float outside = 0;

void setup() {
    //Stepper Motor Driver pin setup
    pinMode(STEPPER_DIRECTION, OUTPUT);
    pinMode(STEPPER_MS2, OUTPUT);
    pinMode(STEPPER_MS1, OUTPUT);
    pinMode(STEPPER_STEPS, OUTPUT);
    pinMode(STEPPER_SLEEP, OUTPUT);
    // Sleep stepper motor initially
    digitalWrite(STEPPER_SLEEP, LOW); 
    
    // Servo Motor setup
    pinMode(SERVO_MOTOR, OUTPUT);

    servoMotor.attach(SERVO_MOTOR);
    // Closes vent initally
    servoMotor.write(SERVO_CLOSED_VAL);
      
    // Relay Modules setup
    pinMode(RELAY_FAN, OUTPUT);
    pinMode(RELAY_HEATER, OUTPUT);

    digitalWrite(RELAY_FAN, HIGH);    //off
    digitalWrite(RELAY_HEATER, LOW);  //off
    
    // Temperature Sensor setup
    pinMode(TEMP_SENSOR, INPUT);
    pinMode(OUTSIDE_TEMP_SENSOR, INPUT);

    // Starting serial
    Serial.begin(9600);
    
    // Beginning temperature sensor 
    sensors.begin();      

    ventsOpen = false;
}

void loop() {
    // Gets the temperature
    sensors.requestTemperatures();
    inside = sensors.toFahrenheit(sensors.getTempCByIndex(0));
    
    outside = sensors.toFahrenheit(sensors.getTempCByIndex(1));

    Serial.print("outside: "); Serial.print(outside); Serial.print("\n");
    Serial.print("inside: "); Serial.print(inside); Serial.print("\n");

    if (inside < LOW_BOUNDS_TEMP) {
        if (ventsOpen == true) {
            closeServo();
            closeStepper();
            ventsOpen = false;
        }

        digitalWrite(RELAY_FAN, HIGH);  //off
        delay(10);
        digitalWrite(RELAY_HEATER, HIGH);   //on

        delay(60000);
        
    } else if (inside > HIGH_BOUNDS_TEMP) {
        if (ventsOpen == false) {
            openServo();
            openStepper();
            ventsOpen = true;
        }

        digitalWrite(RELAY_HEATER, LOW);   //off
        delay(10);
        digitalWrite(RELAY_FAN, LOW);  //on

        delay(60000);
        
    } else {
        if (outside <= LOW_BOUNDS_TEMP && ventsOpen == true) {
            closeServo();
            closeStepper();
            ventsOpen = false;
        }
    }
   
   delay(500);
}

/** Opens the greenhouse vent if it isn't already open */
void openServo() {
    if(servoMotor.read() != SERVO_OPEN_VAL) {
        servoMotor.write(SERVO_OPEN_VAL);
    }
}

/** Closes the greenhouse vent if it isn't already closed */
void closeServo() {
    if(servoMotor.read() != SERVO_CLOSED_VAL) {
        servoMotor.write(SERVO_CLOSED_VAL);
    }
}

/**
 * Opens the box fan vent with the stepper motor
 * Wakes up stepper motor, turns on for 30 seconds to wind up string to open vent,
 *    then puts the stepper motor back in sleep mode
 */
void openStepper() {
    // Awake
    digitalWrite(STEPPER_SLEEP, HIGH); 
    delay(5);

    digitalWrite(STEPPER_MS1, LOW);
    digitalWrite(STEPPER_MS2, LOW);

    // 30 seconds
    uint32_t period = .5 * 60000L;       

    for(uint32_t tStart = millis();  (millis()-tStart) < period;  ){
        digitalWrite(STEPPER_DIRECTION, HIGH);
        digitalWrite(STEPPER_STEPS, HIGH);
        delay(SPEED_OF_STEPPER);
        digitalWrite(STEPPER_STEPS, LOW);
        delay(SPEED_OF_STEPPER);
    }

    // Sleep stepper motor
    digitalWrite(STEPPER_SLEEP, LOW); 
}

/**
 * Closes the box fan vent with the stepper motor
 * Wakes up stepper motor, turns on for 30 seconds to wind up string to close vent,
 *    then puts the stepper motor back in sleep mode
 */
void closeStepper() {
    // Awake
    digitalWrite(STEPPER_SLEEP, HIGH);
    delay(5);

    digitalWrite(STEPPER_MS1, LOW);
    digitalWrite(STEPPER_MS2, LOW);

    // 30 seconds
    uint32_t period = .5 * 60000L;       

    for(uint32_t tStart = millis();  (millis()-tStart) < period;  ){
        digitalWrite(STEPPER_DIRECTION, LOW);
        digitalWrite(STEPPER_STEPS, HIGH);
        delay(SPEED_OF_STEPPER);
        digitalWrite(STEPPER_STEPS, LOW);
        delay(SPEED_OF_STEPPER);
    }

    // Sleep stepper motor
    digitalWrite(STEPPER_SLEEP, LOW);
}
