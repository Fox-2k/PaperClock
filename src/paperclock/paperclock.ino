#include <SPI.h>
#include <Arduino_HTS221.h>
#include "epd7in5b_HD.h"

#include "epdpaint.h"
#include "fonts.h"

#define REFRESH_PERIOD_MINUTES 5

// ePaper screen
unsigned char bufferBlack[58080];
unsigned char bufferRed[58080];
Paint screenBlack(bufferBlack, 880, 528);
Paint screenRed(bufferRed, 880, 528);
Epd epd;

// Environement datas
float temperature;
float humidity;

// Clock datas
int hours = 0;
int minutes = 0;
static uint32_t last, lastBtnRelease;

// Buttons datas
#define BTN_HOURS_PIN 4
#define BTN_MINUTES_PIN 5
int btnHoursLast;
int btnMinutesLast;
bool forcedRefresh = false;

void setup() {

  // "ON" LED turned off
  digitalWrite(LED_PWR, LOW); 

  // PIN_ENABLE_I2C_PULLUP set to LOW
  // NO : temp & hum. sensor need this
  digitalWrite(PIN_ENABLE_I2C_PULLUP, LOW); 

  //PIN_ENABLE_SENSORS_3V3 set to LOW
  digitalWrite(PIN_ENABLE_SENSORS_3V3, LOW);
  
  // Clock setup
  delay(100);
  last = millis();

  // Meteo data
  //HTS.begin();

  // Button setup
  pinMode(BTN_HOURS_PIN, INPUT_PULLUP);
  pinMode(BTN_MINUTES_PIN, INPUT_PULLUP);
  btnHoursLast = true;
  btnMinutesLast = true;
  
  
  // First ePaper refresh
  RefreshEnvData();
  RefreshScreen();

  // ClearScreen();
}

void loop() {

  // Wait
  PrecisionWaitAndPoll(REFRESH_PERIOD_MINUTES*60000);
  
  // update clock
  IncrementClock(REFRESH_PERIOD_MINUTES);

  // Get temperature and humidity
  RefreshEnvData();

  // Refresh ePaper screen
  RefreshScreen();

}

void IncrementClock(int min) {
  if(forcedRefresh) {
    forcedRefresh = false;
  }
  else {
    minutes += min;
    if(minutes >= 60) {
      hours++;
      minutes = minutes % 60;
    }
    if(hours >= 24) {
      hours = 0;
    }
  }
}

void PrecisionWaitAndPoll(uint32_t ms) {

  // Remove screen refresh time from time to wait
  ms -= millis() - last;
  
  for(uint32_t  i=0; i < ms/100; i++) {
    // Wait 100ms
    delay(100);
    last = millis();
    
    //while((now-last) < 100) {
    //  now = millis();
    //}
    //last = now;  

    // Manage buttons polling
    int btnHoursState = digitalRead(BTN_HOURS_PIN);
    
    // User just pushed hours button
    if(btnHoursState == LOW && btnHoursLast == HIGH) {
      hours++;
      btnHoursLast = btnHoursState;
      digitalWrite(LED_PWR, HIGH);
    }

    // User just released hours button
    if(btnHoursState == HIGH && btnHoursLast == LOW) {
      btnHoursLast = btnHoursState;
      lastBtnRelease = millis();
      forcedRefresh = true;
      digitalWrite(LED_PWR, LOW);
    }

    int btnMinutesState = digitalRead(BTN_MINUTES_PIN);
    
    // User just pushed minutes button
    if(btnMinutesState == LOW && btnMinutesLast == HIGH) {
      minutes++;
      btnMinutesLast = btnMinutesState;
      digitalWrite(LED_PWR, HIGH);
    }

    // User just released minutes button
    if(btnMinutesState == HIGH && btnMinutesLast == LOW) {
      btnMinutesLast = btnMinutesState;
      lastBtnRelease = millis();
      forcedRefresh = true;
      digitalWrite(LED_PWR, LOW);
    }

    if(last - lastBtnRelease > 5000 && forcedRefresh == true) {
      break;
    }
    
  }
  
}

void ManageButtons() {
  
}

void RefreshEnvData() {
    digitalWrite(PIN_ENABLE_SENSORS_3V3, HIGH);
    digitalWrite(PIN_ENABLE_I2C_PULLUP, HIGH); 
    delay(10);
    HTS.begin();
    delay(10);
    temperature = HTS.readTemperature();
    humidity = HTS.readHumidity();
    HTS.end();
    delay(10);
    digitalWrite(PIN_ENABLE_SENSORS_3V3, LOW);
    digitalWrite(PIN_ENABLE_I2C_PULLUP, LOW); 
}

void RefreshScreen() {
  screenBlack.Clear(1);
  screenRed.Clear(1);

  char clck[24];
  char meteo[24];
  sprintf(clck, "%2d:%02d", hours, minutes);
  sprintf(meteo, "%.0f\' %.0f%%", temperature, humidity);
  
  screenBlack.DrawStringAt(100,50,clck, &Font200, 0);
  screenBlack.DrawStringAt(55,340,meteo, &Font160, 0);

  epd.Init();
  epd.Displaypart(bufferBlack,0, 0,880,528, 0);
  epd.Displaypart(bufferRed,0, 0,880,528, 1);

  epd.Sleep();
}

void ClearScreen() {
  const unsigned char blank[1] = {0x00};
  epd.Displaypart(blank,0,0,0,0,0);
  epd.Displaypart(blank,0,0,0,0,1);
}
