/*
	taco-thingsboard-sleep

	Author: Adnan Jalaludin <adnan.jalaludin@gmail.com>
*/

#include <stdlib.h>
#include <esp_deep_sleep.h>
#include "taco.h"

#define console Serial
HardwareSerial rs485(1);
HardwareSerial xbeeUart(2);

// Thingspeak and sensor variables
char hostURL[] = "demo.thingsboard.io";
char apiKey[] = "XXXXXXXXXXXXXXXXXXXX";
float ambTemp, objTemp, mq2val, mq135val;
int seconds = 0;
bool haveConsole = false;
bool relayIsOn = false;

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  45        /* Time ESP32 will go to sleep (in seconds) */
RTC_DATA_ATTR int bootCount = 0;

#if 1
void print_wakeup_reason(int wakeup_reason)
{
    switch(wakeup_reason) {
        case 1  : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
        case 2  : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
        case 3  : Serial.println("Wakeup caused by timer"); break;
        case 4  : Serial.println("Wakeup caused by touchpad"); break;
        case 5  : Serial.println("Wakeup caused by ULP program"); break;
        default : Serial.println("Wakeup was not caused by deep sleep"); break;
    }
}
#endif

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
    { "ATPD", "7fff", "pull direction on DIO pins" },
    { "ATPR", "7fff", "pull up/down on DIO pins" },
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
    console.println("XBee module configuration...");
    while(!SendCMCommand(cmdMode, rspOK));  // AT command mode

    while (xbeeSetups[i].cmd != 0) {
        console.println(xbeeSetups[i].description);
        if (SendATCommand(xbeeSetups[i].cmd, xbeeSetups[i].param) == 0) {
            console.println("FAIL!");
        }
        else {
            console.println("PASSED");
        }
        i++;
    }
    SendATCommand(cmdExit);                 //Exit command mode
    console.println("XBee module set up!");
}


void setup()
{
    esp_sleep_wakeup_cause_t wakeup_reason;
    wakeup_reason = esp_sleep_get_wakeup_cause();
    ioColdSetup();

    digitalWrite(PEN1, HIGH);
    int waited = 0;
    while ((!console) && (waited++ < 5))
        delay(1000);
    console.begin(115200);
    if (console)
        haveConsole = true;
    // Configure baud rate for console ports
    rs485.begin(9600, SERIAL_8N1, 14, 12);
    xbeeUart.begin(9600, SERIAL_8N1, 16, 17);
    xbeeUart.flush();
    console.flush();

    if ((wakeup_reason < 1) || (wakeup_reason > 5)) {
       xbeeColdSetup();
    }
    
    // Print boot count and reason on every reboot
    console.println("Boot number: " + String(bootCount++));
    print_wakeup_reason(wakeup_reason);

//    digitalWrite(XB_DTR, LOW);  // wakeup from Sleep_RQ mode
    verifyXbee();
    enableXbee();
    /* Do something internet thing here */
    disableXbee();  
    
    digitalWrite(XB_DTR, HIGH); // allow pullup to Sleep_RQ mode
    digitalWrite(XB_RST, LOW); // allow pullup to Sleep_RQ mode
    pinMode(XB_DTR, INPUT);
  
    Serial.println("GOING TO SLEEP NOW...");
    digitalWrite(PEN1, LOW);

    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
    Serial.println("Configured all RTC Peripherals to be powered down in sleep");

    delay(100); // wait all serial characters to be printed out
    esp_deep_sleep_start();
    //Serial.println("This will never be printed");
}

void loop(){
    // This is not going to be called
    pinMode(PEN1,     OUTPUT);
    digitalWrite(PEN1, HIGH);
    delay(500);
    digitalWrite(PEN1, LOW);
    delay(500);
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
    String resp = xbeeUart.readStringUntil('\r');
    if (haveConsole) {
        console.print(resp);
        console.print(":");
        for (int i = 0; i < resp.length(); i++) {
            console.print(" 0x");
            console.print(resp[i], HEX);
        }
        console.println();
    }
    return(resp == expected);
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