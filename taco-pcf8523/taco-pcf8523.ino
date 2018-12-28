
#include <Time.h>
#include <TimeLib.h>

#include "taco.h"

#include <Wire.h>
#include <RTClib.h>
RTC_PCF8523 rtc;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};


void PrintHex8(uint8_t *data, uint8_t length) // prints 8-bit data in hex with leading zeroes
{
     char tmp[16];
       for (int i=0; i<length; i++) { 
         sprintf(tmp, "0x%.2X",data[i]); 
         Serial.print(tmp); Serial.print(" ");
       }
}

void setup () {

    uint8_t alarm[4];

    while (!Serial)
        ; // for Leonardo/Micro/Zero
    Serial.begin(115200);
      if (! rtc.begin()) {
          Serial.println("Couldn't find RTC");
          while (1);
    }

    if (! rtc.initialized()) {
        Serial.println("RTC was NOT running! Setting time...");
        // following line sets the RTC to the date & time this sketch was compiled
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        // This line sets the RTC with an explicit date & time, for example to set
        // January 21, 2014 at 3am you would call:
        // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    }
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    rtc.writeSqwPinMode(PCF8523_OFF);
    Serial.println("RTC clockout reg = " + String(rtc.readSqwPinMode()));


    DateTime future (rtc.now() + TimeSpan(0, 0, 2, 0));
    Serial.print("Setting alarm to ");
    Serial.print(future.hour(), DEC);
    Serial.print(':');
    Serial.println(future.minute(), DEC);
    rtc.setAlarm(future.hour(), future.minute());

    rtc.getAlarm(alarm);
    Serial.println("Get Alarm = ");
    Serial.print(alarm[2], DEC);
    Serial.print(",");
    Serial.print(alarm[1], DEC);
    Serial.print(":");
    Serial.print(alarm[0], DEC);
    Serial.println();
    rtc.clear_rtc_interrupt_flags();
    rtc.enableAlarm(true);
}

void loop () {
    DateTime now = rtc.now();
    
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
    
    Serial.print(" since midnight 1/1/1970 = ");
    Serial.print(now.unixtime());
    Serial.print("s = ");
    Serial.print(now.unixtime() / 86400L);
    Serial.println("d");
    
#if 0
    // calculate a date which is 7 days and 30 seconds into the future
    DateTime future (now + TimeSpan(7,12,30,6));
#endif

    Serial.println();
    delay(10000);
}
