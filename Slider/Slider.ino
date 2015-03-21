#include <LiquidCrystal.h>
#include <TimerOne.h>
#include <AccelStepper.h>
#include <digitalWriteFast.h>

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

//define pins for the motor, shutter and endstops
//#define STEP_PIN=12;
#define SHUTTERPIN  22
#define STEPPIN     30
#define DIRPIN      31
#define ENDPIN0     40 //This one is closer to the motor
#define ENDPIN1     41  //While this is further away

//Define Directions
#define dirRight HIGH
#define dirLeft LOW

AccelStepper stepper(1,STEPPIN,DIRPIN);

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
byte opt[8] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
};

#define upAndDownChar 0
#define leftArrChar 1
#define rightArrChar  2
#define optChar 3

//settings variables
float longitude=150; //expressed in cm
float cmPerStep=0.016; //The cms each Step moves
//It would be interesting for this to be automatically calibrated using both endstops

float maxVel=10.0; //expressed in cm/s
float maxAcc=3; //expressed in cm/s*s


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
int firstRunVI=0;
int isAdjustingVI=0;


//for menu management
int actualMenu=0;


int actualMode=0; //the actual mode: 0 being TL and 1 video
char*modes[]={"Time Lapse", "Continuo", "Manual"}; //the names of the modes
int numbOfModes=3;




//variables
int timeIntTL=2; //The interval of time between every picture when doing a TL expressed in seconds

float distIntTL=0.2; //The interval of space between every picture when doing a TL expressed in cm

//Variables for the loading screen
int lastTimeLoad=0;
int piece=0;


void setup()
{

 lcd.begin(16, 2);              // start the library
 lcd.setCursor(0,0);
 lcd.createChar(upAndDownChar, upAndDown);  //Create the chars
 lcd.createChar(leftArrChar,leftArr);
 lcd.createChar(rightArrChar,rightArr);
 lcd.createChar(optChar,opt);
 lcd.clear();
 Serial.begin(9600);
 Serial.println("Hello");
 pinMode(SHUTTERPIN, OUTPUT);

 pinMode(STEPPIN, OUTPUT);
 pinMode(DIRPIN, OUTPUT);

 pinMode(ENDPIN0, INPUT);
 pinMode(ENDPIN1, INPUT);

 digitalWrite(STEPPIN, LOW);
 digitalWrite(DIRPIN, LOW);


}

void loop()
{
  if (isRunningTL==1){

    runTL();

  }else if (isAdjustingTL==1){

    guiSettingsTL();

  }else if(isRunningVI==1){
    runVI();

  }else if (isAdjustingVI==1){
    guiSettingsVI();
  }

  else if (actualMode==1){

    guiPrimarioVI();

  }else if (actualMode==0){

    guiPrimarioTL();

  }else if(actualMode==2){
    guiPrimarioMN();

  }else if(isRunningVI==1){
    runVI();
  }



}



void runTL(){
  setupRunningTL();

  while (longLeft>0){

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

  isRunningTL=0;
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

  lcd.print(variables[actualMenu], 3);
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

void guiPrimarioTL(){ //In here we display and manage settings
  char* menus[]={"Modo", "T entre fotos", "D entre fotos",  "Empezar", "Ajustes", "Calibrar"}; //the different menus for this GUI
  float variables[]={actualMode, timeIntTL, distIntTL, isRunningTL, isAdjustingTL, 0}; //Values to be changed
  char *units[]={"", "s", "cm", "", "", ""}; //Units
  float alts[]={1,1,0.1,0,0,0};
  float altsSel[]={0,0,0,1,1,1};

  lastKey=lcd_key; //so there is not a push per cycle
  lcd_key = read_LCD_buttons();   // read the buttons

  lcd.setCursor(0,0);
  lcd.write(byte(leftArrChar));

  lcd.setCursor(15,0);
  lcd.write(byte(rightArrChar));


  lcd.setCursor(1,0);

  if (actualMenu>5){ //So it returns as a cycle
    actualMenu=0;
  }
  if (actualMenu<0){
    actualMenu=5;
  }



  lcd.print(menus[actualMenu]); //prints the actual menu

  //This four next lines and the variables[] array replace the switch mess I had on the original version, to be tested.
  lcd.setCursor(0,1);

  switch(actualMenu){
    case 0:{
      lcd.print(modes[actualMode]);
      lcd.setCursor(14,1);
      lcd.write(byte(upAndDownChar));
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
    case 5:{
      lcd.print("Sel para calibrar");
      break;
    }
    default:{

      lcd.print(variables[actualMenu]);
      lcd.setCursor(10,1);
      lcd.print(units[actualMenu]);
      lcd.setCursor(14,1);
      lcd.write(byte(upAndDownChar));

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

        variables[actualMenu]=variables[actualMenu]+alts[actualMenu];

        break;

      }

      case btnDOWN:{

        variables[actualMenu]=variables[actualMenu]-alts[actualMenu];

        break;

      }
      case btnSELECT:{
        variables[actualMenu]=variables[actualMenu]+altsSel[actualMenu];
        break;
      }


    }
  }


  //Let's correct the mode
  if (variables[0]==numbOfModes){
    variables[0]=0;
  }else if (variables[0]==-1){
    variables[0]=numbOfModes-1;
  }

  //Let's call calibrate
  if (variables[5]!=0){
    calibrate();
    variables[5]=0;
  }

  //Here we change the variables according to the changes made, with a proper use of pointers this can be wonderfully simplified
  actualMode=variables[0];
  timeIntTL=variables[1];
  distIntTL=variables[2];
  isRunningTL=variables[3];
  isAdjustingTL=variables[4];

}


void guiPrimarioVI(){ //In here we display and manage settings
  char* menus[]={"Modo", "Vel Max", "Acc (0=s/Acc)",  "Empezar", "Ajustes", "Calibrar"}; //the different menus for this GUI
  float variables[]={actualMode, maxVel, maxAcc, isRunningVI, isAdjustingVI, 0}; //Values to be changed
  char *units[]={"", "cm/s", "cm/s2", "", "", ""}; //Units
  float alts[]={1,0.1,0.1,0,0,0};
  float altsSel[]={0,0,0,1,1,1};

  lastKey=lcd_key; //so there is not a push per cycle
  lcd_key = read_LCD_buttons();   // read the buttons

  lcd.setCursor(0,0);
  lcd.write(byte(leftArrChar));

  lcd.setCursor(15,0);
  lcd.write(byte(rightArrChar));


  lcd.setCursor(1,0);

  if (actualMenu>5){ //So it returns as a cycle
    actualMenu=0;
  }
  if (actualMenu<0){
    actualMenu=5;
  }



  lcd.print(menus[actualMenu]); //prints the actual menu

  //This four next lines and the variables[] array replace the switch mess I had on the original version, to be tested.
  lcd.setCursor(0,1);

  switch(actualMenu){
    case 0:{
      lcd.print(modes[actualMode]);
      lcd.setCursor(14,1);
      lcd.write(byte(upAndDownChar));
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
    case 5:{
      lcd.print("Sel para calibrar");
      break;
    }
    default:{

      lcd.print(variables[actualMenu]);
      lcd.setCursor(10,1);
      lcd.print(units[actualMenu]);
      lcd.setCursor(14,1);
      lcd.write(byte(upAndDownChar));

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

        variables[actualMenu]=variables[actualMenu]+alts[actualMenu];

        break;

      }

      case btnDOWN:{

        variables[actualMenu]=variables[actualMenu]-alts[actualMenu];

        break;

      }
      case btnSELECT:{
        variables[actualMenu]=variables[actualMenu]+altsSel[actualMenu];
        break;
      }


    }
  }


  //Let's correct the mode
  if (variables[0]==numbOfModes){
    variables[0]=0;
  }else if (variables[0]==-1){
    variables[0]=numbOfModes-1;
  }

  //Let's call calibrate
  if (variables[5]!=0){
    calibrate();
    variables[5]=0;
  }

  //Here we change the variables according to the changes made, with a proper use of pointers this can be wonderfully simplified
  actualMode=variables[0];
  maxVel=variables[1];
  maxAcc=variables[2];
  isRunningVI=variables[3];
  isAdjustingVI=variables[4];

}

void guiPrimarioMN(){ //In here we display and manage settings
  char* menus[]={"Modo", "Vel", "Acc (0=s/Acc)",  "Manejar", "Ajustes", "Calibrar"}; //the different menus for this GUI
  float variables[]={actualMode, maxVel, maxAcc, 0, isAdjustingVI, 0}; //Values to be changed
  char *units[]={"", "cm/s", "cm/s2", "", "", ""}; //Units
  float alts[]={1,0.1,0.1,0,0,0};
  float altsSel[]={0,0,0,1,1,1};

  lastKey=lcd_key; //so there is not a push per cycle
  lcd_key = read_LCD_buttons();   // read the buttons

  lcd.setCursor(0,0);
  lcd.write(byte(leftArrChar));

  lcd.setCursor(15,0);
  lcd.write(byte(rightArrChar));


  lcd.setCursor(1,0);

  if (actualMenu>5){ //So it returns as a cycle
    actualMenu=0;
  }
  if (actualMenu<0){
    actualMenu=5;
  }



  lcd.print(menus[actualMenu]); //prints the actual menu

  //This four next lines and the variables[] array replace the switch mess I had on the original version, to be tested.
  lcd.setCursor(0,1);

  switch(actualMenu){
    case 0:{
      lcd.print(modes[actualMode]);
      lcd.setCursor(14,1);
      lcd.write(byte(upAndDownChar));
      break;
    }
    case 3:{
      lcd.print("Sel para manejar");
      break;
    }
    case 4:{
      lcd.print("Sel para ajustes");
      break;
    }
    case 5:{
      lcd.print("Sel para calibrar");
      break;
    }
    default:{

      lcd.print(variables[actualMenu]);
      lcd.setCursor(10,1);
      lcd.print(units[actualMenu]);
      lcd.setCursor(14,1);
      lcd.write(byte(upAndDownChar));

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

        variables[actualMenu]=variables[actualMenu]+alts[actualMenu];

        break;

      }

      case btnDOWN:{

        variables[actualMenu]=variables[actualMenu]-alts[actualMenu];

        break;

      }
      case btnSELECT:{
        variables[actualMenu]=variables[actualMenu]+altsSel[actualMenu];
        break;
      }


    }
  }


  //Let's correct the mode
  if (variables[0]==numbOfModes){
    variables[0]=0;
  }else if (variables[0]==-1){
    variables[0]=numbOfModes-1;
  }

  //Let's call calibrate
  if (variables[5]!=0){
    calibrate();
    variables[5]=0;
  }

  if (variables[3]!=0){
    runMN();//ToDo
    variables[3]=0;
  }

  //Here we change the variables according to the changes made, with a proper use of pointers this can be wonderfully simplified
  actualMode=variables[0];
  maxVel=variables[1];
  maxAcc=variables[2];
  isAdjustingVI=variables[4]; //ToDo

}

void runMN(){


  float speed=maxVel/cmPerStep;
  float target=longitude/cmPerStep;
  float maxAccCopy=maxAcc;

  int ended=0;

  if (maxAccCopy<0){
    maxAccCopy=maxAccCopy*-1;
    target=target*-1;
  }

  stepper.move(target);
  stepper.setMaxSpeed(speed);
  stepper.setSpeed(speed);

  float accInSteps=maxAccCopy/cmPerStep;
  stepper.setAcceleration(accInSteps);




  lcd.print("<---    --->");

  while (ended==0){

    lastKey=lcd_key; //so there is not a push per cycle
    lcd_key = read_LCD_buttons();   // read the buttons

    switch (lcd_key){ //an action per button pushed

      case btnRIGHT:{
        if (maxAccCopy<0.05){
          stepper.runSpeed();

        }else{
          stepper.run();

        }
        break;
      }

      case btnLEFT:{
        stepper.setSpeed(-speed);

        if (maxAccCopy<0.05){
          stepper.runSpeed();

        }else{
          stepper.run();

        }
        break;
      }

      case btnSELECT:{
        ended=1;
        break;
      }

      default:{
        stepper.stop();
        break;
      }


    }

  }


}

void guiSettingsVI(){ //In here we display and manage settings
  char* menus[]={"Longitud", "Cm por step"}; //the different menus for this GUI
  float variables[]={longitude, cmPerStep}; //Values to be changed
  char *units[]={"cms", "cms"}; //Units
  float alts[]={0.1,0.001};

  lastKey=lcd_key; //so there is not a push per cycle
  lcd_key = read_LCD_buttons();   // read the buttons

  lcd.setCursor(0,0);
  lcd.write(byte(leftArrChar));

  lcd.setCursor(15,0);
  lcd.write(byte(rightArrChar));


  lcd.setCursor(1,0);

  if (actualMenu>1){ //So it returns as a cycle
    actualMenu=0;
  }
  if (actualMenu<0){
    actualMenu=1;
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

}

void runVI(){


  float speed=maxVel/cmPerStep;
  float target=longitude/cmPerStep;
  float maxAccCopy=maxAcc;


  if (maxAccCopy<0){
    maxAccCopy=maxAccCopy*-1;
    target=target*-1;
  }

  stepper.moveTo(target);
  stepper.setMaxSpeed(speed);
  stepper.setSpeed(speed);


  //bool left=true;

  if (maxAccCopy<0.05){
    while (stepper.distanceToGo()!=0 && (digitalReadFast(ENDPIN0)==HIGH && digitalReadFast(ENDPIN1)==HIGH)){
      stepper.runSpeed();
    }
  }else{
    float accInSteps=maxAccCopy/cmPerStep;
    stepper.setAcceleration(accInSteps);
    //stepper.runToPosition();
    lcd.print(maxAccCopy,6);

    while (stepper.distanceToGo()!=0 && (digitalReadFast(ENDPIN0)==HIGH && digitalReadFast(ENDPIN1)==HIGH)){
      stepper.run();
      //loading("Grabando Video..");

    }
  }
  isRunningVI=0;


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


void calibrate(){
  //Move to one side, position =0, start moving and counting steps (no Acc, regular speed),
  //reach endstop, comeback, compare results, reasonable error, new cmPerStep, longitude, and zeros

  float speed=maxVel/cmPerStep;
  float target=longitude/cmPerStep;
  float maxAccCopy=maxAcc;

  int counter1=0;
  int counter2=0;

  stepper.move(-target*2);
  stepper.setSpeed(speed);

  lcd.print("Calibrando... 1/3");

  while (digitalReadFast(ENDPIN0)==HIGH){
    stepper.runSpeed();
  }

  stepper.setCurrentPosition(0);

  stepper.move(target*2);
  stepper.setSpeed(speed/2);

  lcd.print("Calibrando... 2/3");

  while (digitalReadFast(ENDPIN1)==HIGH){
    stepper.runSpeed();
  }

  counter1=stepper.currentPosition();

  stepper.move(-target*2);
  stepper.setSpeed(speed/2);

  lcd.print("Calibrando... 3/3");

  while (digitalReadFast(ENDPIN0)==HIGH){
    stepper.runSpeed();
  }

  counter2=stepper.currentPosition();

  //Now the math and the error

  //new cmPerStep
  float tempCmPerStep=longitude/counter1;

  float error=abs(cmPerStep-tempCmPerStep)/cmPerStep;

  if (error<0.01){
    cmPerStep=tempCmPerStep;

  }

}

void loading(char myText[]){ //Not Being used but maybe is better if we implement a timer

  char pieces[]={'|','/','-'};

  lcd.setCursor(0,0);
  lcd.print(myText);
  lcd.setCursor(0,1);

  switch (piece){
    case 3:{
      lcd.print("\\");
      break;
    }
    default:{
      lcd.print(pieces[piece]);
      break;
    }
  }




  if (lastTimeLoad<(millis()-300)){
    piece++;

    if (piece==4){
      piece=0;
    }

    lastTimeLoad=millis();
  }

}

bool endstop(int endNumber){


  switch (endNumber){
    case 0:{
      return digitalRead(ENDPIN0);
      break;
    }
    case 1:{
      return digitalRead(ENDPIN1);
      break;
    }
  }
  return 0;
}


void guiRunningTL(){ //Prints the running info

	//prints the ETA
  etaTL=(longLeft/distIntTL)*timeIntTL;
  int etaMin=0;
	lcd.setCursor(0,1);
	lcd.print("ETA: ");

  while (etaTL>=60){
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

}

void recommendCal(){
  bool buttonPressed=false;
  bool calibrateIndeed=false;


  while (!buttonPressed){
    lastKey=lcd_key; //so there is not a push per cycle
    lcd_key = read_LCD_buttons();   // read the buttons

    lcd.setCursor(0,0);
    lcd.print("Calibrar?");
    lcd.setCursor(0,1);
    lcd.write(byte(optChar));
    lcd.print("Si");
    lcd.setCursor(5,1);
    lcd.write(byte(optChar));
    lcd.print("No");

    if (lastKey!=lcd_key){ //so there is an action per push

      lcd.clear();


      switch (lcd_key){ //an action per button pushed

        case btnRIGHT:{
          calibrateIndeed=false;
          buttonPressed=true;
          break;
        }

        case btnLEFT:{
          calibrateIndeed=true;
          buttonPressed=true;
          break;
        }

        default:{
          break;
        }


      }
    }

  }

  if (calibrateIndeed){
    calibrate();
  }

}

void takePic(){
  //Takes one picture in the camera, adds one to takenPics and starts the waiting period

  //takes picture
  digitalWriteFast(SHUTTERPIN, HIGH);   // turn the LED on (HIGH is the voltage level)

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

  Serial.print("waiting ");
  Serial.println(millis());

  if (deltaTime>timeIntInS){
    digitalWriteFast(SHUTTERPIN, LOW);
    runningMoment=2;
    firstMotorMove=1;

  }




}

void movementTL(){

  lastTime=millis();

  float speed=maxVel/cmPerStep;
  float target=distIntTL/cmPerStep;
  float accInSteps=maxAcc/cmPerStep;

  stepper.move(target);
  stepper.setMaxSpeed(speed);
  stepper.setSpeed(speed);
  stepper.setAcceleration(accInSteps);


  while (stepper.distanceToGo()!=0 && (digitalReadFast(ENDPIN1)==HIGH && digitalReadFast(ENDPIN0)==HIGH)){
    stepper.run();
  }

  if (digitalReadFast(ENDPIN0)==LOW || digitalReadFast(ENDPIN1)==LOW){ //Posible error, no probado
    longLeft=0;
    recommendCal();
    //run the calibration recommendation
  }


  runningMoment=0;
  longLeft=longLeft-distIntTL;
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
