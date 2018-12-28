


/*
	taco-pcf8523-settime

    Program to try setting PCF8523 clock from HTTP webpage using XBee3 NB-Iot

	Author: Adnan Jalaludin <adnan.jalaludin@gmail.com>
*/

#include <Time.h>
#include <TimeLib.h>
#include <RTClib.h>
#include <StringSplitter.h>
#include "taco.h"

#define console Serial
HardwareSerial rs485(1);
HardwareSerial xbeeUart(2);
bool haveConsole = false;

RTC_PCF8523 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

char hostURL[] = "google.com";

//XBee AT commands
char cmdMode[] = "+++";     // Enter command mode
char cmdAssoc[] = "ATAI";   // Association Indication
char cmdExit[] = "ATCN";    // Exit command
char cmdApply[] = "ATAC";   // Apply change command
char cmdWrite[] = "ATWR";   // Write to flash
char cmdAirplane[] = "ATAM1";       // Enable airplane mode
char cmdAirplaneOff[] = "ATAM0";    // Disable airplane mode

char rspOK[] = "OK"; // Command mode response
char rspCONN[] = "0"; // 0-Connected, 2A-Airplane, 22-Registering, 23-Connecting to Internet, 25-Denied
char rspAirplane[] = "2A"; // 0-Connected, 2A-Airplane, 22-Registering, 23-Connecting to Internet, 25-Denied

typedef struct {
    char *cmd;
    char *param;
    char *description;
} CMDLINE;

CMDLINE xbeeSetups[] = {
    { "ATDO", "8", "LTE powersaving mode" },
    { "ATPD", "7FFF", "pull direction on DIO pins" },
    { "ATPR", "7FFF", "pull up/down on DIO pins" },
    { "ATD8", "1", "DIO8 sleep-pin enable" },
    { "ATSM", "1", "sleep-on-pin config" },
    { "ATAP", "0", "API mode to disabled" },
    { "ATIP", "1", "IP protocol to 1" },
    { "ATAM", "0", "airplane mode to disabled" },
    { "ATTD", "0", "text-delimiter to 0" },
    { "ATBM", "80080", "LTE-M1 bandmasks" },
    { "ATBN", "80080", "NB-IoT bandmasks" },
    { "ATAN", "stmiot", "network APN" },
    { "ATDL", hostURL, "destination URL" },
    { "ATDE", "50", "dest port to 80" },
    { 0, 0, 0 },
};

void xbeeColdSetup()
{   
    int i = 0;
    char outbuf[20];
    console.println("XBee module configuration...");
    while (!SendCMCommand(cmdMode, rspOK));  // AT command mode

    while (xbeeSetups[i].cmd != 0) {
        console.println(xbeeSetups[i].description);
        while (SendATCommand(xbeeSetups[i].cmd, xbeeSetups[i].param) == 0) {
            console.println("FAIL!");
            strcpy(outbuf, xbeeSetups[i].cmd);
            strcat(outbuf, xbeeSetups[i].param);
            SendATCommand(outbuf);
        }
        console.println("PASSED.");     
        i++;
    }
    SendATCommand(cmdApply);
    SendATCommand(cmdExit);                 //Exit command mode
    console.println("XBee module set up!");
}

void setup () {
    ioColdSetup();

    if (console)
        haveConsole = true;
    console.begin(115200);
    rs485.begin(9600, SERIAL_8N1, 14, 12);
    xbeeUart.begin(9600, SERIAL_8N1, 16, 17);
    xbeeUart.flush();
    console.flush();
      if (! rtc.begin()) {
          Serial.println("Couldn't find RTC");
          while (1);
    }
    xbeeColdSetup();
    rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    rtc.writeSqwPinMode(PCF8523_OFF);

    verifyXbee();
    enableXbee();
    /* Do some internet thing here */
    getHTTPtime();
    disableXbee();  

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

    Serial.println();
    delay(10000);
}

void parseDateString(String dateLine)
{
    console.println("SPLITTING Date String...");
    StringSplitter *splitter = new StringSplitter(dateLine, ' ', 8);
    int itemCount = splitter->getItemCount();
    console.println("Number of substrings = " + String(itemCount));
    for(int i = 0; i < itemCount; i++) {
        String item = splitter->getItemAtIndex(i);
        console.println("Item #" + String(i) + ": " + item);
    }
}


void getHTTPtime()
{
    if (haveConsole)
        console.println("Preparing data...");

    xbeeUart.print("GET / HTTP/1.1\n");
    xbeeUart.print("Host: ");
    xbeeUart.print(hostURL);
    xbeeUart.print("\n");
    xbeeUart.print("Accept: */*\n");
    xbeeUart.print("\r\n\r\n");
    if (haveConsole)
        console.println("Sent!!!");
    myGetResponse(10000);
}

void verifyXbee()
{
    console.println("Verify XBee module configuration.");
    while(!SendCMCommand(cmdMode, rspOK));  // AT command mode
    SendATCommand(cmdExit);                 //Exit command mode
    console.println("XBee module verified!");
}
void enableXbee()
{
    int retries = 0;
    console.println("Enable XBee Module for transmission.");
    while(!SendCMCommand(cmdMode, rspOK));  // AT command mode
    while(!SendATCommand(cmdAirplaneOff, rspOK)); // Turn off airplane mode
    while(!SendATCommand(cmdApply, rspOK)); // Apply change command
    // Wait for Association connection
    while(!SendATCommand(cmdAssoc, rspCONN)) {
        retries++;
        delay(500);
    }
    console.println("Number of cmdAssoc retries = " + String(retries));
    SendATCommand(cmdExit);
    console.println("XBee module enabled!");  
}

void disableXbee()
{
    console.println("Disable XBee Module for sleep mode.");
    while(!SendCMCommand(cmdMode, rspOK));  // AT command mode
    while(!SendATCommand(cmdAirplane, rspOK)); // Turn on airplane mode
    while(!SendATCommand(cmdApply, rspOK)); // Apply change command
    while(!SendATCommand(cmdAssoc, rspAirplane));   // Wait for Airplane mode
    SendATCommand(cmdExit);
    console.println("XBee module in airplane mode!");  
}

boolean SendCMCommand(char* data, char* response)
{
    if (haveConsole) {
        console.print("ATCommand: ");
        console.println(data);
    }
    delay(1000);
    while (xbeeUart.available())
        xbeeUart.read();
    xbeeUart.print(data);
    delay(1000);
    return(myCheckResponse(String(response), 1000));
}

boolean myCheckResponse(String expected, int timout)
{
    xbeeUart.setTimeout(timout);
    String resp = xbeeUart.readStringUntil('\r');
    if (haveConsole) {
        console.print("> ");
         console.print(resp);
#if 0
        for (int i = 0; i < resp.length(); i++) {
            console.print(" 0x");
            console.print(resp[i], HEX);
        }
#endif
        console.println();
    }
    xbeeUart.setTimeout(1000);
    return(resp == expected);
}

boolean myGetResponse(int initialTimeout)
{
    boolean haveResponse = true, firstLine = true;
    xbeeUart.setTimeout(initialTimeout);
    while (haveResponse == true) {
        String resp = xbeeUart.readStringUntil('\r');
        xbeeUart.setTimeout(1000);
        if (resp.length() > 0) {
            haveResponse = true;
            resp.trim();
            if (haveConsole) {
                console.println("> " + resp);
            }
            firstLine = false;
            if (resp.indexOf(String("Date")) == 0) {
                parseDateString(resp);
            }
        }
        else 
            haveResponse = false;
    }
}
// This method sends an ATCommand to the XBee
void SendATCommand(char* data)
{
    if (haveConsole) {
        console.print("ATCommand: ");
        console.println(data);
    }
    xbeeUart.println(data);
    myCheckResponse(String(""), 1000);
}

// This method sends an ATCommand and checks for a response
boolean SendATCommand(char* data, char* expected)
{
    if (haveConsole) {
        console.print("ATCommand: ");
        console.println(data);
    }
    xbeeUart.println(data);
    return myCheckResponse(String(expected), 1000);
}