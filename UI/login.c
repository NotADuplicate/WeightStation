#include <gtk/gtk.h>
#include "login.h"
#include "../Curl/cams.h"

typedef struct {
    GtkWidget *username;
    GtkWidget *password;
} UserInfo;

int loggedIn = 0;
int appending = 0; //0 if making new json, 1 if appending

static void on_button_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget **data = (GtkWidget**)user_data;
    GtkWidget *window = data[0];
    GtkWidget *username_entry = data[1];
    GtkWidget *password_entry = data[2];
    GtkWidget *ip_entry = data[3];
    GtkSpinButton *station_spinner = GTK_SPIN_BUTTON(data[4]);

    const gchar *username = gtk_entry_get_text(GTK_ENTRY(username_entry));
    const gchar *password = gtk_entry_get_text(GTK_ENTRY(password_entry));
    const gchar *ip = gtk_entry_get_text(GTK_ENTRY(ip_entry));
    gint station_index = gtk_spin_button_get_value_as_int(station_spinner);

    // Here, pass the username, password, ip and station_index values to your function
    set_json(username,password,appending);

    gtk_widget_destroy(window);
}

void create_dmx_window() {
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *submit_button;

    GtkWidget *username_entry;
    GtkWidget *password_entry;
    GtkWidget *ip_entry;
    GtkWidget *station_spinner;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "DMX Window");
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 100);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    // Username Entry
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("DMX Username:"), 0, 0, 1, 1);
    username_entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), username_entry, 1, 0, 2, 1);

    // Password Entry
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("DMX Password:"), 0, 1, 1, 1);
    password_entry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);  // Hide password text
    gtk_grid_attach(GTK_GRID(grid), password_entry, 1, 1, 2, 1);

    // IP Entry
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("IP:"), 0, 2, 1, 1);
    ip_entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), ip_entry, 1, 2, 2, 1);

    // Station Spinner
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Station Index:"), 0, 3, 1, 1);
    station_spinner = gtk_spin_button_new_with_range(0, 100, 1);  // min value, max value, step
    gtk_grid_attach(GTK_GRID(grid), station_spinner, 1, 3, 2, 1);

    // Submit Button
    submit_button = gtk_button_new_with_label("Submit");
    gtk_grid_attach(GTK_GRID(grid), submit_button, 1, 4, 1, 1);

    GtkWidget *data[5] = {window, username_entry, password_entry, ip_entry, station_spinner};
    g_signal_connect(submit_button, "clicked", G_CALLBACK(on_button_clicked), data);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);
    gtk_main();
}

static void on_submit_clicked(GtkWidget *widget, gpointer user_data) {
    GtkWidget **data = (GtkWidget**)user_data;
    GtkWidget *window = data[0];
    GtkWidget *username_entry = data[1];
    GtkWidget *password_entry = data[2];
    const gchar *username = gtk_entry_get_text(GTK_ENTRY(username_entry));
    const gchar *password = gtk_entry_get_text(GTK_ENTRY(password_entry));
    //g_print("Username: %s\nPassword: %s\n", username, password);
    printf("Trying to login to cams\n");
    loggedIn = cams_login(username,password);
    if(loggedIn == 1) { //if successfully logged in
        gtk_widget_destroy(window);
        create_dmx_window();
    }
}

void create_login_window() {
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *username_label;
    GtkWidget *password_label;
    GtkWidget *username_entry;
    GtkWidget *password_entry;
    GtkWidget *submit_button;

    // Create window, set title and default size
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Cams Login");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 170);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Create grid and add it to window
    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    // Create labels and entries
    username_label = gtk_label_new("Username");
    password_label = gtk_label_new("Password");
    username_entry = gtk_entry_new();
    password_entry = gtk_entry_new();

    // Add labels and entries to the grid
    gtk_grid_attach(GTK_GRID(grid), username_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), username_entry, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), password_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), password_entry, 1, 1, 1, 1);

    // Create submit button, connect it to callback function, and add it to grid
    submit_button = gtk_button_new_with_label("Submit");
    GtkWidget *entries[3] = {window,username_entry, password_entry};
    g_signal_connect(submit_button, "clicked", G_CALLBACK(on_submit_clicked), entries);
    gtk_grid_attach(GTK_GRID(grid), submit_button, 0, 2, 2, 1);

    // Show all widgets
    gtk_widget_show_all(window);

    // Start the main loop
    gtk_main();
}

void set_append(int append) {
    appending = append;
}
