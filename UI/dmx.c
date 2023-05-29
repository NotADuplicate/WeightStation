#include <gtk/gtk.h>
#include "dmx.h"
#include "login.h"
#include "Window.h"
#include "../Helpers/jsonReader.h"
#include "../HeadshotImporter/headshotRoster.h"

// Callback functions for button clicks
void on_enter_main_window_clicked(GtkWidget *widget) {
    g_print("Enter main window clicked\n");
    gtk_widget_destroy(widget);
    create_main_window();
}

void on_edit_settings_clicked() {
    g_print("Edit settings clicked\n");
}

void on_add_team_clicked() {
    g_print("Add team clicked\n");
    set_append(1); //set login.c to append
    create_login_window();
}

void on_sync_dmx_clicked() {
    g_print("Sync DMX clicked\n");
}

void on_add_practice_game_clicked() {
    g_print("Add practice game clicked\n");
}

void on_import_headshots_clicked(GtkComboBox *combo_box) {
    int index = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_box));
    g_print("Option %d picked\n", index);
    getHeadshots(index);
    //g_free(selected_option);
}

void create_weight_window() {
    GtkWidget *window, *grid, *button, *combo_box;
    int numTeams = 0;
    char **options = get_team_codes("settings.json",&numTeams);
    printf("Num teams =%i\n",numTeams);

    // Create new window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Weight Station");

    // Create grid and add it to window
    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    // Create 'Enter main window' button
    button = gtk_button_new_with_label("Enter main window");
    g_signal_connect_swapped(button, "clicked", G_CALLBACK(on_enter_main_window_clicked), window);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 0, 2, 2);

    // Create 'Add Team' button
    button = gtk_button_new_with_label("Add team");
    g_signal_connect_swapped(button, "clicked", G_CALLBACK(on_add_team_clicked), NULL);
    gtk_grid_attach(GTK_GRID(grid), button, 1, 2, 1, 1);
    
    // Create 'Edit settings' button
    button = gtk_button_new_with_label("Edit settings");
    g_signal_connect_swapped(button, "clicked", G_CALLBACK(on_edit_settings_clicked), NULL);
    gtk_grid_attach(GTK_GRID(grid), button, 2, 2, 1, 1);
    
    // Create 'Sync dmx' button
    button = gtk_button_new_with_label("Sync dmx");
    g_signal_connect_swapped(button, "clicked", G_CALLBACK(on_sync_dmx_clicked), NULL);
    gtk_grid_attach(GTK_GRID(grid), button, 3, 2, 1, 1);

    // Create 'Import headshots' button and combo box
    combo_box = gtk_combo_box_text_new();
    for (int i = 0; options[i] != NULL; i++)
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_box), options[i]);
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_box), 0);
    gtk_grid_attach(GTK_GRID(grid), combo_box, 4, 3, 1, 1);

    button = gtk_button_new_with_label("Import headshots");
    g_signal_connect_swapped(button, "clicked", G_CALLBACK(on_import_headshots_clicked), combo_box);
    gtk_grid_attach(GTK_GRID(grid), button, 4, 2, 1, 1);

    // Connect the destroy signal to gtk_main_quit
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Show all widgets
    gtk_widget_show_all(window);

    // Start main loop
    gtk_main();
}
