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


int main(int argc, char** argv)
{
    int ret;
    int counts;

    my7688 = new NFC();


// Proceed parameters
    if (argc!=4 && argc!=6)
    {
        fprintf(stderr, "\nUsage: CardReader IP DB Offset (Rack) (Slot)\n");
        fprintf(stderr, "Running deamon without PLC connection...\n");
        my7688->useSnap7=false;
    }
    else{
        my7688->Address=argv[1];
        my7688->DB = atoi(argv[2]);
        my7688->Offset = atoi(argv[3]);
        if (argc==5){
            my7688->Rack=atoi(argv[4]);
            my7688->Slot=atoi(argv[5]);
        }
    }
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