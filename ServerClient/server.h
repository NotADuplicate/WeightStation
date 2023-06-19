int initialize_server();
void broadcast(int *message);

typedef void (*update_weighed_func)(int, int);
extern update_weighed_func update_weighed_ptr;
