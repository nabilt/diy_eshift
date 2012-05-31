#include <Servo.h> 
#include <EEPROM.h>
#include <util/delay.h>

Servo myservo; 

/* CHANGE ME */
// Servo limit - These number will change depending on how you installed your servo
//   Set MANUAL_MODE = 1, connect a potentiometer to pin 3 and open the serial monitor
//   Move to the lowest and highest gear to set these variables
int SERVO_STOP_LOW = 179;
int SERVO_STOP_HIGH = 90;

#define MANUAL_MODE 1

// variable to read the value from the analog pin 
int val;    
int potpin = 3;  
int servo_power_pin = 6;
int led_pin = 10;

// Current servo position
int currentPos = 100;
int currentPosAddr = 0;

// Number of positions to move when a button is pused 
int servo_step_size = 10;

// Flag set in the interrupt to signal a gear change
bool pendingShift = true;

void setup() 
{ 
  pinMode(servo_power_pin, OUTPUT);   
  pinMode(led_pin, OUTPUT);   
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object 
  Serial.begin(9600);
  
  digitalWrite(servo_power_pin, LOW);
  
  attachInterrupt(0, shiftUp, FALLING);
  attachInterrupt(1, shiftDown, FALLING);
  
  // Remember last position
  currentPos = EEPROM.read(currentPosAddr);
} 

void shiftUp()
{
  if ( pendingShift == true ) return;
  
  // wait for button to stop bouncing
  delay(50);
  
  currentPos -= servo_step_size;
  if ( currentPos < SERVO_STOP_HIGH) 
    currentPos = SERVO_STOP_HIGH;
    
  pendingShift = true;
  
  //Serial.println("Shift up ");
  //Serial.println(currentPos);
}

void shiftDown()
{
  if ( pendingShift == true ) return;
  
  // wait for button to stop bouncing
  delay(50);  
  
  currentPos += servo_step_size;
  if ( currentPos > SERVO_STOP_LOW) 
    currentPos = SERVO_STOP_LOW;
    
  pendingShift = true;
  
  //Serial.println("Shift down ");
  //Serial.println(currentPos);
}

void loop() 
{ 
#if MANUAL_MODE

  if ( pendingShift )
  {
    //Serial.println("ON");
    digitalWrite(led_pin, HIGH);
    digitalWrite(servo_power_pin, HIGH);
    //_delay_ms(15);
    
    myservo.write(currentPos);
    
    // Save last know position
    EEPROM.write(currentPosAddr, currentPos);
    
    // wait for servo to finish
    _delay_ms(200); 
    
    Serial.println(currentPos);
    digitalWrite(led_pin, LOW);
    digitalWrite(servo_power_pin, LOW);
    
    pendingShift = false;
  }
#else
  
  // This code controls the servo with a POT
  // To use it change MANUAL_MODE 1
  
  digitalWrite(servo_power_pin, HIGH);
  val = analogRead(potpin);            // reads the value of the potentiometer (value between 0 and 1023) 
  val = map(val, 0, 1023, 0, 179);     // scale it to use it with the servo (value between 0 and 180) 
  myservo.write(val);                  // sets the servo position according to the scaled value 
  Serial.println(val);
  delay(15);                           // waits for the servo to get there 
#endif

} 
