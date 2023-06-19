void start_client(char *ip_address, int *window_connected);

void send_weighed(int player, int team);

void close_client();

void initialize_client();

int check_connection();

typedef void (*update_weighed_func)(int, int);
extern update_weighed_func client_update_ptr;

typedef void (*disconnect_func)();
extern disconnect_func disconnect_func_ptr;
