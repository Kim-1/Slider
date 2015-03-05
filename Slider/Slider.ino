#include <LiquidCrystal.h>

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

//settings variables
int longitude=150; //expressed in cm
int maxVel=1; //expressed in cm/s
int maxAcc=1; //expressed in cm/s*s

//Running variables
int totalPicsTL=0; //The total amount of pictures to take on this run
int takenPicsTL=0; //The amount of already taken pics
int leftPicsTL=0; //Pics left=totalPics-takenPics
int etaTL=0; //ETA to finish expressed in s


//for menu management
int actualMenu=0;


int actualMode=0; //the actual mode: 0 being TL and 1 video
char*modes[]={"Time Lapse", "Video"}; //the names of the modes

int isRunning=0;
int isAdjusting=0;


//variables
int timeIntTL=2; //The interval of time between every picture when doing a TL expressed in seconds

int distIntTL=2; //The interval of space between every picture when doing a TL expressed in cm

//must be changed to mm


void setup()
{

 lcd.begin(16, 2);              // start the library
 lcd.setCursor(0,0);
 lcd.createChar(0, upAndDown);
 lcd.createChar(1,leftArr);
 lcd.createChar(2,rightArr);
 lcd.clear();

}

void loop()
{
  guiPrimarioTL();

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
            distIntTL++;
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
            distIntTL--;
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
            isRunning=1;
            break;
          }

          case 4:{
            isAdjusting=1;
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

void setupRunningTL(){
  totalPicsTL=distIntTL/longitude;


}


void guiRunningTL(){

	//prints the ETA
	lcd.setCursor(0,1);
	lcd.print("ETA: ");
	lcd.print((150/distIntTL)*timeIntTL);
	lcd.print(" seg");


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
