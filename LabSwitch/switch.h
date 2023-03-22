
void switch_main(int);

struct switch_job {
   struct packet *packet;
   int in_port_index;
   int out_port_index;
   struct switch_job *next;
};

struct switch_job_queue {
   struct switch_job *head;
   struct switch_job *tail;
   int occ;
};

struct forward_table {
   int size;
   int valid[100];
   int dest[100];
   int port[100];
};
