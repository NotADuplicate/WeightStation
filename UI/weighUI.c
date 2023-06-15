#include <gtk/gtk.h>
#include "weighUI.h"
int playerValue;

void submit_func(const char *text) {
    printf("Text entered: %s\n", text);
    //submit_weight(playerValue,text);
    gtk_main_quit();
}

void create_weigh_input(GdkPixbuf *pixbuf, const char *text, int playerIndex, void (*submit_func)(const char *)) {
	playerValue = playerIndex;
    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *image;
    GtkWidget *entry;
    GtkWidget *button;

    // create new window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "popup_window");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 200);

    // when the window is given the "delete-event" signal (this is given by the window manager, 
    // usually by the "close" option, or on the titlebar), we ask it to call the delete_event () 
    // function as defined above. The data passed to the callback function is NULL and is ignored 
    // in the callback function.
    g_signal_connect(window, "delete-event", G_CALLBACK(gtk_main_quit), NULL);

    // create vbox to hold image, label, entry and button
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // create label with text
    label = gtk_label_new(text);
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 2);

    // create image from pixbuf
    image = gtk_image_new_from_pixbuf(pixbuf);
    gtk_box_pack_start(GTK_BOX(vbox), image, TRUE, TRUE, 2);

    // create text entry
    entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 2);

    // create submit button
    button = gtk_button_new_with_label("Submit");
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 2);
    
    SubmitData *submit_data = g_new(SubmitData, 1);
    submit_data->playerIndex = playerIndex;
    submit_data->entry = entry;

    // connect button click event with callback that closes the window and call the submit function
    g_signal_connect_swapped(button, "clicked", G_CALLBACK(gtk_widget_destroy), window);
    g_signal_connect(button, "clicked", G_CALLBACK(submit_func), submit_data);

    // show all widgets in the window
    gtk_widget_show_all(window);

    // enter the main loop and start processing events
    gtk_main();
}
