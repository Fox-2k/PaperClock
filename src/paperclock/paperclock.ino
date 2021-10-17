// PaperClock
// Copyright (C) 2021  Yoann VERMAUT

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

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

// Arduino setup function
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

  // Button setup
  pinMode(BTN_HOURS_PIN, INPUT_PULLUP);
  pinMode(BTN_MINUTES_PIN, INPUT_PULLUP);
  btnHoursLast = true;
  btnMinutesLast = true;
  
  // First ePaper refresh
  RefreshEnvData();
  RefreshScreen();

}

// Arduino main loop function
void loop() {

  // Wait
  forcedRefresh = PrecisionWaitAndPoll(REFRESH_PERIOD_MINUTES*60000);
  
  if(!forcedRefresh) {
    // update clock
    IncrementClock(REFRESH_PERIOD_MINUTES);
  }
  else {
    // Don't update, as user just set up the clock
    forcedRefresh = false;
  }

  // Get temperature and humidity
  RefreshEnvData();

  // Refresh ePaper screen
  RefreshScreen();

}

// Update the clock value by adding n minute(s)
void IncrementClock(int min) {
  minutes += min;
  if(minutes >= 60) {
    hours += minutes / 60;
    minutes = minutes % 60;
  }
  if(hours >= 24) {
    hours = hours % 24;
  }
}

// Wait during n miliseconds
// but during waiting, poll buttons states
bool PrecisionWaitAndPoll(uint32_t ms) {

  bool needToRefresh = false;

  // Remove screen refresh time from time to wait
  ms -= millis() - last;
  
  // Each 100ms, poll buttons states
  for(uint32_t  i=0; i < ms/100; i++) {
    // Wait 100ms
    delay(100);
    last = millis();

    // Poll hours button state
    int btnHoursState = digitalRead(BTN_HOURS_PIN);
    
    // User just pushed hours button
    if(btnHoursState == LOW && btnHoursLast == HIGH) {
      IncrementClock(60);
      btnHoursLast = btnHoursState;
      digitalWrite(LED_PWR, HIGH);
    }

    // User just released hours button
    if(btnHoursState == HIGH && btnHoursLast == LOW) {
      btnHoursLast = btnHoursState;
      lastBtnRelease = millis();
      needToRefresh = true;
      digitalWrite(LED_PWR, LOW);
    }

    // Poll minutes button state
    int btnMinutesState = digitalRead(BTN_MINUTES_PIN);
    
    // User just pushed minutes button
    if(btnMinutesState == LOW && btnMinutesLast == HIGH) {
      IncrementClock(1);
      btnMinutesLast = btnMinutesState;
      digitalWrite(LED_PWR, HIGH);
    }

    // User just released minutes button
    if(btnMinutesState == HIGH && btnMinutesLast == LOW) {
      btnMinutesLast = btnMinutesState;
      lastBtnRelease = millis();
      needToRefresh = true;
      digitalWrite(LED_PWR, LOW);
    }

    if(last - lastBtnRelease > 5000 && needToRefresh == true) {
      break;
    }
    
  }
  return needToRefresh;
}

// Refresh sensors values (temperature and humidity)
void RefreshEnvData() {
    // Re enable sensors
    digitalWrite(PIN_ENABLE_SENSORS_3V3, HIGH);
    digitalWrite(PIN_ENABLE_I2C_PULLUP, HIGH); 
    delay(10);
    
    // Start HTS sensor
    HTS.begin();
    delay(10);

    // Read sensors values
    temperature = HTS.readTemperature();
    humidity = HTS.readHumidity();

    // Stop sensor
    HTS.end();
    delay(10);

    // Disable sensors
    digitalWrite(PIN_ENABLE_SENSORS_3V3, LOW);
    digitalWrite(PIN_ENABLE_I2C_PULLUP, LOW); 
}

// Refresh the e-ink display
void RefreshScreen() {

  // Clear the display buffers
  screenBlack.Clear(1);
  screenRed.Clear(1);

  // Cast and format clock and sensors values into strings
  char clck[24];
  char meteo[24];
  sprintf(clck, "%2d:%02d", hours, minutes);
  sprintf(meteo, "%.0f\' %.0f%%", temperature, humidity);
  
  // Draw strings values into buffers
  screenBlack.DrawStringAt(100,50,clck, &Font200, 0);
  screenBlack.DrawStringAt(55,340,meteo, &Font160, 0);

  // Wake up display
  epd.Init();

  // Refresh display with new rendered buffers
  epd.Displaypart(bufferBlack,0, 0,880,528, 0);
  epd.Displaypart(bufferRed,0, 0,880,528, 1);

  // Stop display
  epd.Sleep();
}

// Clear the e-ink display (blank white)
void ClearScreen() {
  const unsigned char blank[1] = {0x00};
  epd.Displaypart(blank,0,0,0,0,0);
  epd.Displaypart(blank,0,0,0,0,1);
}
