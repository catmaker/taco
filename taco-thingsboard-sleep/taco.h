#ifndef __TACO_H__
#define __TACO_H__


#define PEN1    15
#define PEN2    2
#define RLY_OFF 32
#define RLY_ON  33
#define LOOPADC 34
#define MAI1    36
#define MAI2    39
#define MIO0    23
#define MIO1    13
#define MIO2    25
#define MIO3    26
#define TXEN485 27
#define TXD485  12
#define RXD485  14
#define XB_RST  4
#define XB_TXD  16
#define XB_RXD  17
#define XB_DTR  5
#define XB_CTS  18
#define XB_RTS  19
#define RTCIRQ  35

#define LED PEN1

void ioColdSetup(void)
{
    // initialize digital pin LED_BUILTIN as an output.
    pinMode(PEN1,     OUTPUT);
    pinMode(PEN2,     OUTPUT);
    pinMode(RLY_OFF,  OUTPUT);
    pinMode(RLY_ON,   OUTPUT);
    pinMode(LOOPADC,  INPUT);
    pinMode(MAI1,     INPUT);
    pinMode(MAI2,     INPUT);
    pinMode(MIO0,     INPUT);
    pinMode(MIO1,     INPUT);
    pinMode(MIO2,     INPUT);
    pinMode(MIO3,     INPUT);
    pinMode(TXEN485,  OUTPUT);
    pinMode(XB_RST,   OUTPUT);
    pinMode(XB_DTR,   OUTPUT);
    pinMode(XB_CTS,   INPUT);
    pinMode(XB_RTS,   OUTPUT);
    pinMode(RTCIRQ,   INPUT);

    digitalWrite(PEN1,     LOW);
    digitalWrite(PEN2,     LOW);
    digitalWrite(RLY_OFF,  LOW);
    digitalWrite(RLY_ON,   LOW);
    digitalWrite(TXEN485,  LOW);
    digitalWrite(XB_RST,   HIGH);
    digitalWrite(XB_DTR,   LOW);
    digitalWrite(XB_RTS,   LOW);
}

#endif /* __TACO_H__ */