#include "common.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <pthread.h>
#define MAX_PAYLOAD_SIZE 1452
#define PACKET_DATA 0
#define PACKET_ACK 1
#define PACKET_NACK 2
#define PACKET_CONTROL 3
#define END_OF_FILE_SEG 4
using namespace std;

class Packet;
class Port;
class Address;
class SendingPort;
pthread_mutex_t lock;

void FileNameSendPacket(char *fname, SendingPort* s_port){
	Packet * p;
        p = new Packet();
	p->setPayloadSize(sizeof(fname));
	p->fillPayload(sizeof(fname)+1,fname);
	PacketHdr *hdr = p->accessHeader();
	hdr->setOctet('A',0); //src
	hdr->setOctet('B',1); //dst
	hdr->setIntegerInfo(3,2);//control 3
	s_port->sendPacket(p);
	cout << "Sending file name to receiver "<< fname << endl;	
}

int main(int argc, const char * argv[])
{
try {
  const char* hname = "localhost";
  Address * my_addr = new Address(hname, 3001); //transmit port
  Address * dst_addr =  new Address(argv[1], (short)(atoi(argv[2]))); //Destination for sending 
  SendingPort *my_port = new SendingPort();
  //const char *ad="08:00:27:9a:04:e5";
  //my_addr->setHWAddrFromColonFormat(ad);
  my_port->setAddress(my_addr);
  my_port->setRemoteAddress(dst_addr);
  my_port->init();
 
  Address * my_rx_addr = new Address(hname, 5000);  //receive port
  LossyReceivingPort * my_rx_port = new LossyReceivingPort(0.2);
  my_rx_port->setAddress(my_rx_addr);
  my_rx_port->init(); 

  const char* filename = argv[3];
  
  int recv_flag = 1;
  int seq_recv_ack;
  char dst_recv_ack;
  int type_recv_ack;

  FileNameSendPacket((char*)filename,my_port); //filename send init packet
  int count1 =0;
  while(1){
      Packet *p_recv = my_rx_port->receivePacket();
      if (p_recv != NULL)
         {
         dst_recv_ack = p_recv->accessHeader()->getOctet(0);
         type_recv_ack = p_recv->accessHeader()->getIntegerInfo(2);//3 control
         seq_recv_ack = p_recv->accessHeader()->getIntegerInfo(6); //-1 default
         cout<<"Received ACK from "<<dst_recv_ack<<" for seq_id "<<seq_recv_ack<<" type "<<type_recv_ack<<endl;
         break;
         }
      else if(count1 == 2){
         FileNameSendPacket((char*)filename,my_port);
         count1=0;
      }
   sleep(1);
   count1 = count1+1;
  }
 
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
      Packet *my_packet = new Packet();
      my_packet->accessHeader()->setOctet('A',0); //src
      my_packet->accessHeader()->setOctet('B',1); //dst
      my_packet->accessHeader()->setIntegerInfo(0,2);//packet type DATA 0,ACK 1, NACK 2
      my_packet->accessHeader()->setIntegerInfo(seq_n,6);//seq_n
      my_packet->accessHeader()->setIntegerInfo(0,10);//eof
      if (seq_n==count){
      	my_packet->accessHeader()->setIntegerInfo(END_OF_FILE_SEG,10);}//end of file marker
      cout<<"Sending a packet with type "<<PACKET_DATA<<" having seq_n "<<seq_n<<" to "<<my_packet->accessHeader()->getOctet(1)<<endl;
      my_packet->setPayloadSize(MAX_PAYLOAD_SIZE);
      my_packet->fillPayload(MAX_PAYLOAD_SIZE,&data_store.at(seq_n*MAX_PAYLOAD_SIZE));
      my_port->sendPacket(my_packet);
      int count2=0;
      while(1){
      Packet *p_recv = my_rx_port->receivePacket();
      if (p_recv != NULL)
         {
         dst_recv_ack = p_recv->accessHeader()->getOctet(0);
         type_recv_ack = p_recv->accessHeader()->getIntegerInfo(2);
         seq_recv_ack = p_recv->accessHeader()->getIntegerInfo(6);
         cout<<"Received ACK from "<<dst_recv_ack<<" for seq_id "<<seq_recv_ack<<" type "<<type_recv_ack<<endl;
         break;
         }
      else if(count2==2)
          { 
          cout<<"ARQ timer expired, sending packet again "<<endl;
	  my_port->sendPacket(my_packet);
          count2=0;
	  }
        sleep(1);
        count2=count2+1;
        }
}
} catch (const char *reason ){
    cerr << "Exception:" << reason << endl;
    exit(-1);
}
 return 0;
}
