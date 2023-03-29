#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#include "main.h"
#include "net.h"
#include "man.h"
#include "host.h"
#include "packet.h"
#include "switch.h"
#include "sockets.h"

#define MAX_FILE_BUFFER 1000
#define MAX_MSG_LENGTH 100
#define MAX_DIR_NAME 100
#define MAX_FILE_NAME 100
#define PKT_PAYLOAD_MAX 100
#define TENMILLISEC 10000    //10 millisecond sleep


/* Job queue operations */
struct host_job *job_q_remove(struct job_queue *j_q);
void job_q_init(struct job_queue *j_q);
int job_q_num(struct job_queue *j_q);

void switch_job_q_add(struct switch_job_queue *j_q, struct switch_job *j)
{
if (j_q->head == NULL ) {
	j_q->head = j;
	j_q->tail = j;
	j_q->occ = 1;
}
else {
	(j_q->tail)->next = j;
	j->next = NULL;
	j_q->tail = j;
	j_q->occ++;
}
}

void display_forward_table(struct forward_table table) {
   int i;
   printf("Forward table:\n");
   printf("Size: %d\n", table.size);
   printf("Valid\tHost ID\tPort\n");
   for (i = 0; i < 100; i++) {
      if (table.valid[i] != 0 || table.HostID[i] != -1) {
         printf("%d\t%d\t%d\n", table.valid[i], table.HostID[i], table.port[i]);
      }
   }
}


void init_forward_table(struct forward_table *table) {
   int i;
   table->size = 0;
   for (i = 0; i < MAX_TABLE_SIZE; i++) {
      table->valid[i] = 0;
      table->HostID[i] = -1;
      table->port[i] = 0;
   }
}

void add_src_to_table(struct forward_table *table, struct packet *pkt, int port_index) {
   if (table->valid[(int)pkt->src]) return;
   table->valid[(int)pkt->src] = 1;
   table->HostID[(int)pkt->src] = pkt->src;
   table->port[(int)pkt->src] = port_index;

}

void send_to_all_ports(int node_port_num, struct net_port **node_port, struct packet *pkt) {
  for (int k = 0; k < node_port_num; k++) {
      packet_send(node_port[k], pkt);
  }
  free(pkt);
}


int is_host_in_table(struct forward_table *table, char dst) {
   return table->valid[(int)dst];
}

void switch_main(int host_id) {

 //initialization
   struct net_port *node_port_list;
   struct net_port **node_port;
   int node_port_num;
   struct packet *in_packet;
   struct packet *new_packet;
   struct net_port *p;
   struct job_queue job_q;
   struct switch_job *new_job;
   int i, k, n;
   struct forward_table table;

   init_forward_table(&table);

   node_port_list = net_get_port_list(host_id);

   node_port_num = 0;
   for(p=node_port_list; p!=NULL; p=p->next) {
      node_port_num++;
   }

   node_port = (struct net_port **) malloc(node_port_num*sizeof(struct net_port *));

   // populate node_port
   p = node_port_list;
   for (k=0; k<node_port_num; k++) {
      node_port[k] = p;
      p = p->next;
   }

   job_q_init(&job_q);

   display_forward_table(table);

      // socket
   int fd[2];
   pid_t pid;

       // Create the pipe
    if (pipe(fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

      // Fork the process
    pid = fork();

      // Check for errors
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

      // Child process
    if (pid == 0) {
        // Close the read end of the pipe
        close(fd[0]);

        create_server(3500, fd[1]);

        // Exit the child process
        exit(EXIT_SUCCESS);
    }

    // Parent process
    else {
        // Close the write end of the pipe
      close(fd[1]);        
              // test packet
        struct packet *p2 = malloc(sizeof(struct packet));
        p2->src = 0;
        p2->dst = 1;
        p2->type = PKT_PING_REPLY;
        p2->length = 11;
        strncpy(p2->payload, "Hello World", p2->length);
      
        create_client("wiliki.eng.hawaii.edu", 3500, p2);

        // Read the message from the read end of the pipe
        struct packet *p3 = malloc(sizeof(struct packet)); 
        int n = receive_packet(fd[0], p3);

        // diplay the data
        printf("client process data received = %d\n", n);
        display_packet_info(p3); 
        
         // net data test
         struct net_data *g_net_data = get_g_net_data();
         printf("\n\nsend_domain=%s, send_port=%d, server_port=%d\n", g_net_data->send_domain, g_net_data->send_port, g_net_data->server_port);

        // Close the read end of the pipe
        close(fd[0]);
         
        // kill child process
        kill(pid, SIGKILL);
   
   //main loop
      while(1) {

    //get packets from incoming links
       //scan all ports
         for (k = 0; k < node_port_num; k++) {
            in_packet = (struct packet *) malloc(sizeof(struct packet));
            n = packet_recv(node_port[k], in_packet);

            if (n > 0) {
            
               display_forward_table(table);
               // add packet routing here
               // check whole table
               if(is_host_in_table(&table, in_packet->dst)) {
                  // port is in table, send it
                  packet_send(node_port[table.port[in_packet->dst]], in_packet);
                  add_src_to_table(&table, in_packet, k);

              } else {
                  // port is not in table
                  add_src_to_table(&table, in_packet, k);
                  send_to_all_ports(node_port_num, node_port, in_packet);
               }
            }
         }
      }
   }
}
