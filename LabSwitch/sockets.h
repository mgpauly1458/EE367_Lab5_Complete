void create_server(int port, int pipe_fd);
void create_client(char* domain_name, int port, char* text);
void send_packet(int pipe_fd, struct packet *p);
int receive_packet(int pipe_fd, struct packet *p);
