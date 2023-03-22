#define SERVER_PORT 3535

int create_server_socket(int);
int accept_incoming_connection(int);
int create_client_socket(char*, int);
void send_message(int);
void receive_message(int);


