#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>

int create_server_socket(int port) {
   int server_socket;
   struct sockaddr_in server_address;

   // create a tcp socket
   server_socket = socket(AF_INET, SOCK_STREAM, 0);
   if (server_socket < 0) {
      perror("socket");
      exit(1);
   }

   //set socket to be non-blocking
   int flags = fcntl(server_socket, F_GETFL, 0);
   fcntl(server_socket, F_SETFL, flags | O_NONBLOCK);


   // bind the socket to a port
   memset(&server_address, 0, sizeof(server_address));
   server_address.sin_family = AF_INET;
   server_address.sin_addr.s_addr = htonl(INADDR_ANY);
   server_address.sin_port = htons(port);
   
   if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
      perror("bind");
      exit(1);
   }
   
   // Listen for incoming connections
   if (listen(server_socket, SOMAXCONN) < 0) {
      perror("listen");
      exit(1);
   }

   return server_socket;
}

int create_client_socket(char* domain, int port) {
   int sockfd;
   struct sockaddr_in servaddr;
   struct hostent *server;

   // create socket
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   if (sockfd == -1) {
      perror("socket creation failed");
      exit(EXIT_FAILURE);
   }

   // set socket to be non-blocking
   int flags = fcntl(sockfd, F_GETFL, 0);
   fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

   // get server info by domain name or ip address
   server = gethostbyname(domain);
   if (server == NULL) {
      fprintf(stderr, "ERROR, no such host\n");
      exit(EXIT_FAILURE);
   }

   // set server address and port
   memset(&servaddr, 0, sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr = *((unsigned long*)server->h_addr_list[0]);
   servaddr.sin_port = htons(port);

   //connect to server
   if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
      if (errno == EINPROGRESS) {
         //connect is in progress, check later
         printf("Connect in progress\n");
      } else {
         perror("connect fialed");
         exit(EXIT_FAILURE);
      }
   }

   printf("Client socket created and connected to %s:%d\n", domain, port);

   return sockfd;
}

int accept_incoming_connection(int server_fd) {
   struct sockaddr_in cliaddr;
   int conn_fd;
   socklen_t len;

   len = sizeof(cliaddr);
   conn_fd = accept(server_fd, (struct sockaddr *)&cliaddr, &len);
   if (conn_fd == -1) {
      if (errno == EWOULDBLOCK || errno == EAGAIN) {
         //no incoming connection
         return -1;
      } else {
         perror("accept failed");
         exit(EXIT_FAILURE);
      }
   }
   printf("Connection accepted from %s:%d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

   return conn_fd;
}

void send_message(int sockfd) {
   char buff[1024];
   snprintf(buff, sizeof(buff), "Hello, World!");
   if (send(sockfd, buff, strlen(buff), 0) == -1) {
      perror("send failed");
      exit(EXIT_FAILURE);
   }
   printf("Sent message: %s\n", buff);
}


void receive_message(int sockfd) {
   char buff[1024];
   int n = recv(sockfd, buff, sizeof(buff), 0);
   if (n == -1) {
      perror("recg failed");
      exit(EXIT_FAILURE);
   } else if (n == 0) {
      printf("Connection closed by remote peer\n");
   } else {
      buff[n] = '\0';
      printf("Received message: %s\n", buff);
   }
}

