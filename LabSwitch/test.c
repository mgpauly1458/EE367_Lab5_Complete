#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define READ_END 0
#define WRITE_END 1
#define BUFFSIZE 1024

void read_and_print_fd(int fd) {
    char buffer[1024];
    ssize_t num_read;

    while ((num_read = read(fd, buffer, sizeof(buffer))) > 0) {
        if (fwrite(buffer, sizeof(char), num_read, stdout) != num_read) {
            perror("Error writing to stdout");
            return;
        }
    }

    if (num_read < 0) {
        perror("Error reading from file descriptor");
    }
}

int main() {
   int in_pipe[2], out_pipe[2];
   pid_t pid;


   if (pipe(in_pipe) < 0 || pipe(out_pipe) < 0) {
      perror("pipe error"); 
      exit(EXIT_FAILURE);  
   }

   pid = fork();
   if (pid < 0) {
      perror("fork error");
      exit(EXIT_FAILURE);
   } else if (pid == 0) {
      // child process
      close(in_pipe[WRITE_END]);
      close(out_pipe[READ_END]);
      
      dup2(in_pipe[READ_END], STDIN_FILENO);
      dup2(out_pipe[WRITE_END], STDOUT_FILENO);

      execl("./net367", "./net367", NULL);

      perror("excel error");
      exit(EXIT_FAILURE);
   }  else {
      // parent process
      close(in_pipe[READ_END]);
      close(out_pipe[WRITE_END]);
      
      char input[] = {"pp.co\n"};
      
      write(in_pipe[WRITE_END], input, sizeof(input));

      char input2[] = {"p 1\n"};

      write(in_pipe[WRITE_END], input2, sizeof(input));
      read_and_print_fd(out_pipe[READ_END]);
   
      exit(0);
   }
   
   return 0;
}
