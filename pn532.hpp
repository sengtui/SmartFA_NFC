#ifndef PN532_HPP
#define PN532_HPP

#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <mraa.hpp>


#define ON_ERROR   0
#define ON_EVENT   1
#define ON_DEBUG   9


void printHEX(const char* topic, char* str, int len);
// Override for unsigned char* string
void printHEX(const char* topic, unsigned char* str, int len);

class PN532 {
  public:
      char UID[12];
      PN532();
      bool connect(const char* ttyS);
      bool wakeUp();
      bool auth(int isLog);
      bool RFConfiguration(int isLog);
      bool GetFirmwareVersion(int isLog);
      bool GetGeneralStatus(int isLog);
      bool ListPassiveTarget(int isLog);
      int Query(char* cmd, int txLen, int isLog);
      
    
      ~PN532();
  private:
    mraa::Uart* dev;
    unsigned char txStr[256];
    char rxStr[256];
    char Reply[256];
    int rawCommand(int txLen, int isLog);
    int lenUID;
    int info_IC, info_Ver, info_Rev, info_Support;
 
};

#endif
