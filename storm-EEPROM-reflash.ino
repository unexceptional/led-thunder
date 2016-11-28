// This is NOT the firmware for the Storm Controller
// Use this to perform a factory refresh of the EEPROM memory, and then immediately load the firmware
// This will reset all user-set values to the original factory settings, and it will set the clock as well

#include <LiquidCrystal.h>
#include <EEPROM.h>
#include "RTClib.h"
#include <Wire.h>
#include <Bounce.h>
#include <rotary.h>

#define BUTTON 7
#define ENCODER_A 8
#define ENCODER_B 13
#define HALF_STEP

RTC_DS1307 RTC;

boolean prev_value = HIGH;
long hold_time = 0;
byte lastPress = 1;
byte pins[] = {3,5,6,9,10,11};
//byte brightness = 70;
volatile byte position = 0;
byte lastPosition = 0;

LiquidCrystal lcd(2, 4, A3, A2, A1, A0);

Bounce bouncer = Bounce( BUTTON,5 ); // Instantiate a Bounce object with a 5 millisecond debounce time

// use these vars for pin interrupt encoder routine
Rotary r = Rotary(ENCODER_A, ENCODER_B);

ISR(PCINT0_vect)
{
  unsigned char result = r.process();
  if (result) {
    result == DIR_CW ? position -= 1 : position += 1;
  }
} 

int buttonPress()
{
  bouncer.update ( ); // Update the debouncer
  byte value = bouncer.read(); // Get the update value
  if ( value == HIGH && prev_value == HIGH ) { // Encoder button is not pressed
    prev_value = value;
    return 0;
  } 
  else if ( value != HIGH && prev_value == HIGH ) { // Catch leading edge of encoder button press, start timing
    prev_value = value;
    hold_time = millis();
    return 0;
  } 
  else if ( value == HIGH && prev_value != HIGH ) { // Catch trailing edge of encoder button press
    prev_value = value;

    if ( millis() - hold_time <= 500 ) { 
      lastPress = 1;
      return 1; // Register a short button press
    }
  } 
  else if ( value != HIGH && prev_value != HIGH ) { // Encoder button is being held down
    if ( (millis() - hold_time > 500) && lastPress != 2 ) { 
      lastPress = 2;
      return 2; // Register a long button press
    } 
    else {
      return 0;
    }
  } 
  else {
    return 0;
  }

}

void setup() {
 
     // start EEPROM variables - only need this the first time the sketch is written to each chip
    // comment it out to save 298 bytes of FLASH space
    
  if (EEPROM.read(511) != 11) {
     EEPROM.write(0,50); // sunrise/sunset transition duration
     EEPROM.write(1,1); // auto or manual mode selector
     
     // default Time Zone in San Francisco, CA
     EEPROM.write(2,90+38); // latitude [-90,90], normalized to +90
     EEPROM.write(3,highByte(58)); // highByte of longitude [-180,180] , normalized to +180
     EEPROM.write(4,lowByte(58)); // lowByte of longitude [-180,180] , normalized to +180
     EEPROM.write(5,4); // GMT offset [-12,12] normalized to +12
     EEPROM.write(6,0); // cloud mode toggle
     
     
     // default brightness for each of 6 Day PWM channels
     for (byte i=7; i<=12; i++) {
     EEPROM.write(i,80);
     }
     
     // default brightness for each of 6 Night PWM channels
     for (byte i=13; i<=18; i++) {
     EEPROM.write(i,3);
     }
     
     // default ramp delay for each of 6 PWM channels
     for (byte i=19;i<=24;i++) {
     EEPROM.write(i,2);
     }
     
     // default cloud channels for each of the 6 PWM channels
     for (byte i=25; i<= 30; i++) {
     EEPROM.write(i,1);
     }
     
     // Sunrise & Sunset Manual Mode Settings
     EEPROM.write(31,7); // sunrise time hour
     EEPROM.write(32,0); // sunrise time minute
     EEPROM.write(33,19); // sunset time hour
     EEPROM.write(34,0); // sunset time minute
     EEPROM.write(35,0); // lightning switch
     EEPROM.write(36,1); // Daylight Savings Mode
     EEPROM.write(37,0); // moon phase adjustment off by default
     EEPROM.write(38,0); // temperature mode C[0] or F[1]
     EEPROM.write(39,0); // fan mode on [1] or off [0]
     EEPROM.write(511,111); // stops this routine from running except the first time the sketch is ever loaded
     }
     
  lcd.begin(16, 2);
      Wire.begin();
    RTC.begin();
  
    DDRD = B01111110;
    DDRB = B0011110;
    DDRC = B00111111;
    PORTB = PORTB & B1101111;

    // enable pin change interrupts on those two pins:
    PCICR |= (1 << PCIE0); 
    PCMSK0 |= (1 << PCINT5) | (1 << PCINT0); // set digital pins 13 and 8 as interrupt pins
    sei(); // enable interrupts
    
    RTC.adjust(DateTime(__DATE__, __TIME__)); // this line sets the datetime to the PC datetime at compile time
    if (! RTC.isrunning()) {
     lcd.clear();
     lcd.setCursor(0, 0);
     lcd.print("RTC Error!");
     
    }
    
    
    
    
}

void loop() {

      DateTime now = RTC.now();
    char buff[15];                               //create a character long variable called buff
    sprintf(buff,"%02d/%02d %02d:%02d:%02d",now.month(),now.day(),now.hour(),now.minute(),now.second());   //build a 7 char long string from the hour minute and second   
  lcd.setCursor(0,0);
  //lcd.print("Storm Controller");
    lcd.print(buff);                  //print the time
    
    
  lcd.setCursor(0,1);
  lcd.print(analogRead(6));
  lcd.print(" - ");
  lcd.print(analogRead(7));
          lcd.setCursor(10,1);
        lcd.print(position);
        lcd.print("  ");
        
    if (buttonPress()) {
     //Drive each PWM in a 'wave'
    //PORTB = B1111111;
    digitalWrite(12,1);
  for (uint16_t i=0; i<=255; i += 1) {
    delay(3);
    for (uint8_t pwmnum=0; pwmnum <=5; pwmnum++) {
      analogWrite(pins[pwmnum], (i + (256/6)*pwmnum) % 256 );
      //delayMicroseconds(10);
    }
  }
  digitalWrite(12,0);
  //PORTB = PORTB & B1101111;
  }
  
  
      for (uint8_t pwmnum=0; pwmnum <=5; pwmnum++) {

        analogWrite(pins[pwmnum], 1 );
        if ( position % 7 == 0 ) {
          digitalWrite(12,1);
        } else {
        analogWrite(pins[position % 7 - 1], 50 );
        digitalWrite(12,0);
        }
    }
  
  
}
