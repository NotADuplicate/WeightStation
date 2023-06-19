int initialize_server();

typedef void (*update_weighed_func)(int, int);
extern update_weighed_func update_weighed_ptr;
