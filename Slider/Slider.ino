#include <LiquidCrystal.h>
#include <TimerOne.h>

//LCD

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);           // select the pins used on the LCD panel

// define some values used by the panel and buttons
int lcd_key     = 0;    //the current pressed button
int lastKey =0;         //the previous cycle pressed button
int adc_key_in  = 0;  //the voltage input

#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

//define pins for the motor and shutter
//#define STEP_PIN=12;
#define SHUTTERPIN  22
#define STEPPIN     30
#define DIRPIN      31


// made up chars for the GUI
byte upAndDown[8] = {
  B00100,
  B01110,
  B11111,
  B00000,
  B11111,
  B01110,
  B00100,
};
byte leftArr[8] = {
  B00011,
  B00111,
  B01111,
  B11111,
  B01111,
  B00111,
  B00011,
};
byte rightArr[8] = {
  B11000,
  B11100,
  B11110,
  B11111,
  B11110,
  B11100,
  B11000,
};

#define upAndDownChar 0
#define leftArrChar 1
#define rightArrChar  2

//settings variables
float longitude=150; //expressed in cm
float maxVel=19.2; //expressed in cm/s
int maxAcc=1; //expressed in cm/s*s

float cmPerStep=0.016; //The cms each Step moves
//It would be interesting for this to be automatically calibrated using both endstops

//Running moment TL variables
int totalPicsTL=0; //The total amount of pictures to take on this run
int takenPicsTL=0; //The amount of already taken pics
int leftPicsTL=0; //Pics left=totalPics-takenPics
int etaTL=0; //ETA to finish expressed in s. Must check if can be a local variable

int tickCount=0;  //For the motor timing
int totalTicks=0;

int stepsLeft=0; //The steps left to be done on this moving cycle
float longLeft=longitude; //The longitude left on the whole slider

int runningMoment=0; //0:Take pic ; 1:Wait ; 2:Move

int lastTime=0;
int actualTime=0;//For time measurement

int firstMotorMove=0;

int isRunningTL=0;
int firstRunTL=0;
int isAdjustingTL=0;

//All the video shit
int isRunningVI=0;
int firstRunVi=0;
int isAdjustingVi=0;


//for menu management
int actualMenu=0;


int actualMode=0; //the actual mode: 0 being TL and 1 video
char*modes[]={"Time Lapse", "Video"}; //the names of the modes




//variables
int timeIntTL=1; //The interval of time between every picture when doing a TL expressed in seconds

float distIntTL=0.2; //The interval of space between every picture when doing a TL expressed in cm

//must be changed to mm


void setup()
{

 lcd.begin(16, 2);              // start the library
 lcd.setCursor(0,0);
 lcd.createChar(upAndDownChar, upAndDown);  //Create the chars
 lcd.createChar(leftArrChar,leftArr);
 lcd.createChar(rightArrChar,rightArr);
 lcd.clear();
 Serial.begin(9600);
 Serial.println("Hello");
 pinMode(SHUTTERPIN, OUTPUT);

 pinMode(STEPPIN, OUTPUT);
 pinMode(STEPPIN, OUTPUT);

 digitalWrite(STEPPIN, LOW);
 digitalWrite(DIRPIN, LOW);


}

void loop()
{
  if (isRunningTL==1){

    runningOrganizer();

  }else if (isAdjustingTL==1){

    guiSettingsTL();

  }else{
    guiPrimarioTL();
  }



}

void runningOrganizer(){
  if (firstRunTL==1){
    setupRunningTL();
    firstRunTL=0;
  }
  switch(runningMoment){
    case 0:{
      takePic();
      break;
    }

    case 1:{
      wait();
      break;
    }

    case 2:{
      movementTL();
      break;
    }

  }

  guiRunningTL();

}


void guiPrimarioTL(){
  char* menus[]={"Modo", "T entre fotos", "D entre fotos",  "Empezar", "Ajustes"}; //the different menus for this mode

  lastKey=lcd_key; //so there is not a push per cycle
  lcd_key = read_LCD_buttons();   // read the buttons


  lcd.setCursor(0,0);
  lcd.write(byte(1));

  lcd.setCursor(15,0);
  lcd.write(byte(2));


  lcd.setCursor(1,0);

  if (actualMenu>4){ //So it returns as a cycle
    actualMenu=0;
  }

  if (actualMenu<0){
    actualMenu=4;
  }


  lcd.print(menus[actualMenu]); //prints the actual menu

  lcd.setCursor(0,1);

  //this switch prints the under line
  switch (actualMenu){

    case 0:{
      lcd.print(modes[actualMode]);
      lcd.setCursor(14,1);
      lcd.write(byte(0));
      break;
    }

    case 1:{
      lcd.print(timeIntTL);
      lcd.setCursor(10,1);
      lcd.print("seg");
      lcd.setCursor(14,1);
      lcd.write(byte(0));
      break;
    }

    case 2:{
      lcd.print(distIntTL);
      lcd.setCursor(10,1);
      lcd.print("cm");
      lcd.setCursor(14,1);
      lcd.write(byte(0));
      break;
    }

    case 3:{
      lcd.print("Sel para empezar");
      break;
    }

    case 4:{
      lcd.print("Sel para ajustes");
      break;
    }
  }


  if (lastKey!=lcd_key){ //so there is an action per push

    lcd.clear();

    switch (lcd_key){ //an action per button pushed

      case btnRIGHT:{
        actualMenu++;
        break;
      }

      case btnLEFT:{
        actualMenu--;
        break;
      }

      case btnUP:{

        switch (actualMenu){ //this switch changes values

          case 0:{
            actualMode++;
            if (actualMode==2){
              actualMode=0;
            }
            break;
          }

          case 1:{
            timeIntTL++;
            break;
          }

          case 2:{
            distIntTL=distIntTL+0.1;
            break;
          }

          default:{
            break;
          }
        }
        break;

      }

      case btnDOWN:{

        switch (actualMenu){ //this switch changes values

          case 0:{
            actualMode--;
            if (actualMode==-1){
              actualMode=1;
            }
            break;
          }

          case 1:{
            timeIntTL--;
            break;
          }

          case 2:{
            distIntTL=distIntTL-0.1;
            break;
          }

          default:{
            break;
          }
        }
        break;
      }

      case btnSELECT:{
        switch (actualMenu){
          case 3:{
            isRunningTL=1;
            firstRunTL=1;
            break;
          }

          case 4:{
            isAdjustingTL=1;
            break;
          }

          default:{
            break;
          }
        }
        break;
      }


    }
  }



}

void guiSettingsTL(){ //In here we display and manage settings
  char* menus[]={"Longitud", "Cm por step", "Vel Max",  "Acc Max"}; //the different menus for this GUI
  float variables[]={longitude, cmPerStep, maxVel, maxAcc}; //Values to be changed
  char *units[]={"cms", "cms", "cm/s", "c/s2"}; //Units
  float alts[]={0.1,0.001,0.1,0.1};

  lastKey=lcd_key; //so there is not a push per cycle
  lcd_key = read_LCD_buttons();   // read the buttons

  lcd.setCursor(0,0);
  lcd.write(byte(leftArrChar));

  lcd.setCursor(15,0);
  lcd.write(byte(rightArrChar));


  lcd.setCursor(1,0);

  if (actualMenu>3){ //So it returns as a cycle
    actualMenu=0;
  }
  if (actualMenu<0){
    actualMenu=3;
  }



  lcd.print(menus[actualMenu]); //prints the actual menu

  //This four next lines and the variables[] array replace the switch mess I had on the original version, to be tested.
  lcd.setCursor(0,1);

  lcd.print(variables[actualMenu]);
  lcd.setCursor(10,1);
  lcd.print(units[actualMenu]);
  lcd.setCursor(14,1);
  lcd.write(byte(upAndDownChar));



  if (lastKey!=lcd_key){ //so there is an action per push

    lcd.clear();

    switch (lcd_key){ //an action per button pushed

      case btnRIGHT:{
        actualMenu++;
        break;
      }

      case btnLEFT:{
        actualMenu--;
        break;
      }

      case btnUP:{

        variables[actualMenu]=variables[actualMenu]+alts[actualMenu];

        break;

      }

      case btnDOWN:{

        variables[actualMenu]=variables[actualMenu]-alts[actualMenu];

        break;

      }
      case btnSELECT:{
        isAdjustingTL=0;
        break;
      }


    }
  }


  //Here we change the variables according to the changes made, with a proper use of pointers this can be wonderfully simplified
  longitude=variables[0];
  cmPerStep=variables[1];
  maxVel=variables[2];
  maxAcc=variables[3];

}


void setupRunningTL(){
  totalPicsTL=longitude/distIntTL;
  takenPicsTL=0;
  leftPicsTL=totalPicsTL;
  etaTL=0;
  longLeft=longitude;
  runningMoment=0;
  lastTime=millis();

}


void guiRunningTL(){ //Prints the running info

	//prints the ETA
  etaTL=(longLeft/distIntTL)*timeIntTL;
  int etaMin=0;
	lcd.setCursor(0,1);
	lcd.print("ETA: ");

  while (etaTL>60){
    etaTL=etaTL-60;
    etaMin=etaMin+1;

  }
  lcd.print(etaMin);
  lcd.print(" m ");

	lcd.print(etaTL);
	lcd.print(" s ");

  lcd.setCursor(0,0);
  lcd.print(takenPicsTL);
  lcd.print("/");
  lcd.print(totalPicsTL);

  if (longLeft<1){
    isRunningTL=0;
  }



}



void takePic(){
  //Takes one picture in the camera, adds one to takenPics and starts the waiting period

  //takes picture
  digitalWrite(SHUTTERPIN, HIGH);   // turn the LED on (HIGH is the voltage level)

  takenPicsTL++;

  Serial.print("taking pics");
  Serial.println(millis());

  runningMoment=1;
  //lastTime=millis();


}

void wait(){
  //Waits what the timeIntTL says, when the counter reaches the limit, the motors start to turn
  actualTime=millis();
  int deltaTime=actualTime-lastTime;
  int timeIntInS=timeIntTL*1000;
  //int timeIntForThy=timeIntInS-((distIntTL/maxVel)*1000);
  //Serial.println(timeIntForThy);

  //Serial.print("waiting ");
  //Serial.println(millis());

  if (deltaTime>timeIntInS){
    digitalWrite(SHUTTERPIN, LOW);
    runningMoment=2;
    firstMotorMove=1;

  }




}

// Runs the motor according to a chosen direction, speed (rounds per seconds) and the number of steps
void run(boolean runForward, double speedRPS, int stepCount) {
  digitalWrite(DIRPIN, runForward);
  for (int i = 0; i < stepCount; i++) {
    digitalWrite(STEPPIN, HIGH);
    holdHalfCylce(speedRPS);
    digitalWrite(STEPPIN, LOW);
    holdHalfCylce(speedRPS);
  }
}

// A custom delay function used in the run()-method
void holdHalfCylce(double speedRPS) {
  long holdTime_us = (long)(1.0 / (double) 200 / speedRPS / 2.0 * 1E6);
  int overflowCount = holdTime_us / 65535;
  for (int i = 0; i < overflowCount; i++) {
    delayMicroseconds(65535);
  }
  delayMicroseconds((unsigned int) holdTime_us);
}

void motorMove(float speed, float distance){ //This function takes a speed and a distance and moves the motor
  totalTicks=10000/(speed/cmPerStep);
  double RPS=(speed/cmPerStep)/200;
  stepsLeft=distance/cmPerStep;
  Serial.print("rps: ");
  Serial.println(RPS);
  run(false, 6, stepsLeft);
  /*Timer1.initialize(100);
  Timer1.attachInterrupt(timerIsr);

  while (stepsLeft>0){
    Serial.println("Hello");
  }

  Timer1.detachInterrupt();
  */
}

void movementTL(){
  if (firstMotorMove==1){
    lastTime=millis();

    firstMotorMove=0;
  }

  motorMove(maxVel, distIntTL);
  runningMoment=0;
  longLeft=longLeft-distIntTL;

}

/*void motorMove(){
  //Moves the steps indicated by the distIntTL converted to steps

  if (firstMotorMove==1){
    lastTime=millis();

    stepsLeft=distIntTL/cmPerStep;
    totalTicks=10000/(maxVel/cmPerStep);  //To do: think how to go from one speed to the fucking ticking
    Serial.println("totalTicks ");
    Serial.println(totalTicks);
    firstMotorMove=0;
    Timer1.initialize(100);
    Timer1.attachInterrupt(timerIsr);//the period depends on the current speed


  }

  if (stepsLeft<1){
    runningMoment=0;

    Timer1.detachInterrupt();
    longLeft=longLeft-distIntTL;

  }

}*/

void timerIsr() {

  //if(actual_speed == 0) return;

  tickCount++;

  if(tickCount > totalTicks) { //instead of 200 the number of ticks according to the velocity

    // make a step
    digitalWrite(STEPPIN, HIGH);
    delayMicroseconds(800);
    digitalWrite(STEPPIN, LOW);

    delayMicroseconds(800);
    stepsLeft--;
    Serial.print("Timer ");
    Serial.print(millis());


    tickCount = 0;
  }
}


int read_LCD_buttons(){               // read the buttons
    adc_key_in = analogRead(0);       // read the value from the sensor


    if (adc_key_in > 1000) return btnNONE;

   // For V1.0 comment the other threshold and use the one below:

     if (adc_key_in < 50)   return btnRIGHT;
     if (adc_key_in < 195)  return btnUP;
     if (adc_key_in < 380)  return btnDOWN;
     if (adc_key_in < 555)  return btnLEFT;
     if (adc_key_in < 790)  return btnSELECT;


    return btnNONE;                // when all others fail, return this.
}
