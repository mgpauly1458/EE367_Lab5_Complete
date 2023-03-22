#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define _GNU_SOURCE
#include <fcntl.h>
#include <unistd.h>

#include "main.h"
#include "man.h"
#include "host.h"
#include "net.h"
#include "packet.h"


#define MAX_FILE_NAME 100
#define PIPE_READ 0
#define PIPE_WRITE 1


