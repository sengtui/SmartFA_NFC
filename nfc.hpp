#ifndef NFC_HPP
#define NFC_HPP

#include <mraa.hpp>
#include <pthread.h>
#include <signal.h>
#include "pn532.hpp"
#include "snap7.h"
#include "gpio.hpp"


class NFC {
  public:
    PN532* reader;
    TS7Client* Snap7Client;
    mraa::Gpio *Buzzer, *Relay;
    mraa::Pwm *Blue, *Red, *Green;


    NFC();
    ~NFC();
    void initialize(void);
    void beep(int mode);
    bool scanCard(void);
    bool checkCard(int);
    bool writePLC(void);
    int logLevel;

    bool useSnap7;
    bool noNFC;
    int Rack, Slot, DB, Offset, ok, k0, watchdog;
    char* Address; // PLC IP Address
    bool isValidCard;
    bool isNewCard;
    bool isCard;
    bool isCardEntering;
    
    //Define Vars for NFC reader
  
   private:
    int counts;
    int failCounts;
    char* uid;
    char saved_uid[6];
    char id_table[80];
    int breatheCount;
    float breathe;
    float breatheDir;
    float breatheOut;
};




#endif