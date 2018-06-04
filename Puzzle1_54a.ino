//Puzzle nr1 v.1.54, Martin Agnar Dahl, MAD Robot
//1.1 Second button added
//1.2 Timer added
//1.3 MP3 added
//1.4 Keypad, 2 diods and lock added
//1.5 LCD Display added
//1.51 Pause function added to key A
//1.52 Smal corrections and second MP3 track added
//1.53 Software serial to MP3
//1.54 Changed buttons behavior

#include <DFRobotDFPlayerMini.h>
#include <Keypad.h>
#include <A6seglcd.h>
#include <SoftwareSerial.h>

SoftwareSerial softSerial(8, 9); //RX, TX

unsigned long startTimer;
unsigned long startCountDown;
unsigned long currentTime;
unsigned long newCurrentTime;

//Constants sec, min, hour
const unsigned long oneSec = 1000;
const unsigned long oneMin = 60000;
//const unsigned long oneHour = 3600000;

//Start values of timer
//int newLocalHour = 1; // Just 1 hour, no need to define. Can uses if more hours 
int newLocalMin = 59;
int newLocalSec = 59;
int secPassedCounter = 0;

//button status
int counterA = 0; //0 not pressed, 1 pressed once, 2 pressed twice
int counterB = 0; //0 not pressed, 1 pressed once, 2 pressed twice
int buttonA; //current button status
int buttonB; //current button status

//MP3 status
int mp3play = 0; //1 plays
int mp3pause = 0; //1 pauses

///Mp3
DFRobotDFPlayerMini mp3;

//LCD
a6seglcd lcd;
String timePassed; //String for display

//Keypad

//define keypad size
const byte ROWS = 4; 
const byte COLS = 3; 

int const pass_len = 7; // Lenght of the passwort + 1
char pass[pass_len] = "150490"; // Password here
char pass_input[pass_len];
int pass_counter = 0;

char key;

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3'},  //28
  {'4', '5', '6'},  //23
  {'7', '8', '9'},  //24
  {'*', '0', '#'}}; //26
  //27  29   25

byte rowPins[ROWS] = {28, 23, 24, 26};
byte colPins[COLS] = {27, 29, 25};


Keypad myKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

//Lock
const int lock = 30;  //pin 30 for relay controlling lock

//LED pins
const int redLed = 40;
const int greenLed = 42;

//program status
boolean running = true;

void setup() {
  Serial.begin(9600); // Start serial monitor
  //Serial.println("Serial starts");

  pinMode (5, INPUT_PULLUP); //Signal from receiver, button A
  pinMode (6, INPUT_PULLUP); //Signal from receiver, button B

  //Just to be sure startvalues are correct on setup
  digitalWrite (5, HIGH); // Startvalue = 1
  digitalWrite (6, HIGH); // Startvalue = 1
  
  softSerial.begin(9600); // MP3 communication via Serial 1 (Pins 9+8)
//  while(softSerial){
//     Serial.println("Software serial1 starts"); 
//     break;
//  }

  //Serial.println("Mp3 setup starts");
  mp3.begin(softSerial);
  mp3.outputDevice(DFPLAYER_DEVICE_SD);
  mp3.volume (28); // Between 0-30
  //Serial.println("MP3 setup done");


  //Lock setup
  //Serial.println("Lock setup");
  pinMode(lock, OUTPUT);
  digitalWrite(lock, LOW); //lock is now blocked
  //Serial.println("Lock setup done");

  //LEDs
  pinMode (redLed, OUTPUT);
  pinMode (greenLed, OUTPUT);
  digitalWrite (redLed, LOW);
  digitalWrite (greenLed, LOW);

  //LCD Setup
  //Serial.println("LCD Setup starts");
  lcd.run(53,52,51,50); //LCD Pins: CS, WR, Data, LED+ to Arduino pins 53, 52, 51 ,50
  lcd.backlighton(); //backlight
  lcd.conf();
  lcd.clr(); //clear
  //Serial.println("LCD Setup done");
  
  startTimer = millis();
  
  delay(300);
  //Serial.println("\nArduino setup done\n");

  Serial.print("Welcome! Press A button to start...");
}

void loop() {


while(running){

buttonA = digitalRead(5);
buttonB = digitalRead(6);

  delay(20); //button status changes

  //Button A
  //button A is pressed (on)
  if (buttonA==0 && counterA == 0 && counterB== 0){ 
    //Serial.println("\nGame starts!"); 
    counterA = 1; //A pressed once
    startCountDown = millis();
    
    //mp3 starts playing
    if (mp3play==0){
      //Serial.println("Mp3 Starts");
      startMp3();
      mp3play=1;
    }
  }
  
  //sceond press without B pressed before - NOTHING
  if (buttonA ==1 && counterA==1 && counterB == 0){
    timer();
    keyPad();

    if(mp3pause==1){
      //Serial.println("Mp3 resumes");
      mp3.start();
      mp3pause=0;
      mp3play=1;
      delay(20);
    }
  }
  
//==========================//
  //Button B
  //first press - PAUSE
  if (buttonB==0 && counterB == 0 &&  counterA == 1){

      counterB = 1; //B pressed once

    //mp3 pause
    if (mp3play==1){
      //Serial.println("Mp3 pauses");
      pauseMp3();
      mp3pause=1;
      mp3play=2;
      delay(20);
    }
  }
  
  //second press - Game continues
  if (buttonB == 1 && counterB == 1){
    counterB = 0;
  }

  //A is pressed and the game is running - MAIN loop
  if (buttonA == 0 && counterA == 1 && counterB == 0){
    timer();
    keyPad();

    //if MP3 is paused
    if(mp3pause==1){
      //Serial.println("Mp3 resumes");
      mp3.start();
      mp3pause=0;
      mp3play=1;
      delay(20);
    }
  }

}
}

//Timer
void timer(){
  
digitalWrite(greenLed, HIGH);

  currentTime = millis();
  if ( ((currentTime-newCurrentTime)) >= oneSec){
    
//    int hr = (oneHour-currentTime)/oneHour;

    //for timer debug
//    Serial.print("\nTime left: ");
//    Serial.print(hr); //hour counter is set to max 1 h
//    Serial.print("h ");
//    Serial.print(newLocalMin);
//    Serial.print("min ");
    
    //Second counter
    if (newLocalSec >= 0){
//      Serial.print(newLocalSec); 
//      Serial.println("sec.");

      //Create value for LCD
      //    timePassed = '0';
      //    timePassed +=  hr;
      timePassed = newLocalMin;
      timePassed += '.';
      
      String secToString = (String)newLocalSec;
      if (newLocalSec < 10){
        secToString = '0';
        secToString += newLocalSec;
      }
      
      timePassed += secToString;
      toLCD(timePassed);
            
      newLocalSec= newLocalSec -1;
      
      if (newLocalSec <0) {
        newLocalSec=59; 
        newLocalMin=newLocalMin-1;
      }
    
      secPassedCounter++;
    }

    newCurrentTime = currentTime;

    //Check if time passed
    if (newLocalMin<=0 && newLocalSec<=0){
      Serial.println("");
      Serial.println("TIME PASSED! GAME OVER!");
      running=false;
    }    
  }
}

//Resets all values neded for timer
void resetValues(){
  newLocalSec=59; 
  newLocalMin=59;
//  newLocalHour=1;
  secPassedCounter=0;
  
  int mp3play = 0;
  int mp3pause = 0;
  int mp3stopp = 0;
  clearPassValues();
}

//Mp3 starts playing
void startMp3(){
  mp3.play(1);
  
}

//Mp3 is paused
void pauseMp3(){
  mp3.pause();
}

void keyPad() {
  key = myKeypad.getKey();
  
  if (key){
    pass_input[pass_counter]=key;
    pass_counter++;
    digitalWrite(greenLed, LOW);
    delay(100);
  }

  if (pass_counter == pass_len-1){
    //Serial.println("\nPassword received");
    if (strcmp(pass_input, pass)==0){
      //Serial.println("\nPassword correct! Dor opens...\n");
      gameEndsWin();
    }
    else{
      Serial.println("\nWrong password!\n");
      digitalWrite(redLed, HIGH);
      delay(300);
      digitalWrite(redLed, LOW);
      delay(300);
      digitalWrite(redLed, HIGH);
      delay(300);
      digitalWrite(redLed, LOW);
      delay(300);      
      digitalWrite(redLed, HIGH);
      delay(300);
      digitalWrite(redLed, LOW);
      delay(300);
      clearPassValues();
    }
  }

}

//Resets input password
void clearPassValues(){
  pass_counter=0;
  char pass_input[pass_len];
}

void gameEndsWin(){
  mp3.play(3);
  digitalWrite(lock, HIGH); // lock opens
  Serial.println("\nCONGRATULATIONS! You won!");
  digitalWrite(greenLed, LOW);
  delay(300);
  digitalWrite(greenLed, HIGH);
  delay(300);
  digitalWrite(greenLed, LOW);
  delay(300);
  digitalWrite(greenLed, HIGH);
  delay(300);
  digitalWrite(greenLed, LOW);
  delay(300);  
  digitalWrite(greenLed, HIGH);
  delay(300);
  digitalWrite(greenLed, LOW);
  
  running=false;
  
}

void resetAllValues(){
  resetValues();
  counterA = 0;
  counterB = 0;
  digitalWrite(lock, LOW); //lock closes
  Serial.print("\nWelcome! Press A button to start...");
}

void toLCD(String timePassed){
  float newTime = timePassed.toFloat();
  //Serial.println(newTime);
  lcd.dispnum(newTime);
}

