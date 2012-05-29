#include <SoftwareServo.h>
#include <Servo.h> 
#include <EEPROM.h>
#include <util/delay.h>
//#include <EEPROMWearLeveler.h>

/*
http://www.thunderboltrc.com/index.php?main_page=product_info&products_id=386
(1) Servo Mounting Bracket (For Futaba).
(4) 2-56 Socket cap machine screws.
(4) #2 flat washers.
(4) 2-56 Nylon insert Lock Nuts.
*/

Servo myservo; 
//SoftwareServo myservo;
int potpin = 3;  // analog pin used to connect the potentiometer
int servo_power_pin = 6;
int val;    // variable to read the value from the analog pin 

int led_pin = 10;

int currentPos = 1;
int servo_step_size = 30;
bool pendingShift = false;

const int PARAM_BUFF_LENGTH = 20;

void setup() 
{ 
  pinMode(servo_power_pin, OUTPUT);   
  pinMode(led_pin, OUTPUT);   
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object 
  Serial.begin(9600);
  
  digitalWrite(servo_power_pin, LOW);
  
  attachInterrupt(0, shiftUp, FALLING);
  attachInterrupt(1, shiftDown, FALLING);
  //EEPROMWearLeveler.read(1);
} 

void EEPROMWearLevelWrite(int address, uint8_t value)
{
}

int pin_debounce_state(int pin)
{  
  if ( digitalRead(pin) == LOW )
  {
    // debounce
    delay(30);
    if ( digitalRead(pin) == LOW )
    {
      // Low
      return 0;
    } 
    // High
    return 1;
  }
  return 1;
}



void shiftUpISR()
{
}
void shiftUp()
{
  if ( pendingShift == true ) return;
  
  //pin_debounce_state(2);
  delay(50);
  
  currentPos -= servo_step_size;
  if ( currentPos < 1) currentPos = 1;
  pendingShift = true;
  
  Serial.println("Shift up ");
  Serial.println(currentPos);
}

void shiftDown()
{
  if ( pendingShift == true ) return;
  
  //pin_debounce_state(3);
  delay(50);  
  
  currentPos += servo_step_size;
  if ( currentPos > 170) currentPos = 170;
  pendingShift = true;
  
  Serial.println("Shift down ");
  Serial.println(currentPos);
}

void loop() 
{ 

  //noInterrupts();
  if ( pendingShift )
  {
    //Serial.println("ON");
    digitalWrite(led_pin, HIGH);
    digitalWrite(servo_power_pin, HIGH);
    _delay_ms(15);
    myservo.write(currentPos);
    
    // wait for servo to finish
    _delay_ms(100); 
    
    //Serial.println("OFF");
    digitalWrite(led_pin, LOW);
    digitalWrite(servo_power_pin, LOW);
    //delay(5000);
    
    pendingShift = false;
  }
  //interrupts();
  
  
  // Sleep
  
  /*
  val = analogRead(potpin);            // reads the value of the potentiometer (value between 0 and 1023) 
  val = map(val, 0, 1023, 0, 179);     // scale it to use it with the servo (value between 0 and 180) 
  myservo.write(val);                  // sets the servo position according to the scaled value 
  Serial.println(val);
  delay(15);                           // waits for the servo to get there 
*/
} 
