/* udp_server.c */
/* Programmed by Adarsh Sethi */
/* Sept. 13, 2018 */

#include <ctype.h>      /* for toupper */
#include <stdio.h>      /* for standard I/O functions */
#include <stdlib.h>     /* for exit */
#include <string.h>     /* for memset */
#include <sys/socket.h> /* for socket, sendto, and recvfrom */
#include <netinet/in.h> /* for sockaddr_in */
#include <unistd.h>     /* for close */
#include <time.h>       /* for random num */

#define STRING_SIZE 80

// Used to easily toggle more verbose debugging if desired
#define DEBUG

#undef DEBUG

/* SERV_UDP_PORT is the port number on which the server listens for
   incoming messages from clients. You should change this to a different
   number to prevent conflicts with others in the class. */

#define SERV_UDP_PORT 63232

/*
Overal TODOs for Reciever:

[x] Read command line arguments and load them into globals

[ ] Write to output.txt file at start and close file at close of program

[ ] Implement stop and wait reciever side

[ ] Implement SimulateLoss
[ ] Implement SimulateACKLoss
*/


// Globals
double packet_loss_rate;
double ack_loss_rate;

// Method Stubs to be implemented
int SimulateLoss();
int SimulateACKLoss();

struct message{
   int count;
   int sequence_number;
   char data[80];
};

int main(int argc, char **argv)
{

   int sock_server; /* Socket on which server listens to clients */

   struct sockaddr_in server_addr; /* Internet address structure that
                                        stores server address */
   unsigned short server_port;     /* Port number used by server (local port) */

   struct sockaddr_in client_addr; /* Internet address structure that
                                        stores client address */
   unsigned int client_addr_len;   /* Length of client address structure */

   char sentence[STRING_SIZE];         /* receive message */
   char modifiedSentence[STRING_SIZE]; /* send message */
   unsigned int msg_len;               /* length of message */
   int bytes_sent, bytes_recd;         /* number of bytes sent or received */
   unsigned int i;                     /* temporary loop variable */
   int next_sequence_number = 0;            /* sequence number */
   short data_length;                    /* length of data lines */
   FILE *writeFile;                   /* file data will be written to */


   int data_packets_recieved_succesfully = 0;
   int data_bytes_delivered = 0;
   int duplicat_packets_recieved_without_loss = 0;
   int data_packets_recieved_but_dropped_loss = 0;
   int total_data_packets_recieved_succ_loss_dup = 0;
   int acks_trans_without_loss = 0;
   int acks_generated_but_dropped = 0;
   int toal_acks_generated_w_w_loss = 0;

   srand((unsigned int) time(NULL));

   /* assign command line arguments to appropriate variables */

   #ifdef DEBUG
   printf("%d command line variables entered\n", argc);
   #endif

   // Make sure proper number of arguments (2 plus program name)
   if(argc != 3)
   {
      printf("Invalid number of command line arguments, please try again\n");
      return 1;
   }

   packet_loss_rate = atof(argv[1]);
   ack_loss_rate = atof(argv[2]);

   #ifdef DEBUG
   printf("Packet Loss Rate Set To: %f\n", packet_loss_rate);
   printf("Ack Loss Rate Set To: %f\n", ack_loss_rate);
   #endif


   /* open a socket */

   if ((sock_server = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
   {
      perror("Server: can't open datagram socket\n");
      exit(1);
   }

   /* initialize server address information */

   memset(&server_addr, 0, sizeof(server_addr));
   server_addr.sin_family = AF_INET;
   server_addr.sin_addr.s_addr = htonl(INADDR_ANY); /* This allows choice of
                                        any host interface, if more than one
                                        are present */
   server_port = SERV_UDP_PORT;                     /* Server will listen on this port */
   server_addr.sin_port = htons(server_port);

   /* bind the socket to the local server port */

   if (bind(sock_server, (struct sockaddr *)&server_addr,
            sizeof(server_addr)) < 0)
   {
      perror("Server: can't bind to local address\n");
      close(sock_server);
      exit(1);
   }

   /* wait for incoming messages in an indefinite loop */

   printf("Waiting for incoming messages on port %hu\n\n",
          server_port);

   client_addr_len = sizeof(client_addr);

   writeFile = fopen("output.txt", "w");    //Opening output file

   for (;;)
   {  
      int b_is_new_packet = 0;

      // allocate max needed size (just in case)
      char recieved_packet[84];

      // max size of 84: 4 bytes of header, 80 bytes of data
      bytes_recd = recvfrom(sock_server, recieved_packet, 84, 0,
                            (struct sockaddr *)&client_addr, &client_addr_len);
      

      short raw_data_length;
      short raw_sequence_number;

      
      recieved_packet[83] = '\0';


      memcpy(&raw_data_length, recieved_packet, sizeof(raw_data_length));

      data_length = ntohs(raw_data_length);

      char data[data_length];
      
      memcpy(&raw_sequence_number, recieved_packet + 2, sizeof(raw_sequence_number));


      uint16_t sequence_number = ntohs(raw_sequence_number);

      memcpy(data, recieved_packet + 4, data_length);

      

      
      if(data_length == 0){       //If EoT trans. detected, close file and terminate.
         printf("End of Transmission Packet with sequence number %d recieved with %d data bytes.\n", sequence_number, data_length);
         break;
      }
      
      total_data_packets_recieved_succ_loss_dup += 1;
      //ASSUMING THE ABOVE WORKS, message is arbitrary, replace later!


      // check if new message

      

      //If loss is detected, drop the packet, no ack sent back
      if(SimulateLoss() == 0){
         printf("Packet %d lost.\n", sequence_number);
         data_packets_recieved_but_dropped_loss +=1;
         continue;
      }

      if(sequence_number == next_sequence_number)
      {
         b_is_new_packet = 1;
      }
      else
      {
         b_is_new_packet = 0;
      }
       

      // Check if new message
      if(b_is_new_packet)
      {
         // new message
         printf("Packet %d recieved with %d data bytes.\n", sequence_number, data_length);
         data_packets_recieved_succesfully += 1;

         data_bytes_delivered += data_length;

         //Swap sequence number for new message recieved succesfully
         if(next_sequence_number == 0)
         {
            next_sequence_number = 1;
         }
         else
         {
            next_sequence_number = 0;
         }
      }
      else
      {
         // duplicate message
         printf("Duplicate packet %d recieved with %d data bytes.\n", sequence_number, data_length);
         duplicat_packets_recieved_without_loss += 1;

         // we are done in this case, no need to copy the data since it is a duplicate
      }

      


     

      /********************* WRITE TO FILE *****************************/
      if(b_is_new_packet)
      {
         memcpy(sentence, data, data_length);      //Receive actual text line
         sentence[data_length] = '\0';     //Add null terminator
         #ifdef DEBUG
         printf("String to add to file: %s\n", sentence);
         #endif
         fputs(sentence,writeFile);     //Write message to file
      }
      /*********************************************************/


      /************** SEND ACK ************************/
      short ACK_val = sequence_number; // duplicate or new this is true
      short ACK = htons(ACK_val);

      toal_acks_generated_w_w_loss += 1;
      if(SimulateACKLoss() == 0)
      {
         // simulate ack loss
         printf("ACK %d lost.\n", ACK_val);
         acks_generated_but_dropped += 1;
      }
      else
      {
         bytes_sent = sendto(sock_server, &ACK, 2, 0,
                     (struct sockaddr *)&client_addr, client_addr_len);
         printf("ACK %d transmitted.\n", ACK_val);
         acks_trans_without_loss += 1;
      }
      /************************************************/

   }

   // all done, time to tidy up
   fclose(writeFile);

   printf("\n\n***************************** FINAL STATISTICS *****************************************\n");
   printf("Number of data packets received successfully:                         %d\n", data_packets_recieved_succesfully);
   printf("Total number of data bytes received which are delivered to user:      %d\n", data_bytes_delivered);
   printf("Total number of duplicate data packets received (without loss) :      %d\n", duplicat_packets_recieved_without_loss);
   printf("Number of data packets received but dropped due to loss:              %d\n", data_packets_recieved_but_dropped_loss);
   printf("Total number of data packets received:                                %d\n", total_data_packets_recieved_succ_loss_dup);
   printf("Number of ACKs transmitted without loss:                              %d\n", acks_trans_without_loss);
   printf("Number of ACKs generated but dropped due to loss:                     %d\n", acks_generated_but_dropped);
   printf("Total number of ACKs generated (with and without loss):               %d\n", toal_acks_generated_w_w_loss);
   printf("******************************************************************************************\n");

}

int SimulateLoss(){
   float randomNum = ((float)rand())/RAND_MAX;
   if(randomNum < packet_loss_rate){
      return 0;
   }
   return 1;
}

int SimulateACKLoss(){
   float randomNum = ((float)rand())/RAND_MAX;
   if(randomNum < ack_loss_rate){
      return 0;
   }
   return 1;
}