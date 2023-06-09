#include <gtk/gtk.h>
#include "login.h"
#include "../Helpers/jsonReader.h"
#include "../HeadshotImporter/headshotUI.h"
#include "../Curl/schedule.h"
#include "../Curl/roster.h"
#include "dmx.h"
#include "Window.h"

Settings *globalSettings;
extern int numTeams;

// Callback functions for button clicks
void on_enter_main_window_clicked(GtkWidget *widget) {
    g_print("Enter main window clicked\n");
    //cleanup();
    //load_globals(0);
    gtk_widget_destroy(widget);
    create_main_window();
}

void on_edit_settings_clicked() {
    g_print("Edit settings clicked\n");
}

void on_manual_override_clicked(GtkComboBox *combo_box) {
    int index = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_box));
    g_print("Option %d picked\n", index);
    if(index == 0)
        set_mode(WEIGH_IN,1);
    else if(index == 1)
        set_mode(WEIGH_OUT,1);
    else if(index == 2)
        set_mode(WAITING,0);
    //g_free(selected_option);
}

void on_add_team_clicked() {
    g_print("Add team clicked\n");
    set_append(1); //set login.c to append
    create_login_window();
}

void on_sync_dmx_clicked() {
    g_print("Sync DMX clicked\n");
    for(int i = 0; i < numTeams; i++) {
        get_next_practices(globalSettings->baseUrl,i);
    }
}

void on_add_practice_game_clicked() {
    g_print("Add practice game clicked\n");
}

void on_import_headshots_clicked() {
    //int index = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_box));
    //g_print("Option %d picked\n", index);
    create_import_window(0);
    //getHeadshots(index);
    //g_free(selected_option);
}

void create_weight_window(Settings *set_settings) {
    globalSettings = set_settings;
    GtkWidget *window, *grid, *button, *combo_box, *override_combo_box;
    int numTeams = 0;
    char modes[][20] = {"Override Weigh In", "Override Weight Out", "Default"};
    printf("Num teams =%i\n",numTeams);

    // Create new window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Weight Station");
    
    // Set the size of the window to 3 times larger
    gtk_window_set_default_size(GTK_WINDOW(window), 1200, 600);

    // Create grid and add it to window
    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    // function to create buttons with size request
    GtkWidget* create_button(const gchar *label){
        GtkWidget *button = gtk_button_new_with_label(label);
        gtk_widget_set_size_request(button, 300, 100); // size up the button
        return button;
    }

    // Create 'Enter main window' button
    button = create_button("Enter main window");
    g_signal_connect_swapped(button, "clicked", G_CALLBACK(on_enter_main_window_clicked), window);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 0, 2, 2);

    // Create 'Add Team' button
    button = create_button("Add team");
    g_signal_connect_swapped(button, "clicked", G_CALLBACK(on_add_team_clicked), NULL);
    gtk_grid_attach(GTK_GRID(grid), button, 1, 2, 1, 1);
    
    // Create 'Edit settings' button
    button = create_button("Edit settings");
    g_signal_connect_swapped(button, "clicked", G_CALLBACK(on_edit_settings_clicked), NULL);
    gtk_grid_attach(GTK_GRID(grid), button, 2, 2, 1, 1);
    
    // Create 'Sync dmx' button
    button = create_button("Sync dmx");
    g_signal_connect_swapped(button, "clicked", G_CALLBACK(on_sync_dmx_clicked), NULL);
    gtk_grid_attach(GTK_GRID(grid), button, 3, 2, 1, 1);
    
    // Create 'Manual Override' button and combo box
    override_combo_box = gtk_combo_box_text_new();
    for (int i = 0; i<3; i++)
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(override_combo_box), modes[i]);
    gtk_combo_box_set_active(GTK_COMBO_BOX(override_combo_box), 0);
    gtk_grid_attach(GTK_GRID(grid), override_combo_box, 5, 3, 1, 1);

    button = gtk_button_new_with_label("Import headshots");
    g_signal_connect_swapped(button, "clicked", G_CALLBACK(on_import_headshots_clicked), NULL);
    gtk_grid_attach(GTK_GRID(grid), button, 4, 2, 1, 1);
    
    button = gtk_button_new_with_label("Manual Override");
    g_signal_connect_swapped(button, "clicked", G_CALLBACK(on_manual_override_clicked), override_combo_box);
    gtk_grid_attach(GTK_GRID(grid), button, 5, 2, 1, 1);

    // Connect the destroy signal to gtk_main_quit
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Show all widgets
    gtk_widget_show_all(window);

    // Start main loop
    gtk_main();
}
