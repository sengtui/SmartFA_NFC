#include<stdio.h>
#include<stdlib.h>
#include<getopt.h>

#include "nfc.hpp"
using namespace std;

NFC* my7688;
static int optHasPLC=1;
static int  optHasNFC=1;
static int optQuiet=0;

void signalHandler(int signum){
    cout << "\nSignal received, preparing to stop\n";
    // Delete objects
    delete my7688;
    // Bye bye, adios!
    cout << "\nBye Bye, Adios!\n";
    exit(signum);
}

static struct option long_options[]=
{
    {"ip", required_argument, 0, 'i'},
    {"db", required_argument, 0, 'd'},
    {"offset", required_argument, 0, 'o'},
    {"rack", required_argument, 0, 'r'},
    {"slot", required_argument, 0, 's'},
    {"debug", required_argument, 0, 'g'},
    {"help", no_argument, 0, 'h'},
    {"noplc", no_argument, &optHasPLC, 0},
    {"nonfc", no_argument, &optHasNFC, 0},
    {"quite", no_argument, &optQuiet,1 },
    {0,0,0,0}
};

int main(int argc, char** argv)
{

    int ret;
    int counts;

    my7688 = new NFC();
    int option_index=0;
    int c;
    my7688->useSnap7=true;
    while(1){
        c = getopt_long(argc, argv, "i:d:o:r:s:g:h", long_options, &option_index);
        if(c==-1) break;
        // For parameters flags setting directly, ex: noplc, nonfc, quite, c==0
        if(c==0) break;
        switch(c){
            case 'i':
                my7688->Address=optarg;
                break;
            case 'd':
                my7688->DB=atoi(optarg);
                break;
            case 'o':
                my7688->Offset=atoi(optarg);
                break;
            case 'r':
                my7688->Rack=atoi(optarg);
                break;
            case 's':
                my7688->Slot=atoi(optarg);;
                break;
            case 'g':
                my7688->logLevel=atoi(optarg);
        fprintf(stderr,"Loglevel:%d\n", my7688->logLevel);
                break;
          

            case 'h':
                cout << endl << "Usage: CardReader -i IP_address -d DB_number -o Offset ";
                cout << "[-r Rack -s Slot -g DebugLevel --noplc]" << endl;
                cout << "Options:" <<endl ;
                cout << "   -i, --ip <ip_address>     Set IP address of PLC\n";
                cout << "   -d, --db <db_number>      Set DB number of PLC, default 50 if not set\n";
                cout << "   -o, --offset <db_offset>  Set Offset of DB, default 0 if not set\n";
                cout << "   -r, --rack <number>       Set Rack number of PLC, default 0 if not set\n";
                cout << "   -s, --slot <number>       Set Slot numberof PLC, default 1 if not set\n";
                cout << "   -g, --debug <number>      Set Debug Level 7: DEBUG, 3: EVENT, 1: ERROR, default:1\n";
                cout << "   --noplc               Disable communication with PLC\n";
                cout << "   --nonfc               Disable NFC function\n";
                cout << "   --quiet                Disable Buzzer\n";
                cout << "   -h, --help                Show this message\n\n";
                cout << "Running CardReader without parameters will disable connection to PLC";
                cout << ", same as --noplc\n";
                return 0;
                break;

            default:
                cout << "Default: " << c <<"\n";
                abort();

        }
    }
    // If there is no IP Address, do not connect to PLC
    if(my7688->Address==NULL) optHasPLC=false;
    
    // Debug other parts when  there is no NFC module installed.

    if(optHasNFC) cout << "Run with NFC module\n";
    my7688->useNFC=(bool)optHasNFC;
    
    if(optHasPLC) cout << "Run with PLC Communication\n";
    my7688->useSnap7=(bool)optHasPLC;
    
    if(optQuiet) cout << "Run with noisy Buzzer\n";
    my7688->useBuzzer = optQuiet==0?true:false;
    
    my7688->initialize();
    signal(SIGTERM, signalHandler);
    signal(SIGINT, signalHandler);
    counts=0;
    do{
        if(counts%10&& my7688->useNFC) my7688->scanCard();
        my7688->beep(0);
        usleep(20000);
        counts++;
    }while(1);

    
      delete my7688;

}