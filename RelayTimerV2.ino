#include <LiquidCrystal_I2C.h>

#include <Wire.h>

// Initialize the LCD, I2C address 0x3F, 16 columns, 2 rows
LiquidCrystal_I2C lcd(0x3F, 16, 2);

// Button pins
const int buttonSecondsUp = 2;
const int buttonSecondsDown = 3;
const int buttonTenthsUp = 4;
const int buttonTenthsDown = 5;
const int startButton = 6;

// Output pins

const int relay1pin = 7;
const int relay2pin = 8;


// Timer variables
int seconds = 0;
int tenths = 0;
int secondsCurrent = 0;
int tenthsCurrent = 0;
int tMax1 = 0;
int tMax2 = 0;
int tMax3 = 0;
int timer = 0;
int tCur = 0;
int tTar = 0;
bool running = false;

// Previous button states for debouncing
bool prevSecondsUp = false, prevSecondsDown = false, prevTenthsUp = false, prevTenthsDown = false, prevStart = false;



void setup() {
  lcd.init();       // Initialize the LCD
  lcd.backlight();  // Turn on the backlight

  // Configure button pins
  pinMode(buttonSecondsUp, INPUT_PULLUP);
  pinMode(buttonSecondsDown, INPUT_PULLUP);
  pinMode(buttonTenthsUp, INPUT_PULLUP);
  pinMode(buttonTenthsDown, INPUT_PULLUP);
  pinMode(startButton, INPUT_PULLUP);
  pinMode(relay1pin, OUTPUT);
  pinMode(relay2pin, OUTPUT);


  digitalWrite(relay1pin, HIGH);
  digitalWrite(relay2pin, HIGH);

  // Display instructions
  lcd.setCursor(0, 0);
  lcd.print(" Set Timer: ");
}



void loop() {
  // Check buttons and update timer values
  handleButtonPress();

  if (timer ==  0) {
    tTar = tMax1;
  } else if (timer == 1) {
    tTar = tMax2;
  } else {
    tTar = tMax3;
  }



  // Start the countdown if the timer is running
  if (running) {

    countdown();
  } else {
  }
}



void updateTCur() {

  lcd.setCursor(8, 1);
  lcd.print("  C:");
  lcd.print(tCur / 10);
  lcd.print("");
}



void updateTMax() {
  lcd.setCursor(1, 0);
  lcd.print("t1:    ");
  lcd.setCursor(4, 0);
  // int tMaxU = tMax1 / 10;
  // int tMaxT = tMax1 - tMaxU * 10;
  lcd.print(tMax1/10);
  // lcd.print(".");
  // lcd.print(tMaxT);
  //  lcd.print("s  ");
  lcd.setCursor(9, 0);
  lcd.print("t2:    ");
  lcd.setCursor(12, 0);
  // tMaxU = tMax2 / 10;
  // tMaxT = tMax2 - tMaxU * 10;
  lcd.print(tMax2/10);
  // lcd.print(".");
  // lcd.print(tMaxT);
  lcd.setCursor(1, 1);
  lcd.print("t3:    ");
  lcd.setCursor(4, 1);
  // tMaxU = tMax3 / 10;
  // tMaxT = tMax3 - tMaxU * 10;
  lcd.print(tMax3/10);
}




void handleButtonPress() {
  // Read button states
  bool secondsUp = !digitalRead(buttonSecondsUp);
  bool secondsDown = !digitalRead(buttonSecondsDown);
  bool tenthsUp = !digitalRead(buttonTenthsUp);
  bool tenthsDown = !digitalRead(buttonTenthsDown);
  bool start = !digitalRead(startButton);

  // Adjust seconds
  if (secondsUp && !prevSecondsUp) {
    if (timer == 0) {
    tMax1 = (tMax1 + 10) % 1000;  // Max 99 seconds
    } else if (timer == 1) {
    tMax2 = (tMax2 + 10) % 1000;      
    } else {
        tMax3 = (tMax3 + 10) % 1000;  
    }
    updateTMax();
  }
  if (secondsDown && !prevSecondsDown) {
    if (timer == 0) {
    tMax1 = (tMax1 > 9) ? tMax1 - 10 : 0;
    } else if (timer == 1) {
     tMax2 = (tMax2 > 9) ? tMax2 - 10 : 0;     
    } else {
      tMax3 = (tMax3 > 9) ? tMax3 - 10 : 0;     
    }
    updateTMax();
  }

  //switch timer
  if (tenthsUp && !prevTenthsUp) {
    timer++;
    if (timer > 2) {
      timer = 0;
    }

    if (timer == 0) {
      lcd.setCursor(0, 0);
      lcd.print("*");
      lcd.setCursor(8, 0);
      lcd.print(" ");
      lcd.setCursor(0, 1);
      lcd.print(" ");
      

    } else if (timer == 1) {
      lcd.setCursor(0, 0);
      lcd.print(" ");
      lcd.setCursor(8, 0);
      lcd.print("*");
      lcd.setCursor(0, 1);
      lcd.print(" ");
    } else {
      lcd.setCursor(0, 0);
      lcd.print(" ");
      lcd.setCursor(8, 0);
      lcd.print(" ");
      lcd.setCursor(0, 1);
      lcd.print("*");
    }
  }

  // Adjust tenths
  // if (tenthsUp && !prevTenthsUp) {
  //   tMax1 = (tMax1 + 1);

  //   updateTMax();
  // }
  // if (tenthsDown && !prevTenthsDown) {
  //   if (timer == 0) {
  //   tMax1 = (tMax1 > 0) ? tMax1 - 1 : 0;
  //   } else if (timer == 1) {
  //   tMax1 = (tMax2 > 0) ? tMax2 - 1 : 0; 
  //   } else {
  //   tMax1 = (tMax3 > 0) ? tMax3 - 1 : 0;  
  //   }

  //   updateTMax();
  // }

  // Start the timer
  if (start && !prevStart && (tTar > 0)) {

    //turn on the relays
    digitalWrite(relay1pin, LOW);
    digitalWrite(relay2pin, LOW);

    tCur = 0;

    //start the timer
    if (running == false) {
      running = true;
    } else {
      running = false;


      tCur = 0;
      digitalWrite(relay1pin, HIGH);
      digitalWrite(relay2pin, HIGH);
      lcd.setCursor(0, 1);
      lcd.print("Timer Stopped");
    }
  }

  // Update previous button states
  prevSecondsUp = secondsUp;
  prevSecondsDown = secondsDown;
  prevTenthsUp = tenthsUp;
  prevTenthsDown = tenthsDown;
  prevStart = start;
}



void countdown() {
  delay(80);  // Wait for 0.1 seconds (1/10 of a second)

  // Increase tenths and adjust seconds if needed
  if (tCur < tTar) {

    tCur++;



    updateTCur();

  } else {
    // Timer ends
    //turn off the relays
    digitalWrite(relay1pin, HIGH);
    digitalWrite(relay2pin, HIGH);

    running = false;

    tCur = 0;
    lcd.setCursor(8, 1);
    lcd.print("Done!");
  }
}
