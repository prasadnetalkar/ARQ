#include "common.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <pthread.h>
#include <algorithm>
#define MAX_PAYLOAD_SIZE 1452
#define PACKET_DATA 0
#define PACKET_ACK 1
#define PACKET_NACK 2
#define PACKET_CONTROL 3

using namespace std;

class Packet;
class Port;
class Address;
class ReceiverPort;

void SaveFile(const char* Fname, vector<char> *Data)
{
   ofstream OutFile(Fname) ;

   OutFile.write(&Data->at(0) ,Data->size());

   OutFile.close();
}

int main(int argc, const char * argv[])
{

try {
  const char* hname = "localhost";
  Address * my_addr = new Address(hname, (short)(atoi(argv[1])));
  LossyReceivingPort *my_port = new LossyReceivingPort(0.2);
  my_port->setAddress(my_addr);
  my_port->init();
  cout << "begin receiving..." <<endl;

  //configure a sending port to send ACK
  Address * my_tx_addr = new Address(hname, 3005);
  Address * dst_addr =  new Address(argv[2], (short)(atoi(argv[3])));
  SendingPort *my_tx_port = new SendingPort();
  my_tx_port->setAddress(my_tx_addr);
  my_tx_port->setRemoteAddress(dst_addr);
  my_tx_port->init();
  Packet *p;
  char src;
  char dst;
  int type,size,seq_n;
  int eof;
  const char* filename;
  vector<char> data_store;
  const char* rec="recv_";
  string f_n = string(rec) + string(argv[4]);
  filename = f_n.c_str();
  vector<int> seq_rec;
   while(1)
   {
    p = my_port->receivePacket();
    if(p==NULL){
       continue;
    }
    PacketHdr *hdr = p->accessHeader();
    //src = hdr->getOctet(0); 
    //dst = hdr->getOctet(1);
    type = hdr->getIntegerInfo(2);
    //seq_n = hdr->getIntegerInfo(6);
    //eof = hdr->getIntegerInfo(10);
    //size = p->getPayloadSize();
    if(type ==3){
       cout<<"Filename Received"<<endl;
       Packet *ack_packet = new Packet();
       ack_packet->accessHeader()->setOctet(dst,0); //src
       ack_packet->accessHeader()->setOctet(src,1); //ds///t
       ack_packet->accessHeader()->setIntegerInfo(1,2);
       ack_packet->accessHeader()->setIntegerInfo(-1,6);
       my_tx_port->sendPacket(ack_packet);
       continue;
    }
    src = hdr->getOctet(0);
    dst = hdr->getOctet(1);
    seq_n = hdr->getIntegerInfo(6);
    eof = hdr->getIntegerInfo(10);
    size = p->getPayloadSize();

    std::vector<int>::iterator it = std::find(seq_rec.begin(), seq_rec.end(), seq_n);
    if(it == seq_rec.end()){
        seq_rec.push_back(seq_n);
    	const char *dat = p->getPayload();
    	const char *end = dat + strlen(dat);
    	data_store.insert(data_store.end(),dat,end);
     }
    printf("Received packet: type = %d, sequence number = %d, size = %d,src = %c dst = %c\n",type,seq_n,size,src,dst);
    Packet *ack_packet = new Packet();
    ack_packet->accessHeader()->setOctet(dst,0); //src
    ack_packet->accessHeader()->setOctet(src,1); //ds///t
    ack_packet->accessHeader()->setIntegerInfo(1,2);//seq_n
    ack_packet->accessHeader()->setIntegerInfo(seq_n,6);//packet type DATA 0,ACK 1, NACK 2
    cout<<"Sending a packet (ACK) with type "<<PACKET_ACK<<" for seq_n "<<seq_n<<" to "<<ack_packet->accessHeader()->getOctet(1)<<endl;
    my_tx_port->sendPacket(ack_packet);
    if(eof==4){
      SaveFile(filename, &data_store);
      break;}
  }
} catch (const char *reason ) {
    cerr << "Exception:" << reason << endl;
    exit(-1);
}

return 0;
}
