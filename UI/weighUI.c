#include <gtk/gtk.h>
#include <math.h>
#include "weighUI.h"
#include "../Curl/roster.h"

int playerValue;
GdkPixbuf *exitImage;
void (*submit_weight_func)(SubmitData*);

#define CANCEL_X 600
#define CANCEL_Y 20

void draw_weight_sprite(cairo_t *cr, int xp, int yp, int radius, GdkPixbuf *pixbuf) {
    cairo_surface_t *surface;
    cairo_pattern_t *pattern;
    GError *error = NULL;

    // Create a Cairo surface from the GdkPixbuf
    surface = gdk_cairo_surface_create_from_pixbuf(pixbuf, 0, NULL);

    // Create a Cairo pattern from the surface
    pattern = cairo_pattern_create_for_surface(surface);

    // Create a circular clipping path
    cairo_arc(cr, xp, yp, radius, 0, 2 * G_PI);
    cairo_clip(cr);

    // Move the pattern to the desired position
    cairo_matrix_t matrix;
    cairo_matrix_init_translate(&matrix, -xp + radius, -yp + radius);
    cairo_pattern_set_matrix(pattern, &matrix);

    // Paint the pattern onto the Cairo context
    cairo_set_source(cr, pattern);
    cairo_paint(cr);

    // Reset clipping path for the next draw call
    cairo_reset_clip(cr);

    // Clean up
    cairo_pattern_destroy(pattern);
    cairo_surface_destroy(surface);
}


gboolean key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    printf("Got key press\n");
    SubmitData *submit_data = (SubmitData*)data;
    if(event->keyval == 65293) { // if 'Enter' key is pressed
        (*submit_weight_func)(submit_data);
        g_string_erase(submit_data->typed_text, 0, -1); // clear the GString
        gtk_widget_destroy(submit_data->window); // Close window after submission
    } else {
        g_string_append_c(submit_data->typed_text, event->string[0]);
    }

    return FALSE;
}

gboolean auto_close_window(gpointer window) {
    gtk_widget_destroy(GTK_WIDGET(window));
    return G_SOURCE_REMOVE;
}

gboolean weigh_click(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    if (event->button == GDK_BUTTON_PRIMARY) { //check to see if clicked the X
        printf("Clicked %f, %f\n", event->x, event->y);
        gdouble dist = pow(pow(event->x - (CANCEL_X+10), 2) + pow(event->y - (CANCEL_Y+10), 2),0.5);
        printf("Clicked dist =%f\n",dist);
        if(dist < 68) {
            auto_close_window((gpointer)(widget));
            printf("clicked X\n");
        }
    }

    return TRUE;
}

gboolean weigh_draw_event(GtkWidget *widget, cairo_t *cr, DrawData *data) {
    printf("Weight drawn \n\n\n");
    gdk_cairo_set_source_pixbuf(cr, data->image, 0, 20);
    cairo_paint(cr);
    
    gdk_cairo_set_source_pixbuf(cr, exitImage, CANCEL_X, CANCEL_Y);
    cairo_paint(cr);
    
    cairo_set_source_rgb(cr, 0, 0, 0); // Sets the color to black for the text
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 20); // Sets the text size
    cairo_move_to(cr, 380,80); // Sets the position where the text will start
    cairo_show_text(cr, data->player->Name); // The text string to draw
    
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5); // Sets the color to grey for the text
    cairo_move_to(cr, 270,80); // Sets the position where the text will start
    cairo_show_text(cr, concat(
        concat("#",data->player->UniformNumber), concat(" ", data->player->Position)
        )); // The text string to draw
    
    cairo_set_source_rgb(cr, .8, 0, 0); 
    if(data->player->weighed == 0) {//if not weighed yet
        cairo_move_to(cr, 240,140);
        cairo_show_text(cr, "Step on the scale to weigh"); // The text string to draw
    }
    else { //already weighed
        cairo_move_to(cr, 240,140);
        cairo_show_text(cr, "You have already weighed in."); // The text string to draw
        cairo_move_to(cr, 240,160);
        cairo_show_text(cr, "Step on scale to reweigh or press X to cancel"); // The text string to draw
    }
    cairo_set_source_rgb(cr, 0, 0, 0); 
        cairo_move_to(cr, 330,300); // Sets the position where the text will start
        char str[80];
        sprintf(str, "Last Weigh-In:  %.1f lb", data->player->weight);
        cairo_show_text(cr, str); // The text string to draw
        cairo_move_to(cr, 330,360); 
        cairo_show_text(cr, "Weigh-In:        ---"); // The text string to draw

    return FALSE;
}

void create_weigh_input(GdkPixbuf *pixbuf, Player *player, int playerIndex, void (*submit_func)(SubmitData*)) {
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *image;
    GtkWidget *darea;
    GtkWidget *overlay;
    submit_weight_func = submit_func;

    // create new window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Weight Window");

    // Change dimensions here for a larger window
    gtk_window_set_default_size(GTK_WINDOW(window), 700, 400);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    // Create overlay
    overlay = gtk_overlay_new();
    gtk_container_add(GTK_CONTAINER(window), overlay);

    darea = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(overlay), darea);

    DrawData *draw_data = g_new(DrawData,1);
    draw_data->image = pixbuf;
    draw_data->player = player;

    g_signal_connect(G_OBJECT(darea), "draw", G_CALLBACK(weigh_draw_event), draw_data);

    SubmitData *submit_data = g_new(SubmitData, 1);
    submit_data->playerIndex = playerIndex;
    submit_data->window = window;
    submit_data->typed_text = g_string_new(NULL);

    g_signal_connect(window, "key-press-event", G_CALLBACK(key_press_event), submit_data);
    g_signal_connect(window, "button-press-event", G_CALLBACK(weigh_click), NULL);

    g_timeout_add_seconds(10, (GSourceFunc)auto_close_window, (gpointer)window); //time out

    // show all widgets in the window
    gtk_widget_show_all(window);
    printf("Finished weigh window\n");
}

    // create grid to hold image, label and button
    /*grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    // create label with text
    label = gtk_label_new(text);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);  // position the label below the image

    // create image from pixbuf
    image = gtk_image_new_from_pixbuf(pixbuf);
    gtk_grid_attach(GTK_GRID(grid), image, 0, 0, 1, 1);  // position the image at the top left corner*/
