#include "nfc.hpp"

using namespace std;


NFC::NFC()
{
    Snap7Client = new TS7Client();
    reader = new PN532();
    Blue = new mraa::Pwm(GPIO_BLUE);
    Green = new mraa::Pwm(GPIO_GREEN);
    Red = new mraa::Pwm(GPIO_RED);
    Buzzer = new mraa::Gpio(GPIO_BUZZER);
    Relay = new mraa::Gpio(GPIO_RELAY);
    uid=reader->UID;
    Rack=0;
    Slot=1;
    DB=50;
    Offset=0;
    ok=0;
    k0=0;
    watchdog=0;
    breatheCount=0;
    breathe=0;
    breatheDir=0.01;
    breatheOut=0;
    useSnap7=true;
    isValidCard=false;
    isNewCard=false;
    isCard=false;
    isCardEntering=false;
    logLevel=LOG_EVENT;
    failCounts=0;
}

void NFC::beep(int mode)
{
    if(isCardEntering){
        breatheCount=0;
        breathe=0;

        if(isValidCard){
            //Beep 0.5s
            Blue->write(0);
            Red->write(0);
            Green->write(0.8);
            Buzzer->write(1);
            usleep(500000);
            //Stop 0.25s
            Green->write(0);
            Buzzer->write(0);
            usleep(250000);
            //Beep 0.5s
            Green->write(0.8);
            Buzzer->write(1);
            usleep(500000);
            Green->write(0);
            Buzzer->write(0);
        }
        else{
            //Beep 1.5s
            Blue->write(0);
            Red->write(0.8);
            Green->write(0);
            Buzzer->write(1);
            usleep(1500000);
            Red->write(0);
            Buzzer->write(0);
        }
        // Beep once for one card entering.
        isCardEntering=false;
    }
    else{
        breatheCount++;
        if(breatheCount>20) breathe+=breatheDir;
        if(breathe>1) breatheDir=-0.01;
        if(breathe<0) breatheDir=0.01;
        breatheOut= (breathe*breathe)/5;
        if(breatheOut<0.005) breatheOut=0.005;
        Blue->write(breatheOut);
        Green->write(breatheOut);
        Red->write(breatheOut);
    }
//    fprintf(stderr,"[Beep] Write  Count: %d,Breathe: %f,Out: %f\n", breatheCount, breathe, breatheOut);

}



void NFC::initialize(void)
{
    int ret;

    reader->connect("/dev/ttyS1");
    //Init Snap7 connection
    if(useSnap7){
        if(logLevel & ON_DEBUG){
            cout << "Snap7Client connecting, IP:" << Address << "Rack:" << Rack << "Slot:"<< Slot <<endl;
        }
        ret = Snap7Client->ConnectTo(Address, Rack, Slot);
        if(!ret){
            fprintf(stderr, "[Snap7Client] Connect error %s\n", CliErrorText(ret).c_str());
            fprintf(stderr, "[Snap7Client] Running without PLC connection...\n");
            useSnap7=false;
        } else{
            if(logLevel & ON_EVENT){
                cout << "[Snap7Cleint] Connecting successful, Exec time:" << Snap7Client->ExecTime() << endl;
                cout << "     PDU Requestd: " << Snap7Client->PDURequested() <<endl;
                cout << "     PDU Negotiated: " << Snap7Client->PDULength() <<endl;
            }
        }
    }

    //Init PWM
    Blue->period_ms(1);
    Blue->write(1);
    Blue->enable(true);
    Green->period_ms(1);
    Green->write(0);
    Green->enable(true);
    Red->period_ms(1);
    Red->write(0);
    Red->enable(true);

    //Init GPIO
    Buzzer->dir(mraa::DIR_OUT_LOW);
    Relay->dir(mraa::DIR_OUT_LOW);
    Buzzer->write(1);
    Relay->write(0);
    

    //Init ID table
    bzero(id_table,80);
    if(useSnap7) Snap7Client->DBRead(DB, Offset+4, 80, (void*)id_table);

    reader->wakeUp();
    reader->RFConfiguration(logLevel);
    reader->GetFirmwareVersion(logLevel);
    reader->GetGeneralStatus(logLevel);
    usleep(100000);
    Buzzer->write(0);
    Blue->write(0);
 
}

bool NFC::checkCard(int numRecords)
{
    unsigned char sum;
    int diff=0;
    int i,j,pos;

    diff=0;
    for(i=0; i< 4; i++){
        if(uid[i]!=saved_uid[i]) diff++;
        saved_uid[i]=uid[i];
    }
    if(diff=0) isNewCard=false;
    else isNewCard=true;

    for(i=0;i<numRecords;i++){
        pos=i*4;
        sum = id_table[pos] | id_table[pos+1] | id_table[pos+2] | id_table[pos+3];
        if(sum == 0) continue;
        diff=0;
        for(j=0;j<4;j++){
            if(uid[j]!=id_table[pos+j]) diff++;
        }
        if(diff==0) return true;
    }
    return false;    
}

bool NFC::scanCard(void)
{
    bool ret;

    ret=reader->ListPassiveTarget(logLevel);
    if(ret){
        failCounts=0;
        if(logLevel && ON_EVENT) fprintf(stderr,"[ScanCard] There is a card within range\n");
        if(!isCard){
            isCardEntering=true;
            if(logLevel && ON_EVENT) fprintf(stderr,"[ScanCard] First entering this card\n");
        }
        else isCardEntering=false;
        isCard=true;

        isValidCard = checkCard(20);
        // If it's the first time checking this card and find no record, try to sync with PLC again
        if(!isValidCard && isCardEntering){
            if(logLevel & ON_DEBUG) cout << "[Snap7Client] Card ID not found in DB, download DB again...\n";
            if(useSnap7) Snap7Client->DBRead(DB, Offset+4, 80, (void*)id_table);
        }
        if(useSnap7){
            watchdog++;
            if(logLevel & ON_DEBUG) cout << "[Snap7Client] Valid Card found in DB, update uid to DB...\n";
            if(Snap7Client->DBWrite(DB, Offset, 4, (void*)uid)==0) watchdog=0;
            else fprintf(stderr, "[Snap7] DB Write UID error\n");
        }
    } else {
        failCounts++;
        if(failCounts>2) {
            if(isCard) cout<<"[ScanCard] Card leaving.\n";
            isCard=false;
            isCardEntering=false;
        }
    }
    if(isCard) writePLC();

    isCard=ret;
    return ret;
}

bool NFC::writePLC()
{
    if(useSnap7){
        watchdog++;
        if(logLevel & ON_DEBUG) cout << "[writePLC] update DB...\n";
        if(Snap7Client->DBWrite(DB, Offset, 4, (void*)uid)==0) watchdog=0;
        else cout << "[writePLC] DB Write UID error\n";
    }
}

NFC::~NFC()
{
    Blue->write(0);
    Blue->enable(false);
    Red->write(0);
    Red->enable(false);
    Green->write(0);
    Green->enable(false);
    Buzzer->write(0);
    Relay->write(0);
    if(useSnap7) Snap7Client->Disconnect();
    delete Snap7Client;
    delete reader;
    delete Blue, Green, Red;
    delete Buzzer, Relay;
}