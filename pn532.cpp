#include "pn532.hpp"

using namespace std;

// This is a common routine to display a HEX string with it's name and length
void printHEX(const char* topic, char* str, int len)
{

  fprintf(stderr, "[%s %d] ",topic, len);
  if(len > 255) {
    fprintf(stderr, "Length > 256, set to 256\n");
    len = 256;
  }
  for(int i = 0; i< len; i++) fprintf(stderr, "%02X ", (unsigned char)str[i]);
  fprintf(stderr, "\n");
}

// Wrapper for unsigned char* str
void printHEX(const char* topic, unsigned char* str, int len)
{
    printHEX(topic, (char*)str, len);
}

// Constructor
PN532::PN532(){
    
    bzero(rxStr,256);
    bzero(txStr,256);
}

// Set PN532 communication, we are using uart (not I2C or SPI), default baud rate is 115200, N,  8, 1, no flow control
// On libmraa library GIT it says that the timeout is not implemented, setting timeout may cause unknown problem.
// Instead of system timeout from MRAA, we will use Uart->dataAvailable(timeout) to check if the message arrive at the uart within timeout
bool PN532::connect(const char* ttyS){

    dev = new mraa::Uart(ttyS);

    dev->setBaudRate(115200);
    dev->setMode(8, mraa::UART_PARITY_NONE,1);
    dev->setFlowcontrol(false,false);
//    dev->setTimeout(44,44,44);
}

// Destructor
PN532::~PN532(){
 
    char ACK[] = { 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00 };
    dev->write(ACK,6);
    usleep(5000);
    delete dev;
}

// Wake up sequence: 0x55 0x55 + lot of 0x00 until it awake.
// Then send 0x00 0x00 0xff 0x03 0xfd to notify 3 bytes command:
// 0xD4 0x14 0x01  (SamConfigue: Normal mode, timeout=50ms)
// Expected reply:
bool PN532::wakeUp()
{
    int ret;
    
    char WAKEUP[] ={
        0x55, 0x55, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0xff, 0x03, 0xfd, 0xd4,
        0x14, 0x01, 0x17, 0x00
    };

    dev->write(WAKEUP, 24);
    usleep(200000);
    ret = dev->read(rxStr,15);

     if(isLog & ON_DEBUG) cout<<"Send wake up command, get "<<ret<<" chars reply..." <<endl;
   
    return ret>7?true:false;
}

// Setup RF parameters
// D4 32 05 02 01 40(RF_Retry)
// RF_Retry: 0xFF infinite, 0x00 No retry. Suggest > 0xFF, which means continueous retry until timeout
// Expected reply:
bool PN532::RFConfiguration(int isLog)
{
    int ret;
    char RFCFG[]={ 0xD4, 0x32, 0x05, 0x02, 0x01, 0xFF };

    if(isLog & ON_EVENT) cout<<"Configure RF parameters..." <<endl;
    ret = Query(RFCFG, 6, isLog);
    return ret>5?true:false;
}

// GerFirmwareVersion: D4 02 
// Output: D4 03 XX YY ZZ
// Expected reply:
bool PN532::GetFirmwareVersion(int isLog)
{
    int ret;
    
    char cmd[] ={ 0xD4, 0x02 };
    if(isLog & ON_EVENT) cout<<"GetFirmwareVersion..."<<endl;
    ret = Query(cmd, 2, isLog);
    info_IC = Reply[2];
    info_Ver= Reply[3];
    info_Rev= Reply[4];
    info_Support = Reply[5];
    fprintf(stderr, "IC: %02X, Ver: %d, Rev: %d, Support: %02X\n", info_IC, info_Ver, info_Rev, info_Support);
    if(info_Support|1) cout << "Supported: ISO/IEC 14443 TypeA\n";
    if(info_Support|2) cout << "Supported: ISO/IEC 14443 TypeB\n";
    if(info_Support|4) cout << "Supported: ISO18092\n";
    return ret>4?true:false;
}

// GerGeneralStatus: D4 04
// Outp0ut: D4 05 XX YY ZZ
// Expected reply:
bool PN532::GetGeneralStatus(int isLog)
{
    int ret;
    
    char cmd[] ={ 0xD4, 0x04 };
    if(isLog & ON_EVENT) cout<<"GetGeneralStatus..."<<endl;
    ret = Query(cmd, 2, true);
    printHEX("Status", Reply, ret);
 
    return ret>4?true:false;
}


// Auth: D4 40 01 AUTH(0x60) 07 KEY[6] UID[4]
// Usually factory default key is 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
// Expected reply:
bool PN532::auth(int isLog)
{
    int ret;
    
    char cmd[] ={ 0xD4, 0x40, 0x01, 0x60, 0x07, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,0x00,0x00,0x00 };
    if(isLog & ON_EVENT) cout<<"Authencate..."<<endl;
    for(int i=0;i<4;i++) cmd[i+11]=UID[i];

    ret = Query(cmd, 15, isLog);
    printHEX("Status", Reply, ret);
 
    return true;
}

// ListPassiveTarget: D4 4A NumCards(02) 106k_Mifare(00)
// Outpout: D4 4B xxxxxx
// Expected reply:
bool PN532::ListPassiveTarget(int isLog)
{
    int ret;
    int NbTg;
    
    char cmd[] ={
        0xD4, 0x4A, 0x01, 0x00
    };
    
    if(isLog & ON_DEBUG) cout<<"ListPassiveTarget..."<<endl;
    ret = Query(cmd, 4, isLog);
    NbTg = rxStr[7];
    if(!ret){
        if(isLog & ON_DEBUG) cout << "[ListPassiveTarget] Fail.\n";
        return false;
    }
    if((NbTg == 0) || (ret==0)) {
        if(isLog & ON_DEBUG) cout<<"No card listed"<<endl;
        for(int i=0;i<12;i++) UID[i]=0;
        return false;
    } 
    else{
        lenUID = rxStr[12];
        if(lenUID<12){
            memcpy(UID, rxStr+13, lenUID);
            if(isLog & ON_EVENT) printHEX("UID", UID, lenUID);
        }
        return true;
    }

}

/* Query wraps the PN532 command to txString in following spec:
   0x00 0x00 0xFF LEN LCS CMD VAR1 VAR2 ....VARn CHK 0x00
   head = 0x00 0x00 0xFF
   LEN = Bytes counts from CMD to VARn (which is n+1)
   LCS = ~LEN+1  (LCS+LEN = 0x100)
   CHK = ~SUM(CMD, VAR1, ... VARn)+1
   Total lenght transmit = cmdLen + 7
   Expected reply:
*/
int PN532::Query(char* cmd, int cmdLen, int isLog)
{
    unsigned char txLEN, txLCS, txCHK;
    int ret;

    char c =0;
    for(int i=0;i<cmdLen;i++){
        c+= cmd[i];
    }
    txCHK = ~c+1;
    txLEN = (unsigned char) cmdLen;
    txLCS = ~txLEN+1;

    txStr[0]= 0x00;
    txStr[1]= 0x00;
    txStr[2]= 0xFF;
    txStr[3]= txLEN;
    txStr[4]= txLCS;
    memcpy(txStr+5, cmd, cmdLen);
    txStr[5+cmdLen]=txCHK;
    txStr[6+cmdLen]=0x00;
    return(rawCommand(7+cmdLen, isLog));

}

/* rawCommand send raw data to PN532, read ACK, then wait for reply to read and read them.
    1. send cmd
    2. wait 5mS, this is important to keep a gap to earlier communication otherwise PN532 go fucked up.
    3. Read ACK, if can not read 6 chars, read the rest again.
    4. Wait until uart is ready to read, this can be replaced by an IRQ
    5. Read a long line, twice. Don't ask me. PN532 always prefer to answer question in two paragraph, no reason.
    6. Start to analysis the buffer, locate the first 0xFF (which means 0x00 0x00 0xFF <- pointer stop here)
    7. Check next two bytes if they are 2's compliment (LEN = ~LCS+1), otherwise try to find next 0xFF.
    8. Check if the length of read buffer match definition of LEN ( Output= 0x00 0x00 0xFF LEN LCS [DATA LEN] CHK 0x00), 
    if not, we have a problem here.... Let's send ACK to reset PN532's answer, (should send NACK to ask PN532 to send again... but not working so far)
    9. make a copy of [DATA LEN] to Reply[], and return the Reply LEN. 
*/
int PN532::rawCommand(int txLen, int isLog)
{
    int ret, remain;
    char LEN, LCS; 
    int CMD;
    char NACK[] = { 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00 };
    char ACK[] = { 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00 };
   
    
    try{
    // Send Command
        if(isLog & ON_DEBUG) printHEX("TX", txStr, txLen);
        dev->write((char*)txStr, txLen);
    // Wait for PN532 to settle down, >15mS , very very important, otherwise PN532 go crazy and start to lost sync of reply...
        usleep(15000);
    // Wait for data from serial line, timeout 15ms
        if(!dev->dataAvailable(15)){
            if(isLog & ON_ERROR) cout<<"[ACK] Reply timeout 1" << endl;
            wakeUp();
            return false;
        }
        ret = dev->read(rxStr,6);   // Read ACK 6 chars 
        if(ret<6) ret+= dev->read(rxStr+ret, 6-ret); // Buffer run out?? read the rest!
        if(isLog & ON_DEBUG) printHEX("ACK", rxStr, ret);

        if(!dev->dataAvailable(50)){
            if(isLog & ON_DEBUG) cout<<"[RX] Reply timeout" << endl;
            dev->write(ACK,6); // Ask PN532 to give up, reset all pending job, wait for next command.
            return false;
        }

/* Test code*/
// Read Peamble bytes until reach 0xFF
// Minimum Length is 8: 0x00 0x00 0xFF LEN LCS TFI --- DCS 0x00
        ret = dev->read(rxStr,8);
        if(ret<6){
            // Read less than 6 bytes, maybe will not find LEN and TFI, try to wait 15ms and read the rest.
            if(dev->dataAvailable(10)) ret+= dev->read(rxStr+ret, 8-ret);
        }
        // Locate 0xFF
        for(int i=0; i<ret-3; i++){
            if((unsigned char)rxStr[i]==255){
                LEN = rxStr[i+1];
                LCS = rxStr[i+2];
                if((LEN+LCS)==0){
                    //i is the location of 0xFF, which is Bytes of Preamble and 0x00 0xFF, CMD legnth = i + 2(LEN and LCS) +1
                    CMD = i+3;
                    // Total frame length = 5 + LEN + 2
                    if(ret<(CMD+LEN+2)){
                        if(isLog & ON_EVENT) fprintf(stderr, "Frame length %d, read %d, need to read %d more\n", CMD+LEN+2, ret, (CMD+LEN+2-ret));
                        if(dev->dataAvailable(5)) ret+=dev->read(rxStr+ret, (CMD+LEN+2-ret));
                        // If still not read full length, try again..
                        if(ret<(CMD+LEN+2)){
                            if(isLog & ON_EVENT) fprintf(stderr, "1st Cannot read full response frame: target length %d, read %d only\n", CMD+LEN+2, ret);
                            if(dev->dataAvailable(5)) ret+=dev->read(rxStr+ret, (CMD+LEN+2-ret));
                            if(ret<(CMD+LEN+2)){
                                if(isLog & ON_ERROR) fprintf(stderr, "2nd Cannot read full response frame: target length %d, read %d only\n", CMD+LEN+2, ret);
                                dev->write(ACK,6);
                                usleep(15000);
                                ret+=dev->read(rxStr+ret, (CMD+LEN+2-ret));
                                if(isLog & ON_ERROR) fprintf(stderr, "Finally: target length %d, read %d \n", CMD+LEN+2, ret);
                                // Do nothing, just pray if the information is enough to read UID....
                            }
                        }

                    }
                }
                break;
            }
        }
        if((LEN+LCS)!=0){
            if(isLog & ON_ERROR) fprintf(stderr, "Paragraph LEN %d does not match to LCS %d, reset peer\n", LEN, LCS);
            dev->write(ACK, 6);
            usleep(50000);
            return 0;
        }

        if(isLog & ON_DEBUG) printHEX("HEAD", rxStr, ret);

        if(isLog & ON_DEBUG) cout<< "LEN: " << (int)LEN <<" LCS: "<< (int)LCS << endl;

        if((LEN+LCS)!=0){
            cout << "[RX] Read header error" << LEN+LCS << endl;
            printHEX("RX", rxStr, ret);
        }
        memcpy(Reply, rxStr+CMD, LEN);
        // Received response, send ACK to finish this transaction
        dev->write(ACK,6);
        usleep(15000);



    } catch (std::exception& e)
    {
        std::cout<<e.what() << "Read / Write Error" << std::endl;
    }
    // Send ACK to acknowledge the receipt of reply, or to reset the reply
 //   dev->write(ACK,6);
 
    return LEN;
}

