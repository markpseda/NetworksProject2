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

#define STRING_SIZE 1024

// Used to easily toggle more verbose debugging if desired
#define DEBUG

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
int timeout; // 10^timeout microseconds

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

   int data_packets_trans = 0;
   int data_bytes_trans = 0;
   int total_retransmissions = 0;
   int total_data_packets_trans = 0;
   int total_acks_recieved = 0;
   int num_timeouts = 0;

   int sequence_num = 0;

   /* assign command line arguments to appropriate variables */

   #ifdef DEBUG
   printf("%d command line variables entered\n", argc);
   #endif

   // Make sure proper number of arguments (2 plus program name)
   if(argc != 2)
   {
      printf("Invalid number of command line arguments, please try again\n");
      return 1;
   }

   timeout = atof(argv[1]);
   
   #ifdef DEBUG
   printf("Timeout Set To: %d\n", timeout);
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

   /* user interface */

   printf("Please input a sentence:\n");
   scanf("%s", sentence);
   msg_len = strlen(sentence) + 1;

   /* send message */

   while(/*still a line*/1)
   {
      
      // compose message

      // send message

      // get response from server


      // analyse response and act on it


   }

   // send final message to close connection

   bytes_sent = sendto(sock_client, sentence, msg_len, 0,
                       (struct sockaddr *)&server_addr, sizeof(server_addr));

   /* get response from server */

   printf("Waiting for response from server...\n");
   bytes_recd = recvfrom(sock_client, modifiedSentence, STRING_SIZE, 0,
                         (struct sockaddr *)0, (int *)0);
   printf("\nThe response from server is:\n");
   printf("%s\n\n", modifiedSentence);

   /* close the socket */

   close(sock_client);
}