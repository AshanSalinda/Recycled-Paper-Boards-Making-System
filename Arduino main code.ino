#include <dht.h>                        // Including libraries 
#include <Wire.h>
#include <Stepper.h>
#include <LiquidCrystal_I2C.h>
#include <string.h>

int keypad();                           //function prototypes
int getIrValue(int);
int getTemperature();
void getUserInput();
void actuatorChecker();
void starting();
void drying();
void warning();
void display(int);
void buzzer(int);
void actuator(int);
void blenderStepper();
void blending();
void thicknessStepper();
void sizing();

void (*func[4])() = {&blending, &thicknessStepper, &sizing, &drying};       //array of four functions

#define fan_pin 22                      //Pin declaration for components
#define coil_pin 24
#define blenderMotor 26
#define buzzer_pin 28
//Pin defining for Sensors
#define blenderIR_pin 2
#define thicknessIR_pin 3
#define sizingIR_1_pin 4
#define sizingIR_2_pin 5
#define dryingIR_pin 6
#define dht11_pin 7
//Pin defining for Keypad
#define key2 13
#define key1 12
#define key4 11
#define key3 10
//Pin defining for Actuator
#define actuator_IN1 36
#define actuator_IN2 38
#define actuator_IN3 40
#define actuator_IN4 42
//Pin defining for Blender Stepper Motor
#define blenderStepper_IN1 37
#define blenderStepper_IN2 39
#define blenderStepper_IN3 41
#define blenderStepper_IN4 43
//Pin defining for Thikness Stepper Motor
#define thicknessStepper_IN1 46
#define thicknessStepper_IN2 48
#define thicknessStepper_IN3 50
#define thicknessStepper_IN4 52
//Pin defining for Sizing Stepper Motor
#define sizingStepper_IN1 47
#define sizingStepper_IN2 49
#define sizingStepper_IN3 51
#define sizingStepper_IN4 53

dht dht11;                                                                                                               //Creating Object for DHT_11, on dht
LiquidCrystal_I2C lcd(0x3F, 20, 4);                                                                                      //Creating Object for LCD Display, on LiquidCrystal_I2C
Stepper actuatorStepper(2048, actuator_IN1, actuator_IN3, actuator_IN2, actuator_IN4);                                   //Creating Object for actuator, on Stepper
Stepper blenderStepperMotor(2048, blenderStepper_IN1, blenderStepper_IN3, blenderStepper_IN2, blenderStepper_IN4);       //Creating Object for Blender Stepper Motor, on Stepper
Stepper thicknessStepperMotor(2048, thicknessStepper_IN1, thicknessStepper_IN3, thicknessStepper_IN2, thicknessStepper_IN4);  //Creating Object for Thickness Stepper Motor, on Stepper
Stepper sizingStepperMotor(2048, sizingStepper_IN1, sizingStepper_IN3, sizingStepper_IN2, sizingStepper_IN4);            //Creating Object for Sizing Stepper Motor, on Stepper

int size, thickness, step=1;
//step  1 - Starting
//step  2 - Size Choosing
//step  3 - Thickness Choosing
//step  4 - Getting papers
//step  5 - Confirmation
//step  6 - Blending
//step  7 - Setting Thikness
//step  8 - Sizing
//step  9 - Drying
//step 10 - Completed

void setup() {
  lcd.init();                           // Initialize the lcd
  lcd.backlight();                      // Turn on the LCD screen backlight
  lcd.begin(20, 4);

  pinMode(key1, INPUT_PULLUP);          // Pin declarition of keypad
  pinMode(key2, INPUT_PULLUP);          // Pin declarition of keypad
  pinMode(key3, INPUT_PULLUP);          // Pin declarition of keypad
  pinMode(key4, INPUT_PULLUP);          // Pin declarition of keypad
  pinMode(blenderIR_pin, INPUT);        // Pin declarition of Thikness IR
  pinMode(thicknessIR_pin, INPUT);      // Pin declarition of Thikness IR
  pinMode(sizingIR_1_pin, INPUT);       // Pin declarition of Sizing IR 1
  pinMode(sizingIR_2_pin, INPUT);       // Pin declarition of Sizing IR 2  
  pinMode(dryingIR_pin, INPUT);         // Pin declarition of Sizing IR 2
  pinMode(blenderMotor, OUTPUT);        // Pin declarition of blender DC Motor
  pinMode(fan_pin, OUTPUT);             // Pin declarition of Fan
  pinMode(coil_pin, OUTPUT);            // Pin declarition of coil
  pinMode(buzzer_pin , OUTPUT);         // Pin declarition of buzzer
  
}

void loop() {
  display(step);                        //Display particular message according to step number

  switch(step){
    case 1:
      starting();
      break;
    case 2:
    case 3:
    case 4:
    case 5:
      getUserInput();
    break;    
    case 6:
    case 7:
    case 8:
    case 9:
      actuatorChecker();
      func[step-6]();
      if(step==9) display(190);         //Display process by percentage
      else        display(step*20);     //Display process by percentage
      actuator(0);                      //Set actuator to thikness setting point, Rotate actuator CW
    break;
    case 10:
      keypad();                         //Get input for restart
      step = 0;                         //restart 
  }
  
  delay(500);
  step++;
}

void getUserInput(){
  int input_value = keypad();           //Get input for choocing
  display(input_value+10);              //Display input
      
    switch (input_value){
      case 3:                           //If input is invalid
        display(99);                    //Display invalid input message
        keypad();                       //Get input for go back
        step--;                         //reload step
      break;

      case 4:                           //if press 4
        if (step==2)   step--;          //if press 4 on step 2 reload step
        else  step-=2;                  //if press 4 on step 3 or 4 go back
      break; 
    }

    if(step==2) size = input_value;              //Assign inputkey to size, only when step 2        1- 3x3, 2- 5x5 inch
    if(step==3) thickness = input_value;         //Assign inputkey to thikness only when step 3     1- 3mm, 2- 5mm
}

void actuatorChecker(){
  int irNumber, irValue;
  switch(step){
    case 6: irNumber = 1; break;
    case 7: irNumber = 2; break;
    case 8:
      if(size==1)   irNumber = 3;
      if(size==2)   irNumber = 4;
    break;
    case 9: irNumber = 5; break;  
  }

  irValue = getIrValue(irNumber);        //Getting Input from particular IR for Check whether actuator arrived or not
  if(irValue==1){                        //If not..
    actuator(1);                         //Set actuator to end point, Rotate actuator CW
    display(98);                         //display error message 
    warning();                           //blinking Backlight & beeping
  }
}

void display(int id){

  if(id<100) {
    lcd.clear();
  }
  
  switch (id){
    case 1:                               //Starting
      lcd.setCursor(5, 0);
      lcd.print("STARTING..");
      delay(200);
      lcd.setCursor(2, 3);
      lcd.print("DO NOT POWER OFF");    
    break;

    case 2:                               //Size Choosing
      lcd.setCursor(4, 0);
      lcd.print("SIZE CHOOSING");
      lcd.setCursor(0, 2);
      lcd.print("3x3 Inc - Press No 1");
      lcd.setCursor(0, 3);
      lcd.print("4x4 Inc - Press No 2");
    break;
      
    case 3:                               //Thickness Choosing
      lcd.setCursor(1, 0);
      lcd.print("THICKNESS CHOOSING");
      lcd.setCursor(1, 2);
      lcd.print("3 mm - Press No 1");
      lcd.setCursor(1, 3);
      lcd.print("5 mm - Press No 2");
    break;

    case 4:                               //Getting papers
      lcd.setCursor(5, 0);
      lcd.print("Put Papers");
      lcd.setCursor(8, 1);
      lcd.print("and");
      lcd.setCursor(3, 2);
      lcd.print("Press Number 1");
    break;

    case 5:
      lcd.setCursor(5, 0);
      lcd.print("DIMENSIONS");
      lcd.setCursor(2, 1);
      lcd.print("SIZE - 3X3 INCH");
      lcd.setCursor(2, 2);
      lcd.print("THICKNESS - 3 mm");
      if(size==2){
        lcd.setCursor(9, 1);
        lcd.print("4X4");
      }
      if(thickness==2){
        lcd.setCursor(14, 2);
        lcd.print("5");
      }
      lcd.setCursor(0, 3);
      lcd.print("Ps 1- Cnfrm, 4- Back"); 
    break;

    case 6:                               //Blending
      lcd.setCursor(6, 0);
      lcd.print("BLENDING");
      lcd.setCursor(9, 2);
      lcd.print("0 %");
    break;

    case 7:                               //Setting Thikness
      lcd.setCursor(2, 0);
      lcd.print("SETTING THICKNESS");
      lcd.setCursor(8, 2);
      lcd.print("30 %");
    break;
      
    case 8:                               //Sizing
      lcd.setCursor(7, 0);
      lcd.print("SIZING");
      lcd.setCursor(8, 2);
      lcd.print("50 %");
    break;

    case 9:                               //Drying
      lcd.setCursor(7, 0);
      lcd.print("DRYING");
      lcd.setCursor(8, 2);
      lcd.print("70 %");
    break;
      
    case 10:                               //Completed
      lcd.setCursor(5, 0);
      lcd.print("COMPLETED!");
      lcd.setCursor(8, 2);
      lcd.print("100%");
      delay(3000);
      lcd.setCursor(0, 3);
      lcd.print("Ps ANYKEY to restart");
    break; 


    case 11:                             //Got input 01
    case 12:                             //Got input 02
    case 13:                             //Got input 03
    case 14:                             //Got input 04
      lcd.setCursor(5, 0);
      lcd.print("Got Input :");
      lcd.setCursor(9, 2);
      lcd.print(id-10);
      delay(200);
    break;    

    case 98:                             //Warning
      lcd.setCursor(6, 0);
      lcd.print("Warning!");
      lcd.setCursor(0, 1);
      lcd.print("!THERE IS A PROBLEM!");
      lcd.setCursor(1, 3);
      lcd.print("Need a Maintenance");
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

    default:                             //Display process by percentage, called by functionSelector
      lcd.setCursor(8, 2);
      lcd.print(id-100);
      lcd.setCursor(11, 2);
      lcd.print("%");
  }
}

int keypad(){
  int key[4];
  while (true) {
    if(step==10){
      delay(1000);
      buzzer(1000);                      //buzzing_time = 1000
      delay(1000);
    }
    
    for(int i=0; i<30; i++){
      key[0] = digitalRead(key1);
      key[1] = digitalRead(key2);
      key[2] = digitalRead(key3);
      key[3] = digitalRead(key4);

      for(int i=0; i<4; i++){
        if (key[i]==0) {                 //if i th key pressed,
          buzzer(100);                   //buzzing_time = 100
          return i+1;                    //return corresponding i th value 
        }
      }  
    }
    delay(100);   
  }
}

void starting(){
  int percentage, irValue=1;
    for(percentage=100; percentage<200; percentage+=10){
      display(percentage);               //Display process by percentage
      actuator(0);                       //Set actuator to starting point, Rotate actuator CCW
      irValue = getIrValue(1);           //check value of 1st IR sensor
      if (irValue == 0)  break;
    }
  display(200);                          //Display process by percentage
}

void actuator(int isError){
  int toNext, toEnd, round;

  switch(step){
    case 1:
      toNext = -1; toEnd = 0;   break;
    case 6:
      toNext = 3;  toEnd = 13;  break;
    case 7:
      toEnd = 10;
      if (size==1) toNext = 2;
      if (size==2) toNext = 4;
      break;
    case 8:
      if (size==1) toNext = 5;  toEnd = 8;
      if (size==2) toNext = 3;  toEnd = 6;
      break;
    case 9:
      toNext = 3;  toEnd = 3;
  }

  if(isError==1)  round = toEnd;
  else round = toNext;

  actuatorStepper.setSpeed(15);          //set Motor Speed
  actuatorStepper.step(round*2048);      //2048 = steps per revolution

  digitalWrite(actuator_IN1, LOW);       //Power off pins
  digitalWrite(actuator_IN2, LOW);
  digitalWrite(actuator_IN3, LOW);
  digitalWrite(actuator_IN4, LOW);
}

void blending(){
  digitalWrite(blenderMotor, HIGH);  //Switch on blender DC motor
  delay(5000);                       //Blending time
  digitalWrite(blenderMotor, LOW);   //Switch off blender DC motor

  display(110);                      //Display process by percentage
  blenderStepper();                  //Start blender Stepper motor
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

void thicknessStepper() {
  int rotation = 0;
  if (thickness == 1) {
    rotation = 480;
  } else if (thickness == 2) {
    rotation = 500;
  }
  thicknessStepperMotor.setSpeed(10);    //set Motor Speed
  thicknessStepperMotor.step(rotation);  //Rotate thikness stepper CW
  delay(1000);
  thicknessStepperMotor.step(-rotation); //Rotate thikness stepper CCW

  digitalWrite(thicknessStepper_IN1, LOW); //Power off pins
  digitalWrite(thicknessStepper_IN2, LOW);
  digitalWrite(thicknessStepper_IN3, LOW);
  digitalWrite(thicknessStepper_IN4, LOW);    
}

void sizing() {
  sizingStepperMotor.setSpeed(10);      //set Motor Speed
  if (size == 1) {                      //If Size 01 chosen
    sizingStepperMotor.step(512);       //Rotate Sizing stepper CW
    delay(1000);
    sizingStepperMotor.step(-512);      //Rotate Sizing stepper CCW
  } else if (size == 2) {               //If Size 02 Choosen
    sizingStepperMotor.step(-512);      //Rotate Sizing stepper CCW
    delay(1000);
    sizingStepperMotor.step(512);       //Rotate Sizing stepper CW
  }

  digitalWrite(sizingStepper_IN1, LOW); //Power off pins
  digitalWrite(sizingStepper_IN2, LOW);
  digitalWrite(sizingStepper_IN3, LOW);
  digitalWrite(sizingStepper_IN4, LOW); 
}

void drying(){
  int temperature;
  digitalWrite(fan_pin, HIGH);              //Switch on Fans  
  digitalWrite(coil_pin, HIGH);             //Switch on coil
  for(int i=0; i<10; i++){
      if(i==5)     display(180);            //Display process by percentage

      temperature= getTemperature();        //Check temperature 10 times
      if(temperature>50){
         digitalWrite(coil_pin, LOW);       //Switch off coil
      }
       
      delay(1000);      
  }
  digitalWrite(fan_pin, LOW);               //Switch off Fans
  digitalWrite(coil_pin, LOW);              //Switch off coil
}

int getTemperature(){
  dht11.read11(dht11_pin);                  //read temperature
  return (dht11.temperature);               //return dht_11 output
}

int getIrValue(int irNumber) {
  return digitalRead(irNumber+1);     
  /*always irNumber + 1 equal to pin number of corresponding IR Senser
  IR NAME           IR NUMBER     IR PIN
  blenderIR_pin        1            2
  thiknessIR_pin       2            3
  sizingIR_1_pin       3            4
  sizingIR_2_pin       4            5
  dryingIR_pin         5            6     */
}

void buzzer(int buzzing_time){
  
  tone(buzzer_pin, 700, buzzing_time);
    //When called by keypad,                buzzing_time = 100
    //When called by step(9), keypad(),     buzzing_time = 1000
    //When called by Warning function,      buzzing_time = 500
}

void warning(){
  //Warning display & buzzer, called by display(98)  
  while (true){
    lcd.backlight();
    delay(500);
    lcd.noBacklight();
    delay(500); 
    buzzer(500);                         //buzzing_time = 500
  }
}
