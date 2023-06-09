#include <gtk/gtk.h>
#include "headshotRoster.h"
#include "../Helpers/jsonReader.h"


static GtkWidget *window = NULL;
GtkWidget *label;
GtkWidget *combobox;
GtkWidget *text_view;

void add_text_to_importer_window(char *text) {
    printf("Adding text: %s\n",text);
    GtkTextBuffer *buffer;
    GtkTextIter iter;

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_text_buffer_get_end_iter(buffer, &iter);
    gtk_text_buffer_insert(buffer, &iter, text, -1);
    gtk_text_buffer_insert(buffer, &iter, "\n", -1);
}


void button_clicked_cb(GtkComboBox *combo_box) {
    int index = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_box));
    printf("Button clicked: %i\n",index);
    //add_text_to_importer_window("Hello");
    getHeadshots(index);
    //g_free(text);
}

void create_import_window() {
    GtkWidget *window;
    GtkWidget *scrolled_window;
    GtkWidget *button;
    GtkWidget *combobox;
    GtkTextBuffer *buffer;
    int numTeams = 0;
    
    char **options = get_team_codes("settings.json",&numTeams);

    gtk_init(NULL, NULL);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    // Set window default size
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);  // You can modify 800 (width) and 600 (height) to the desired size

    // Add a vertical box (vbox) as the main container
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Create the combo box and add it to the vbox
    combobox = gtk_combo_box_text_new();
    for (int i = 0; i<numTeams; i++)
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combobox), options[i]);
    gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), 0);
    gtk_box_pack_start(GTK_BOX(vbox), combobox, FALSE, FALSE, 0);
    
    /*/ Create 'Import headshots' button and combo box
    combo_box = gtk_combo_box_text_new();
    for (int i = 0; options[i] != NULL; i++)
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_box), options[i]);
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_box), 0);
    gtk_grid_attach(GTK_GRID(grid), combo_box, 4, 3, 1, 1);

    button = gtk_button_new_with_label("Import headshots");
    g_signal_connect_swapped(button, "clicked", G_CALLBACK(on_import_headshots_clicked), combo_box);
    gtk_grid_attach(GTK_GRID(grid), button, 4, 2, 1, 1);*/

    // Create the button and connect it to the callback function, then add it to the vbox
    button = gtk_button_new_with_label("Import headshots");
    g_signal_connect_swapped(button, "clicked", G_CALLBACK(button_clicked_cb), combobox);
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);

    // Create the text view and add it to the vbox inside a scrolled window
    text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), FALSE);
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

    gtk_widget_show_all(window);

    gtk_main();
}


void destroy_import_window() {
    if (!window) {
        g_printerr("Window does not exist. Call create_import_window() to create it.\n");
        return;
    }
    gtk_widget_destroy(window);
    window = NULL;
    label = NULL;
}
