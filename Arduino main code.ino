#include <dht.h>                        // Including libraries 
#include <Wire.h>
#include <Stepper.h>
#include <LiquidCrystal_I2C.h>

int keypad(int x);                      //function prototypes
int thiknessIR();
int sizingIR_1();
int sizingIR_2();
int getTemperature();
int motorController(int x);
void drying();
void warning();
void buzzer(int x);
void display(int x);
void actuator(int round);
void blenderStepper();
void blenderDCMotor();
void thiknessStepper();
void sizingStepper();

#define fan_pin 22                      //Pin declaration for components 
#define coil_pin 24
#define blenderMotor 26
#define buzzer_pin 7
//Pin declarion for Sensors
#define thiknessIR_pin 2
#define sizingIR_1_pin 3
#define sizingIR_2_pin 4
#define dht11_pin 5
//Pin declarion for Keypad
#define key2 13
#define key1 12
#define key4 11
#define key3 10
//Pin declarion for Actuator
#define actuator_IN1 36
#define actuator_IN2 38
#define actuator_IN3 40
#define actuator_IN4 42
//Pin declarion for Blender Stepper Motor
#define blenderStepper_IN1 37
#define blenderStepper_IN2 39
#define blenderStepper_IN3 41
#define blenderStepper_IN4 43
//Pin declarion for Thikness Stepper Motor
#define thiknessStepper_IN1 46
#define thiknessStepper_IN2 48
#define thiknessStepper_IN3 50
#define thiknessStepper_IN4 52
//Pin declarion for Sizing Stepper Motor
#define sizingStepper_IN1 47
#define sizingStepper_IN2 49
#define sizingStepper_IN3 51
#define sizingStepper_IN4 53

dht dht11;                                                                                                               //Creating Object for DHT_11, on dht
LiquidCrystal_I2C lcd(0x3F, 20, 4);                                                                                      //Creating Object for LCD Display, on LiquidCrystal_I2C
Stepper actuatorStepper(2048, actuator_IN1, actuator_IN3, actuator_IN2, actuator_IN4);                                   //Creating Object for actuator, on Stepper
Stepper blenderStepperMotor(2048, blenderStepper_IN1, blenderStepper_IN3, blenderStepper_IN2, blenderStepper_IN4);       //Creating Object for Blender Stepper Motor, on Stepper
Stepper thiknessStepperMotor(2048, thiknessStepper_IN1, thiknessStepper_IN3, thiknessStepper_IN2, thiknessStepper_IN4);  //Creating Object for Thikness Stepper Motor, on Stepper
Stepper sizingStepperMotor(2048, sizingStepper_IN1, sizingStepper_IN3, sizingStepper_IN2, sizingStepper_IN4);            //Creating Object for Sizing Stepper Motor, on Stepper

int input_value, size, thikness, step=5;

//step  5 - Size Choosing
//step  6 - Thikness Choosing
//step  7 - Getting papers
//step  8 - Blending
//step  9 - Setting Thikness
//step 10 - Sizing
//step 11 - Drying
//step 12 - Completed

void setup() {
  lcd.init();                           // initialize the lcd
  lcd.backlight();                      // Turn on the LCD screen backlight
  lcd.begin(20, 4);

  pinMode(key1, INPUT_PULLUP);          //Pin declarition of keypad
  pinMode(key2, INPUT_PULLUP);          //Pin declarition of keypad
  pinMode(key3, INPUT_PULLUP);          //Pin declarition of keypad
  pinMode(key4, INPUT_PULLUP);          //Pin declarition of keypad
  pinMode(thiknessIR, INPUT);           //Pin declarition of Thikness IR
  pinMode(sizingIR_1, INPUT);           //Pin declarition of Sizing IR 1
  pinMode(sizingIR_2, INPUT);           //Pin declarition of Sizing IR 2  
  pinMode(blenderMotor, OUTPUT);        //Pin declarition of blender DC Motor
  pinMode(fan_pin, OUTPUT);             //Pin declarition of Fan
  pinMode(coil_pin, OUTPUT);            //Pin declarition of coil
  pinMode(buzzer_pin , OUTPUT);         //Pin declarition of buzzer
  
  display(0);                           //Display wait
  motorController(0);                   //Set actuator to starting point
}

void loop() {
  input_value = 0;                      //reset input_value
  display(step);                        //Display particular message according to step number

  if (step<=7) {
      input_value = keypad(0);          //Get input for choocing
      display(input_value);             //Display input
      if (input_value==3) {             //If input is invalid
        display(99);                    //Display invalid input message
        input_value = keypad(0);        //Get input for go back
        step--;                         //reload step
      }else if (input_value==4){        //if press 4
        if (step==5)                    //if press 4 on step 5
          { step--; }                   //reload step 5
        else                            //if press 4 on step 6 or 7
          { step-=2; }                  //go back
      }          
  }
  
  switch (step){
    case 5:                             //Assign inputkey to size, only when step 5
      size = input_value;               // 1- 3x3, 2- 5x5 inch
      break;
    case 6:                             //Assign inputkey to thikness only when step 6
      thikness = input_value;           // 1- 3mm, 2- 5mm
      break;      
    case 8:
    case 9:
    case 10:
    case 11:
      input_value= motorController(step);  //Rotate particular Motor according to step number
      if(input_value==98) display(98);     //Display Problem message and warning
      break;
    case 12:                            //At the End
      keypad(1);                        //Get input for restart
      display(0);                       //Display wait
      motorController(0);               //Set actuator to starting point 
      step = 4;                         //restart 
  }

  delay(500);
  step++;                               //go to next step
}

void display(int x) {
  
  switch (x){
    case 0:                               //Starting
      lcd.clear();
      lcd.setCursor(5, 1);
      lcd.print("PLEASE WAIT");
      delay(200);
      lcd.setCursor(2, 3);
      lcd.print("DO NOT POWER OFF");    
    break;

    case 1:                               //Got input 01
    case 2:                               //Got input 02
    case 3:                               //Got input 03
    case 4:                               //Got input 04
      lcd.clear();
      lcd.setCursor(5, 0);
      lcd.print("Got Input :");
      lcd.setCursor(9, 2);
      lcd.print(x);
      delay(200);
    break;

    case 5:                               //Size Choosing
      lcd.clear();
      lcd.setCursor(4, 0);
      lcd.print("Size Choosing");
      lcd.setCursor(0, 2);
      lcd.print("3x3 Inc - Press No 1");
      lcd.setCursor(0, 3);
      lcd.print("4x4 Inc - Press No 2");
    break;
      
    case 6:                               //Thikness Choosing
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print("THIKNESS CHOOSING");
      lcd.setCursor(1, 2);
      lcd.print("3 mm - Press No 1");
      lcd.setCursor(1, 3);
      lcd.print("5 mm - Press No 2");
    break;

    case 7:                               //Getting papers
      lcd.clear();
      lcd.setCursor(5, 0);
      lcd.print("Put Papers");
      lcd.setCursor(8, 1);
      lcd.print("and");
      lcd.setCursor(3, 2);
      lcd.print("Press Number 1");
    break;

    case 8:                               //Blending
      lcd.clear();
      lcd.setCursor(6, 1);
      lcd.print("BLENDING");
      lcd.setCursor(8, 2);
      lcd.print("10 %");
    break;

    case 9:                               //Setting Thikness
      lcd.clear();
      lcd.setCursor(2, 1);
      lcd.print("SETTING THIKNESS");
      lcd.setCursor(8, 2);
      lcd.print("25 %");
    break;
      
    case 10:                              //Sizing
      lcd.clear();
      lcd.setCursor(7, 1);
      lcd.print("SIZING");
      lcd.setCursor(8, 2);
      lcd.print("50 %");
    break;

    case 11:                              //Drying
      lcd.clear();
      lcd.setCursor(7, 1);
      lcd.print("DRYING");
      lcd.setCursor(8, 2);
      lcd.print("75 %");
    break;
      
    case 12:                              //Completed
      lcd.clear();
      lcd.setCursor(5, 1);
      lcd.print("COMPLETED!");
      lcd.setCursor(8, 2);
      lcd.print("100 %");
      delay(3000);
      lcd.setCursor(0, 3);
      lcd.print("Ps ANYKEY to restart");
    break;

    case 99:                              //Invalid Input
      lcd.setCursor(0, 0);
      lcd.print("____________________");
      lcd.setCursor(3, 1);
      lcd.print("INVALID INPUT!");
      lcd.setCursor(0, 2);
      lcd.print("____________________");
      lcd.setCursor(0, 3);
      lcd.print("PRESS ANYKEY TO BACK");
    break;

    default:                              //Warning
      lcd.clear();
      lcd.setCursor(6, 0);
      lcd.print("Warning!");
      lcd.setCursor(0, 1);
      lcd.print("!THERE IS A PROBLEM!");
      lcd.setCursor(1, 3);
      lcd.print("Need a Maintenance");
      warning();                          //blinking Backlight & beeping
  }
}

int keypad(int x) {
  int key=0;
  for (key = 0; key == 0;) {
    int key1s = digitalRead(key1);
    int key2s = digitalRead(key2);
    int key3s = digitalRead(key3);
    int key4s = digitalRead(key4);
    if (key1s==0) {
      key = 1;
    }
    if (key2s==0) {
      key = 2;
    }
    if (key3s==0) {
      key = 3;
    }
    if (key4s==0) {
      key = 4;
    } 
    if(x>0){
      x++;
    	if(x%30==0){                      //Set a delay using x%30
          delay(1000);
          buzzer(1);
          delay(1000);
        }
    }
    delay(100);   
  }
  return key;
}

int motorController (int x) {
  int irValue=1;
  switch (x){
    case 0:
      actuator(-13);                     //Set actuator to starting point, Rotate actuator CCW
      return  0;
      
    case 8:
      blenderDCMotor();                  //Start blender DC motor
      blenderStepper();                  //Start blender Stepper motor
      actuator(3);                       //Set actuator to thikness setting point, Rotate actuator CW
      return  0;
      
    case 9:
      irValue=thiknessIR();              //Getting Input from thikness IR for Check whether actuator arrived or not
      if(irValue==1){                    //If not..
          actuator(10);                  //Set actuator to end point, Rotate actuator CW
          return  98;                    //Return 98 for display warning
      }else{                             //If arrived..
        thiknessStepper();               //Rotate Thikness Stepper
          if (size == 1)
            { actuator(2); }             //Set actuator to Sizing point 01, Rotate actuator CW
          else if (size == 2)
            { actuator(4);  }            //Set actuator to Sizing point 02, Rotate actuator CW
        return  0;     
      }                    

    case 10:
        if (size == 1){
          irValue=sizingIR_1();          //Getting Input from Sizing IR 1 for Check whether actuator arrived or not
          if(irValue==1){                //If not..
              actuator(8);               //Set actuator to end point, Rotate actuator CW
              return  98;                //Return 98 for display warning
          }else{                         //If arrived..
              sizingStepper();           //Rotate Sizing Stepper
              actuator(5);               //Set actuator to Drying point, Rotate actuator CW
              return  0;
          }   
                   
        }else if (size == 2){            
          irValue=sizingIR_2();          //Getting Input from Sizing IR 2 for Check whether actuator arrived or not
          if(irValue==1){                //If not..
              actuator(6);               //Set actuator to end point, Rotate actuator CW
              return  98;                //Return 98 for display warning
          }else{                         //If arrived..
              sizingStepper();           //Rotate Sizing Stepper
              actuator(3);               //Set actuator to Drying point, Rotate actuator CW
              return  0;             
          }                
        }
              
    case 11:
      drying();                          //Do the drying Function
      actuator(3);                       //Set actuator to end point, Rotate actuator CW
      return  0;      
  }
}

void actuator(int round){
  actuatorStepper.setSpeed(15);          //set Motor Speed
  actuatorStepper.step(round*2048);      //2048 = steps per revolution

  digitalWrite(actuator_IN1, LOW);       //Power off pins
  digitalWrite(actuator_IN2, LOW);
  digitalWrite(actuator_IN3, LOW);
  digitalWrite(actuator_IN4, LOW);
}

void blenderDCMotor(){
  digitalWrite(blenderMotor, HIGH);      //Switch on blender DC motor
  delay(5000);
  digitalWrite(blenderMotor, LOW);       //Switch off blender DC motor
}

void blenderStepper() {
  blenderStepperMotor.setSpeed(10);      //set Motor Speed
  blenderStepperMotor.step(680);         //Rotate blender stepper CW
  delay(2000);
  blenderStepperMotor.step(-680);        //Rotate blender stepper CCW

  digitalWrite(blenderStepper_IN1, LOW); //Power off pins
  digitalWrite(blenderStepper_IN2, LOW);
  digitalWrite(blenderStepper_IN3, LOW);
  digitalWrite(blenderStepper_IN4, LOW);  
}

void thiknessStepper() {
  int rotation = 0;
  if (thikness == 1) {
    rotation = 480;
  } else if (thikness == 2) {
    rotation = 500;
  }
  thiknessStepperMotor.setSpeed(10);     //set Motor Speed
  thiknessStepperMotor.step(rotation);   //Rotate thikness stepper CW
  delay(1000);
  thiknessStepperMotor.step(-rotation);  //Rotate thikness stepper CCW

  digitalWrite(thiknessStepper_IN1, LOW);//Power off pins
  digitalWrite(thiknessStepper_IN2, LOW);
  digitalWrite(thiknessStepper_IN3, LOW);
  digitalWrite(thiknessStepper_IN4, LOW);    
}

void sizingStepper() {
  sizingStepperMotor.setSpeed(10);       //set Motor Speed
  if (size == 1) {                       //If Size 01 chosen
    sizingStepperMotor.step(512);        //Rotate Sizing stepper CW
    delay(1000);
    sizingStepperMotor.step(-512);       //Rotate Sizing stepper CCW
  } else if (size == 2) {                //If Size 02 Choosen
    sizingStepperMotor.step(-512);       //Rotate Sizing stepper CCW
    delay(1000);
    sizingStepperMotor.step(512);        //Rotate Sizing stepper CW
  }

  digitalWrite(sizingStepper_IN1, LOW);  //Power off pins
  digitalWrite(sizingStepper_IN2, LOW);
  digitalWrite(sizingStepper_IN3, LOW);
  digitalWrite(sizingStepper_IN4, LOW); 
}

void drying(){
  int temperature;
  digitalWrite(fan_pin, HIGH);           //Switch on Fans  
  digitalWrite(coil_pin, HIGH);          //Switch on coil
  for(int i=0; i>10; i++){
      temperature= getTemperature();        
      if(temperature>50){                //if temperature greater than 50C
         digitalWrite(coil_pin, LOW);    //Switch off coil
      } 
      delay(1000);                       //total drying time divided by 10 = 10s/10 = 1s  
  }
  digitalWrite(fan_pin, LOW);            //Switch off Fan
  digitalWrite(coil_pin, LOW);           //Switch off coil
}

int getTemperature(){
  dht11.read11(dht11_pin);               //read temperature
  return (dht11.temperature);            //return dht_11 output
}

int thiknessIR(){
  return digitalRead(thiknessIR_pin);    //return thiknessIR output
}

int sizingIR_1(){
  return digitalRead(sizingIR_1_pin);    //return sizingIR_1 output
}

int sizingIR_2(){
  return digitalRead(sizingIR_2_pin);    //return sizingIR_2 output
}

void buzzer(int x){
  switch (x){
    case 1:                              //called by step(12), keypad(1)
      tone(buzzer_pin, 700, 1000);      
      break;
    case 2:                              //called by warning function
      tone(buzzer_pin, 700, 500);
      break;
  }
}

void warning(){
  //Warning display & buzzer, called by display(98)  
  for(int i=1; i>0; i++){
    lcd.backlight();
    delay(500);
    lcd.noBacklight();
    delay(500); 
    buzzer(2);
  }
}