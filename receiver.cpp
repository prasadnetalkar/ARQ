#include "common.h"
#include <iostream>
using namespace std;

class Packet;
class Port;
class Address;
class ReceiverPort;

int main(int argc, const char * argv[])
{

try {
  const char* hname = "localhost";
  Address * my_addr = new Address(hname, (short)(atoi(argv[1])));
  ReceivingPort *my_port = new ReceivingPort();
  my_port->setAddress(my_addr);
  my_port->init();
  cout << "begin receiving..." <<endl;

  Packet *p;
  while (1)
  {
    p = my_port->receivePacket();
    /**
    * Post-processing received packet
    */
    if (p !=NULL)
      cout << "received a packet with seq id "<< p->accessHeader()->getIntegerInfo(2)<<endl;

  }
} catch (const char *reason ) {
    cerr << "Exception:" << reason << endl;
    exit(-1);
}

return 0;
}
