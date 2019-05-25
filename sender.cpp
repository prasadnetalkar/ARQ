#include "common.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <pthread.h>
#define MAX_PAYLOAD_SIZE 1466
#define PACKET_DATA 0
#define PACKET_ACK 1
#define PACKET_NACK 2

using namespace std;

class Packet;
class Port;
class Address;
class SendingPort;
pthread_mutex_t lock;

int main(int argc, const char * argv[])
{

try {
  const char* hname = "localhost";
  Address * my_addr = new Address(hname, 3000);
  Address * dst_addr =  new Address(argv[1], (short)(atoi(argv[2])));
  const char* filename = argv[3];
  SendingPort *my_port = new SendingPort();
  my_port->setAddress(my_addr);
  my_port->setRemoteAddress(dst_addr);
  my_port->init();

  Packet * packet_f;
  packet_f = new Packet();
  packet_f->setPayloadSize(sizeof(filename));
  packet_f->fillPayload(sizeof(filename)+1,(char*)filename);
  PacketHdr *hdr = packet_f->accessHeader();
  hdr->setOctet('A',0);
  hdr->setOctet('B',1);
  my_port->sendPacket(packet_f);
  cout << "Sending file name to receiver"<< filename << endl;

  ifstream file(filename);
  vector<char>data_store;

  while (file.good()){
    data_store.push_back(file.get());
  }

  ifstream fileread;
  fileread.open(filename,ios::in|ios::ate);
  int fsize = fileread.tellg();
  fileread.close();
  int count = fsize/MAX_PAYLOAD_SIZE;
  for(int seq_n = 0;seq_n<count+1;seq_n++){
      pthread_mutex_lock(&lock);
      Packet *my_packet = new Packet();
      my_packet->accessHeader()->setOctet('A',0); //src
      my_packet->accessHeader()->setOctet('B',1); //dst
      my_packet->accessHeader()->setIntegerInfo(seq_n,2);//seq_n
      my_packet->accessHeader()->setIntegerInfo(PACKET_DATA,6);//packet type DATA 0,ACK 1, NACK 2
      cout<<"Sending a packet with type "<<PACKET_DATA<<" having seq_n "<<seq_n<<" to "<<my_packet->accessHeader()->getOctet(1)<<endl;
      my_packet->setPayloadSize(MAX_PAYLOAD_SIZE);
      my_packet->fillPayload(MAX_PAYLOAD_SIZE,&data_store.at(seq_n*MAX_PAYLOAD_SIZE));
      my_port->sendPacket(my_packet);
      sleep(1);
      pthread_mutex_unlock(&lock);
}
} catch (const char *reason ){
    cerr << "Exception:" << reason << endl;
    exit(-1);
}
 return 0;
}
