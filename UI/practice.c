#include <gtk/gtk.h>
#include "practice.h"

void create_practice_window(json_t *practices) {
    GtkWidget *window;
    GtkWidget *vbox;

    //gtk_init(0, NULL);

    // Create a new window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Practices");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    // When the window is given the "delete-event" signal, we ask it to call
    // the gtk_main_quit() function to make the application terminate.
    g_signal_connect(window, "delete-event", G_CALLBACK(gtk_main_quit), NULL);

    // Create a vertical box
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    // Iterate over the practices and add them to the vbox
    size_t index;
    json_t *practice;
    json_array_foreach(practices, index, practice) {
        const char *name = json_string_value(json_object_get(practice, "Name"));
        int practiceTypeId = json_integer_value(json_object_get(practice, "PracticeTypeId"));

        char buffer[256];
        snprintf(buffer, sizeof(buffer), "Name: %s, PracticeTypeId: %d", name, practiceTypeId);

        GtkWidget *frame = gtk_frame_new(NULL);
        GtkWidget *label = gtk_label_new(buffer);

        gtk_widget_set_margin_top(frame, 5); 
        gtk_widget_set_margin_bottom(frame, 5); 
        gtk_widget_set_margin_start(frame, 5); 
        gtk_widget_set_margin_end(frame, 5); 

        gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_OUT);

        gtk_container_add(GTK_CONTAINER(frame), label);
        gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);
    }

    // Add the vbox to the window
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Show the window along with all its content
    gtk_widget_show_all(window);

    // All GTK applications must have a gtk_main(). Control ends here and waits
    // for an event to occur (like a key press or mouse event).
    gtk_main();
    
    gtk_widget_destroy(window);
}
