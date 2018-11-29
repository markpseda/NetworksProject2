/* udp_client.c */
/* Programmed by Adarsh Sethi */
/* Sept. 13, 2018 */

#include <stdio.h>      /* for standard I/O functions */
#include <stdlib.h>     /* for exit */
#include <string.h>     /* for memset, memcpy, and strlen */
#include <netdb.h>      /* for struct hostent and gethostbyname */
#include <sys/socket.h> /* for socket, sendto, and recvfrom */
#include <netinet/in.h> /* for sockaddr_in */
#include <unistd.h>     /* for close */
#include <math.h>       /* for pow */

#define STRING_SIZE 1024

// Used to easily toggle more verbose debugging if desired
#define DEBUG

#undef DEBUG
/*
Overal TODOs for Sender:

[ ] Read command line arguments and load them into globals

[ ] Implement stop and wait sender side

[ ] Implement reading from input.txt file

Statistics:

Number of data packets transmitted (initial transmission only) 
Total number of data bytes transmitted (this should be the sum of the count fields of all transmitted packets when transmitted for the first time only) 
Total number of retransmissions 
Total number of data packets transmitted (initial transmissions plus retransmissions) 
Number of ACKs received 
Count of how many times timeout expired

*/


// Method Stubs to be implemented


// Globals
int timeout_input; // 10^timeout microseconds


int main(int argc, char *argv[])
{

   int sock_client; /* Socket used by client */

   struct sockaddr_in client_addr; /* Internet address structure that
                                        stores client address */
   unsigned short client_port;     /* Port number used by client (local port) */

   struct sockaddr_in server_addr;    /* Internet address structure that
                                        stores server address */
   struct hostent *server_hp;         /* Structure to store server's IP
                                        address */
   char server_hostname[STRING_SIZE]; /* Server's hostname */
   unsigned short server_port;        /* Port number used by server (remote port) */

   char sentence[STRING_SIZE];         /* send message */
   char modifiedSentence[STRING_SIZE]; /* receive message */
   unsigned int msg_len;               /* length of message */
   int bytes_sent, bytes_recd;         /* number of bytes sent or received */
   FILE *readFile;                      /* The input file to be read and transmitted*/ 

   int data_packets_trans = 0;
   int data_bytes_trans = 0;
   int total_retransmissions = 0;
   int total_data_packets_trans = 0;
   int total_acks_recieved = 0;
   int num_timeouts = 0;

   short sequence_num = 0;

   /* assign command line arguments to appropriate variables */

   #ifdef DEBUG
   printf("%d command line variables entered\n", argc);
   #endif

   // Make sure proper number of arguments (2 plus program name)
   if(argc != 2)
   {
      printf("Invalid number of command line arguments, 2 expected, please try again\n");
      return 1;
   }

   timeout_input = atof(argv[1]);
   
   #ifdef DEBUG
   printf("Timeout Set To: %d\n", timeout_input);
   #endif



   /* open a socket */

   if ((sock_client = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
   {
      perror("Client: can't open datagram socket\n");
      exit(1);
   }

   /* Note: there is no need to initialize local client address information
            unless you want to specify a specific local port.
            The local address initialization and binding is done automatically
            when the sendto function is called later, if the socket has not
            already been bound. 
            The code below illustrates how to initialize and bind to a
            specific local port, if that is desired. */

   /* initialize client address information */

   client_port = 0; /* This allows choice of any available local port */

   /* Uncomment the lines below if you want to specify a particular 
             local port: */
   /*
   printf("Enter port number for client: ");
   scanf("%hu", &client_port);
   */

   /* clear client address structure and initialize with client address */
   memset(&client_addr, 0, sizeof(client_addr));
   client_addr.sin_family = AF_INET;
   client_addr.sin_addr.s_addr = htonl(INADDR_ANY); /* This allows choice of
                                        any host interface, if more than one 
                                        are present */
   client_addr.sin_port = htons(client_port);

   /* bind the socket to the local client port */

   if (bind(sock_client, (struct sockaddr *)&client_addr,
            sizeof(client_addr)) < 0)
   {
      perror("Client: can't bind to local address\n");
      close(sock_client);
      exit(1);
   }

   /* end of local address initialization and binding */

   /* initialize server address information */

   printf("Enter hostname of server: ");
   scanf("%s", server_hostname);
   if ((server_hp = gethostbyname(server_hostname)) == NULL)
   {
      perror("Client: invalid server hostname\n");
      close(sock_client);
      exit(1);
   }
   printf("Enter port number for server: ");
   scanf("%hu", &server_port);

   /* Clear server address structure and initialize with server address */
   memset(&server_addr, 0, sizeof(server_addr));
   server_addr.sin_family = AF_INET;
   memcpy((char *)&server_addr.sin_addr, server_hp->h_addr,
          server_hp->h_length);
   server_addr.sin_port = htons(server_port);

   // Timeout configuration


   //10^timout_input microseconds

   long microseconds_timeout = pow(10,timeout_input);

   long seconds_component = microseconds_timeout / ((long)pow(10, 6));
   long microseconds_component = microseconds_timeout % ((long)pow(10, 6)); 

   #ifdef DEBUG
   printf("Seconds and microseconds part of timeout: %ld, %ld\n", seconds_component, microseconds_component);
   #endif

   struct timeval timeout;
   timeout.tv_sec = seconds_component;
   timeout.tv_usec = microseconds_component;
   setsockopt(sock_client, SOL_SOCKET, SO_RCVTIMEO, 
               (const void *) &timeout, sizeof(timeout));

   /* user interface */

   //Opens INPUT file to be read, breaks program if incorrect.
   if((readFile = fopen("input.txt", "r")) == NULL){
	printf("Error opening file!");
	exit(1);
   }
   //Checks to see if the file is empty, breaks if so.
   if(feof(readFile)){
	printf("The contents of the file are empty.");
	exit(1);
   }

   /************ TRANSITION TO WAIT FOR ACK *******************/

   //Loop that reads file line by line and copies it into sentence.
   //Sentence is copied to message without NULL terminator.
   while(fgets(sentence,80,readFile))
   {
      #ifdef DEBUG
      printf("SENTENCE: %s\n", sentence);
      #endif
      
      short data_len = strlen(sentence);
	   char info[data_len];
	   memcpy(info, sentence, data_len);

	   


      int first_transmission = 1;
      

      // make_pkt
      int packet_to_send_size = 4 + data_len; 
      #ifdef DEBUG
      printf("Packet to send size: %d\n", packet_to_send_size);
      #endif

      char packet[packet_to_send_size];

      uint16_t data_len_to_send = htons(data_len);
      uint16_t sequence_num_to_send = htons(sequence_num);

      printf("data_len_to_send: %d\n", data_len_to_send);

      short newthing = ntohs(data_len_to_send);

      printf("Back again: %d\n", newthing);



      memcpy(packet, &data_len_to_send, 2);
      memcpy(packet + 2, &sequence_num_to_send, 2);
      memcpy(packet + 4, info, data_len);

      /*
      short raw_data_length;
      memcpy(&raw_data_length, packet, sizeof(raw_data_length));

      short raw_seq;
      memcpy(&raw_seq, packet + 2, sizeof(raw_data_length));


      int data_length = ntohs(raw_data_length);
      int seq = ntohs(raw_seq);

      char newDAta[84];
      memcpy(newDAta, packet + 4, data_length);
      newDAta[83] = '\0';

      printf("data length: %d\n", data_length);
      printf("sequnce: %d\n", seq);

      printf("LKDjfkldjfl: %s\n", newDAta);
      */








      /*
      struct message new_message;
      new_message.count = data_len;
		new_message.sequence_number = sequence_num;

      

      memcpy(new_message.data, info, data_len);

      #ifdef DEBUG
      printf("*** MESSAGE DETAILS ***");
      printf("New message data_len: %d\n", data_len);
      printf("New message sequence number: %d\n", sequence_num);
      //message[data_len] = '\0';
      //printf("Message data: %s\n", message);
      printf("***********************\n");
      #endif

      msg_len = sizeof(new_message);
      */


      // send NEW message

      // keep looping until message is sent succesfully
      while(1) 
      {
         bytes_sent = sendto(sock_client, packet, packet_to_send_size, 0,
                        (struct sockaddr *)&server_addr, sizeof(server_addr));
         // Increment number of transmissions and bytes sent

         
         if(first_transmission)
         {
            printf("Packet %d transmitted with %d data bytes\n",sequence_num, data_len);
            data_packets_trans += 1;
            data_bytes_trans += data_len;

            total_data_packets_trans += 1;
         }
         else
         {
            printf("Packet %d retransmitted with %d data bytes\n",sequence_num, data_len);

            total_data_packets_trans += 1;

            total_retransmissions += 1;
         }

         
         // start timer

         /********** WAIT FOR ACK *****************/

         // Buffer for recieving the ack
         short ACK_number_recieved;
         int response_len = sizeof(ACK_number_recieved);

         
         bytes_recd = recvfrom(sock_client, &ACK_number_recieved, response_len, 0,
                           (struct sockaddr *)0, (int *)0);

         uint16_t ACK_number_recieved_converted = ntohs(ACK_number_recieved);
         
         //TESTING ONLY
         //bytes_recd = 2;
         //ACK_number_recieved = sequence_num;

         if (bytes_recd <=0)
         {
            printf("Timeout expired for packet numberd %d\n", sequence_num);
            // timeout, resend
            num_timeouts += 1;
            // now it is no longer the first transmission
            first_transmission = 0;
         }
         else
         {
            total_acks_recieved += 1;
            printf("ACK %d recieved\n", ACK_number_recieved_converted);
            // is ACK what was expected?
            if(ACK_number_recieved_converted == sequence_num)
            {
               // success! change sequence number
               if(sequence_num == 0)
               {
                  sequence_num = 1;
               }
               else
               {
                  sequence_num = 0;
               }
               break; //exit this loop, send the next line of file
            }
            // going to retransmit
         }
      }
      //if(feof(readFile)) break;
   }
   fclose(readFile); //Close file

    

   /************** SEND FINAL PACKET **********************/

   short size_of_zero = 0;
   int packet_to_send_size = sizeof(sequence_num) + sizeof(size_of_zero);

   uint16_t sequence_num_to_send = htons(sequence_num);
   uint16_t data_len_to_send = htons(size_of_zero);

   char last_packet[packet_to_send_size];

   memcpy(last_packet, &data_len_to_send, sizeof(size_of_zero));
   memcpy(last_packet + 2, &sequence_num_to_send, sizeof(sequence_num));

   sendto(sock_client, last_packet, packet_to_send_size, 0,
                        (struct sockaddr *)&server_addr, sizeof(server_addr));
   /* close the socket */

   printf("End of Transmission Packet with sequence number %d transmitted with %d data bytes.\n", sequence_num, size_of_zero);

   close(sock_client);

   printf("\n\n***************************** FINAL STATISTICS *****************************************\n");
   printf("Number of data packets transmitted (inital transmission only):              %d\n", data_packets_trans);
   printf("Total number of data bytes transmitted:                                     %d\n", data_bytes_trans);
   printf("Total number of retransmissions:                                            %d\n", total_retransmissions);
   printf("Total number of data packets transmitted (initial and retrans):             %d\n", total_data_packets_trans);
   printf("Number of ACKs recieved:                                                    %d\n", total_acks_recieved);
   printf("Count of how many times timeout expired:                                    %d\n", num_timeouts);
   printf("******************************************************************************************\n");
}
