#include<stdio.h>
#include<stdlib.h>
#include<getopt.h>

#include "nfc.hpp"
NFC* my7688;

void signalHandler(int signum){
    fprintf(stderr, "\nSignal received, preparing to stop\n");
    // Delete objects
    delete my7688;
    // Bye bye, adios!
    fprintf(stderr, "\nBye Bye, Adios!\n");
    exit(signum);
}

static int opt_plc = 1;
static struct option long_options[]=
{
    {"ip", required_argument, 0, 'i'},
    {"db", required_argument, 0, 'd'},
    {"offset", required_argument, 0, 'o'},
    {"rack", required_argument, 0, 'r'},
    {"slot", required_argument, 0, 's'},
    {"debug", required_argument, 0, 'g'},
    {"help", no_argument, 0, 'h'},
    {"noplc", no_argument, 0, 'n'},
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
        case 'n':
            my7688->useSnap7=false;
            break;

        case 'h':
            fprintf(stderr, "\nUsage: CardReader -i IP_address  -d DB_number -o Offset [-r Rack -s Slot -g DebugLevel --noplc]\n");
            fprintf(stderr,   "                  [-o|--offset] offset  [-r|--rach] rack\nOptions:");
            fprintf(stderr, "-i, --ip ip_address    Set IP address of PLC\n");
            fprintf(stderr, "-d, --db dbnum         Set DB number of PLC\n");
            fprintf(stderr, "-o, --offset offset    Set Offset of DB\n");
            fprintf(stderr, "-r, --rack number      Set Rack number of PLC\n");
            fprintf(stderr, "-s, --slot number      Set Slot numberof PLC\n");
            fprintf(stderr, "-g, --debug number     Set Debug Level (1: DEBUG, 3: EVENT, 7: ERROR\n");
            fprintf(stderr, "-n, --noplc            Disable communication with PLC\n");
            fprintf(stderr, "-h, --help             Show this message\n");
            return 0;
            break;

        default:
            abort();

    }
 }
    if(argc<3) my7688->useSnap7=false;
    my7688->initialize();
    signal(SIGTERM, signalHandler);
    signal(SIGINT, signalHandler);
    counts=0;
    do{
        if(counts%10) my7688->scanCard();
        my7688->beep(0);
        usleep(20000);
        counts++;
    }while(1);

    
      delete my7688;

}