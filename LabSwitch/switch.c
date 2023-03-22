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

#define MAX_FILE_BUFFER 1000
#define MAX_MSG_LENGTH 100
#define MAX_DIR_NAME 100
#define MAX_FILE_NAME 100
#define PKT_PAYLOAD_MAX 100
#define TENMILLISEC 10000   /* 10 millisecond sleep */

/* Job queue operations */
void job_q_add(struct job_queue *j_q, struct host_job *j);
struct host_job *job_q_remove(struct job_queue *j_q);
void job_q_init(struct job_queue *j_q);
int job_q_num(struct job_queue *j_q);

void switch_main(int host_id) {
  printf("hello from switch\n\n");
}
