#include <gtk/gtk.h>

void on_ok_button_clicked(GtkWidget *widget, gpointer window)
{
    gtk_widget_destroy(GTK_WIDGET(window));
    exit(EXIT_FAILURE);
}

void show_error_window(char *error_message)
{
	printf("Show error window\n");
    GtkWidget *window;
    GtkWidget *label;
    GtkWidget *button;
    GtkWidget *vbox;

    // Create a new window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Error");
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 100);

    // Vertically box everything
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Create the label from error message
    label = gtk_label_new(error_message);
    gtk_box_pack_start(GTK_BOX(vbox), label, TRUE, TRUE, 0);

    // Create an OK button
    button = gtk_button_new_with_label("Okay");
    g_signal_connect(button, "clicked", G_CALLBACK(on_ok_button_clicked), window);
    gtk_box_pack_start(GTK_BOX(vbox), button, TRUE, TRUE, 0);

    // Make sure the window and all its contents are visible
    gtk_widget_show_all(window);
    
    gtk_main();
}
