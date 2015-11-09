//PiMios PM-2 Speed Controller Shield Code (Uses Arduino Mega ADK)
//By: BDG Customs

//Add PS3 Bluetooth library
#include <PS3BT.h>
#include <Servo.h>

//Connects PWMs to names
#define Motor1ON  2
#define Motor2ON  3
#define Motor1F  5
#define Motor1R  4
#define Motor2F  7
#define Motor2R  6
#define CON_LED  9
#define R_LED 10
#define G_LED 12

//Sets Weapon motor PWM name
Servo Hit;

//Setup USB
USB Usb;
//Setup Bluetooth
BTD Btd(&Usb);
//Used to write Dongle MAC to PS3 Controller
PS3BT PS3(&Btd);

//Setups A5 to read Battery Voltage
int VinPin = A5;

//Setup Variables
int Vin = 0;
int valL;                                                                  
int valR; 
int valH;
int set_speed;
int speed_mode;

//Setup PWMs as outputs
void init_pwm()
{
        pinMode(Motor1ON, OUTPUT);
	pinMode(Motor2ON, OUTPUT);
	pinMode(Motor1F, OUTPUT);
        pinMode(Motor1R, OUTPUT);
        pinMode(Motor2F, OUTPUT);
        pinMode(Motor2R, OUTPUT);
        pinMode(CON_LED, OUTPUT);
        pinMode(R_LED, OUTPUT);
        pinMode(G_LED, OUTPUT);
}

void setup() {
 //Check for USB Initialize
  if (Usb.Init() == -1) {
    while(1); //halt
  }
    //Setup Weapon motor to PWM 8
    Hit.attach(8);
    //Run init_pwm
    init_pwm();
    //Set Drive Motors to OFF
    analogWrite(Motor1ON, 0);
    analogWrite(Motor1F,0);
    analogWrite(Motor1R,0);  
    analogWrite(Motor2ON, 0);
    analogWrite(Motor2F,0);
    analogWrite(Motor2R,0);
    //Set LED 3 Red OFF
    analogWrite(R_LED,0); 
    //Set LED 3 Green ON (This is to show power is ON)
    analogWrite(G_LED,50); 
    //Speed mode is set to 1 (4 levels are allowed and changed can be changed by R1)
    speed_mode = 1;
    //Motor speed can be set from 0 to 255, 255 = Full battery voltage
    set_speed = 45;
    //Weapon motor is set to neutral
    Hit.write(90);
    //LED 1&2 are sett to Blue to signal ready to connect PS3 controller
    analogWrite(CON_LED, 50);
        
}
//Arduino Loop that is always running
void loop() {
  Usb.Task();
  //Read Vin Voltage connected to A5
  Vin = analogRead(VinPin);
  //Checks Battery voltage is high enough, Can be calculated by: 
  //Desired Voltage x .091 = voltage at A5 pin, Then A5 pin voltage x 204.6 = Value to use below
  if (Vin > 211){
    //Turns LED 3 Green ON and Red OFF, this is to reset it if falls into under voltage condition
    analogWrite(G_LED, 50);
    analogWrite(R_LED, 0); 
     //Checks if PS3 controller is connected, if connected then enters  
     if(PS3.PS3Connected || PS3.PS3NavigationConnected) {
      //Turns LED 1&2 Blue OFF 
      analogWrite(CON_LED, 0);
      //Gets Right Analog Stick Value
      valR = PS3.getAnalogHat(LeftHatY);
      //Gets Left Analog Stick Value
      valL = PS3.getAnalogHat(RightHatY);
      //Gets R2 button Value for Weapon Motor
      valH = PS3.getAnalogButton(R2);
      //Takes R2 button Value and changes it for a 0 to 255 value to a 90 to 40 value
      //Setup to work with Castle Mamba Monster 2 (90 = neutral & 40 = 1 ms pulse for full forward)
      valH = map(valH, 0, 255, 90, 40);
      //Write value to PWM connected to Weapon motor
      Hit.write(valH);
      
      //If PS3 controler R1 is pressed it cycles through speed modes
      if(PS3.getButtonClick(R1)){
        //speed mode cycles one when R1 is pressed
        speed_mode = speed_mode +1;
        //Case to setup for each Speed mode
        switch (speed_mode){
          //Mode 1 sets LED 1 ON and LED2,3,&4 OFF and sets speed to below
          case 1:
          set_speed = 45;
          PS3.setAllOff();
          PS3.setLedOn(LED1);
          break;
          //Mode 2 sets LED 2 ON and sets speed to below
          case 2:
          set_speed = 90;
          PS3.setLedOn(LED2);
          break;
          //Mode 3 sets LED 3 ON and sets speed to below
          case 3:
          set_speed = 135;
          PS3.setLedOn(LED3);
          break;
          //Mode 4 sets LED 4 ON and sets speed to below
          case 4:
          set_speed = 180;
          PS3.setLedOn(LED4);
          speed_mode = 0;
          break;
        }}
        
      //If Right Analog stick is center turn OFF Right motor
      if (valR >= 120 && valR <= 134){
        analogWrite(Motor1ON, 0);
        analogWrite(Motor1F,0);
        analogWrite(Motor1R,0);      
      }
      //If Right Analog stick is pushed forward Right motor goes forward
      if (valR <= 119){
        analogWrite(Motor1ON, 255);
        valR = map(valR, 0, 119, set_speed, 0);
        analogWrite(Motor1R,0);  
        analogWrite(Motor1F,valR);
        valR = 127;    
      }
      //If Right Analog stick is pushed back Right motor goes reverse
      if (valR >= 135){
        analogWrite(Motor1ON, 255);
        valR = map(valR, 135, 255, 0, set_speed);
        analogWrite(Motor1F,0);
        analogWrite(Motor1R,valR);    
        valR = 127;   
      }
      //If Left Analog stick is center turn OFF Left motor
      if (valL >= 120 && valL <= 134){
        analogWrite(Motor2ON, 0);
        analogWrite(Motor2F,0);
        analogWrite(Motor2R,0);      
      }
      //If Left Analog stick is pushed forward Left motor goes forward
      if (valL <= 119){
        analogWrite(Motor2ON, 255);
        valL = map(valL, 0, 119, set_speed, 0);
        analogWrite(Motor2F,valL);
        analogWrite(Motor2R,0); 
        valL = 127;      
      }
      //If Left Analog stick is pushed back Left motor goes reverse
      if (valL >= 135){
        analogWrite(Motor2ON, 255);
        valL = map(valL, 135, 255, 0, set_speed);
        analogWrite(Motor2R,valL); 
        analogWrite(Motor2F,0);
        valL = 127;      
      }
    }
    
    //If PS button is pressed controller is disconnected all motors are turned OFF
    if(PS3.getButtonClick(PS)) {
      PS3.disconnect();
      analogWrite(Motor1ON, 0);
      analogWrite(Motor1F,0);
      analogWrite(Motor1R,0);  
      analogWrite(Motor2ON, 0);
      analogWrite(Motor2F,0);
      analogWrite(Motor2R,0); 
      speed_mode = 1;
      set_speed = 45;
     
     // Can be used if weapon is large and needs reversed supplied to slow faster
     // Hit.write(140); 
     // delay(1000);
     
      Hit.write(90);
      analogWrite(CON_LED, 50);
    } }
    
    //If battery is too low turns LED 3 Red (If only temp then will pulse motors OFF until battery voltage is regained.)
    else {
      analogWrite(R_LED,50);
      analogWrite(G_LED,0);
      analogWrite(Motor1ON, 0);
      analogWrite(Motor1F,0);
      analogWrite(Motor1R,0);  
      analogWrite(Motor2ON, 0);
      analogWrite(Motor2F,0);
      analogWrite(Motor2R,0);  
      Hit.write(90);
    }
      

}
