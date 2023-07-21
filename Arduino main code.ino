#include <OneWire.h>                    // Including libraries 
#include <DallasTemperature.h>          
#include <Wire.h>
#include <Stepper.h>
#include <LiquidCrystal_I2C.h>

int keypad();                           //function prototypes
int getIrValue(int);
double getTemperature();
void waterLevelChecker();
void getUserInput();
void actuatorChecker();
void drying();
void warning();
void display(int);
void displayMessage(int, String message);
void buzzer(int);
void actuator(int);
void blending();
void thicknessStepper();
void sizing();

void (*func[4])() = {&blending, &thicknessStepper, &sizing, &drying};       //array of four functions

#define coil_pin 46                     //Pin declaration for components
#define fan_pin 48                      
#define irPower 50
#define waterPump 52
#define buzzer_pin 9
//Pin defining for Sensors
#define DS18B20_pin 23
#define blenderIR_pin 2
#define thicknessIR_pin 3
#define sizingIR_1_pin 4
#define sizingIR_2_pin 5
#define dryingIR_pin 6
#define endIR_pin 7
#define trig_pin 53
#define echo_pin 51
//Pin defining for Keypad
#define key2 13
#define key1 12
#define key4 11
#define key3 10
//Pin defining for Actuator
#define enable_pin 36
#define step_pin 38
#define direction_pin 40
//Pin defining for Blender Motors
#define IN1 25
#define IN2 27
#define IN3 29
#define IN4 31
//Pin defining for Thikness Stepper Motor
#define thicknessStepper_IN1 41
#define thicknessStepper_IN2 43
#define thicknessStepper_IN3 45
#define thicknessStepper_IN4 49
//Pin defining for Sizing Stepper Motor
#define sizingStepper_IN1 33
#define sizingStepper_IN2 35
#define sizingStepper_IN3 37
#define sizingStepper_IN4 39

OneWire oneWire(DS18B20_pin);
DallasTemperature tmpSensor(&oneWire);                                                                                                               //Creating Object for DHT_11, on dht
LiquidCrystal_I2C lcd(0x3F, 20, 4);                                                                                      //Creating Object for LCD Display, on LiquidCrystal_I2C
Stepper thicknessStepperMotor(2048, thicknessStepper_IN1, thicknessStepper_IN3, thicknessStepper_IN2, thicknessStepper_IN4);  //Creating Object for Thickness Stepper Motor, on Stepper
Stepper sizingStepperMotor(2048, sizingStepper_IN1, sizingStepper_IN3, sizingStepper_IN2, sizingStepper_IN4);            //Creating Object for Sizing Stepper Motor, on Stepper

int size, thickness, step=0;
//step  0 - Size Choosing
//step  1 - Thickness Choosing
//step  2 - Getting papers
//step  3 - Confirmation
//step  4 - Filling water
//step  5 - Starting process
//step  6 - Blending
//step  7 - Setting Thikness
//step  8 - Sizing
//step  9 - Drying
//step 10 - Completed

void setup() {
  lcd.init();                           // Initialize the lcd
  lcd.backlight();                      // Turn on the LCD screen backlight
  lcd.begin(20, 4);
  tmpSensor.begin();

  pinMode(key1, INPUT_PULLUP);          // Pin declarition of keypad
  pinMode(key2, INPUT_PULLUP);          // Pin declarition of keypad
  pinMode(key3, INPUT_PULLUP);          // Pin declarition of keypad
  pinMode(key4, INPUT_PULLUP);          // Pin declarition of keypad
  pinMode(blenderIR_pin, INPUT);        // Pin declarition of Thikness IR
  pinMode(thicknessIR_pin, INPUT);      // Pin declarition of Thikness IR
  pinMode(sizingIR_1_pin, INPUT);       // Pin declarition of Sizing IR 1
  pinMode(sizingIR_2_pin, INPUT);       // Pin declarition of Sizing IR 2  
  pinMode(dryingIR_pin, INPUT);         // Pin declarition of Sizing IR 2
  pinMode(trig_pin, OUTPUT);            // Pin declarition of ultrasonic
  pinMode(echo_pin, INPUT);             // Pin declarition of ultrasonic
  pinMode(step_pin, OUTPUT);            // Pin declarition of nema 17
  pinMode(direction_pin, OUTPUT);       // Pin declarition of nema 17
  pinMode(enable_pin, OUTPUT);          // Pin declarition of nema 17
  pinMode(IN1, OUTPUT);           // Pin declarition of 550 DC Motor
  pinMode(IN2, OUTPUT);         // Pin declarition of Gear Motor
  pinMode(IN3, OUTPUT);                 // Pin declarition of Gear Motor
  pinMode(IN4, OUTPUT);                 // Pin declarition of Gear Motor
  pinMode(coil_pin, OUTPUT);            // Pin declarition of coil
  pinMode(fan_pin, OUTPUT);             // Pin declarition of Fan
  pinMode(irPower, OUTPUT);             // Pin declarition of ir Power
  pinMode(waterPump, OUTPUT);           // Pin declarition of Water Pump
  pinMode(buzzer_pin , OUTPUT);         // Pin declarition of buzzer

  digitalWrite(enable_pin, HIGH);
  digitalWrite(coil_pin, HIGH);
  digitalWrite(fan_pin, HIGH);
  digitalWrite(irPower, LOW);
  digitalWrite(waterPump, HIGH);
}

void loop() {
   display(step);                        //Display particular message according to step number

  if(step <= 3){
    getUserInput();
  }
  if(step == 4) {
    waterLevelChecker();
  }
  if(step == 5) {
    actuator(1);
  }
  if(step >= 6 && step < 10){
    actuatorChecker();
    func[step-6]();
    if(step==9) display(190);         //Display process by percentage
    else        display(step*20);     //Display process by percentage
    actuator(0);                      //Set actuator to thikness setting point, Rotate actuator CW
    delay(2000);
  }
  if (step == 10){
    keypad();                         //Get input for restart
    step = -1;                        //restart 
  }

  step++;
}

void getUserInput() {
  int input_value = keypad();           //Get input for choocing
  display(input_value+10);              //Display input
  if(step==0) size = input_value;       //Assign inputkey to size, only when step 2        1- 3x3, 2- 5x5 inch
  if(step==1) thickness = input_value;  //Assign inputkey to thikness only when step 3     1- 3mm, 2- 5mm
      
    switch (input_value){
      case 3:                           //If input is invalid
        display(99);                    //Display invalid input message
        keypad();                       //Get input for go back
        step--;                         //reload step
      break;

      case 4:                           //if press 4
        if (step==0)   step--;          //if press 4 on step 2 reload step
        else  step-=2;                  //if press 4 on step 3 or 4 go back
      break; 
    }
}

int keypad() {
  int key[4];
  while (true) {
    if(step==10){
      buzzer(1000);                      //buzzing_time = 1000
    }
    
    for(int i=0; i<50; i++){
      key[0] = digitalRead(key1);
      key[1] = digitalRead(key2);
      key[2] = digitalRead(key3);
      key[3] = digitalRead(key4);

      for(int i=0; i<4; i++){
        if (key[i]==0) {                 //if i th key pressed,
          buzzer(100);                   //buzzing_time = 100
          return i+1;                    //return corresponding i th value 
        }
      } delay(100);
    }  
  }
}

void display(int id) {
  
  switch (id){
    case 0:                               //Size Choosing
      displayMessage(1, "SIZE CHOOSING");
      displayMessage(3, "1.  4x4 Inches      ");
      displayMessage(4, "2.  3x3 Inches      ");
    break;
      
    case 1:                               //Thickness Choosing
      displayMessage(1, "THICKNESS CHOOSING");
      displayMessage(2, "1.  3mm             ");
      displayMessage(3, "2.  5mm             ");
      displayMessage(4, "4.  Back            ");
    break;

    case 2:                               //Getting papers
      displayMessage(1, "INPUT PAPERS");
      delay(5000);
      displayMessage(3, "1.  Next            ");
      displayMessage(4, "4.  Back            ");
    break;

    case 3:
      displayMessage(1, "DIMENSIONS");
      if(size==1){      displayMessage(2, "Size      - 4x4 INCH");  }
      if(size==2){      displayMessage(2, "Size      - 3x3 INCH");  }
      if(thickness==1){ displayMessage(3, "Thickness - 3 mm    ");  }
      if(thickness==2){ displayMessage(3, "Thickness - 5 mm    ");  }
      displayMessage(4, "1. Confirm   4. Back");
    break;

    case 4:                               //Water level
      displayMessage(1, "WATER LEVEL");
      displayMessage(4, "FILL WATER");
    break;

    case 5:                               //Starting
      displayMessage(1, "STARTING PROCESS"); 
    break;

    case 6:                               //Blending
      displayMessage(1, "BLENDING");
      displayMessage(3, " 0 %");
    break;

    case 7:                               //Setting Thikness
      displayMessage(1, "SETTING THICKNESS");
      displayMessage(3, "30 %");
    break;
      
    case 8:                               //Sizing
      displayMessage(1, "SIZING");
      displayMessage(3, "50 %");
    break;

    case 9:                               //Drying
      displayMessage(1, "DRYING");
      displayMessage(3, "70 %");
      displayMessage(4, "Temperature: 00.00 C");
    break;
      
    case 10:                               //Completed
      displayMessage(1, "COMPLETED!");
      displayMessage(3, "100%");
      delay(3000);
      displayMessage(4, "PS ANYKEY to restart");
    break; 


    case 11:                             //Got input 01
    case 12:                             //Got input 02
    case 13:                             //Got input 03
    case 14:                             //Got input 04
      displayMessage(1, "Got Input :");
      lcd.setCursor(9, 2);
      lcd.print(id-10);
      delay(400);
    break;    

    case 97:
      displayMessage(1, "Returning The Tray");
    break;

    case 98:                             //Warning
      displayMessage(1, "Warning!");
      displayMessage(2, "!THERE IS A PROBLEM!");
      displayMessage(4, "Need a Maintenance");
    break;

    case 99:                              //Invalid Input
      displayMessage(1, "____________________");
      displayMessage(2, "INVALID INPUT!");
      displayMessage(3, "____________________");
      displayMessage(4, "PRESS ANYKEY TO BACK");
    break;

    default:                             //Display process by percentage, called by functionSelector
      lcd.setCursor(8, 2);
      lcd.print(id-100);
      lcd.setCursor(11, 2);
      lcd.print("%");
  }
}

void displayMessage(int rowNo, String message) {
  if(rowNo==1)
  { lcd.clear(); }

  int colNo= (20-message.length())/2;

  lcd.setCursor(colNo, rowNo-1);
  lcd.print(message);
}

void buzzer(int buzzing_time) {
  
  tone(buzzer_pin, 900, buzzing_time);
    //When called by keypad,                buzzing_time = 100
    //When called by step(9), keypad(),     buzzing_time = 1000
    //When called by Warning function,      buzzing_time = 500
}

void actuatorChecker() {
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
    display(97);                         //display error message 
    actuator(2);                         //Set actuator to end point, Rotate actuator CW
    display(98);                         //display error message 
    warning();                           //blinking Backlight & beeping
  }
}

void actuator(int runningType) {
  int stepCount, delayTime;

  if (runningType == 0){

    switch(step){
    case 6:
      stepCount = 3800; break;
    case 7:
      if (size==1) stepCount = 4300;
      if (size==2) stepCount = 7100;
      break;
    case 8:
      if (size==1) stepCount = 6100;
      if (size==2) stepCount = 3300;
      break;
    case 9:
      stepCount = 3100;
      break;
    }

    digitalWrite(direction_pin, HIGH);
    digitalWrite(enable_pin, LOW);
    delayMicroseconds(500);
    
    for (int i = 0; i < stepCount; i++) {
      if (i <= 500)   delayTime = 1000 - i;

      digitalWrite(step_pin, HIGH); 
      delayMicroseconds(delayTime); 
      digitalWrite(step_pin, LOW); 
      delayMicroseconds(delayTime); 
    }

    delayMicroseconds(500);
    digitalWrite(enable_pin,HIGH);
  }

  if (runningType == 1){
    int irNumber, irValue, percentage=0;

    (step == 5)? irNumber = 1 : irNumber = 6;
    irValue = getIrValue(irNumber);
    stepCount = 35;
    delayTime = 900;

    (step == 5)? digitalWrite(direction_pin, LOW) : digitalWrite(direction_pin, HIGH);
    digitalWrite(enable_pin, LOW);
    delayMicroseconds(500);

    while (irValue==1){
      if(percentage%5==0){
      display((percentage/5)+100);           //Display process by percentage
      }
      
      for (int i = 0; i < stepCount; i++) {
        digitalWrite(step_pin, HIGH); 
        delayMicroseconds(delayTime); 
        digitalWrite(step_pin, LOW); 
        delayMicroseconds(delayTime); 
      }

      irValue = getIrValue(irNumber);
      percentage += 1;
    }

    delayMicroseconds(500);
    digitalWrite(enable_pin, HIGH);
    display(200);
    delay(1000);
  }

}

int getIrValue(int irNumber) {
  int value = digitalRead(irNumber+1);   
  return value;
  /*always irNumber + 1 equal to pin number of corresponding IR Senser
  IR NAME           IR NUMBER     IR PIN
  blenderIR_pin        1            2
  thiknessIR_pin       2            3
  sizingIR_1_pin       3            4
  sizingIR_2_pin       4            5
  dryingIR_pin         5            6     */
}

void waterLevelChecker() {
  int waterLevel=0;
  long time;
  bool isFirstRound = true;
  
  delay(1000);

  while (true){
    digitalWrite(trig_pin, LOW);
    delayMicroseconds(2);
    digitalWrite(trig_pin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig_pin, LOW);
    

    time = pulseIn(echo_pin, HIGH);
    waterLevel = 100 - map(time, 200, 1445, 0, 100);

    display(waterLevel + 100);

    if (isFirstRound == true && waterLevel > 60){
      break;
    } 
      
    if (waterLevel >= 98) {
      buzzer(3000);
      break;
    }

    isFirstRound = false;
    delay(800);
  }
  delay(2000);
}

void blending() {
  delay(5000);
  digitalWrite(waterPump, LOW);      //Switch on water pump
  delay(90000);
  digitalWrite(waterPump, HIGH);     //Switch off water pump
  delay(5000);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);           //Switch on blender DC motor
  delay(240000);                     //Blending time
  digitalWrite(IN2, LOW);            //Switch off blender DC motor

  display(110);                      //Display process by percentage
  delay(5000);

  digitalWrite(IN3, HIGH);           //Rotate blender
  digitalWrite(IN4, LOW);
  delay(9900);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);            //stop rotating
  delay(5000);

  digitalWrite(waterPump, LOW);      //Switch on water pump
  delay(10000);
  digitalWrite(waterPump, HIGH);     //Switch off water pump
  delay(10000);


  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  delay(10025);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  delay(2000);
}

void thicknessStepper() {
  int rotation = 0;
  if (thickness == 1) {
    rotation = 5000;
  } else if (thickness == 2) {
    rotation = 4500;
  }
  thicknessStepperMotor.setSpeed(10);    //set Motor Speed
  thicknessStepperMotor.step(-rotation);  //Rotate thikness stepper CW
  delay(10000);
  thicknessStepperMotor.step(rotation); //Rotate thikness stepper CCW

  digitalWrite(thicknessStepper_IN1, LOW); //Power off pins
  digitalWrite(thicknessStepper_IN2, LOW);
  digitalWrite(thicknessStepper_IN3, LOW);
  digitalWrite(thicknessStepper_IN4, LOW);   
  delay(2000);

}

void sizing() {
  sizingStepperMotor.setSpeed(10);      //set Motor Speed
  if (size == 1) {                      //If Size 01 chosen
    sizingStepperMotor.step(-320);       //Rotate Sizing stepper CW
    delay(10000);
    sizingStepperMotor.step(320);      //Rotate Sizing stepper CCW
  } else if (size == 2) {               //If Size 02 Choosen
    sizingStepperMotor.step(290);      //Rotate Sizing stepper CCW
    delay(10000);
    sizingStepperMotor.step(-290);       //Rotate Sizing stepper CW
  }

  digitalWrite(sizingStepper_IN1, LOW); //Power off pins
  digitalWrite(sizingStepper_IN2, LOW);
  digitalWrite(sizingStepper_IN3, LOW);
  digitalWrite(sizingStepper_IN4, LOW);
  delay(2000); 
}

void drying() {
  double temperature;
  int m= 9, s= 59;
  char time[6] = {"10:00"};

  digitalWrite(fan_pin, LOW);              //Switch on Fans  
  digitalWrite(coil_pin, LOW);             //Switch on coil

  for(int i=0; i<=600; i++){

    temperature = getTemperature();        //Check temperature 10 times
    lcd.setCursor(15, 0);
    lcd.print(time);
    lcd.setCursor(13, 3);
    lcd.print(temperature);

    (temperature >= 50)? digitalWrite(coil_pin, HIGH) : digitalWrite(coil_pin, LOW);       //Switch off coil
    (s<10)? sprintf(time, "%c%d%s%d", '0', m, ":0", s) : sprintf(time, "%c%d%c%d", '0', m, ':', s);

    if(s==0) {
      s=59;
      m-=1;
    }else {
      s-=1;
    }
    if(i==300)     display(180);             //Display process by percentage

    delay(900);      
  }

  digitalWrite(fan_pin, HIGH);               //Switch off Fans
  digitalWrite(coil_pin, HIGH);              //Switch off coil
  delay(1000);
}

double getTemperature() {
  tmpSensor.requestTemperatures();          // Request temperature readings
  return (tmpSensor.getTempCByIndex(0));    // Return temperature in Celsius
}

void warning() {
  //Warning display & buzzer, called by display(98)  
  while (true){
    lcd.backlight();
    delay(500);
    lcd.noBacklight();
    delay(500); 
    buzzer(500);                         //buzzing_time = 500
  }
}
