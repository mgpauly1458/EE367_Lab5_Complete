#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <unistd.h>
#include <fcntl.h>

#include "main.h"
#include "net.h"
#include "man.h"
#include "host.h"
#include "packet.h"
#include "switch.h"

#define MAX_FILE_BUFFER 1000
#define MAX_MSG_LENGTH 100
#define MAX_DIR_NAME 100
#define MAX_FILE_NAME 100
#define PKT_PAYLOAD_MAX 100
#define MAX_FTABLE_SIZE 100
#define TENMILLISEC 10000   /* 10 millisecond sleep */

void switch_main(int switch_id) 
{
/* State Variable Initialization Taken From host.c*/
char dir[MAX_DIR_NAME];
int dir_valid = 0;

char man_msg[MAN_MSG_LENGTH];
char man_reply_msg[MAN_MSG_LENGTH];
char man_cmd;
struct man_port_at_host *man_port;  // Port to the manager

struct net_port *node_port_list;
struct net_port **node_port;  // Array of pointers to node ports
int node_port_num;            // Number of node ports

int ping_reply_received;

int i, k, n;
int dst;
char name[MAX_FILE_NAME];
char string[PKT_PAYLOAD_MAX+1];

FILE *fp;

struct packet *in_packet; /* Incoming packet */
struct packet *new_packet;

struct net_port *p;
struct host_job *new_job;
struct host_job *new_job2;

struct job_queue job_q;
// Dont need the upload and download structs

// Create and initialize the forward table 
struct fwd_table forwardTable[MAX_FTABLE_SIZE];
initialize_ftable(forwardTable);

// Get the port the host is using to connect to the switch node
man_port = net_get_host_port(switch_id);

/*
 * Create an array node_port[ ] to store all the port links
 * from the switch node.  The number of ports is node_port_num
 */
node_port_list = net_get_port_list(switch_id);

/*  Count the number of network link ports */
node_port_num = 0;
for (p=node_port_list; p!=NULL; p=p->next) {
	node_port_num++;
}
/* Create memory space for the array */
node_port = (struct net_port **) 
	malloc(node_port_num*sizeof(struct net_port *));

/* Load ports into the array */
p = node_port_list;
for (k = 0; k < node_port_num; k++) {
	node_port[k] = p;
	p = p->next;
}	

// Now man_port has the port linking the switch node to the manager
// And node_port[] has the ports linking other nodes to each other

/* Initialize the job queue */
job_q_init(&job_q);

// Infinite Switch Loop
while(1) 
{
   // Add any new job request to the job queue before starting anything
   for (k = 0; k < node_port_num; k++) {  // Scan all ports
      in_packet = (struct packet*)malloc(sizeof(struct packet)); // Allocate space
      n = packet_recv(node_port[k], in_packet); // Receive packet from the current port
      
      // Check if the packet was received
      if (n > 0) {
         new_job = (struct host_job *)malloc(sizeof(struct host_job));
         new_job->packet = in_packet;
         new_job->in_port_index = k;  
         job_q_add(&job_q, new_job);
      }
      else {
         free(in_packet); //Job was not meant for the switch node
      }
   }
   // Now execute the first job in queue if it exists
   if (job_q_num(&job_q) > 0) {
      new_job = job_q_remove(&job_q);

      // Define a port # to send to
      int output_port = -1;    // Initialize to -1 to ensure no accidental sends
      
      // Check if any forward table rows match the new job request
      for (i = 0; i < MAX_FTABLE_SIZE; i++) {
         if (forwardTable[i].valid == 1 && forwardTable[i].dst == (int)new_job->packet->dst) {
            output_port = forwardTable[i].port;
            break;
         }
      }
      // If the output_port was found in the ftable, send the packet
      if (output_port != -1) {
         packet_send(node_port[output_port], new_job->packet);
      }
      // Else 
      else {
         // Ping all hosts besides the src
         for (k = 0; k < node_port_num; k++) {
            if (k == new_job->in_port_index)
               continue;
            else {
               packet_send(node_port[k], new_job->packet);
            }
         }
         // Add src + src port to the forward table
         for (i = 0; i < MAX_FTABLE_SIZE; i++) {
         }
         // If the entry is empty
         if (forwardTable[i].valid == 0) {
            forwardTable[i].valid = 1;
            forwardTable[i].dst = new_job->packet->src;       //where the request came from
            forwardTable[i].port = new_job->in_port_index;    //port from where the request came from  
            break;
         }
      }
      // Free up job struct space 
      free(new_job);
   }
   // Switch goes to sleep for 10 ms
   usleep(TENMILLISEC);
}
}

void initialize_ftable(struct fwd_table *forwardTable) {
struct fwd_table *f = forwardTable;
for (int i = 0; i < MAX_FTABLE_SIZE; i++) {
   f->valid = 0;
   f->dst = -1;
   f->port = -1;
}
}
